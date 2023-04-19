#include "morphemesequenceconstraint.h"

#include <QRegularExpression>
#include <stdexcept>

#include "datatypes/allomorph.h"
#include "nodes/abstractnode.h"

QRegularExpression MorphemeSequenceConstraint::morphemeStringFormat = QRegularExpression("^(\\[.*\\])+$");

MorphemeSequenceConstraint::MorphemeSequenceConstraint() : AbstractGenerationConstraint(AbstractGenerationConstraint::MorphemeSequenceConstraint)
{

}

MorphemeSequenceConstraint::MorphemeSequenceConstraint(const MorphemeSequence &sequence)
    : AbstractGenerationConstraint(AbstractGenerationConstraint::MorphemeSequenceConstraint),
      mMorphemeNames(sequence), mOriginalSequence(sequence)
{
}

MorphemeSequenceConstraint::MorphemeSequenceConstraint(const MorphemeSequenceConstraint &other) : AbstractGenerationConstraint(AbstractGenerationConstraint::MorphemeSequenceConstraint), mMorphemeNames(other.mMorphemeNames)
{

}

MorphemeSequenceConstraint &MorphemeSequenceConstraint::operator=(const MorphemeSequenceConstraint & other)
{
    mMorphemeNames = other.mMorphemeNames;
    return *this;
}

void MorphemeSequenceConstraint::addMorpheme(const QString &morphemeName)
{
    mMorphemeNames.append(morphemeName);
}

QString MorphemeSequenceConstraint::currentMorpheme() const
{
    if( mMorphemeNames.isEmpty() )
        return QString();
    else
        return mMorphemeNames.first();
}

void MorphemeSequenceConstraint::removeCurrentMorpheme(int n)
{
    for(int i=0; i<n; i++)
    {
        if( !mMorphemeNames.isEmpty() )
            mMorphemeNames.removeFirst();
    }
}

bool MorphemeSequenceConstraint::hasNoMoreMorphemes() const
{
    return mMorphemeNames.isEmpty();
}

int MorphemeSequenceConstraint::remainingMorphemes() const
{
    return mMorphemeNames.count();
}

bool MorphemeSequenceConstraint::satisfied(const AbstractNode *node, const Allomorph &allomorph) const
{
    if( mMorphemeNames.isEmpty() && !allomorph.isEmpty() )
    {
        return true;
    }

    if( node->label() == currentMorpheme() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool MorphemeSequenceConstraint::matchesPortmanteau(const Portmanteau &portmanteau) const
{
    if( portmanteau.count() <= mMorphemeNames.count()
            && mMorphemeNames.mid(0, portmanteau.count()) == portmanteau.morphemes() )
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MorphemeSequenceConstraint::setMorphemeSequence(const MorphemeSequence &sequence)
{
    mMorphemeNames = sequence;
    mOriginalSequence = sequence;
}

QString MorphemeSequenceConstraint::remainingMorphemeString() const
{
    return mMorphemeNames.toString();
}

QString MorphemeSequenceConstraint::summary() const
{
    QString dbgString;
    QTextStream dbg(&dbgString);

    dbg << "MorphemeSequenceConstraint ([" << mMorphemeNames.join("], [") << "])";

    return dbgString;
}

MorphemeSequence MorphemeSequenceConstraint::originalSequence() const
{
    return mOriginalSequence;
}
