#ifndef SUGGESTIONTEST_H
#define SUGGESTIONTEST_H

#include "abstracttest.h"

namespace ME {

class SuggestionTest : public AbstractTest
{
public:
    explicit SuggestionTest(Morphology *morphology);
    ~SuggestionTest() override;

    //! \brief The test succeeds if the set of generated parsings exactly matches the set of target parsings.
    bool succeeds() const override;

    //! \brief Summary message of how/whether the test succeeded or failed.
    QString message() const override;

    QString barebonesOutput() const override;

    //! \brief Runs the test
    void runTest() override;

    //! \brief Add \a parsing to the list of target parsings.
    void addTargetParsing(const QString & parsing);

private:
    QSet<QString> mTargetParsings, mActualParsings;

};

} // namespace ME

#endif // SUGGESTIONTEST_H
