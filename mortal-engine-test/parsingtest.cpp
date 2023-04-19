#include "parsingtest.h"

ParsingTest::ParsingTest(const Morphology *morphology) : AbstractTest(morphology), mTotalParsingCount(-1), mUniqueParsingCount(-1)
{

}

ParsingTest::~ParsingTest()
{

}

bool ParsingTest::succeeds() const
{
    return mTargetParsings == mActualParsings;
}

QString ParsingTest::message() const
{
    QString ret;
    if( succeeds() )
    {
        ret = QObject::tr("%1The input %2 (%3) produced %4, which is correct.")
                .arg( summaryStub(),
                      mInput.text(),
                      mInput.writingSystem().abbreviation(),
                      setToString(mActualParsings) );
    }
    else
    {
        ret = QObject::tr("%1The input %2 (%3) produced %4, but it should have been %5.")
                .arg( summaryStub(),
                      mInput.text(),
                      mInput.writingSystem().abbreviation(),
                      setToString(mActualParsings),
                      setToString(mTargetParsings) );
    }
    if( mTotalParsingCount != mUniqueParsingCount )
        ret += QObject::tr( " (%1 unique parsings out of %2)").arg(mUniqueParsingCount).arg(mTotalParsingCount);
    return ret;
}

QString ParsingTest::barebonesOutput() const
{
    return setToBarebonesString(mActualParsings);
}

void ParsingTest::runTest()
{
    mActualParsings.clear();
    QList<Parsing> parsings = mMorphology->possibleParsings( mInput );
    foreach (Parsing p, parsings)
    {
        mActualParsings << p.morphemeSequence();
    }
    mTotalParsingCount = parsings.count();
    mUniqueParsingCount = mActualParsings.count();
}

void ParsingTest::addTargetParsing(const MorphemeSequence &sequence)
{
    mTargetParsings << sequence;
}
