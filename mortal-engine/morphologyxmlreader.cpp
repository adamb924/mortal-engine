#include "morphologyxmlreader.h"

#include <QXmlStreamReader>

#include "morphology.h"

#include "datatypes/writingsystem.h"

#include <QFile>
#include <QStack>
#include <QRegularExpression>

/// these includes need to be there so we get the derived version of the static functions
#include "nodes/morphologicalmodel.h"
#include "nodes/morphemenode.h"
#include "nodes/xmlstemlist.h"
#include "nodes/sqlitestemlist.h"
#include "nodes/mutuallyexclusivemorphemes.h"
#include "nodes/fork.h"
#include "nodes/path.h"
#include "nodes/jump.h"
#include "nodes/sequence.h"
#include "nodes/copynode.h"

/// these includes need to be there so we get the derived version of the static functions
#include "constraints/tagmatchcondition.h"
#include "constraints/phonologicalcondition.h"
#include "constraints/followingphonologicalcondition.h"
#include "constraints/orcondition.h"
#include "constraints/andcondition.h"
#include "constraints/notcondition.h"
#include "constraints/pointertoconstraint.h"
#include "constraints/followingnodeconstraint.h"
#include "constraints/precedingnodeconstraint.h"
#include "constraints/wordfinalcondition.h"
#include "constraints/boundcondition.h"
#include "constraints/satisfiedcondition.h"

#include <stdexcept>

QString MorphologyXmlReader::XML_MORPHOLOGY = "morphology";
QString MorphologyXmlReader::XML_SHARED_CREATE_ALLOMORPHS = "shared-create-allomorphs";
QString MorphologyXmlReader::XML_SHARED_CONDITIONS = "interrupted-by";

MorphologyXmlReader::MorphologyXmlReader(Morphology *morphology) : mMorphology(morphology)
{
    registerConstraintMatcher<TagMatchCondition>();
    registerConstraintMatcher<PhonologicalCondition>();

    registerConstraintMatcher<OrCondition>();
    registerConstraintMatcher<AndCondition>();
    registerConstraintMatcher<NotCondition>();

    registerConstraintMatcher<PointerToConstraint>();

    registerConstraintMatcher<FollowingPhonologicalCondition>();
    registerConstraintMatcher<FollowingNodeConstraint>();
    registerConstraintMatcher<PrecedingNodeConstraint>();
    registerConstraintMatcher<WordFinalCondition>();
    registerConstraintMatcher<BoundCondition>();
    registerConstraintMatcher<SatisfiedCondition>();

    registerNodeMatcher<MorphologicalModel>();
    registerNodeMatcher<MorphemeNode>();
    registerNodeMatcher<XmlStemList>();
#ifndef Q_OS_WASM
    registerNodeMatcher<SqliteStemList>();
#endif
    registerNodeMatcher<MutuallyExclusiveMorphemes>();
    registerNodeMatcher<Fork>();
    registerNodeMatcher<Path>();
    registerNodeMatcher<Jump>();
    registerNodeMatcher<Sequence>();
    registerNodeMatcher<CopyNode>();
}


void MorphologyXmlReader::readXmlFile(const QString &path)
{
    /// read the xml from the file
    parseXml(path);

    /// populate
    populateNodeHashes();

    /// for every jump pointer, fill in the pointer to the target of the to="node-id" attribute
    fillInJumpPointers();
    /// for every constraint pointer, fill in the pointer to the target of the id="constraint-id" attribute
    fillInConstraintPointers();

    /// call generateAllomorphs for every node
    generateAllomorphsFromRules();
    /// call initializePortmanteaux for every node with portmanteau. can't recall right now whether the ordering here is significant but I think it is.
    parsePortmanteaux();
    /// call calculateModelProperties for every node. At this point it only ends up calling checkHasOptionalCompletionPath, but it might do more in the future.
    calculateModelProperties();
    /// need to check here whether there are inconsistent nested constraints, i.e., once the pointers have been filled in
    checkNestedConstraintConsistency();

    /// check whether there are any nodes with ambiguous node/label combinations
    checkForNonUniqueIdsAndLabels();
}

