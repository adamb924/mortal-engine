#ifndef DEBUG_H
#define DEBUG_H

#include "mortal-engine_global.h"

class QString;

#include <QTextStream>

namespace ME {

class MORTAL_ENGINE_EXPORT Debug {
public:
    enum Control { endl };

    explicit Debug(QString * string);
    Debug(const Debug & other);
    inline ~Debug() {}

    Debug operator <<(const QString & output);
    Debug operator <<(const int & output);
    Debug operator <<(const long long & output);
    Debug operator <<(Control c);

    void indent();
    void unindent();

    static int indentLevel;
    static bool atBeginning;

private:
    QString * mString;
    QTextStream mStream;
};

} // namespace ME

#define newline Debug::endl

#endif // DEBUG_H
