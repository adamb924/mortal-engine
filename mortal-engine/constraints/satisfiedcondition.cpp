#include "satisfiedcondition.h"

#include <QXmlStreamReader>

SatisfiedCondition::SatisfiedCondition() : AbstractConstraint(AbstractConstraint::MatchCondition)
{

}

SatisfiedCondition::~SatisfiedCondition()
{

}

bool SatisfiedCondition::matches(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED(parsing)

    Q_UNUSED(node)
    Q_UNUSED(allomorph)
    return true;
}

QString SatisfiedCondition::summary(const QString &suffix) const
{
    Q_UNUSED(suffix)
    return "Satisfied()";
}

QString SatisfiedCondition::elementName()
{
    return "satisfied";
}

AbstractConstraint *SatisfiedCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_UNUSED(morphologyReader)

    Q_ASSERT( in.isStartElement() );
    SatisfiedCondition *c = new SatisfiedCondition;
    c->readId(in);
    in.readNext();
    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return c;
}

bool SatisfiedCondition::matchesElement(QXmlStreamReader &in)
{
    return in.name() == elementName();
}
