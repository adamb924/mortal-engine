#ifndef NOTCONDITION_H
#define NOTCONDITION_H

#include <QList>

#include "abstractnestedconstraint.h"

class NotCondition : public AbstractNestedConstraint
{
public:
    NotCondition();
    ~NotCondition() override;

    bool matchesThisConstraint( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;
    QString satisfactionSummary( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const QString & suffix = QString()) const override;

    bool isNot() const override;

    static QString elementName();
    static AbstractConstraint *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);
};

#endif // NOTCONDITION_H
