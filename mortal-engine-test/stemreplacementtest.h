#ifndef STEMREPLACEMENTTEST_H
#define STEMREPLACEMENTTEST_H

#include "abstracttest.h"
#include "datatypes/lexicalstem.h"

class StemReplacementTest : public AbstractTest
{
public:
    explicit StemReplacementTest(const Morphology * morphology);
    ~StemReplacementTest() override;

    //! \brief Return true if the test succeeds, otherwise returns false.
    bool succeeds() const override;

    //! \brief Returns a message about the test (e.g., a plain text summary of what happened and whether that was correct). Whether the test succeeds or fails is entirely implementation-dependent.
    QString message() const override;

    QString barebonesOutput() const override;

    //! \brief Evaluate the function using the input provided by setInput().
    void runTest() override;

    void setReplacementStem(const LexicalStem replacementStem);

    //! \brief Add an output that the model must produce.
    void addTargetOutput( Form output );

    void setOutputWritingSystem(const WritingSystem &outputWritingSystem);

private:
    LexicalStem mReplacementStem;
    WritingSystem mOutputWritingSystem;
    QSet<Form> mActualOutputs, mTargetOutputs;
};

#endif // STEMREPLACEMENTTEST_H
