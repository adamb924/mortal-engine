#include <QCoreApplication>

#include <QCommandLineParser>
#include <QTextStream>
#include <QFile>
#include <QElapsedTimer>

#include "testharness.h"
#include "morphology.h"
#include "messages.h"

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

    QCommandLineOption verborseOutputOption(QStringList() << "f" << "verbose", QCoreApplication::translate("main", "Verbose output."));
    parser.addOption(verborseOutputOption);

    QCommandLineOption barebonesOutputOption(QStringList() << "b" << "barebones", QCoreApplication::translate("main", "Barebones output."));
    parser.addOption(barebonesOutputOption);

    QCommandLineOption showModelOption(QStringList() << "m" << "model", QCoreApplication::translate("main", "Display the model."));
    parser.addOption(showModelOption);

    QCommandLineOption debugOption(QStringList() << "d" << "debug", QCoreApplication::translate("main", "Print exhaustive debug information."));
    parser.addOption(debugOption);

    QCommandLineOption checkOption(QStringList() << "c" << "check", QCoreApplication::translate("main", "Perform checks on the model."));
    parser.addOption(checkOption);

    QCommandLineOption debugOutputOption(QStringList() << "l" << "log", QCoreApplication::translate("main", "File for debug output."), "log");
    parser.addOption(debugOutputOption);

    QCommandLineOption inspectOption(QStringList() << "i" << "inspect", QCoreApplication::translate("main", "Element ID to print summary for."), "log");
    parser.addOption(inspectOption);

    parser.process(a);

    const bool showModel = parser.isSet(showModelOption);
    const bool verbose = parser.isSet(verborseOutputOption);
    const bool barebones = parser.isSet(barebonesOutputOption);
    Morphology::DebugOutput = parser.isSet(debugOption);
    const bool check = parser.isSet(checkOption);
    const QString logfile = parser.value(debugOutputOption);
    const QString inspectId = parser.value(inspectOption);

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

    redirectMessagesTo(logfile);

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
