#include "morphemesequenceconstraint.h"

#include <QRegularExpression>
#include <stdexcept>

#include "datatypes/allomorph.h"
#include "nodes/abstractnode.h"

QRegularExpression MorphemeSequenceConstraint::morphemeStringFormat = QRegularExpression("^(\\[.*\\])+$");

MorphemeSequenceConstraint::MorphemeSequenceConstraint() : AbstractGenerationConstraint(AbstractGenerationConstraint::MorphemeSequence)
{

}

MorphemeSequenceConstraint::MorphemeSequenceConstraint(const QString &morphemeString) : AbstractGenerationConstraint(AbstractGenerationConstraint::MorphemeSequence), mOriginalMorphemeString(morphemeString)
{
    if( ! morphemeStringFormat.match(morphemeString).hasMatch() )
    {
        std::string str = morphemeString.toUtf8().constData();
        throw std::runtime_error( "Invalid morpheme sequence string: " + str );
    }
    QStringList morphemes = morphemeString.mid(1, morphemeString.length()-2).split("][");
    foreach(QString morpheme, morphemes)
    {
        addMorpheme(morpheme);
    }
}

MorphemeSequenceConstraint::MorphemeSequenceConstraint(const MorphemeSequenceConstraint &other) : AbstractGenerationConstraint(AbstractGenerationConstraint::MorphemeSequence), mOriginalMorphemeString(other.mOriginalMorphemeString)
{

}

MorphemeSequenceConstraint &MorphemeSequenceConstraint::operator=(const MorphemeSequenceConstraint & other)
{
    mOriginalMorphemeString = other.mOriginalMorphemeString;
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

void MorphemeSequenceConstraint::setMorphemeNames(const QList<QString> &morphemeNames)
{
    /// this longer way of doing it handles portmanteaux
    foreach( QString name, morphemeNames )
    {
        mMorphemeNames << name.split("][");
    }
    mOriginalMorphemeString = remainingMorphemeString();
}

QString MorphemeSequenceConstraint::remainingMorphemeString() const
{
    return "[" + mMorphemeNames.join("][") + "]";
}

QString MorphemeSequenceConstraint::originalMorphemeString() const
{
    return mOriginalMorphemeString;
}

QString MorphemeSequenceConstraint::summary() const
{
    QString dbgString;
    QTextStream dbg(&dbgString);

    dbg << "MorphemeSequenceConstraint ([" << mMorphemeNames.join("], [") << "])";

    return dbgString;
}
