#ifndef MORPHEMESEQUENCE_H
#define MORPHEMESEQUENCE_H

#include <QList>
#include "morphemelabel.h"

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT MorphemeSequence : public QList<MorphemeLabel>
{
public:
    MorphemeSequence();
    explicit MorphemeSequence(const QList<MorphemeLabel> & list);
    explicit MorphemeSequence(const MorphemeLabel & label);

    MorphemeSequence operator<<(const MorphemeLabel & newLabel);
    MorphemeSequence operator<<(const MorphemeSequence & newSequence);
    MorphemeSequence operator+(const MorphemeLabel & newLabel) const;

    bool isEmpty() const;

    bool contains(const MorphemeSequence & needle, int *at = nullptr) const;
    MorphemeSequence replace(const MorphemeSequence & replaceThis, const MorphemeSequence & withThis) const;

    QString toString() const;
    static MorphemeSequence fromString(const QString & str);

    static QString STEM_LABEL;
    static QRegularExpression morphemeStringFormat;
};

#endif // MORPHEMESEQUENCE_H
