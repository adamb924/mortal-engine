#include "generationtest.h"

#include "datatypes/generation.h"

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

void GenerationTest::setLexicalStemId(const qlonglong &lexicalStemId)
{
    mLexicalStemId = lexicalStemId;
}

void GenerationTest::setMorphologicalString(const QString &morphologicalString)
{
    mMorphemeSequence = MorphemeSequence::fromString(morphologicalString);
}
