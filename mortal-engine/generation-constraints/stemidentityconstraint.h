#ifndef STEMIDENTITYCONSTRAINT_H
#define STEMIDENTITYCONSTRAINT_H

#include "abstractgenerationconstraint.h"
#include "datatypes/lexicalstem.h"

#include "mortal-engine_global.h"

namespace ME {

class MORTAL_ENGINE_EXPORT StemIdentityConstraint : public AbstractGenerationConstraint
{
public:
    StemIdentityConstraint();
    explicit StemIdentityConstraint(const QList<LexicalStem> &lexicalStems);
    StemIdentityConstraint(const StemIdentityConstraint &other);
    StemIdentityConstraint &operator=(const StemIdentityConstraint &other);

    const LexicalStem currentLexicalStem() const;
    void addLexicalStem(const LexicalStem &lexicalStem);
    void resolveCurrentStemRequirement();

    bool hasStemRequirement() const;

    bool satisfied(const AbstractNode *node, const Allomorph &allomorph ) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const override;

private:
    /// these need to be objects rather than pointers because even LexicalStems that don't exist in the model can be specified in generations
    QList<LexicalStem> mLexicalStems;
};

} // namespace ME

#endif // STEMIDENTITYCONSTRAINT_H
