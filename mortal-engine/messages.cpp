#include "messages.h"

#include <QtGlobal>
#include <QCoreApplication>
#include <QtDebug>
#include <QTextStream>
#include <QFile>

using namespace MortalEngineDebug;

QFile MortalEngineDebug::DEBUG_FILE;
QTextStream MortalEngineDebug::STREAM;
QString MortalEngineDebug::logFilename("log");

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg)
{
    QString txt;
    switch (type) {
    case QtDebugMsg:
        txt = QString("Debug: %1").arg(msg);
        break;
    case QtWarningMsg:
        txt = QString("Warning: %1").arg(msg);
    break;
    case QtCriticalMsg:
        txt = QString("Critical: %1").arg(msg);
    break;
    case QtFatalMsg:
        txt = QString("Fatal: %1").arg(msg);
    break;
    case QtInfoMsg:
        txt = msg;
    break;
    }
    STREAM << txt << Qt::endl;
}

void redirectMessagesTo(const QString &outfile, bool resetFile)
{
    logFilename = outfile;

    qInstallMessageHandler(myMessageHandler);

    if( resetFile )
    {
        DEBUG_FILE.setFileName(logFilename);

        /// Reset the debug file
        DEBUG_FILE.open(QIODevice::WriteOnly);
        STREAM.setDevice(&DEBUG_FILE);
        STREAM << "";
        DEBUG_FILE.close();

        /// open the file for subsequent writing
        DEBUG_FILE.open(QIODevice::WriteOnly | QIODevice::Append);
        STREAM.setDevice(&DEBUG_FILE);

        #if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
            STREAM.setEncoding( QStringConverter::Utf8 );
        #else
            STREAM.setCodec("UTF-8");
        #endif
        STREAM << "";
    }
}

void redirectMessagesToDefault()
{
    qInstallMessageHandler(nullptr);
}
