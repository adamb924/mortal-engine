#ifndef MORPHEMESEQUENCE_H
#define MORPHEMESEQUENCE_H

#include <QList>
#include "morphemelabel.h"

#include "mortal-engine_global.h"

namespace ME {

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
    MorphemeSequence mid(int pos, int length = -1) const;
    bool beginsWith(const MorphemeSequence & needle) const;

    QString toString() const;
    static MorphemeSequence fromString(const QString & str);

    static QString STEM_LABEL;
};

} // namespace ME

#endif // MORPHEMESEQUENCE_H
