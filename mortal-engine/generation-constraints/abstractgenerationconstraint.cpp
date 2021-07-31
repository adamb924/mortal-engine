#include "abstractgenerationconstraint.h"

AbstractGenerationConstraint::AbstractGenerationConstraint(ContraintType constraintType) : mConstraintType(constraintType)
{

}

AbstractGenerationConstraint::~AbstractGenerationConstraint()
{

}

AbstractGenerationConstraint::ContraintType AbstractGenerationConstraint::constraintType() const
{
    return mConstraintType;
}
