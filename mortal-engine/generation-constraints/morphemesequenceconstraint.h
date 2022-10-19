#ifndef MORPHEMESEQUENCECONSTRAINT_H
#define MORPHEMESEQUENCECONSTRAINT_H

#include "abstractgenerationconstraint.h"

#include <QList>

#include "mortal-engine_global.h"

class Portmanteau;

class MORTAL_ENGINE_EXPORT MorphemeSequenceConstraint : public AbstractGenerationConstraint
{
public:
    MorphemeSequenceConstraint();
    explicit MorphemeSequenceConstraint(const QString & morphemeString);
    MorphemeSequenceConstraint(const MorphemeSequenceConstraint & other);
    MorphemeSequenceConstraint &operator=(const MorphemeSequenceConstraint & other);

    void addMorpheme( const QString & morphemeName );
    QString currentMorpheme() const;
    void removeCurrentMorpheme(int n = 1);
    bool hasNoMoreMorphemes() const;
    int remainingMorphemes() const;

    bool satisfied( const AbstractNode *node, const Allomorph &allomorph ) const override;

    bool matchesPortmanteau(const Portmanteau & portmanteau) const;

    void setMorphemeNames(const QList<QString> &morphemeNames);

    QString remainingMorphemeString() const;
    QString originalMorphemeString() const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const override;

    static QRegularExpression morphemeStringFormat;

private:
    QString mOriginalMorphemeString;
    QStringList mMorphemeNames;
};

#endif // MORPHEMESEQUENCECONSTRAINT_H
