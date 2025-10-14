#ifndef MORPHEMESEQUENCECONSTRAINT_H
#define MORPHEMESEQUENCECONSTRAINT_H

#include "abstractgenerationconstraint.h"
#include "datatypes/morphemesequence.h"

#include <QList>

#include "mortal-engine_global.h"

namespace ME {

class Portmanteau;

class MORTAL_ENGINE_EXPORT MorphemeSequenceConstraint : public AbstractGenerationConstraint
{
public:
    MorphemeSequenceConstraint();
    explicit MorphemeSequenceConstraint(const MorphemeSequence & sequence);
    MorphemeSequenceConstraint(const MorphemeSequenceConstraint & other);
    MorphemeSequenceConstraint &operator=(const MorphemeSequenceConstraint & other);

    void addMorpheme( const MorphemeLabel & label );
    MorphemeLabel currentMorpheme() const;
    void removeCurrentMorpheme(int n = 1);
    bool hasNoMoreMorphemes() const;
    int remainingMorphemeCount() const;
    MorphemeSequence remainingMorphemes() const;

    bool satisfied( const AbstractNode *node, const Allomorph &allomorph ) const override;

    bool matchesPortmanteau(const Portmanteau & portmanteau) const;

    void setMorphemeSequence(const MorphemeSequence &sequence);

    QString remainingMorphemeString() const;


    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const override;

    MorphemeSequence originalSequence() const;

private:
    MorphemeSequence mMorphemeNames;
    MorphemeSequence mOriginalSequence;
};

} // namespace ME

#endif // MORPHEMESEQUENCECONSTRAINT_H
