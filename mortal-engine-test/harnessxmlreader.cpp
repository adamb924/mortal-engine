#include "harnessxmlreader.h"

#include "recognitiontest.h"
#include "transductiontest.h"
#include "parsingtest.h"
#include "morphology.h"
#include "testschema.h"
#include "message.h"
#include "stemreplacementtest.h"
#include "datatypes/lexicalstem.h"
#include "suggestiontest.h"
#include "nodes/sqlitestemlist.h"
#include "generationtest.h"
#include "interlinearglosstest.h"
#include "datatypes/morphemesequence.h"

#include <QTextStream>
#include <QXmlStreamReader>
#include <QFile>

using namespace ME;

QString HarnessXmlReader::XML_SHOW_MODEL = "show-model";
QString HarnessXmlReader::XML_TRUE = "true";
QString HarnessXmlReader::XML_INPUT = "input";
QString HarnessXmlReader::XML_OUTPUT = "output";
QString HarnessXmlReader::XML_SHOULD_BE_ACCEPTED = "should-be-accepted";
QString HarnessXmlReader::XML_YES = "yes";
QString HarnessXmlReader::XML_RECOGNITION_TEST = "recognition-test";
QString HarnessXmlReader::XML_TRANSDUCTION_TEST = "transduction-test";
QString HarnessXmlReader::XML_PARSING_TEST = "parsing-test";
QString HarnessXmlReader::XML_GENERATION_TEST = "generation-test";
QString HarnessXmlReader::XML_GENERATION_TEST_SHORT = "generate";
QString HarnessXmlReader::XML_LABEL = "label";
QString HarnessXmlReader::XML_STEM = "stem";
QString HarnessXmlReader::XML_ID = "id";
QString HarnessXmlReader::XML_FORM = "form";
QString HarnessXmlReader::XML_SUGGESTION_TEST = "suggestion-test";
QString HarnessXmlReader::XML_STEM_REPLACEMENT_TEST = "stem-replacement-test";
QString HarnessXmlReader::XML_TAG = "tag";
QString HarnessXmlReader::XML_MORPHEMES = "morphemes";
QString HarnessXmlReader::XML_LANG = "lang";
QString HarnessXmlReader::XML_DEBUG = "debug";
QString HarnessXmlReader::XML_STEM_DEBUG = "stem-debug";
QString HarnessXmlReader::XML_INTERLINEAR_GLOSS_TEST = "interlinear-gloss-test";
QString HarnessXmlReader::XML_INCLUDE = "include";
QString HarnessXmlReader::XML_SCHEMA = "schema";
QString HarnessXmlReader::XML_BLANK = "blank";
QString HarnessXmlReader::XML_MESSAGE = "message";
QString HarnessXmlReader::XML_ACCEPT = "accept";
QString HarnessXmlReader::XML_REJECT = "reject";
QString HarnessXmlReader::XML_MORPHOLOGY_FILE = "morphology-file";
QString HarnessXmlReader::XML_SQLITE_DATABASE = "sqlite-database";
QString HarnessXmlReader::XML_SRC = "src";
QString HarnessXmlReader::XML_FILENAME = "filename";
QString HarnessXmlReader::XML_DATABASE_NAME = "database-name";

HarnessXmlReader::HarnessXmlReader(TestHarness *harness) : mHarness(harness)
{

}

void HarnessXmlReader::readTestFile(const QString &filename)
{
    QFile file(filename);
    if( file.open( QFile::ReadOnly ) )
    {
        QXmlStreamReader in(&file);

        while(!in.atEnd())
        {
            in.readNext();

            if( in.tokenType() == QXmlStreamReader::StartElement )
            {
                if (in.name() == XML_SCHEMA) {
                    mHarness->mSchemata.append(readSchema(in));
                } else if (in.name() == XML_INCLUDE) {
                    auto *schema = readSchema(in.attributes().value(XML_SRC).toString());
                    if (schema != nullptr)
                        mHarness->mSchemata.append(schema);
                }
#ifndef Q_OS_WASM
                else if (in.name() == XML_SQLITE_DATABASE) {
                    SqliteStemList::openSqliteDatabase(
                        in.attributes().value(XML_FILENAME).toString(),
                        in.attributes().value(XML_DATABASE_NAME).toString());
                }
#endif
            }
        }
    }
}

