#include "abstractlongdistanceconstraint.h"

using namespace ME;

AbstractLongDistanceConstraint::AbstractLongDistanceConstraint() : AbstractConstraint(AbstractConstraint::LongDistanceConstraint)
{

}

bool AbstractLongDistanceConstraint::matchesThisConstraint(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED(node)
    Q_UNUSED(allomorph)
    return satisfied(parsing);
}
