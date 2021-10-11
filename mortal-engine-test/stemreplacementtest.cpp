#include "stemreplacementtest.h"

#include "generation-constraints/morphemesequenceconstraint.h"
#include "generation-constraints/stemidentityconstraint.h"

#include "datatypes/generation.h"
#include "datatypes/writingsystem.h"

StemReplacementTest::StemReplacementTest(const Morphology *morphology) : AbstractTest(morphology), mOutputWritingSystem("")
{

}

StemReplacementTest::~StemReplacementTest()
{

}

bool StemReplacementTest::succeeds() const
{
    return mTargetOutputs == mActualOutputs;
}

QString StemReplacementTest::message() const
{
    if( succeeds() )
    {
        return QObject::tr("%1The input %2 (%3) produced %4, which is correct.")
                .arg( summaryStub(),
                      mInput.text(),
                      mInput.writingSystem().abbreviation(),
                      setToString(mActualOutputs) );
    }
    else
    {
        return QObject::tr("%1The input %2 (%3) produced %4, but it should have been %5.")
                .arg( summaryStub(),
                      mInput.text(),
                      mInput.writingSystem().abbreviation(),
                      setToString(mActualOutputs),
                      setToString(mTargetOutputs) );
    }
}

QString StemReplacementTest::barebonesOutput() const
{
    return setToBarebonesString(mActualOutputs);
}

void StemReplacementTest::runTest()
{
    mActualOutputs.clear();

    WritingSystem ws = mOutputWritingSystem.isNull() ? mInput.writingSystem() : mOutputWritingSystem;
    QList<Generation> newParsings = mMorphology->replaceStemInto( mInput, mReplacementStem, ws );
    QListIterator<Generation> newParsingIterator(newParsings);
    while( newParsingIterator.hasNext() )
    {
        mActualOutputs << newParsingIterator.next().form();
    }
}

void StemReplacementTest::setReplacementStem(const LexicalStem replacementStem)
{
    mReplacementStem = replacementStem;
}

void StemReplacementTest::addTargetOutput(Form output)
{
    mTargetOutputs << output;
}

void StemReplacementTest::setOutputWritingSystem(const WritingSystem &outputWritingSystem)
{
    mOutputWritingSystem = outputWritingSystem;
}
