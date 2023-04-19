#include "morphemesequence.h"

#include <QRegularExpression>

QString MorphemeSequence::STEM_LABEL = "Stem";
QRegularExpression MorphemeSequence::morphemeStringFormat = QRegularExpression("^(\\[.*\\])+$");

MorphemeSequence::MorphemeSequence()
{

}

MorphemeSequence::MorphemeSequence(const QStringList &list) : QStringList(list)
{
}

bool MorphemeSequence::isEmpty() const
{
    return length() == 0;
}

QString MorphemeSequence::toString() const
{
    return QString("[%1]").arg( join("][") );
}

MorphemeSequence MorphemeSequence::fromString(const QString &str)
{
    Q_ASSERT(str.length() > 2);
    /// ignore the opening and closing bracket
    return MorphemeSequence( str.mid(1, str.length() - 2).split("][") );
}
