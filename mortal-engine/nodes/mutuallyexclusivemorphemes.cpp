#include "mutuallyexclusivemorphemes.h"

#include <QtDebug>

#include "morphemenode.h"
#include "datatypes/generation.h"

#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include "debug.h"

MutuallyExclusiveMorphemes::MutuallyExclusiveMorphemes(const MorphologicalModel *model) : AbstractNode(model)
{

}

MutuallyExclusiveMorphemes::~MutuallyExclusiveMorphemes()
{
}

void MutuallyExclusiveMorphemes::addMorpheme(MorphemeNode *m)
{
    mMorphemes.insert(m);
}

QList<Parsing> MutuallyExclusiveMorphemes::parsingsUsingThisNode(const Parsing &parsing, Parsing::Flags flags) const
{
    QList<Parsing> candidates;

    /// It's not clear to me why I'm returning a list rather than a set in the first place,
    /// but this at least prevents duplicate parsings from MutuallyExclusiveMorphemes nodes
    QSet<Parsing> candidateSet;

    foreach(MorphemeNode * node, mMorphemes)
    {
        QList<Parsing> parsings = node->possibleParsings(parsing, flags );
        candidateSet.unite( QSet<Parsing>( parsings.begin(), parsings.end() ) );
    }
    candidates.append( candidateSet.values() );

    return candidates;
}

QList<Generation> MutuallyExclusiveMorphemes::generateFormsUsingThisNode(const Generation &generation) const
{
    QList<Generation> candidates;

    foreach(MorphemeNode * node, mMorphemes)
    {
        candidates << node->generateForms(generation);
    }

    return candidates;
}

void MutuallyExclusiveMorphemes::setNext(AbstractNode *next)
{
    AbstractNode::setNext( next );
    foreach(MorphemeNode * node, mMorphemes)
    {
        node->setNext(next);
    }
    /// we also need to set the next node for the
    /// MEM node itself, in case it is optional
    /// and can be skipped
    AbstractNode::setNext(next);
}

QString MutuallyExclusiveMorphemes::elementName()
{
    return "mutually-exclusive-morphemes";
}

AbstractNode *MutuallyExclusiveMorphemes::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel *model)
{
    Q_ASSERT( in.isStartElement() );
    MutuallyExclusiveMorphemes* mem = new MutuallyExclusiveMorphemes(model);
    mem->readInitialNodeAttributes( in, morphologyReader );

    /// store any MEM-level constraints
    QSet<const AbstractConstraint*> memLevelConstraints;

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName() ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == AbstractNode::XML_OPTIONAL )
            {
                mem->setOptional(true);
            }
            else if( MorphemeNode::matchesElement(in) )
            {
                MorphemeNode * m = dynamic_cast<MorphemeNode *>( MorphemeNode::readFromXml(in, morphologyReader, model) );
                if( m != nullptr )
                {
                    mem->addMorpheme( m );
                    morphologyReader->registerNode(m);
                }
            }
            else if ( morphologyReader->currentNodeMatchesConstraint(in) )
            {
                memLevelConstraints << morphologyReader->tryToReadConstraint(in);
            }
            else
            {
                qWarning() << "Unexpected tag: " << in.name() << " on line " << in.lineNumber();
            }
        }
    }

    mem->addConstraintsToAllMorphemes( memLevelConstraints );

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return mem;
}

bool MutuallyExclusiveMorphemes::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

const AbstractNode *MutuallyExclusiveMorphemes::followingNodeHavingLabel(const QString &targetLabel) const
{
    /// NB: this will just return the first one, not all possible ones
    foreach(MorphemeNode * node, mMorphemes)
    {
        const AbstractNode * n = node->followingNodeHavingLabel(targetLabel);
        if( n != nullptr )
        {
            return n;
        }
    }
    return nullptr;
}

bool MutuallyExclusiveMorphemes::checkHasOptionalCompletionPath() const
{
    /// NB: this will just return the first one, not all possible ones
    foreach(MorphemeNode * node, mMorphemes)
    {
        bool hasPath = node->checkHasOptionalCompletionPath();
        if( hasPath )
        {
            return true;
        }
    }
    return true;
}

void MutuallyExclusiveMorphemes::addConstraintsToAllMorphemes(const QSet<const AbstractConstraint *> &constraints)
{
    foreach(MorphemeNode * node, mMorphemes)
    {
        node->addConstraintsToAllAllomorphs(constraints);
    }
}

QString MutuallyExclusiveMorphemes::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "MutuallyExclusiveMorphemes(" << newline;
    dbg.indent();
    dbg << "Label: " << label() << newline;
    dbg << "Type: " << AbstractNode::typeToString(type()) << newline;
    dbg << "Optional: " << (optional() ? "true" : "false" ) << newline;

    dbg << mMorphemes.count() << " morphemes(s)" << newline;
    foreach( MorphemeNode* m, mMorphemes )
    {
        /// call the summary function rather than using the QString cast
        /// so that the “next()” node is not displayed, which leads to
        /// multiple copies of the tree
        dbg << m->summaryWithoutFollowing() << newline;
    }
    dbg.unindent();
    dbg << ")" << newline;

    if( hasNext() )
    {
        dbg << next()->summary();
    }

    return dbgString;
}

