#ifndef BOUNDCONDITION_H
#define BOUNDCONDITION_H

#include "abstractlongdistanceconstraint.h"

namespace ME {

class BoundCondition : public AbstractLongDistanceConstraint
{
public:
    BoundCondition();
    ~BoundCondition() override;

    bool matchesThisConstraint( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

    bool satisfied( const Parsing * p ) const override;

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

} // namespace ME

#endif // BOUNDCONDITION_H
