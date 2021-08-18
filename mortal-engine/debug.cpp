#include "debug.h"

#include <QtDebug>
#include <QTextStream>
#include <QIODevice>

int Debug::indentLevel = 0;
bool Debug::atBeginning = true;

Debug::Debug(QString *string) : mString(string), mStream(string, QIODevice::Append)
{
}

Debug::Debug(const Debug &other) : mString(other.mString), mStream(other.mString, QIODevice::Append)
{
}

Debug Debug::operator <<(const QString &output)
{
    QTextStream stream(mString);
    if(atBeginning)
    {
        stream << QString("\t").repeated(indentLevel);
        atBeginning = false;
    }
    stream << output;
    return *this;
}

Debug Debug::operator <<(const int &output)
{
    QTextStream stream(mString);
    if(atBeginning)
    {
        stream << QString("\t").repeated(indentLevel);
        atBeginning = false;
    }
    stream << output;
    return *this;
}

Debug Debug::operator <<(const long long &output)
{
    QTextStream stream(mString);
    if(atBeginning)
    {
        stream << QString("\t").repeated(indentLevel);
        atBeginning = false;
    }
    stream << output;
    return *this;
}

Debug Debug::operator <<(Debug::Control c)
{
    if( c == Debug::endl )
    {
        QTextStream stream(mString);
        stream << Qt::endl;
        atBeginning = true;
    }
    return *this;
}

void Debug::indent()
{
    indentLevel++;
}

void Debug::unindent()
{
    indentLevel--;
}
