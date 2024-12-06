#include "transductiontest.h"

#include "datatypes/generation.h"

using namespace ME;

TransductionTest::TransductionTest(const Morphology *morphology) : AbstractInputOutputTest(morphology)
{

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
