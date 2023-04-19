/*!
  \class ParsingTest
  \brief An AbstractTest subclass for testing the parsing of a particular form.
*/

#ifndef PARSINGTEST_H
#define PARSINGTEST_H

#include "abstracttest.h"
#include "datatypes/morphemesequence.h"

class ParsingTest : public AbstractTest
{
public:
    explicit ParsingTest(const Morphology *morphology);
    ~ParsingTest() override;

    //! \brief The test succeeds if the set of generated parsings exactly matches the set of target parsings.
    bool succeeds() const override;

    //! \brief Summary message of how/whether the test succeeded or failed.
    QString message() const override;

    QString barebonesOutput() const override;

    //! \brief Runs the test
    void runTest() override;

    //! \brief Add \a parsing to the list of target parsings.
    void addTargetParsing(const MorphemeSequence & sequence);

private:
    QSet<MorphemeSequence> mTargetParsings, mActualParsings;
    int mTotalParsingCount, mUniqueParsingCount;
};

#endif // PARSINGTEST_H
