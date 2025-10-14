#include "abstractgenerationconstraint.h"

using namespace ME;

AbstractGenerationConstraint::AbstractGenerationConstraint(ContraintType constraintType) : mConstraintType(constraintType)
{

}

AbstractGenerationConstraint::~AbstractGenerationConstraint()
{

}

AbstractGenerationConstraint::AbstractGenerationConstraint(const AbstractGenerationConstraint &other)
{
    mConstraintType = other.mConstraintType;
}


AbstractGenerationConstraint &AbstractGenerationConstraint::operator=(const AbstractGenerationConstraint &other)
{
    mConstraintType = other.mConstraintType;
    return *this;
}

AbstractGenerationConstraint::ContraintType AbstractGenerationConstraint::constraintType() const
{
    return mConstraintType;
}

QString AbstractGenerationConstraint::oneLineSummary() const
{
    return toOneLine(summary());
}
