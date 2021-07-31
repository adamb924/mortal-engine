#ifndef MESSAGES_H
#define MESSAGES_H

#include "mortal-engine_global.h"

class QMessageLogContext;
class QString;

/// I'm just putting this in a special namespace because it's a global
namespace MortalEngineDebug {
    extern QString logFilename;
}

void myMessageHandler(QtMsgType type, const QMessageLogContext &, const QString & msg);

MORTAL_ENGINE_EXPORT void redirectMessagesTo(const QString & outfile, bool resetFile = true);
MORTAL_ENGINE_EXPORT void redirectMessagesToDefault();


#endif // MESSAGES_H
