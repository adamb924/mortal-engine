#include "sequence.h"

#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include "debug.h"

Sequence::Sequence(const MorphologicalModel *model) : AbstractPath(model)
{

}

Sequence::~Sequence()
{

}

QString Sequence::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "Sequence(" << label() << ")" << newline;
    dbg.indent();
    dbg << "Optional: " << (optional() ? "true" : "false" ) << newline;
    if( mInitialNode == nullptr )
    {
        dbg << "No initial node.";
    }
    else
    {
        dbg << mInitialNode->summary();
    }
    dbg.unindent();
    dbg << ") (End of " << label() << ")";

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
