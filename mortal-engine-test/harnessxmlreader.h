/*!
  \class HarnessXmlReader
  \brief An class containing the logic for creating a TestHarness object from its XML representation.
*/

#ifndef HARNESSXMLREADER_H
#define HARNESSXMLREADER_H

#include <QString>

class QTextStream;
class QXmlStreamReader;

class TestSchema;
class RecognitionTest;
class TransductionTest;
class ParsingTest;
class StemReplacementTest;
class SuggestionTest;

class TestHarness;

class HarnessXmlReader
{
public:
    explicit HarnessXmlReader(TestHarness * harness);

    //! \brief Reads the XML at the given filename and populates mHarness with its data.
    void readTestFile(const QString & filename);

private:
    RecognitionTest *readRecognitionTest(QXmlStreamReader &in, const TestSchema * schema);
    TransductionTest *readTransductionTest(QXmlStreamReader &in, const TestSchema * schema);
    ParsingTest *readParsingTest(QXmlStreamReader &in, const TestSchema * schema);
    StemReplacementTest *readStemReplacementTest(QXmlStreamReader &in, const TestSchema * schema);
    SuggestionTest *readSuggestionTest(QXmlStreamReader &in, const TestSchema * schema);

    TestHarness *mHarness;
};

#endif // HARNESSXMLREADER_H
