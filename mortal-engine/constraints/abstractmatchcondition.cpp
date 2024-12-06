#include "abstractmatchcondition.h"

#include <QXmlStreamReader>

using namespace ME;

AbstractMatchCondition::AbstractMatchCondition() : AbstractConstraint(AbstractConstraint::MatchCondition)
{

}

AbstractMatchCondition::~AbstractMatchCondition()
{

}

QString AbstractMatchCondition::elementName()
{
    return "";
}

AbstractMatchCondition *AbstractMatchCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader)
{
    Q_UNUSED(in)
    Q_UNUSED(morphologyReader)
    return nullptr;
}

bool AbstractMatchCondition::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}
