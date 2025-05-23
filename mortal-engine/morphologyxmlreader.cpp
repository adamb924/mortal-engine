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
#include "nodes/sqlserverstemlist.h"
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

using namespace ME;

QString MorphologyXmlReader::XML_MORPHOLOGY = "morphology";
QString MorphologyXmlReader::XML_MAXIMUM_JUMPS = "maximum-jumps";
QString MorphologyXmlReader::XML_PATH = "path";
QString MorphologyXmlReader::XML_SHARED_CREATE_ALLOMORPHS = "shared-create-allomorphs";
QString MorphologyXmlReader::XML_SHARED_CONDITIONS = "interrupted-by";

QString MorphologyXmlReader::XML_NORMALIZATION_FUNCTION = "normalization-function";
QString MorphologyXmlReader::XML_LANG = "lang";
QString MorphologyXmlReader::XML_FUNCTION = "function";
QString MorphologyXmlReader::XML_LOWERCASE = "lowercase";
QString MorphologyXmlReader::XML_REPLACEMENTS = "replacements";
QString MorphologyXmlReader::XML_REPLACE = "replace";
QString MorphologyXmlReader::XML_THIS = "this";
QString MorphologyXmlReader::XML_WITH = "with";

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
    registerNodeMatcher<SqlServerStemList>();
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
    mMorphology->mMorphologyPath = path;

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

        if( in.attributes().hasAttribute(XML_MAXIMUM_JUMPS) )
        {
            Parsing::MAXIMUM_JUMPS = in.attributes().value(XML_MAXIMUM_JUMPS).toInt();
        }

        if( in.attributes().hasAttribute(XML_PATH) )
        {
            Morphology::setPath( in.attributes().value(XML_PATH).toString() );
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
                else if ( name == XML_NORMALIZATION_FUNCTION )
                {
                    readNormalizationFunction(in);
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
    QHash<MorphemeLabel,QPair<MorphemeNode*,MorphemeNode*>> ambiguous;
    QList<MorphemeLabel> labels = mMorphology->mMorphemeNodesByLabel.keys();
    foreach(MorphemeLabel label, labels)
    {
        QList<MorphemeNode*> nodes = mMorphology->mMorphemeNodesByLabel.values(label);
        for(int i=0; i<nodes.count(); i++)
        {
            for(int j=i+1; j<nodes.count(); j++)
            {
                if( nodes.at(i)->id() == nodes.at(j)->id() || nodes.at(i)->id().isNull() || nodes.at(j)->id().isNull() )
                {
                    ambiguous[label] = QPair<MorphemeNode*,MorphemeNode*>( nodes.at(i), nodes.at(j) );
                }
            }
        }
    }
    for (const auto &value : ambiguous)
    {
        qWarning() << QString("Morpheme nodes with both the same label and non-distinct (or missing) IDs: %1, %2").arg(value.first->debugIdentifier()).arg(value.second->debugIdentifier());
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
                    mStemNodes << dynamic_cast<AbstractStemList*>(node);
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

void MorphologyXmlReader::readNormalizationFunction(QXmlStreamReader &in)
{
    WritingSystem ws = mMorphology->mWritingSystems.value( in.attributes().value(XML_LANG).toString());
    if( ws.isNull() )
    {
        qWarning() << "Unknown writing system in normalization function:" << in.attributes().value(XML_LANG).toString();
        return;
    }

    if( in.attributes().value(XML_FUNCTION).toString() == XML_LOWERCASE )
    {
        mMorphology->setNormalizationFunction( ws, [](QString s) { return s.toLower(); } );
    }
    else if( in.attributes().value(XML_FUNCTION).toString() == XML_REPLACEMENTS )
    {
        QList<RegularExpressionReplacement> replacements;
        while( !( in.name() == XML_NORMALIZATION_FUNCTION && in.isEndElement() ) )
        {
            in.readNext();
            if( in.isStartElement() && in.name() == XML_REPLACE )
            {
                if( in.attributes().hasAttribute(XML_THIS) && in.attributes().hasAttribute(XML_WITH) )
                {
                    const QRegularExpression re( in.attributes().value(XML_THIS).toString() );
                    const QString replaceWith = in.attributes().value(XML_WITH).toString();
                    replacements << RegularExpressionReplacement(re, replaceWith);
                }
            }
        }
        InputNormalizer n = [replacements](QString s) {
            QString result = s;
            for (const auto& replacement : replacements)
            {
                result.replace(replacement.first, replacement.second);
            }
            return result;
        };
        mMorphology->setNormalizationFunction( ws, n );
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
            std::string label = j->targetId().toString().toUtf8().constData();
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
    QSetIterator<AbstractStemList *> i(mStemNodes);
    while( i.hasNext() )
    {
        i.next()->generateAllomorphsFromRules();
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

    QSetIterator<AbstractStemList *> i2(mStemNodes);
    while( i2.hasNext() )
    {
        i2.next()->initializePortmanteaux();
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

Morphology *MorphologyXmlReader::readMorphology(const QString &path)
{
    Morphology *m = new Morphology;
    MorphologyXmlReader reader(m);
    try {
        reader.readXmlFile(path);
    }  catch (const std::runtime_error& e) {
        m->mIsOk = false;
        qCritical() << e.what() << "(" << path << ")";
    }
    return m;
}
