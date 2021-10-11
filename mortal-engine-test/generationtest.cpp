#include "generationtest.h"

#include "datatypes/generation.h"

GenerationTest::GenerationTest(const Morphology *morphology) : AbstractTest(morphology)
{

}

bool GenerationTest::succeeds() const
{
    return mTargetOutputs == mActualOutputs;
}

QString GenerationTest::message() const
{
    if( succeeds() )
    {
        return QObject::tr("%1The input %2, %3 (%4) produced %5, which is correct.")
                .arg( summaryStub() )
                .arg(mLexicalStemId)
                .arg(mMorphologicalString,
                      mOutputWritingSystem.abbreviation(),
                      setToString(mActualOutputs) );
    }
    else
    {
        return QObject::tr("%1The input %2, %3 (%4) produced %5, but it should have been %6.")
                .arg( summaryStub() )
                .arg(mLexicalStemId)
                .arg(mMorphologicalString,
                      mOutputWritingSystem.abbreviation(),
                      setToString(mActualOutputs),
                      setToString(mTargetOutputs) );
    }
}

void GenerationTest::runTest()
{
    const LexicalStem * ls = mMorphology->getLexicalStem( mLexicalStemId );
    if( ls == nullptr )
    {
        qCritical() << "Lexical stem id not found:" << mLexicalStemId;
        return;
    }

    QList<Generation> gs = mMorphology->generateForms(mOutputWritingSystem, *ls, mMorphologicalString);
    QListIterator<Generation> i(gs);
    while( i.hasNext() )
    {
        mActualOutputs << i.next().form();
    }
}

void GenerationTest::addTargetOutput(Form output)
{
    mOutputWritingSystem = output.writingSystem();
    mTargetOutputs << output;
}

void GenerationTest::setLexicalStemId(const qlonglong &lexicalStemId)
{
    mLexicalStemId = lexicalStemId;
}

void GenerationTest::setMorphologicalString(const QString &morphologicalString)
{
    mMorphologicalString = morphologicalString;
}
