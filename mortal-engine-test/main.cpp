#include <QCoreApplication>

#include <QCommandLineParser>
#include <QTextStream>
#include <QFile>
#include <QElapsedTimer>

#include "testharness.h"
#include "morphology.h"
#include "messages.h"

using namespace ME;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QCoreApplication::setApplicationName("Mortal Engine Tester");
    QCoreApplication::setApplicationVersion("1.0");

    QElapsedTimer timer;
    timer.start();

    QCommandLineParser parser;
    parser.setApplicationDescription("Program to process tests of Mortal Engine files.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("input", QCoreApplication::translate("main", "Input file."));
    parser.addPositionalArgument("output", QCoreApplication::translate("main", "Output file."));

    parser.addOption({"verbose", QCoreApplication::translate("main", "Verbose output.")});
    parser.addOption({"barebones", QCoreApplication::translate("main", "Barebones output.")});
    parser.addOption({"model", QCoreApplication::translate("main", "Display the model.")});
    parser.addOption({"check", QCoreApplication::translate("main", "Perform checks on the model.")});
    parser.addOption({"log", QCoreApplication::translate("main", "File for debug output."), "log"});
    parser.addOption({"inspect", QCoreApplication::translate("main", "Element ID to print summary for."), "inpect"});
    parser.addOption({"path", QCoreApplication::translate("main", "Path with data files."), "path"});

    parser.process(a);

    const bool showModel = parser.isSet("model");
    const bool verbose = parser.isSet("verbose");
    const bool barebones = parser.isSet("barebones");
    const bool check = parser.isSet("check");
    QString logfile = parser.value("log");
    const NodeId inspectId = NodeId( parser.value("inspect") );
    const QString path = parser.value("path");

    /// set the path here to read the files in the correct location
    if( !path.isEmpty() )
    {
        Morphology::setPath(path);
    }

    if(logfile.isEmpty())
        logfile = "log.txt";

    const QStringList args = parser.positionalArguments();
    if( args.count() == 0 )
    {
        parser.showHelp();
    }
    const QString input = args.at(0);
    const QString output = args.length() > 1 ? args.at(1) : "";

    TestHarness::VerbosityLevel verbosity;
    if( verbose && barebones )
    {
        qInfo() << "You can't do verbose and barebones at the same time.";
        parser.showHelp();
    }
    else if ( verbose )
    {
        verbosity = TestHarness::AllResults;
    }
    else if ( barebones )
    {
        verbosity = TestHarness::BareBones;
    }
    else
    {
        verbosity = TestHarness::ErrorsOnly;
    }

    Messages::redirectMessagesTo(logfile);

    TestHarness harness;
    harness.readTestFile( input );

    if( output.isEmpty() )
    {
        QTextStream out(stdout);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        out.setEncoding( QStringConverter::Utf8 );
#else
        out.setCodec("UTF-8");
#endif

        harness.printReport(out, verbosity, showModel, check, inspectId );
    }
    else
    {
        QFile outFile( output );
        if ( ! outFile.open(QFile::WriteOnly))
        {
            qCritical() << "Could not open output file:" << output;
            return 1;
        }
        QTextStream out(&outFile);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        out.setEncoding( QStringConverter::Utf8 );
#else
        out.setCodec("UTF-8");
#endif

        harness.printReport(out, verbosity, showModel, check, inspectId );
        outFile.close();
    }

    qInfo().noquote() << QString("Completed in %1 ms").arg(timer.elapsed());

    return 0;
}
