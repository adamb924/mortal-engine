/*!
  \class HarnessXmlReader
  \brief An class containing the logic for creating a TestHarness object from its XML representation.
*/

#ifndef HARNESSXMLREADER_H
#define HARNESSXMLREADER_H

#include <QString>

class QTextStream;
class QXmlStreamReader;

namespace ME {

class TestSchema;
class RecognitionTest;
class TransductionTest;
class ParsingTest;
class StemReplacementTest;
class SuggestionTest;
class GenerationTest;
class InterlinearGlossTest;
class TestHarness;

class HarnessXmlReader
{
public:
    explicit HarnessXmlReader(TestHarness * harness);

    //! \brief Reads the XML at the given filename and populates mHarness with its data.
    void readTestFile(const QString & filename);

private:
    RecognitionTest *readRecognitionTest(QXmlStreamReader &in, const TestSchema * schema);
    RecognitionTest *readQuickAcceptanceTest(QXmlStreamReader &in, const TestSchema * schema);
    RecognitionTest *readQuickRejectionTest(QXmlStreamReader &in, const TestSchema * schema);
    TransductionTest *readTransductionTest(QXmlStreamReader &in, const TestSchema * schema);
    ParsingTest *readParsingTest(QXmlStreamReader &in, const TestSchema * schema);
    StemReplacementTest *readStemReplacementTest(QXmlStreamReader &in, const TestSchema * schema);
    SuggestionTest *readSuggestionTest(QXmlStreamReader &in, const TestSchema * schema);
    GenerationTest *readGenerationTest(QXmlStreamReader &in, const TestSchema * schema);
    GenerationTest *readQuickGenerationTest(QXmlStreamReader &in, const TestSchema * schema);
    InterlinearGlossTest * readInterlinearGlossTest(QXmlStreamReader &in, const TestSchema * schema);

    TestHarness *mHarness;

public:
    static QString XML_SHOW_MODEL;
    static QString XML_TRUE;
    static QString XML_INPUT;
    static QString XML_OUTPUT;
    static QString XML_SHOULD_BE_ACCEPTED;
    static QString XML_YES;
    static QString XML_RECOGNITION_TEST;
    static QString XML_TRANSDUCTION_TEST;
    static QString XML_PARSING_TEST;
    static QString XML_SUGGESTION_TEST;
    static QString XML_STEM_REPLACEMENT_TEST;
    static QString XML_GENERATION_TEST;
    static QString XML_GENERATION_TEST_SHORT;
    static QString XML_LABEL;
    static QString XML_STEM;
    static QString XML_ID;
    static QString XML_FORM;
    static QString XML_TAG;
    static QString XML_MORPHEMES;
    static QString XML_LANG;
    static QString XML_DEBUG;
    static QString XML_STEM_DEBUG;
    static QString XML_INTERLINEAR_GLOSS_TEST;
};

} // namespace ME

#endif // HARNESSXMLREADER_H
