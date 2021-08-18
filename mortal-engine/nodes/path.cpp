#include "path.h"

#include "datatypes/generation.h"

#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include "debug.h"

Path::Path(const MorphologicalModel *model) : AbstractPath(model)
{

}

Path::~Path()
{

}

Path *Path::copy(MorphologyXmlReader *morphologyReader, const QString &idSuffix) const
{
    Path * p = new Path( model() );

    /// copy AbstractNode properties
    p->setLabel( label() );
    /// mType will be set by the constructor
    /// mNext will be set by the constructor
    p->setOptional( optional() );
    if( !id().isEmpty() )
    {
        p->setId( id() + idSuffix );
    }
    /// mHasPathToEnd should be calculated automaticall
    p->mGlosses = mGlosses;

    /// copy AbstractPath properties
    ///
    /// this means copying all of the nodes that follow from this one

    Q_ASSERT(mInitialNode != nullptr);
    p->setInitialNode( mInitialNode->copy(morphologyReader, idSuffix) );
    AbstractNode * previous = p->initialNode();
    const AbstractNode * current = mInitialNode->next(); /// this needs to refer to mInitialNode, because p->initialNode()->next() will be zero
    /// as long as the current node has a next node, copy it
    while( current != nullptr )
    {
        AbstractNode * copyOfCurrent = current->copy(morphologyReader, idSuffix);
        previous->setNext(copyOfCurrent);
        morphologyReader->registerNode(copyOfCurrent);
        previous = copyOfCurrent;
        current = copyOfCurrent->next();
    }

    morphologyReader->registerNode(p);

    return p;
}

QString Path::elementName()
{
    return "path";
}

AbstractNode *Path::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel *model)
{
    Q_ASSERT( in.isStartElement() );
    Path * p = new Path(model);
    p->readInitialNodeAttributes(in, morphologyReader);

    /// move past the initial node
    in.readNextStartElement();

    if( in.name() == AbstractNode::XML_OPTIONAL )
    {
        p->setOptional(true);
        /// move past the optional node
        in.readNext();
        in.readNextStartElement();
    }

    p->setInitialNode( morphologyReader->readNodes( in, elementName(), model ) );

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );

    return p;
}

bool Path::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

QString Path::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "Path[" << label() << "] (" << newline;
    dbg.indent();
    dbg << "Optional: " << (optional() ? "true" : "false" ) << newline;
    dbg << "Has optional completion path: " << ( hasPathToEnd() ? "true" : "false" ) << newline;
    if( mInitialNode == nullptr )
    {
        dbg << "No initial node." << newline;
    }
    else
    {
        dbg << mInitialNode->summary() << newline;
    }
    dbg << ") (End of " << label() << ")";
    dbg.unindent();

    return dbgString;
}

bool Path::checkHasOptionalCompletionPath() const
{
    /// this is like AbstractNode::checkHasOptionalCompletionPath(), but here the 'next' node is the initial node of the path
    if( mInitialNode->optional() || mInitialNode->isFork() || mInitialNode->isSequence() || mInitialNode->isJump() )
    {
        return mInitialNode->checkHasOptionalCompletionPath();
    }
    else
    {
        return false;
    }
}
