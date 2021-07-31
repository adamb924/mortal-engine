#ifndef PHONOLOGICALCONDITION_H
#define PHONOLOGICALCONDITION_H

#include "abstractconstraint.h"

#include "datatypes/writingsystem.h"

class PhonologicalCondition : public AbstractConstraint
{
public:
    PhonologicalCondition();
    ~PhonologicalCondition() override;

    bool matches( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const QString & suffix = QString()) const override;

    void addRegularExpression( const WritingSystem & ws, const QRegularExpression & re );

    static QString elementName();
    static AbstractConstraint *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);

private:
    QHash<WritingSystem,QRegularExpression> mRegularExpressions;
};

#endif // PHONOLOGICALCONDITION_H