TestSchema *HarnessXmlReader::readSchema(QXmlStreamReader &in)
{
    Q_ASSERT(in.name() == XML_SCHEMA && in.isStartElement());

    TestSchema *schema = new TestSchema(in.attributes().value(XML_LANG).toString());
    if (in.attributes().hasAttribute(XML_SHOW_MODEL)
        && in.attributes().value(XML_SHOW_MODEL) == XML_TRUE) {
        schema->setShowModel(true);
    }

    while (!in.atEnd()) {
        in.readNext();

        if (in.tokenType() == QXmlStreamReader::StartElement) {
            QString name = in.name().toString();
            QXmlStreamAttributes attr = in.attributes();

            if (name == XML_MORPHOLOGY_FILE) {
                schema->setMorphologyFile(in.readElementText());
            } else if (name == XML_RECOGNITION_TEST) {
                schema->addTest(readRecognitionTest(in, schema));
            } else if (name == XML_TRANSDUCTION_TEST) {
                schema->addTest(readTransductionTest(in, schema));
            } else if (name == XML_PARSING_TEST) {
                schema->addTest(readParsingTest(in, schema));
            } else if (name == XML_BLANK) {
                schema->addTest(new Message("", schema->morphology()));
            } else if (name == XML_MESSAGE) {
                schema->addTest(new Message(in.readElementText(), schema->morphology()));
            } else if (name == XML_STEM_REPLACEMENT_TEST) {
                schema->addTest(readStemReplacementTest(in, schema));
            } else if (name == XML_SUGGESTION_TEST) {
                schema->addTest(readSuggestionTest(in, schema));
            } else if (name == XML_GENERATION_TEST) {
                schema->addTest(readGenerationTest(in, schema));
            } else if (name == XML_GENERATION_TEST_SHORT) {
                schema->addTest(readQuickGenerationTest(in, schema));
            } else if (name == XML_INTERLINEAR_GLOSS_TEST) {
                schema->addTest(readInterlinearGlossTest(in, schema));
            } else if (name == XML_ACCEPT) {
                schema->addTest(readQuickAcceptanceTest(in, schema));
            } else if (name == XML_REJECT) {
                schema->addTest(readQuickRejectionTest(in, schema));
            }
        } else if (in.tokenType() == QXmlStreamReader::EndElement) {
            break;
        }
    }

    Q_ASSERT(in.name() == XML_SCHEMA && in.isEndElement());

    return schema;
}

TestSchema *HarnessXmlReader::readSchema(const QString &path)
{
    QFile file(path);
    if (file.open(QFile::ReadOnly)) {
        QXmlStreamReader in(&file);
        in.readNextStartElement();
        return readSchema(in);
    } else {
        qWarning() << "Could not open included file:" << path;
        return nullptr;
    }
}

RecognitionTest *HarnessXmlReader::readRecognitionTest(QXmlStreamReader &in, const TestSchema *schema)
{
    RecognitionTest* test = new RecognitionTest(schema->morphology());
    test->setPropertiesFromAttributes(in);

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_RECOGNITION_TEST ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_INPUT )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );
            }
            else if( in.name() == XML_SHOULD_BE_ACCEPTED )
            {
                test->setShouldBeAccepted( in.readElementText() == XML_YES );
            }
        }
    }

    test->evaluate();

    return test;
}

RecognitionTest *HarnessXmlReader::readQuickAcceptanceTest(QXmlStreamReader &in, const TestSchema *schema)
{
    RecognitionTest* test = new RecognitionTest(schema->morphology());
    test->setPropertiesFromAttributes(in);

    WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
    Form f = Form( ws, in.readElementText() );
    test->setInput( f );
    test->setShouldBeAccepted( true );
    test->evaluate();
    return test;
}

RecognitionTest *HarnessXmlReader::readQuickRejectionTest(QXmlStreamReader &in, const TestSchema *schema)
{
    RecognitionTest* test = new RecognitionTest(schema->morphology());
    test->setPropertiesFromAttributes(in);

    WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
    Form f = Form( ws, in.readElementText() );
    test->setInput( f );
    test->setShouldBeAccepted( false );
    test->evaluate();
    return test;
}

TransductionTest *HarnessXmlReader::readTransductionTest(QXmlStreamReader &in, const TestSchema *schema)
{
    TransductionTest* test = new TransductionTest(schema->morphology());
    test->setPropertiesFromAttributes(in);

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_TRANSDUCTION_TEST ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_INPUT )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );

            }
            else if( in.name() == XML_OUTPUT )
            {
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->addTargetOutput( f );
            }
        }
    }

    test->evaluate();

    return test;
}

