#include "abstractlongdistanceconstraint.h"

AbstractLongDistanceConstraint::AbstractLongDistanceConstraint() : AbstractConstraint(AbstractConstraint::LongDistanceConstraint)
{

}

bool AbstractLongDistanceConstraint::matches(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED(node)
    Q_UNUSED(allomorph)
    return satisfied(parsing);
}
