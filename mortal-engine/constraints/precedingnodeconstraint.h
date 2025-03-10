#ifndef PRECEDINGNODECONSTRAINT_H
#define PRECEDINGNODECONSTRAINT_H

#include "abstractconstraint.h"

namespace ME {

class PrecedingNodeConstraint : public AbstractConstraint
{
public:
    enum Identifier { Null, Id, Label };
    enum Scope { AnyPreceding, ImmediatelyPreceding };

    PrecedingNodeConstraint();

    bool matchesThisConstraint( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

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
    bool matchImmediatelyPreceding(const Parsing * parsing) const;
    bool matchAnyPreceding(const Parsing * parsing) const;

    QString mIdentifierString;
    Identifier mIdentifier;
    Scope mScope;
};

} // namespace ME

#endif // PRECEDINGNODECONSTRAINT_H
