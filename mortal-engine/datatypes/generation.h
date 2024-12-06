#ifndef GENERATION_H
#define GENERATION_H

#include "parsing.h"

#include "generation-constraints/morphemesequenceconstraint.h"
#include "generation-constraints/stemidentityconstraint.h"

#include "mortal-engine_global.h"

namespace ME {

class MORTAL_ENGINE_EXPORT Generation : public Parsing
{
public:
    Generation(const WritingSystem & outputWs, const MorphologicalModel * morphologicalModel);
    Generation();

    void append(const AbstractNode* node, const Allomorph &allomorph, const LexicalStem &lexicalStem = LexicalStem(), bool isStem = false ) override;

    /// Calls allConstraintsSatisfied() and sets the status either to Completed or Ongoing as a result
    /// Does not change the status if the status is Failed
    void setCompleteIfAllConstraintsSatisfied();

    StemIdentityConstraint *stemIdentityConstraint();
    const StemIdentityConstraint *stemIdentityConstraint() const;
    void setStemIdentityConstraint(StemIdentityConstraint stemIdentityConstraint);

    MorphemeSequenceConstraint *morphemeSequenceConstraint();
    const MorphemeSequenceConstraint *morphemeSequenceConstraint() const;
    void setMorphemeSequenceConstraint(MorphemeSequenceConstraint morphemeSequenceConstraint);

    bool isGeneration() const override;

    bool ableToAppend(const MorphemeLabel &label) const;

    QString intermediateSummary() const override;

protected:
    bool constraintsSetSatisfied(const QSet<const AbstractConstraint *> &set, const AbstractNode *node, const Allomorph &allomorph) const;

private:
    StemIdentityConstraint mStemIdentityConstraint;
    MorphemeSequenceConstraint mMorphemeSequenceConstraint;
};

} // namespace ME

#endif // GENERATION_H
