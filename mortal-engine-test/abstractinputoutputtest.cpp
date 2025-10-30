#include "abstractinputoutputtest.h"

using namespace ME;

#include <QObject>

AbstractInputOutputTest::AbstractInputOutputTest(Morphology *morphology) : AbstractTest(morphology)
{

}

bool AbstractInputOutputTest::succeeds() const
{
    return mTargetOutputs == mActualOutputs;
}

QString AbstractInputOutputTest::message() const
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

QString AbstractInputOutputTest::barebonesOutput() const
{
    return setToBarebonesString(mActualOutputs);
}

void AbstractInputOutputTest::addTargetOutput(Form output)
{
    mOutputWritingSystem = output.writingSystem();
    mTargetOutputs << output;
}

void AbstractInputOutputTest::setOutputWritingSystem(const WritingSystem &newOutputWritingSystem)
{
    mOutputWritingSystem = newOutputWritingSystem;
}
