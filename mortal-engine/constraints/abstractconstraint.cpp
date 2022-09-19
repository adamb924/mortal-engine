#include "abstractconstraint.h"

#include "abstractlongdistanceconstraint.h"
#include "abstractnestedconstraint.h"

#include <QXmlStreamReader>
#include <QRegularExpression>

QString AbstractConstraint::XML_MATCH_EXPRESSION = "match-expression";
QString AbstractConstraint::XML_OPTIONAL = "optional";
QString AbstractConstraint::XML_IGNORE_WHEN_PARSING = "ignore-when-parsing";
QString AbstractConstraint::XML_IGNORE_WHEN_GENERATING = "ignore-when-generating";

AbstractConstraint::AbstractConstraint(AbstractConstraint::Type t) : mType(t)
{

}

bool AbstractConstraint::matches(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    if( shouldBeIgnored(parsing) )
        return true;
    else
        return matchesThisConstraint(parsing, node, allomorph);
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

bool AbstractConstraint::shouldBeIgnored(const Parsing *parsing) const
{
    QListIterator<IgnoreFlag> i(mIgnoreFlags);
    while( i.hasNext() )
    {
        if( i.next().shouldBeIgnored(parsing) )
            return true;
    }
    return false;
}

void AbstractConstraint::readCommonAttributes(QXmlStreamReader &in)
{
    mId = in.attributes().value("id").toString();

    if( in.attributes().hasAttribute(XML_IGNORE_WHEN_PARSING) )
    {
        QStringList writingSystems = in.attributes().value(XML_IGNORE_WHEN_PARSING).toString().split(" ");
        foreach(QString ws, writingSystems)
        {
            /// I'm creating a throwaway, incomplete WritingSystem because it's only ever going to look at the id anyway
            mIgnoreFlags << IgnoreFlag( IgnoreFlag::IgnoreParsing, WritingSystem(ws) );
        }
    }

    if( in.attributes().hasAttribute(XML_IGNORE_WHEN_GENERATING) )
    {
        QStringList writingSystems = in.attributes().value(XML_IGNORE_WHEN_GENERATING).toString().split(" ");
        foreach(QString ws, writingSystems)
        {
            /// I'm creating a throwaway, incomplete WritingSystem because it's only ever going to look at the id anyway
            mIgnoreFlags << IgnoreFlag( IgnoreFlag::IgnoreGeneration, WritingSystem(ws) );
        }
    }
}

void AbstractConstraint::setType(const Type &type)
{
    mType = type;
}
