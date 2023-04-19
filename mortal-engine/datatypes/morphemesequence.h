#ifndef MORPHEMESEQUENCE_H
#define MORPHEMESEQUENCE_H

#include <QStringList>

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT MorphemeSequence : public QStringList
{
public:
    MorphemeSequence();
    explicit MorphemeSequence(const QStringList & list);

    bool isEmpty() const;

    QString toString() const;
    static MorphemeSequence fromString(const QString & str);

    static QString STEM_LABEL;
    static QRegularExpression morphemeStringFormat;
};

#endif // MORPHEMESEQUENCE_H
