#ifndef STEMREPLACEMENTTEST_H
#define STEMREPLACEMENTTEST_H

#include "abstractinputoutputtest.h"
#include "datatypes/lexicalstem.h"

class StemReplacementTest : public AbstractInputOutputTest
{
public:
    explicit StemReplacementTest(const Morphology * morphology);

    //! \brief Evaluate the function using the input provided by setInput().
    void runTest() override;

    void setReplacementStem(const LexicalStem replacementStem);

private:
    LexicalStem mReplacementStem;
};

#endif // STEMREPLACEMENTTEST_H