ParsingTest *HarnessXmlReader::readParsingTest(QXmlStreamReader &in, const TestSchema *schema)
{
    ParsingTest* test = new ParsingTest(schema->morphology());
    test->setPropertiesFromAttributes(in);

    MorphemeSequence sequence;

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_PARSING_TEST ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_INPUT )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );

            }
            else if( in.name() == XML_OUTPUT )
            {
                sequence.clear();
            }
            else if( in.name() == XML_LABEL )
            {
                sequence.append( MorphemeLabel( in.readElementText() ) );
            }
        }
        else if( in.tokenType() == QXmlStreamReader::EndElement )
        {
            if( in.name() == XML_OUTPUT )
            {
                test->addTargetParsing( sequence );
            }
        }
    }

    test->evaluate();

    return test;
}

StemReplacementTest *HarnessXmlReader::readStemReplacementTest(QXmlStreamReader &in, const TestSchema *schema)
{
    StemReplacementTest* test = new StemReplacementTest(schema->morphology());
    test->setPropertiesFromAttributes(in);
    test->setOutputWritingSystem( WritingSystem( in.attributes().value("output-lang").toString() ) );

    Allomorph allomorph(Allomorph::Original);

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_STEM_REPLACEMENT_TEST) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_INPUT )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );

            }
            else if( in.name() == XML_OUTPUT )
            {
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->addTargetOutput( f );
            }
            else if( in.name() == XML_FORM )
            {
                QXmlStreamAttributes attr = in.attributes();
                if( attr.hasAttribute(XML_LANG) )
                {
                    WritingSystem ws(in.attributes().value(XML_LANG).toString());
                    allomorph.setForm( Form( ws, in.readElementText() ) );
                }
            }
            else if( in.name() == XML_TAG )
            {
                allomorph.addTag( in.readElementText() );
            }

        }
    }

    test->setReplacementStem( LexicalStem(allomorph) );

    test->evaluate();

    return test;

}

SuggestionTest *HarnessXmlReader::readSuggestionTest(QXmlStreamReader &in, const TestSchema *schema)
{
    SuggestionTest* test = new SuggestionTest(schema->morphology());
    test->setPropertiesFromAttributes(in);

    QStringList outputList;
    QStringList stemList;

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_SUGGESTION_TEST) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_INPUT )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );
            }
            else if( in.name() == XML_OUTPUT )
            {
                outputList.clear();
            }
            else if( in.name() == XML_LABEL )
            {
                outputList.append( in.readElementText() );
            }
            else if( in.name() == XML_STEM )
            {
                outputList.append( XML_STEM );
                stemList.append( in.readElementText() );
            }
        }
        else if( in.tokenType() == QXmlStreamReader::EndElement )
        {
            if( in.name() == XML_OUTPUT )
            {
                QString targetParsing;
                foreach( QString output, outputList )
                {
                    if( output == XML_STEM )
                    {
                        targetParsing += "[" + stemList.takeFirst() + "]";
                    }
                    else
                    {
                        targetParsing += "[" + output + "]";
                    }
                }
                test->addTargetParsing( targetParsing );
            }
        }
    }

    test->evaluate();

    return test;
}

GenerationTest *HarnessXmlReader::readGenerationTest(QXmlStreamReader &in, const TestSchema *schema)
{
    GenerationTest* test = new GenerationTest(schema->morphology());
    test->setPropertiesFromAttributes(in);

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_GENERATION_TEST ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_MORPHEMES )
            {
                test->setMorphologicalString( in.readElementText() );
            }
            else if( in.name() == XML_STEM )
            {
                test->setLexicalStemId( in.attributes().value( XML_ID ).toLongLong() );
            }
            else if( in.name() == XML_OUTPUT )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->addTargetOutput( f );
            }
        }
    }

    test->evaluate();

    return test;
}

GenerationTest *HarnessXmlReader::readQuickGenerationTest(QXmlStreamReader &in, const TestSchema *schema)
{
    GenerationTest* test = new GenerationTest(schema->morphology());
    test->setPropertiesFromAttributes(in);

    test->setMorphologicalString( in.attributes().value( XML_MORPHEMES ).toString() );
    test->setLexicalStemId( in.attributes().value( XML_STEM ).toLongLong() );

    WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value( XML_LANG ).toString() );
    Form f = Form( ws, in.attributes().value( XML_OUTPUT ).toString() );
    test->addTargetOutput( f );

    test->evaluate();

    return test;
}

InterlinearGlossTest *HarnessXmlReader::readInterlinearGlossTest(QXmlStreamReader &in, const TestSchema *schema)
{
    InterlinearGlossTest* test = new InterlinearGlossTest(schema->morphology());
    test->setPropertiesFromAttributes(in);

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_INTERLINEAR_GLOSS_TEST ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_INPUT )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );

            }
            else if( in.name() == XML_OUTPUT )
            {
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value(XML_LANG).toString() );
                Form f = Form( ws, in.readElementText() );
                test->addTargetOutput( f );
            }
        }
    }

    test->evaluate();

    return test;

}
