#ifndef MESSAGES_H
#define MESSAGES_H

#include "mortal-engine_global.h"

#include <QTextStream>
#include <QFile>
class QMessageLogContext;
class QString;

/// I'm just putting this in a special namespace because it's a global
namespace MortalEngineDebug {
    MORTAL_ENGINE_EXPORT extern QString logFilename;
    extern QFile DEBUG_FILE;
    extern QTextStream STREAM;
}

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

MORTAL_ENGINE_EXPORT void redirectMessagesTo(const QString & outfile, bool resetFile = true);
MORTAL_ENGINE_EXPORT void redirectMessagesToDefault();


#endif // MESSAGES_H
