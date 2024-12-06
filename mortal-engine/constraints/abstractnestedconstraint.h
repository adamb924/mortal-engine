#ifndef ABSTRACTNESTEDCONSTRAINT_H
#define ABSTRACTNESTEDCONSTRAINT_H

#include "abstractconstraint.h"

#include <QSet>

namespace ME {

class AbstractNestedConstraint : public AbstractConstraint
{
public:
    AbstractNestedConstraint();

    void setConstraints(const QSet<const AbstractConstraint *> &constraints);
    void addConstraint(const AbstractConstraint * c);

    bool isMatchCondition() const override;
    bool isLocalConstraint() const override;
    bool isLongDistanceConstraint() const override;

    int count() const;
    QSet<const AbstractConstraint *> children() const;

    void checkIfConstraintsAreOfSameType() const;

protected:
    QSet<const AbstractConstraint *> mConstraints;

};

} // namespace ME

#endif // ABSTRACTNESTEDCONSTRAINT_H
