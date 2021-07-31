#include "suggestiontest.h"

SuggestionTest::SuggestionTest(const Morphology *morphology) : AbstractTest(morphology)
{

}

SuggestionTest::~SuggestionTest()
{

}

bool SuggestionTest::succeeds() const
{
    return mTargetParsings == mActualParsings;
}

QString SuggestionTest::message() const
{
    if( succeeds() )
    {
        return QObject::tr("%1The input %2 (%3) produced %4, which is correct.")
                .arg( summaryStub(),
                      mInput.text(),
                      mInput.writingSystem().abbreviation(),
                      setToString(mActualParsings) );
    }
    else
    {
        return QObject::tr("%1The input %2 (%3) produced %4, but it should have been %5.")
                .arg( summaryStub(),
                      mInput.text(),
                      mInput.writingSystem().abbreviation(),
                      setToString(mActualParsings),
                      setToString(mTargetParsings) );
    }

}

void SuggestionTest::runTest()
{
    mActualParsings.clear();
    QList<Parsing> parsings = mMorphology->guessStem( mInput );
    foreach (Parsing p, parsings)
    {
        mActualParsings << p.labelSummary();
    }
}

void SuggestionTest::addTargetParsing(const QString &parsing)
{
    mTargetParsings << parsing;
}
