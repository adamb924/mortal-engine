#ifndef GENERATIONTEST_H
#define GENERATIONTEST_H

#include "abstractinputoutputtest.h"
#include "datatypes/morphemesequence.h"

class GenerationTest : public AbstractInputOutputTest
{
public:
    explicit GenerationTest(const Morphology *morphology);

    //! \brief Runs the test.
    void runTest() override;

    void setLexicalStemId(const qlonglong &lexicalStemId);

    void setMorphologicalString(const QString &morphologicalString);

private:
    qlonglong mLexicalStemId;
    MorphemeSequence mMorphemeSequence;
};

#endif // GENERATIONTEST_H
