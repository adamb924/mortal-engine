#include "abstractconstraint.h"

#include "abstractlongdistanceconstraint.h"
#include "abstractnestedconstraint.h"

#include "pointertoconstraint.h"

#include <QXmlStreamReader>
#include <QRegularExpression>

QString AbstractConstraint::XML_MATCH_EXPRESSION = "match-expression";
QString AbstractConstraint::XML_OPTIONAL = "optional";

AbstractConstraint::AbstractConstraint(AbstractConstraint::Type t) : mType(t)
{

}

QString AbstractConstraint::satisfactionSummary(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    bool result = matches(parsing, node, allomorph);
    return oneLineSummary( result ? "+" : "*" );
}

QString AbstractConstraint::oneLineSummary(const QString &suffix) const
{
    return summary(suffix).replace(QRegularExpression("\\s+")," ").trimmed();
}

AbstractConstraint::~AbstractConstraint()
{

}

QString AbstractConstraint::id() const
{
    return mId;
}

void AbstractConstraint::setId(const QString &id)
{
    mId = id;
}

AbstractConstraint::Type AbstractConstraint::type() const
{
    return mType;
}

QString AbstractConstraint::typeString() const
{
    return AbstractConstraint::stringFromType(mType);
}

bool AbstractConstraint::isMatchCondition() const
{
    return mType == AbstractConstraint::MatchCondition;
}

bool AbstractConstraint::isLocalConstraint() const
{
    return mType == AbstractConstraint::LocalConstraint;
}

bool AbstractConstraint::isLongDistanceConstraint() const
{
    return mType == AbstractConstraint::LongDistanceConstraint;
}

AbstractLongDistanceConstraint *AbstractConstraint::toLongDistanceConstraint()
{
    return dynamic_cast<AbstractLongDistanceConstraint *>(this);
}

AbstractNestedConstraint *AbstractConstraint::toNestedConstraint()
{
    return dynamic_cast<AbstractNestedConstraint *>(this);
}

const AbstractLongDistanceConstraint *AbstractConstraint::toLongDistanceConstraint() const
{
    return dynamic_cast<const AbstractLongDistanceConstraint *>(this);
}

const AbstractNestedConstraint *AbstractConstraint::toNestedConstraint() const
{
    return dynamic_cast<const AbstractNestedConstraint *>(this);
}

QString AbstractConstraint::stringFromType(const AbstractConstraint::Type &t)
{
    switch( t )
    {
    case AbstractConstraint::MatchCondition:
        return "MatchCondition";
    case AbstractConstraint::LocalConstraint:
        return "LocalConstraint";
    case AbstractConstraint::LongDistanceConstraint:
        return "LongDistanceConstraint";
    case AbstractConstraint::Nested:
        return "Nested";
    case AbstractConstraint::Pointer:
        return "Pointer";
    }
    return "";
}

bool AbstractConstraint::isNot() const
{
    return false;
}

void AbstractConstraint::readId(QXmlStreamReader &in)
{
    mId = in.attributes().value("id").toString();
}

void AbstractConstraint::setType(const Type &type)
{
    mType = type;
}
