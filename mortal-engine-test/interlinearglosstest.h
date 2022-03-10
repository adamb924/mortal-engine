#ifndef INTERLINEARGLOSSTEST_H
#define INTERLINEARGLOSSTEST_H

#include "abstractinputoutputtest.h"

class InterlinearGlossTest : public AbstractInputOutputTest
{
public:
    explicit InterlinearGlossTest(const Morphology *morphology);

    //! \brief Runs the test.
    void runTest() override;
};

#endif // INTERLINEARGLOSSTEST_H
