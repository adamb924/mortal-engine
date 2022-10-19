#ifndef STEMIDENTITYCONSTRAINT_H
#define STEMIDENTITYCONSTRAINT_H

#include "abstractgenerationconstraint.h"
#include "datatypes/lexicalstem.h"

#include "mortal-engine_global.h"

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
    QList<LexicalStem> mLexicalStems;
};

#endif // STEMIDENTITYCONSTRAINT_H
