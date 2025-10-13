#ifndef INTERLINEARGLOSSTEST_H
#define INTERLINEARGLOSSTEST_H

#include "abstractinputoutputtest.h"

namespace ME {

class InterlinearGlossTest : public AbstractInputOutputTest
{
public:
    explicit InterlinearGlossTest(Morphology *morphology);

    //! \brief Runs the test.
    void runTest() override;
};

} // namespace ME

#endif // INTERLINEARGLOSSTEST_H
