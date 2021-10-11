/*!
  \class RecognitionTest
  \brief An AbstractTest subclass for testing whether an input is accepted or rejected by the model. The user can specify that the input should be accepted or rejected.
*/

#ifndef RECOGNITIONTEST_H
#define RECOGNITIONTEST_H

#include "abstracttest.h"

class RecognitionTest : public AbstractTest
{
public:
    explicit RecognitionTest(const Morphology * morphology);
    ~RecognitionTest() override;

    //! \brief Returns true if the input should be accepted and is accepted, or if it should be rejected and was rejected.
    bool succeeds() const override;

    //! \brief Summary of the result of the test.
    QString message() const override;

    QString barebonesOutput() const override;

    //! \brief Evaluates the test.
    void runTest() override;

    //! \brief Set whether the input should be accepted or rejected.
    void setShouldBeAccepted(bool shouldBeAccepted);

private:
    bool mShouldBeAccepted;
    bool mInputIsAccepted;
    bool mTestSucceeds;
    QSet<QString> mActualParsings;
    int mTotalParsingCount, mUniqueParsingCount;
};

#endif // RECOGNITIONTEST_H
