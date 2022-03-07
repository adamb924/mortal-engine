#include "boundcondition.h"

#include "datatypes/parsing.h"

#include <QXmlStreamReader>
#include "debug.h"

BoundCondition::BoundCondition()
{

}

BoundCondition::~BoundCondition()
{

}

bool BoundCondition::matchesThisConstraint(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED(node)
    Q_UNUSED(allomorph)

    return parsing->steps().length() > 1;
}

bool BoundCondition::satisfied(const Parsing *p) const
{
    return p->steps().length() > 1;
}

QString BoundCondition::elementName()
{
    return "bound-morpheme";
}

AbstractConstraint *BoundCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_UNUSED(morphologyReader)
    Q_ASSERT( in.isStartElement() );
    BoundCondition *c = new BoundCondition;

    while( !in.isEndElement() )
    {
        in.readNext();
    }

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return c;
}

bool BoundCondition::matchesElement(QXmlStreamReader &in)
{
    return in.name() == elementName();
}

QString BoundCondition::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "Bound Morpheme Constraint" + suffix;

    return dbgString;
}
