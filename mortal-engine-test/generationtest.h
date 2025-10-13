#ifndef GENERATIONTEST_H
#define GENERATIONTEST_H

#include "abstractinputoutputtest.h"
#include "datatypes/morphemesequence.h"

namespace ME {

class GenerationTest : public AbstractInputOutputTest
{
public:
    explicit GenerationTest(Morphology *morphology);

    //! \brief Runs the test.
    void runTest() override;

    //! \brief Returns a summary of results of the test.
    QString message() const override;

    void setLexicalStemId(const qlonglong &lexicalStemId);

    void setMorphologicalString(const QString &morphologicalString);

private:
    qlonglong mLexicalStemId;
    MorphemeSequence mMorphemeSequence;
};

} // namespace ME

#endif // GENERATIONTEST_H
