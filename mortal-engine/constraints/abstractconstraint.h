#ifndef ABSTRACTCONSTRAINT_H
#define ABSTRACTCONSTRAINT_H

#include <QString>

#include "ignoreflag.h"
#include "mortal-engine_global.h"

class Allomorph;
class Parsing;
class AbstractNode;
class QXmlStreamReader;

class AbstractLongDistanceConstraint;
class AbstractNestedConstraint;
class MorphologyXmlReader;

class MORTAL_ENGINE_EXPORT AbstractConstraint
{
public:
    enum Type { MatchCondition, LocalConstraint, LongDistanceConstraint, Nested, Pointer };

    AbstractConstraint(Type t);
    virtual ~AbstractConstraint() = 0;

    bool matches( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const;
    virtual QString satisfactionSummary( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    virtual QString summary(const QString & suffix = QString()) const = 0;
    QString oneLineSummary(const QString & suffix = QString()) const;

    QString id() const;
    void setId(const QString &id);

    Type type() const;
    QString typeString() const;

    virtual bool isMatchCondition() const;
    virtual bool isLocalConstraint() const;
    virtual bool isLongDistanceConstraint() const;

    AbstractLongDistanceConstraint * toLongDistanceConstraint();
    AbstractNestedConstraint * toNestedConstraint();

    const AbstractLongDistanceConstraint * toLongDistanceConstraint() const;
    const AbstractNestedConstraint * toNestedConstraint() const;

    static QString stringFromType( const Type & t );

    virtual bool isNot() const;

    static QString XML_MATCH_EXPRESSION;
    static QString XML_OPTIONAL;
    static QString XML_IGNORE_WHEN_PARSING;
    static QString XML_IGNORE_WHEN_GENERATING;

private:
    virtual bool matchesThisConstraint( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const = 0;
    bool shouldBeIgnored( const Parsing * parsing ) const;

protected:
    void readCommonAttributes(QXmlStreamReader &in);
    void setType(const Type &type);

protected:
    QString mId;
    Type mType;

private:
    QList<IgnoreFlag> mIgnoreFlags;
};

#endif // ABSTRACTCONSTRAINT_H
