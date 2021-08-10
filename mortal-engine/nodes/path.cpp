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

    dbg << "Path(" << label() << ")" << newline;
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
