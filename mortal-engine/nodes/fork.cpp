#include "fork.h"

#include <QTextStream>

#include "datatypes/generation.h"

#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include "debug.h"

Fork::Fork(const MorphologicalModel *model) : AbstractNode(model)
{

}

Fork::~Fork()
{
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

const AbstractNode *Fork::followingNodeHavingLabel(const QString &targetLabel) const
{
    /// NB: this will just return the first one, not all possible ones
    foreach(Path * p, mPaths)
    {
        const AbstractNode * n = p->followingNodeHavingLabel(targetLabel);
        if( n != nullptr )
        {
            return n;
        }
    }
    return nullptr;
}

bool Fork::checkHasOptionalCompletionPath() const
{
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

QString Fork::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "Fork(" << newline;
    dbg.indent();
    dbg << "Label: " << label() << newline;
    dbg << "Type: " << AbstractNode::typeToString(type()) << newline;
    dbg << "Optional: " << (optional() ? "true" : "false" ) << newline << newline;

    dbg << mPaths.count() << " paths(s)," << newline;
    foreach( Path* p, mPaths )
    {
        dbg << p->summary() << newline;
    }
    dbg.unindent();
    dbg << ")" << newline;

    if( hasNext() )
    {
        dbg << next()->summary();
    }

    return dbgString;
}
