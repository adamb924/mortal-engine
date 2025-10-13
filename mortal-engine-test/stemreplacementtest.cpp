#include "stemreplacementtest.h"

#include "datatypes/generation.h"
#include "datatypes/writingsystem.h"

using namespace ME;

StemReplacementTest::StemReplacementTest(Morphology *morphology) : AbstractInputOutputTest(morphology)
{

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
