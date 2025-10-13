#include "recognitiontest.h"

using namespace ME;

RecognitionTest::RecognitionTest(Morphology *morphology) : AbstractTest(morphology), mShouldBeAccepted(true), mInputIsAccepted(false), mTestSucceeds(false), mTotalParsingCount(-1), mUniqueParsingCount(-1)
{

}

RecognitionTest::~RecognitionTest()
{

}

bool RecognitionTest::succeeds() const
{
    return mTestSucceeds;
}

QString RecognitionTest::message() const
{
    QString ret;
    ret = QObject::tr("%1The input %2 (%3) was %4 by the model, which is %5. %6")
            .arg( summaryStub(),
                  mInput.text(),
                  mInput.writingSystem().abbreviation(),
                  mInputIsAccepted ? "accepted" : "rejected",
                  mTestSucceeds ? "correct" : "incorrect",
                  mInputIsAccepted ? setToString(mActualParsings) : "" );
    if( mTotalParsingCount != mUniqueParsingCount )
        ret += QObject::tr( " (%1 unique parsings out of %2)").arg(mUniqueParsingCount).arg(mTotalParsingCount);
    return ret;
}

QString RecognitionTest::barebonesOutput() const
{
    return mInputIsAccepted ? "accepted" : "rejected";
}

void RecognitionTest::runTest()
{
    QList<Parsing> parsings = mMorphology->possibleParsings( mInput );

    foreach (Parsing p, parsings)
    {
        mActualParsings << p.labelSummary();
    }

    mInputIsAccepted = parsings.count() > 0;
    mTestSucceeds = (mInputIsAccepted && mShouldBeAccepted) || (!mInputIsAccepted && !mShouldBeAccepted);

    mTotalParsingCount = parsings.count();
    mUniqueParsingCount = mActualParsings.count();
}

void RecognitionTest::setShouldBeAccepted(bool shouldBeAccepted)
{
    mShouldBeAccepted = shouldBeAccepted;
}
