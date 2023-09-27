#ifndef MORPHEMELABEL_H
#define MORPHEMELABEL_H

#include "mortal-engine_global.h"

#include <QString>

class MORTAL_ENGINE_EXPORT MorphemeLabel
{
public:
    MorphemeLabel();
    explicit MorphemeLabel(const QString & label);

    bool operator==(const MorphemeLabel & other) const;
    bool operator!=(const MorphemeLabel & other) const;

    QString toString() const;
    bool isNull() const;

    QString summary() const;

private:
    QString mLabel;
};

Q_DECL_EXPORT uint qHash(const MorphemeLabel &key);

#endif // MORPHEMELABEL_H
