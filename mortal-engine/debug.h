#ifndef DEBUG_H
#define DEBUG_H

class QString;

#include <QTextStream>

class Debug {
public:
    enum Control { endl };

    explicit Debug(QString * string);
    Debug(const Debug & other);
    inline ~Debug() {}

    Debug operator <<(const QString & output);
    Debug operator <<(const int & output);
    Debug operator <<(Control c);

    void indent();
    void unindent();

    static int indentLevel;
    static bool atBeginning;

private:
    QString * mString;
    QTextStream mStream;
};

#define newline Debug::endl

#endif // DEBUG_H
