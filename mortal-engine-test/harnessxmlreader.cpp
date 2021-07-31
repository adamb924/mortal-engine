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

#include <QTextStream>
#include <QXmlStreamReader>
#include <QFile>

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
                    mHarness->mSchemata.append( new TestSchema( attr.value("label").toString() ) );
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
                else if ( name == "recognition-test" )
                {
                    mHarness->mSchemata.last()->addTest( readRecognitionTest(in, mHarness->mSchemata.last() ) );
                }
                else if ( name == "transduction-test" )
                {
                    mHarness->mSchemata.last()->addTest( readTransductionTest(in, mHarness->mSchemata.last() ) );
                }
                else if ( name == "parsing-test" )
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
                else if ( name == "stem-replacement-test" )
                {
                    mHarness->mSchemata.last()->addTest( readStemReplacementTest(in, mHarness->mSchemata.last()) );
                }
                else if ( name == "suggestion-test" )
                {
                    mHarness->mSchemata.last()->addTest( readSuggestionTest(in, mHarness->mSchemata.last()) );
                }
                else if ( name == "sqlite-database" )
                {
                    SqliteStemList::openDatabase( attr.value("filename").toString(), attr.value("database-name").toString() );
                }
            }
        }
    }
}

RecognitionTest *HarnessXmlReader::readRecognitionTest(QXmlStreamReader &in, const TestSchema *schema)
{
    RecognitionTest* test = new RecognitionTest(schema->morphology());
    test->setLabel( in.attributes().value("label").toString() );
    test->setShowDebug( in.attributes().value("debug").toString() == "true" );

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == "recognition-test") )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == "input" )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value("lang").toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );
            }
            else if( in.name() == "should-be-accepted" )
            {
                test->setShouldBeAccepted( in.readElementText() == "yes" );
            }
        }
    }

    test->evaluate();

    return test;
}

TransductionTest *HarnessXmlReader::readTransductionTest(QXmlStreamReader &in, const TestSchema *schema)
{
    TransductionTest* test = new TransductionTest(schema->morphology());
    test->setLabel( in.attributes().value("label").toString() );
    test->setShowDebug( in.attributes().value("debug").toString() == "true" );

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == "transduction-test") )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == "input" )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value("lang").toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );

            }
            else if( in.name() == "output" )
            {
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value("lang").toString() );
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
    test->setLabel( in.attributes().value("label").toString() );
    test->setShowDebug( in.attributes().value("debug").toString() == "true" );

    QStringList outputList;

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == "parsing-test") )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == "input" )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value("lang").toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );

            }
            else if( in.name() == "output" )
            {
                outputList.clear();
            }
            else if( in.name() == "label" )
            {
                outputList.append( in.readElementText() );
            }
        }
        else if( in.tokenType() == QXmlStreamReader::EndElement )
        {
            if( in.name() == "output" )
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
    test->setLabel( in.attributes().value("label").toString() );
    test->setOutputWritingSystem( WritingSystem( in.attributes().value("output-lang").toString() ) );
    test->setShowDebug( in.attributes().value("debug").toString() == "true" );

    Allomorph allomorph(Allomorph::Original);

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == "stem-replacement-test") )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == "input" )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value("lang").toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );

            }
            else if( in.name() == "output" )
            {
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value("lang").toString() );
                Form f = Form( ws, in.readElementText() );
                test->addTargetOutput( f );
            }
            else if( in.name() == "form" )
            {
                QXmlStreamAttributes attr = in.attributes();
                if( attr.hasAttribute("lang") )
                {
                    WritingSystem ws(in.attributes().value("lang").toString());
                    allomorph.setForm( Form( ws, in.readElementText() ) );
                }
            }
            else if( in.name() == "tag" )
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
    test->setLabel( in.attributes().value("label").toString() );
    test->setShowDebug( in.attributes().value("debug").toString() == "true" );

    QStringList outputList;
    QStringList stemList;

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == "suggestion-test") )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == "input" )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = schema->morphology()->writingSystem( in.attributes().value("lang").toString() );
                Form f = Form( ws, in.readElementText() );
                test->setInput( f );
            }
            else if( in.name() == "output" )
            {
                outputList.clear();
            }
            else if( in.name() == "label" )
            {
                outputList.append( in.readElementText() );
            }
            else if( in.name() == "stem" )
            {
                outputList.append( "stem" );
                stemList.append( in.readElementText() );
            }
        }
        else if( in.tokenType() == QXmlStreamReader::EndElement )
        {
            if( in.name() == "output" )
            {
                QString targetParsing;
                foreach( QString output, outputList )
                {
                    if( output == "stem" )
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
