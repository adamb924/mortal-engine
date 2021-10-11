#include "transductiontest.h"

#include "datatypes/generation.h"

TransductionTest::TransductionTest(const Morphology *morphology) : AbstractTest(morphology)
{

}

bool TransductionTest::succeeds() const
{
    return mTargetOutputs == mActualOutputs;
}

QString TransductionTest::message() const
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

QString TransductionTest::barebonesOutput() const
{
    return setToBarebonesString(mActualOutputs);
}

void TransductionTest::runTest()
{
    mActualOutputs.clear();

    QList<Generation> outputs = mMorphology->transduceInto( mInput, mOutputWritingSystem );
    QListIterator<Generation> i(outputs);
    while( i.hasNext() )
    {
        mActualOutputs << i.next().form();
    }
}

void TransductionTest::addTargetOutput(Form output)
{
    mOutputWritingSystem = output.writingSystem();
    mTargetOutputs << output;
}
