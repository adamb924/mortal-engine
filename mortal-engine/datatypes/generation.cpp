#include "generation.h"

#include "constraints/abstractconstraint.h"

#include "nodes/morphologicalmodel.h"
#include "allomorph.h"
#include "morphology.h"

#include "generation-constraints/morphemesequenceconstraint.h"
#include "generation-constraints/stemidentityconstraint.h"


Generation::Generation(const WritingSystem &outputWs, const MorphologicalModel *morphologicalModel)
  : Parsing( Form(outputWs, ""), morphologicalModel )
{

}

Generation::Generation()
{

}

void Generation::append(const AbstractNode *node, const Allomorph & allomorph, const LexicalStem & lexicalStem, bool isStem)
{
    if( constraintsSetSatisfied( mLocalConstraints, node, allomorph) )
    {
        /// the local constraints are kept in the parsing object,
        /// so they need to be cleared if the allomorph satisfies them
        mLocalConstraints.clear();

        /// update the position of the parsing
        Form allomorphForm = allomorph.form( writingSystem() );
        mForm += allomorphForm;
        mPosition += allomorphForm.text().length();

        ParsingStep ps(node, allomorph, lexicalStem);
        ps.setIsStem(isStem);
        mSteps.append( ps );

        /// remove a morpheme from the morpheme sequence constraint
        /// i.e., move on to the next morpheme in the sequence
        if( allomorph.hasPortmanteau( writingSystem() ) )
        {
            mMorphemeSequenceConstraint.removeCurrentMorpheme( allomorph.portmanteau().count() );
        }
        else
        {
            mMorphemeSequenceConstraint.removeCurrentMorpheme();
        }

        addLocalConstraints( allomorph.localConstraints() );
        addLongDistanceConstraints( allomorph.longDistanceConstraints() );

        setStatus( Parsing::Ongoing );
    }
    else
    {
        setStatus( Parsing::Failed );
    }
    calculateHash();
}

void Generation::setCompleteIfAllConstraintsSatisfied()
{
    if( status() != Parsing::Failed && status() != Parsing::Null )
    {
        if( allConstraintsSatisfied() )
        {
            setStatus( Parsing::Completed );
            if( Morphology::DebugOutput )
            {
                qInfo() << "Generation completed:" << intermediateSummary();
            }
        }
        else
        {
            setStatus( Parsing::Ongoing );
        }
    }
}

StemIdentityConstraint *Generation::stemIdentityConstraint()
{
    return &mStemIdentityConstraint;
}

const StemIdentityConstraint *Generation::stemIdentityConstraint() const
{
    return &mStemIdentityConstraint;
}

void Generation::setStemIdentityConstraint(StemIdentityConstraint stemIdentityConstraint)
{
    mStemIdentityConstraint = stemIdentityConstraint;
}

MorphemeSequenceConstraint *Generation::morphemeSequenceConstraint()
{
    return &mMorphemeSequenceConstraint;
}

const MorphemeSequenceConstraint *Generation::morphemeSequenceConstraint() const
{
    return &mMorphemeSequenceConstraint;
}

void Generation::setMorphemeSequenceConstraint(MorphemeSequenceConstraint morphemeSequenceConstraint)
{
    mMorphemeSequenceConstraint = morphemeSequenceConstraint;
}

bool Generation::isGeneration() const
{
    return true;
}

bool Generation::ableToAppend(const QString &label) const
{
    bool mscHasNoMoreMorphemes = mMorphemeSequenceConstraint.hasNoMoreMorphemes();
    bool mscFailed = mMorphemeSequenceConstraint.currentMorpheme() != label;
    if( Morphology::DebugOutput )
    {
        qInfo().noquote() << qPrintable("\t") << QString("MSC has no more morphemes: %1; MSC failed: %2 (expecting: %3, trying to append %4).").arg(mscHasNoMoreMorphemes).arg(mscFailed).arg(mMorphemeSequenceConstraint.currentMorpheme()).arg(label);
    }
    if( mscHasNoMoreMorphemes || mscFailed )
        return false;
    else
        return true;
}

QString Generation::intermediateSummary() const
{
    if( isCompleted() )
    {
        return QString("'%1' generated from %2 (%3).")
                .arg( form().text(),
                      mMorphemeSequenceConstraint.originalSequence().toString(),
                      morphemeDelimitedSummary( form().writingSystem() ) );
    }
    else
    {
        return QString("'%1' generated from %2 (%3), with %4 remaining.")
                .arg( form().text(),
                      mMorphemeSequenceConstraint.originalSequence().toString(),
                      morphemeDelimitedSummary( form().writingSystem() ),
                      mMorphemeSequenceConstraint.remainingMorphemeString() );
    }
}

bool Generation::constraintsSetSatisfied(const QSet<const AbstractConstraint *> &set, const AbstractNode *node, const Allomorph &allomorph) const
{
    QSetIterator<const AbstractConstraint *> i( set );
    while( i.hasNext() )
    {
        const AbstractConstraint * c = i.next();
        if( ! c->matches(this, node, allomorph) )
        {
            return false;
        }
    }
    return true;
}
