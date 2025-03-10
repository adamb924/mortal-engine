#include "sequence.h"

#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include "debug.h"

using namespace ME;

Sequence::Sequence(const MorphologicalModel *model) : AbstractPath(model)
{

}

Sequence::~Sequence()
{

}

Sequence *Sequence::copy(MorphologyXmlReader *morphologyReader, const NodeId &idSuffix) const
{
    Sequence * s = new Sequence( model() );

    /// copy AbstractNode properties
    s->setLabel( label() );
    /// mType will be set by the constructor
    /// mNext will be set by the constructor
    s->setOptional( optional() );
    if( !id().isNull() )
    {
        s->setId( id() + idSuffix );
    }
    /// mHasPathToEnd should be calculated automaticall
    s->mGlosses = mGlosses;

    /// copy AbstractPath properties
    ///
    /// this means copying all of the nodes that follow from this one

    Q_ASSERT(mInitialNode != nullptr);
    s->setInitialNode( mInitialNode->copy(morphologyReader, idSuffix) );
    AbstractNode * previous = s->initialNode();
    const AbstractNode * current = mInitialNode->next(); /// this needs to refer to mInitialNode, because s->initialNode()->next() will be zero
    /// as long as the current node has a next node, copy it
    while( current != nullptr )
    {
        AbstractNode * copyOfCurrent = current->copy(morphologyReader, idSuffix);
        previous->setNext(copyOfCurrent);
        morphologyReader->registerNode(copyOfCurrent);
        previous = copyOfCurrent;
        current = copyOfCurrent->next();
    }

    morphologyReader->registerNode(s);

    return s;
}

QString Sequence::summary(const AbstractNode *doNotFollow) const
{
    Q_UNUSED(doNotFollow)
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "Sequence(" << label().toString() << ")" << newline;
    dbg.indent();
    dbg << "Optional: " << (optional() ? "true" : "false" ) << newline;
    dbg << "Has optional completion path: " << ( hasPathToEnd() ? "true" : "false" ) << newline;
    if( mInitialNode == nullptr )
    {
        dbg << "No initial node." << newline;
    }
    else
    {
        dbg << mInitialNode->summary( next() ) << newline;
    }
    dbg << ") (End of " << label().toString() << ")";
    dbg.unindent();

    return dbgString;
}

bool Sequence::isSequence() const
{
    return true;
}

QString Sequence::elementName()
{
    return "sequence";
}

AbstractNode *Sequence::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel *model)
{
    Q_ASSERT( in.isStartElement() );
    Sequence * s = new Sequence(model);
    s->readInitialNodeAttributes(in, morphologyReader);

    /// move past the initial node
    in.readNextStartElement();

    if( in.name() == AbstractNode::XML_OPTIONAL )
    {
        s->setOptional(true);
        /// move past the optional node
        in.readNext();
        in.readNextStartElement();
    }

    s->setInitialNode( morphologyReader->readNodes( in, elementName(), model ) );

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );

    return s;
}

bool Sequence::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}
