#ifndef FOLLOWINGNODECONSTRAINT_H
#define FOLLOWINGNODECONSTRAINT_H

#include "abstractconstraint.h"

class FollowingNodeConstraint : public AbstractConstraint
{
public:
    enum Identifier { Null, Id, Label };

    FollowingNodeConstraint();
    ~FollowingNodeConstraint() override;

    bool matches( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const QString & suffix = QString()) const override;

    static QString elementName();
    static AbstractConstraint *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);

    void setIdentifierString(const QString &identifierString);

    void setIdentifier(const Identifier &identifier);

private:
    QString mIdentifierString;
    Identifier mIdentifier;
};

#endif // FOLLOWINGNODECONSTRAINT_H
