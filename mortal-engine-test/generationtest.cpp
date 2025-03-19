#include "generationtest.h"

#include "datatypes/generation.h"

using namespace ME;

GenerationTest::GenerationTest(const Morphology *morphology) : AbstractInputOutputTest(morphology)
{

}

void GenerationTest::runTest()
{
    const LexicalStem * ls = mMorphology->getLexicalStem( mLexicalStemId );
    if( ls == nullptr )
    {
        qCritical() << "Lexical stem id not found:" << mLexicalStemId;
        return;
    }

    QList<Generation> gs = mMorphology->generateForms(mOutputWritingSystem, *ls, mMorphemeSequence);
    QListIterator<Generation> i(gs);
    while( i.hasNext() )
    {
        mActualOutputs << i.next().form();
    }
}

QString GenerationTest::message() const
{
    if( succeeds() )
    {
        return QObject::tr("%1Lexical stem ID %2 and morphological string %3 generated %4, which is correct.")
        .arg( summaryStub())
             .arg(mLexicalStemId)
             .arg(mMorphemeSequence.toString(),
             setToString(mActualOutputs) );
    }
    else
    {
        return QObject::tr("%1Lexical stem ID %2 and morphological string %3 generated %4, but it should have been %5.")
            .arg( summaryStub())
             .arg(mLexicalStemId)
            .arg(mMorphemeSequence.toString(),
             setToString(mActualOutputs),
             setToString(mTargetOutputs) );
    }
}

void GenerationTest::setLexicalStemId(const qlonglong &lexicalStemId)
{
    mLexicalStemId = lexicalStemId;
}

void GenerationTest::setMorphologicalString(const QString &morphologicalString)
{
    mMorphemeSequence = MorphemeSequence::fromString(morphologicalString);
}
