/*!
  \class TransductionTest
  \brief An AbstractTest subclass for testing the transduction of a form from one writing system to another.
*/

#ifndef TRANSDUCTIONTEST_H
#define TRANSDUCTIONTEST_H

#include "abstractinputoutputtest.h"

namespace ME {

class TransductionTest : public AbstractInputOutputTest
{
public:
    explicit TransductionTest(const Morphology *morphology);

    //! \brief Runs the test.
    void runTest() override;
};

} // namespace ME

#endif // TRANSDUCTIONTEST_H
