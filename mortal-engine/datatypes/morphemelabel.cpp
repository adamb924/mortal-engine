#include "morphemelabel.h"

#include <QHash>
#include "hashseed.h"

using namespace ME;

MorphemeLabel::MorphemeLabel()
{

}

MorphemeLabel::MorphemeLabel(const QString & label) : mLabel(label)
{

}

bool MorphemeLabel::operator==(const MorphemeLabel &other) const
{
    return mLabel == other.mLabel;
}

bool MorphemeLabel::operator!=(const MorphemeLabel &other) const
{
    return mLabel != other.mLabel;
}

QString MorphemeLabel::toString() const
{
    return mLabel;
}

bool MorphemeLabel::isNull() const
{
    return mLabel.isEmpty();
}

QString MorphemeLabel::summary() const
{
    return QString("MorphemeLabel(%1)").arg(mLabel);
}

uint ME::qHash(const MorphemeLabel &key)
{
    return qHash(key.toString(), HASH_SEED);
}
