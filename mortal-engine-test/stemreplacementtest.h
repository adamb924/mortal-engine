#ifndef STEMREPLACEMENTTEST_H
#define STEMREPLACEMENTTEST_H

#include "abstractinputoutputtest.h"
#include "datatypes/lexicalstem.h"

namespace ME {

class StemReplacementTest : public AbstractInputOutputTest
{
public:
    explicit StemReplacementTest(Morphology * morphology);

    //! \brief Evaluate the function using the input provided by setInput().
    void runTest() override;

    void setReplacementStem(const LexicalStem replacementStem);

private:
    LexicalStem mReplacementStem;
};

} // namespace ME

#endif // STEMREPLACEMENTTEST_H