void MorphologyXmlReader::parseXml(const QString &path)
{
    QFile file(path);
    if( file.open( QFile::ReadOnly ) )
    {
        QXmlStreamReader in(&file);

        in.readNextStartElement();
        if( in.name() != XML_MORPHOLOGY )
        {
            throw std::runtime_error( "Expected 'morphology' as a root element" );
        }

        while(!in.atEnd())
        {
            in.readNext();

            if( in.isStartElement() )
            {
                QString name = in.name().toString();
                QXmlStreamAttributes attr = in.attributes();

                if ( name == WritingSystem::XML_WRITING_SYSTEMS )
                {
                    mMorphology->mWritingSystems.insert( WritingSystem::readWritingSystems(in) );
                }
                else if( name == "shared-conditions" )
                {
                    readSharedConditions(in);
                }
                else if( name == "shared-create-allomorphs" )
                {
                    readSharedCreateAllomorphs(in);
                }
                else if( name == "model" )
                {
                    /// this will read all of the models
                    readMorphologicalModels( in );
                }
                else
                {
                    if( !in.atEnd() )
                        qWarning() << "Unrecognized tag:" << name << "on line" << in.lineNumber() << ".";
                }
            }
        }
    }
}

void MorphologyXmlReader::populateNodeHashes()
{
    foreach(AbstractNode * n, mMorphology->mNodes)
    {
        mMorphology->mNodesByLabel.insert( n->label(), n );
        if( n->isMorphemeNode() )
        {
            mMorphology->mMorphemeNodesByLabel.insert( n->label(), dynamic_cast<MorphemeNode*>(n) );
        }
        if( n->isModel() )
        {
            mMorphology->mMorphologicalModelsByLabel.insert( n->label(), dynamic_cast<MorphologicalModel*>(n) );
        }
    }
}

void MorphologyXmlReader::checkForNonUniqueIdsAndLabels()
{
    QSet<QString> ambiguous;
    QStringList labels = mMorphology->mMorphemeNodesByLabel.keys();
    foreach(QString label, labels)
    {
        QList<MorphemeNode*> nodes = mMorphology->mMorphemeNodesByLabel.values(label);
        for(int i=0; i<nodes.count(); i++)
        {
            for(int j=i+1; j<nodes.count(); j++)
            {
                if( nodes.at(i)->id() == nodes.at(j)->id() || nodes.at(i)->id().isEmpty() || nodes.at(j)->id().isEmpty() )
                {
                    ambiguous << label;
                }
            }
        }
    }
    foreach(QString label, ambiguous)
    {
        qWarning() << "Morpheme nodes with both the same label and non-distinct (or missing) IDs:" << label;
    }
}

void MorphologyXmlReader::readSharedConditions(QXmlStreamReader &in)
{
    /// read past the <shared-conditions> tag
    in.readNext();

    QSet<const AbstractConstraint *> constraints = readConstraints("shared-conditions", in);

    foreach( const AbstractConstraint *ac, constraints )
    {
        if( mConstraintsById.contains( ac->id() ) )
            qWarning() << "Duplicate shared condition id:" << ac->id();
        mConstraintsById.insert( ac->id(), ac );
    }
}

void MorphologyXmlReader::readSharedCreateAllomorphs(QXmlStreamReader &in)
{
    Q_ASSERT( in.isStartElement() && in.name() == XML_SHARED_CREATE_ALLOMORPHS );

    mCreateAllomorphsById = CreateAllomorphs::readListFromXml(in, this, "shared-create-allomorphs");

    Q_ASSERT( in.isEndElement() && in.name() == XML_SHARED_CREATE_ALLOMORPHS );
}

void MorphologyXmlReader::recordStemAcceptingNewStems(AbstractStemList *stemList) const
{
    mMorphology->mStemAcceptingStemLists.insert( stemList );
}

void MorphologyXmlReader::recordStemList(AbstractStemList *stemList) const
{
    mMorphology->mStemLists.insert( stemList );
}

