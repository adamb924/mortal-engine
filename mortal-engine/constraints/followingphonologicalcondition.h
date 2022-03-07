#ifndef FOLLOWINGPHONOLOGICALCONDITION_H
#define FOLLOWINGPHONOLOGICALCONDITION_H

#include "abstractconstraint.h"

#include "datatypes/writingsystem.h"
class Parsing;

class FollowingPhonologicalCondition : public AbstractConstraint
{
public:
    FollowingPhonologicalCondition();
    ~FollowingPhonologicalCondition() override;

    bool matchesThisConstraint( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

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

#endif // FOLLOWINGPHONOLOGICALCONDITION_H
