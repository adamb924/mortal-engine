#include "fork.h"

#include <QTextStream>

#include "datatypes/generation.h"

#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include "debug.h"

using namespace ME;

Fork::Fork(const MorphologicalModel *model) : AbstractNode(model)
{

}

Fork::~Fork()
{
}

Fork *Fork::copy(MorphologyXmlReader *morphologyReader, const QString &idSuffix) const
{
    Fork * f = new Fork( model() );

    /// copy AbstractNode properties
    f->setLabel( label() );
    /// mType will be set by the constructor
    /// mNext will be set by the constructor
    f->setOptional( optional() );
    if( !id().isEmpty() )
    {
        f->setId( id() + idSuffix );
    }
    /// mHasPathToEnd should be calculated automatically
    f->mGlosses = mGlosses;

    /// copy base class properties
    foreach(Path * p, mPaths)
    {
        Path * np = p->copy(morphologyReader, idSuffix);
        f->addPath( np );
        morphologyReader->registerNode( np );
    }

    morphologyReader->registerNode(f);

    return f;
}

QList<Parsing> Fork::parsingsUsingThisNode(const Parsing &parsing, Parsing::Flags flags) const
{
    QList<Parsing> candidates;

    foreach(Path * p, mPaths)
    {
        candidates << p->possibleParsings(parsing, flags);
    }

    return candidates;
}

QList<Generation> Fork::generateFormsUsingThisNode(const Generation &generation) const
{
    QList<Generation> candidates;

    foreach(Path * p, mPaths)
    {
        candidates << p->generateForms(generation);
    }

    return candidates;
}

void Fork::setNext(AbstractNode *next)
{
    foreach(Path * p, mPaths)
    {
        p->setNext(next);
    }
    /// we also need to set the next node for the
    /// Fork node itself, in case it is optional
    /// and can be skipped
    AbstractNode::setNext(next);
}

void Fork::addPath(Path *p)
{
    mPaths.append(p);
}

QString Fork::elementName()
{
    return "fork";
}

AbstractNode *Fork::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel * model)
{
    Q_ASSERT( in.isStartElement() );
    Fork * f = new Fork(model);
    f->readInitialNodeAttributes(in, morphologyReader);

    while( !in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName()) )
    {
        in.readNext();
        if( Path::matchesElement(in) )
        {
            Path * p = dynamic_cast<Path*>( Path::readFromXml(in, morphologyReader, model) );
            f->addPath( p );
        }
        if( in.tokenType() == QXmlStreamReader::StartElement && in.name() == AbstractConstraint::XML_OPTIONAL )
        {
            f->setOptional(true);
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return f;
}

bool Fork::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

const AbstractNode *Fork::followingNodeHavingLabel(const MorphemeLabel &targetLabel, QHash<const Jump *, int> &jumps) const
{
    /// NB: this will just return the first one, not all possible ones
    foreach(Path * p, mPaths)
    {
        const AbstractNode * n = p->followingNodeHavingLabel(targetLabel, jumps);
        if( n != nullptr )
        {
            return n;
        }
    }
    return nullptr;
}

bool Fork::checkHasOptionalCompletionPath() const
{
    /// if the fork is optional, then we either return true if this is the end of the line, or else check whatever node follows
    if( optional() )
    {
        if( next() == nullptr )
        {
            return true;
        }
        else
        {
            return next()->checkHasOptionalCompletionPath();
        }
    }
    /// if the fork is not optional, then we have to check whether one of the paths provides an optional completion path
    foreach(Path * p, mPaths)
    {
        bool hasPath = p->checkHasOptionalCompletionPath();
        if( hasPath )
        {
            return true;
        }
    }
    return false;
}

bool Fork::isFork() const
{
    return true;
}

QList<const AbstractNode *> Fork::availableMorphemeNodes(QHash<const Jump *, int> &jumps) const
{
    QList<const AbstractNode *> list;
    foreach(Path * path, mPaths)
    {
        list << path->availableMorphemeNodes(jumps);
    }


    /// move on to the next node if this one is optional
    if( optional() && next() != nullptr )
    {
        list.append( next()->availableMorphemeNodes(jumps) );
    }


    return list;
}

QString Fork::summary(const AbstractNode *doNotFollow) const
{
    QString dbgString;
    Debug dbg(&dbgString);
    Debug::atBeginning = false;

    dbg << "Fork(" << newline;
    dbg.indent();
    dbg << "Label: " << label().toString() << newline;
    dbg << "ID: " << id() << newline;
    dbg << "Type: " << AbstractNode::typeToString(type()) << newline;
    dbg << "Optional: " << (optional() ? "true" : "false" ) << newline;
    dbg << "Has optional completion path: " << ( hasPathToEnd() ? "true" : "false" ) << newline << newline;

    dbg << mPaths.count() << " paths(s)," << newline;
    foreach( Path* p, mPaths )
    {
        dbg << p->summary( next() ) << newline;
    }
    dbg.unindent();
    dbg << ")" << newline;

    if( hasNext() && next() != doNotFollow )
    {
        dbg << next()->summary( doNotFollow );
    }

    Debug::atBeginning = true;
    return dbgString;
}
