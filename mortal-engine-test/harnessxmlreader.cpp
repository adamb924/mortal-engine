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

#include <QTextStream>
#include <QXmlStreamReader>
#include <QFile>

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
QString HarnessXmlReader::XML_INTERLINEAR_GLOSS_TEST = "interlinear-gloss-test";


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
                QString name = in.name().toString();
                QXmlStreamAttributes attr = in.attributes();


                if ( name == "schema" )
                {
                    TestSchema * schema = new TestSchema( attr.value(XML_LANG).toString() );
                    if( attr.hasAttribute(XML_SHOW_MODEL) && attr.value(XML_SHOW_MODEL) == XML_TRUE )
                    {
                        schema->setShowModel(true);
                    }
                    mHarness->mSchemata.append( schema );
                }
                else if ( name == "morphology-file" )
                {
                    mHarness->mSchemata.last()->setMorphologyFile( in.readElementText() );
                    if( ! mHarness->mSchemata.last()->morphology()->isOk() )
                    {
                        qCritical() << "Error reading morphology file. (There should be an error above.)";
                        return;
                    }
                }
                else if ( name == XML_RECOGNITION_TEST )
                {
                    mHarness->mSchemata.last()->addTest( readRecognitionTest(in, mHarness->mSchemata.last() ) );
                }
                else if ( name == XML_TRANSDUCTION_TEST )
                {
                    mHarness->mSchemata.last()->addTest( readTransductionTest(in, mHarness->mSchemata.last() ) );
                }
                else if ( name == XML_PARSING_TEST )
                {
                    mHarness->mSchemata.last()->addTest( readParsingTest(in, mHarness->mSchemata.last() ) );
                }
                else if ( name == "blank" )
                {
                    mHarness->mSchemata.last()->addTest( new Message( "", mHarness->mSchemata.last()->morphology()) );
                }
                else if ( name == "message" )
                {
                    mHarness->mSchemata.last()->addTest( new Message( in.readElementText(), mHarness->mSchemata.last()->morphology()) );
                }
                else if ( name == XML_STEM_REPLACEMENT_TEST )
                {
                    mHarness->mSchemata.last()->addTest( readStemReplacementTest(in, mHarness->mSchemata.last()) );
                }
                else if ( name == XML_SUGGESTION_TEST )
                {
                    mHarness->mSchemata.last()->addTest( readSuggestionTest(in, mHarness->mSchemata.last()) );
                }
                else if ( name == XML_GENERATION_TEST )
                {
                    mHarness->mSchemata.last()->addTest( readGenerationTest(in, mHarness->mSchemata.last()) );
                }
                else if ( name == XML_GENERATION_TEST_SHORT )
                {
                    mHarness->mSchemata.last()->addTest( readQuickGenerationTest(in, mHarness->mSchemata.last()) );
                }
                else if ( name == XML_INTERLINEAR_GLOSS_TEST )
                {
                    mHarness->mSchemata.last()->addTest( readInterlinearGlossTest(in, mHarness->mSchemata.last()) );
                }
                else if ( name == "accept" )
                {
                    mHarness->mSchemata.last()->addTest( readQuickAcceptanceTest( in, mHarness->mSchemata.last() ) );
                }
                else if ( name == "reject" )
                {
                    mHarness->mSchemata.last()->addTest( readQuickRejectionTest( in, mHarness->mSchemata.last() ) );
                }
#ifndef Q_OS_WASM
                else if ( name == "sqlite-database" )
                {
                    SqliteStemList::openDatabase( attr.value("filename").toString(), attr.value("database-name").toString() );
                }
#endif
            }
        }
    }
}

RecognitionTest *HarnessXmlReader::readRecognitionTest(QXmlStreamReader &in, const TestSchema *schema)
{
    RecognitionTest* test = new RecognitionTest(schema->morphology());
    test->setLabel( in.attributes().value(XML_LABEL).toString() );
    test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

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

    if( in.attributes().hasAttribute(XML_LABEL) )
        test->setLabel( in.attributes().value(XML_LABEL).toString() );
    if( in.attributes().hasAttribute(XML_DEBUG) )
        test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

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

    if( in.attributes().hasAttribute(XML_LABEL) )
        test->setLabel( in.attributes().value(XML_LABEL).toString() );
    if( in.attributes().hasAttribute(XML_DEBUG) )
        test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

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
    test->setLabel( in.attributes().value(XML_LABEL).toString() );
    test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

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
    test->setLabel( in.attributes().value(XML_LABEL).toString() );
    test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

    QStringList outputList;

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
                outputList.clear();
            }
            else if( in.name() == XML_LABEL )
            {
                outputList.append( in.readElementText() );
            }
        }
        else if( in.tokenType() == QXmlStreamReader::EndElement )
        {
            if( in.name() == XML_OUTPUT )
            {
                test->addTargetParsing( "[" + outputList.join("][") + "]" );
            }
        }
    }

    test->evaluate();

    return test;
}

StemReplacementTest *HarnessXmlReader::readStemReplacementTest(QXmlStreamReader &in, const TestSchema *schema)
{
    StemReplacementTest* test = new StemReplacementTest(schema->morphology());
    test->setLabel( in.attributes().value(XML_LABEL).toString() );
    test->setOutputWritingSystem( WritingSystem( in.attributes().value("output-lang").toString() ) );
    test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

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
    test->setLabel( in.attributes().value(XML_LABEL).toString() );
    test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

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
    test->setLabel( in.attributes().value(XML_LABEL).toString() );
    test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

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
    test->setLabel( in.attributes().value(XML_LABEL).toString() );
    test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

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
    test->setLabel( in.attributes().value(XML_LABEL).toString() );
    test->setShowDebug( in.attributes().value(XML_DEBUG).toString() == XML_TRUE );

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_GENERATION_TEST ) )
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
