#ifndef ABSTRACTGENERATIONCONSTRAINT_H
#define ABSTRACTGENERATIONCONSTRAINT_H

class Allomorph;
class Parsing;
class AbstractNode;

#include <QString>

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT AbstractGenerationConstraint
{
public:
    enum ContraintType { MorphemeSequenceConstraint, StemIdentityConstraint };

    AbstractGenerationConstraint(ContraintType constraintType);
    virtual ~AbstractGenerationConstraint();
    AbstractGenerationConstraint(const AbstractGenerationConstraint & other);
    AbstractGenerationConstraint &operator=(const AbstractGenerationConstraint & other);

    ContraintType constraintType() const;

    virtual bool satisfied( const AbstractNode *node, const Allomorph &allomorph ) const = 0;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    virtual QString summary() const = 0;

private:
    ContraintType mConstraintType;
};

#endif // ABSTRACTGENERATIONCONSTRAINT_H
