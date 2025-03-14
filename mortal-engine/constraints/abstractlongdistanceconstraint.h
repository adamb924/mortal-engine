#ifndef ABSTRACTLONGDISTANCECONSTRAINT_H
#define ABSTRACTLONGDISTANCECONSTRAINT_H

#include "abstractconstraint.h"
#include "mortal-engine_global.h"

namespace ME {

class MORTAL_ENGINE_EXPORT AbstractLongDistanceConstraint : public AbstractConstraint
{
public:
    AbstractLongDistanceConstraint();

    /// TODO this may or may not be helpful/necessary
    bool matchesThisConstraint( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

    virtual bool satisfied( const Parsing * p ) const = 0;
};

} // namespace ME

#endif // ABSTRACTLONGDISTANCECONSTRAINT_H
