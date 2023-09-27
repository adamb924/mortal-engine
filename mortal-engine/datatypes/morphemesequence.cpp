#include "morphemesequence.h"

#include <QRegularExpression>

QString MorphemeSequence::STEM_LABEL = "Stem";
QRegularExpression MorphemeSequence::morphemeStringFormat = QRegularExpression("^(\\[.*\\])+$");

MorphemeSequence::MorphemeSequence()
{

}

MorphemeSequence::MorphemeSequence(const QList<MorphemeLabel> &list) : QList<MorphemeLabel>(list)
{
}

MorphemeSequence MorphemeSequence::operator<<(const MorphemeLabel &newLabel)
{
    append(newLabel);
    return *this;
}

MorphemeSequence MorphemeSequence::operator<<(const MorphemeSequence & newSequence)
{
    append(newSequence);
    return *this;
}

MorphemeSequence MorphemeSequence::operator+(const MorphemeLabel &newLabel) const
{
    return MorphemeSequence( *this ) << newLabel;
}

bool MorphemeSequence::isEmpty() const
{
    return length() == 0;
}

QString MorphemeSequence::toString() const
{
    QString retval;
    foreach(MorphemeLabel l, *this)
        retval += QString("[%1]").arg( l.toString() );
    return retval;
}

MorphemeSequence MorphemeSequence::fromString(const QString &str)
{
    Q_ASSERT(str.length() > 2);
    MorphemeSequence retval;
    /// ignore the opening and closing bracket
    QStringList labels = str.mid(1, str.length() - 2).split("][");
    foreach(QString label, labels)
        retval << MorphemeLabel(label);
    return retval;
}
