#include "messages.h"

#include <QtGlobal>
#include <QCoreApplication>
#include <QtDebug>
#include <QTextStream>
#include <QFile>

using namespace ME;

QString Messages::logFilename("log.xml");

ME::Messages::Messages()
{
}

ME::Messages::~Messages()
{
    /// closeLogFile() will be called here:
    redirectMessagesToDefault();
}

Messages &Messages::instance()
{
    static Messages messages;
    return messages;
}

void ME::Messages::handler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    switch (type) {
    case QtDebugMsg:
        instance().xml.writeTextElement("debug",msg);
        break;
    case QtWarningMsg:
        instance().xml.writeTextElement("warning",msg);
        break;
    case QtCriticalMsg:
        instance().xml.writeTextElement("critical",msg);
        break;
    case QtFatalMsg:
        instance().xml.writeTextElement("fatal",msg);
        break;
    case QtInfoMsg:
        instance().xml.writeTextElement("info",msg);
        break;
    }
}

void Messages::redirectMessagesTo(const QString &outfile, bool resetFile)
{
    logFilename = outfile;
    if( resetFile )
        instance().openLogFile();
    qInstallMessageHandler( Messages::handler );
}

void Messages::redirectMessagesToDefault()
{
    qInstallMessageHandler(nullptr);
    instance().closeLogFile();
}

QXmlStreamWriter &Messages::stream()
{
    return instance().xml;
}

void Messages::openLogFile()
{
    logFile.setFileName(logFilename);

    resetLogFile();

    /// open the file for subsequent writing
    if( ! logFile.open(QIODevice::WriteOnly | QIODevice::Append) )
    {
        qCritical() << "Could not open log:" << logFilename;
        return;
    }

    xml.setDevice(&logFile);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    STREAM.setEncoding( QStringConverter::Utf8 );
#else
    xml.setCodec("UTF-8");
#endif
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);
    xml.writeStartDocument();

    xml.writeStartElement("mortal-engine-log");
}

void Messages::resetLogFile()
{
    QFile f(logFilename);
    f.open(QIODevice::WriteOnly);
    QTextStream textStream(&f);
    textStream << "";
    f.close();
}

void Messages::closeLogFile()
{
    if( logFile.isOpen() )
    {
        xml.writeEndDocument();
        logFile.close();
    }
}