AbstractNode *MorphologyXmlReader::readNodes(QXmlStreamReader &in, const QString &untilEndOf, const MorphologicalModel * currentModel)
{
    AbstractNode * initialNode = tryToReadMorphemeNode( in, currentModel );
    if( initialNode == nullptr )
        return nullptr;

    while( notAtEndOf(untilEndOf, in) && !in.isStartElement() )
    {
        in.readNext();
    }

    AbstractNode * lastNode = initialNode;
    while( notAtEndOf(untilEndOf, in) )
    {
        AbstractNode * node = tryToReadMorphemeNode( in, currentModel );
        if( node != nullptr )
        {
            lastNode->setNext( node );
            lastNode = node;

            while( notAtEndOf(untilEndOf, in) && !in.isStartElement() )
            {
                in.readNext();
            }
        }
        else
        {
            qWarning() << "Null node returned on line" << in.lineNumber();
            return initialNode;
        }
    }
    return initialNode;
}

AbstractNode *MorphologyXmlReader::tryToReadMorphemeNode(QXmlStreamReader &in, const MorphologicalModel * currentModel)
{
    QListIterator<NodeMatcher> i( mNodeMatchers );
    while( i.hasNext() )
    {
        NodeMatcher amnm = i.next();
        if( amnm.matcher(in) )
        {
            AbstractNode * node = amnm.reader( in, this, currentModel );
            if( node != nullptr )
            {
                registerNode(node);

                /// keep a copy of the pointer for later if it's a stem node
                if( node->type() == AbstractNode::StemNodeType )
                {
                    mStemNodes << node;
                }
                return node;
            }
        }
    }
    if( !in.atEnd() && in.isStartElement() )
        qWarning() << "Expected some sort of morpheme node but found instead:" << in.name() << "on line" << in.lineNumber();
    return nullptr;
}

QSet<const AbstractConstraint *> MorphologyXmlReader::readConstraints(const QString & untilEndOf, QXmlStreamReader &in)
{
    QSet<const AbstractConstraint *> set;
    while( notAtEndOf(untilEndOf, in) )
    {
        Q_ASSERT( notAtEndOf(untilEndOf, in) );
        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            AbstractConstraint * ac = tryToReadConstraint(in);
            if( ac != nullptr )
            {
                Q_ASSERT( in.isEndElement() );
                set << ac;
                continue;
            }
            qWarning() << "Unknown tag:" << in.name() << "on line" << in.lineNumber() << "(called from MorphologyXmlReader::readConstraints).";
        }
        in.readNext();
    }
    Q_ASSERT(in.isEndElement() && in.name() == untilEndOf );
    return set;
}

void MorphologyXmlReader::readMorphologicalModels(QXmlStreamReader &in)
{
    /// At this level, they will all be MorphologicalModel pointers
    MorphologicalModel* node = dynamic_cast<MorphologicalModel *>( tryToReadMorphemeNode( in, nullptr ) );

    while( node != nullptr )
    {
        mMorphology->mNodesById.insert( node->id(), node );
        /// Again, at this level, they will all be MorphologicalModel pointers
        mMorphology->mMorphologicalModels.append( node );

        /// try to read the next one
        node = dynamic_cast<MorphologicalModel *>( tryToReadMorphemeNode( in, nullptr ) );
    }
}

QHash<QString, WritingSystem> MorphologyXmlReader::writingSystems() const
{
    return mMorphology->writingSystems();
}

AbstractConstraint *MorphologyXmlReader::tryToReadConstraint(QXmlStreamReader &in)
{
    QListIterator<ConstraintMatcher> i( mConstraintMatchers );
    while( i.hasNext() )
    {
        ConstraintMatcher acm = i.next();
        if( acm.matcher(in) )
        {
            return acm.reader( in, this );
        }
    }
    return nullptr;
}

bool MorphologyXmlReader::currentNodeMatchesConstraint(QXmlStreamReader &in) const
{
    QListIterator<ConstraintMatcher> i( mConstraintMatchers );
    while( i.hasNext() )
    {
        ConstraintMatcher acm = i.next();
        if( acm.matcher(in) )
        {
            return true;
        }
    }
    return false;
}

CreateAllomorphs MorphologyXmlReader::createAllomorphsFromId(const QString &id) const
{
    return mCreateAllomorphsById.value(id);
}

