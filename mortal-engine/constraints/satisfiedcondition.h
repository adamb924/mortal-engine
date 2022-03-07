#ifndef SATISFIEDCONDITION_H
#define SATISFIEDCONDITION_H

#include "abstractconstraint.h"

class SatisfiedCondition : public AbstractConstraint
{
public:
    SatisfiedCondition();
    ~SatisfiedCondition() override;

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

};

#endif // SATISFIEDCONDITION_H
