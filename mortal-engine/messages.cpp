#include "messages.h"

#include <QtGlobal>
#include <QCoreApplication>
#include <QtDebug>
#include <QTextStream>
#include <QFile>

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
    QFile outFile(MortalEngineDebug::logFilename);
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    ts.setEncoding( QStringConverter::Utf8 );
#else
    ts.setCodec("UTF-8");
#endif

    ts << txt << Qt::endl;
}

void redirectMessagesTo(const QString &outfile, bool resetFile)
{
    MortalEngineDebug::logFilename = outfile;

    qInstallMessageHandler(myMessageHandler);

    if( resetFile )
    {
        /// Reset the debug file
        QFile debugFile(MortalEngineDebug::logFilename);
        debugFile.open(QIODevice::WriteOnly);
        QTextStream ts(&debugFile);

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        ts.setEncoding( QStringConverter::Utf8 );
#else
        ts.setCodec("UTF-8");
#endif

        ts << "";
        debugFile.close();
    }
}

void redirectMessagesToDefault()
{
    qInstallMessageHandler(nullptr);
}
