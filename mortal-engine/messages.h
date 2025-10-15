#ifndef MESSAGES_H
#define MESSAGES_H

#include "mortal-engine_global.h"

#include <QTextStream>
#include <QFile>
#include <QXmlStreamWriter>
class QMessageLogContext;
class QString;

namespace ME {

class MORTAL_ENGINE_EXPORT Messages {
public:

    static void redirectMessagesTo(const QString & outfile, bool resetFile = true);
    static void redirectMessagesToDefault();
    static QXmlStreamWriter &stream();

    static QString logFilename;

private:
    Messages();
    ~Messages();

    static Messages& instance();

    static void handler(QtMsgType type, const QMessageLogContext &, const QString & msg);

    void openLogFile();
    void resetLogFile();
    void closeLogFile();

    QFile logFile;
    QXmlStreamWriter xml;
};

}

#endif // MESSAGES_H
