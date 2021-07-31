#ifndef ORCONDITION_H
#define ORCONDITION_H

#include <QList>

#include "abstractnestedconstraint.h"

class OrCondition : public AbstractNestedConstraint
{
public:
    OrCondition();
    ~OrCondition() override;

    bool matches( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;
    QString satisfactionSummary( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const QString & suffix = QString()) const override;

    static QString elementName();
    static AbstractConstraint *readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);
};

#endif // ORCONDITION_H
