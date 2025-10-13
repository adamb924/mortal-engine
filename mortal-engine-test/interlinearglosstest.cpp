#include "interlinearglosstest.h"

#include "datatypes/parsing.h"

using namespace ME;

InterlinearGlossTest::InterlinearGlossTest(Morphology *morphology) : AbstractInputOutputTest(morphology)
{

}

void InterlinearGlossTest::runTest()
{
    mActualOutputs.clear();

    QList<Parsing> parsings = mMorphology->possibleParsings( mInput );
    QListIterator<Parsing> i(parsings);
    while( i.hasNext() )
    {
        mActualOutputs << Form( mOutputWritingSystem, i.next().interlinearStyleSummary( mOutputWritingSystem ) );
    }
}