void MorphologyXmlReader::registerJump(Jump *j)
{
    mJumps.insert(j);
}

void MorphologyXmlReader::registerPointer(PointerToConstraint *pointer)
{
    mPointers.insert( pointer );
}

void MorphologyXmlReader::registerNode(AbstractNode *node) const
{
    mMorphology->mNodes << node;
    mMorphology->mNodesById.insert( node->id(), node );
    if( node->isMorphemeNode() )
    {
        MorphemeNode * mn = dynamic_cast<MorphemeNode*>(node);
        mMorphology->mMorphemeNodes << mn;

        if( mn->hasZeroLengthForms() )
        {
            /// wonky workaround to avoid having to make a lot of const pointers normal pointers
            const MorphologicalModel * cmodel = mn->model();
            MorphologicalModel * model = const_cast<MorphologicalModel *>( cmodel );
            model->setHasZeroLengthForms(true);
        }
    }
}

void MorphologyXmlReader::registerNestedConstraint(const AbstractNestedConstraint * c)
{
    mNestedConstraints << c;
}

void MorphologyXmlReader::fillInJumpPointers()
{
    QSetIterator<Jump*> i( mJumps );
    while( i.hasNext() )
    {
        Jump *j = i.next();
        if( mMorphology->mNodesById.contains( j->targetId() ) )
        {
            j->setNodeTarget( mMorphology->mNodesById.value( j->targetId() ) );
        }
        else
        {
            std::string label = j->targetId().toUtf8().constData();
            throw std::runtime_error( "Jump tag with no corresponding id: " + label );
        }
    }
}

void MorphologyXmlReader::fillInConstraintPointers()
{
    QSetIterator<PointerToConstraint *> i(mPointers);
    while( i.hasNext() )
    {
        PointerToConstraint * pointer = i.next();
        const AbstractConstraint * constraint = mConstraintsById.value( pointer->targetId(), nullptr );
        if( constraint != nullptr )
        {
            pointer->setTheConstraint( constraint );
        }
        else
        {
            std::string message = "Condition pointer with unknown id: ";
            message += pointer->targetId().toUtf8().constData();
            throw std::runtime_error( message );
        }
    }
}

void MorphologyXmlReader::generateAllomorphsFromRules()
{
    QSetIterator<AbstractNode *> i(mStemNodes);
    while( i.hasNext() )
    {
        AbstractStemList * sl = dynamic_cast<AbstractStemList*>( i.next() );
        sl->generateAllomorphsFromRules();
    }

    QSetIterator<MorphemeNode *> i2(mMorphology->mMorphemeNodes);
    while( i2.hasNext() )
    {
        i2.next()->generateAllomorphs();
    }

}

void MorphologyXmlReader::parsePortmanteaux()
{
    QSetIterator<MorphemeNode *> i(mMorphology->mMorphemeNodes);
    while( i.hasNext() )
    {
        i.next()->initializePortmanteaux();
    }
}

void MorphologyXmlReader::calculateModelProperties()
{
    QSetIterator<AbstractNode *> i( mMorphology->mNodes );
    while( i.hasNext() )
    {
        AbstractNode * n = i.next();
        n->calculateModelProperties();
    }
}

void MorphologyXmlReader::checkNestedConstraintConsistency()
{
    QSetIterator<const AbstractNestedConstraint*> i( mNestedConstraints );
    while( i.hasNext() )
    {
        i.next()->checkIfConstraintsAreOfSameType();
    }
}

bool MorphologyXmlReader::notAtEndOf(const QString &tagName, QXmlStreamReader &in)
{
    return !in.atEnd() && !(in.isEndElement() && in.name() == tagName);
}

Morphology *MorphologyXmlReader::morphology() const
{
    return mMorphology;
}

template<class T>
void MorphologyXmlReader::registerConstraintMatcher()
{
    mConstraintMatchers.append( ConstraintMatcher( T::matchesElement, T::readFromXml ) );
}

template<class T>
void MorphologyXmlReader::registerNodeMatcher()
{
    mNodeMatchers.append( NodeMatcher( T::matchesElement, T::readFromXml ) );
}
