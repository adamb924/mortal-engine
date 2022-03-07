#ifndef ABSTRACTLONGDISTANCECONSTRAINT_H
#define ABSTRACTLONGDISTANCECONSTRAINT_H

#include "abstractconstraint.h"

class AbstractLongDistanceConstraint : public AbstractConstraint
{
public:
    AbstractLongDistanceConstraint();

    /// TODO this may or may not be helpful/necessary
    bool matchesThisConstraint( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

    virtual bool satisfied( const Parsing * p ) const = 0;
};

#endif // ABSTRACTLONGDISTANCECONSTRAINT_H
