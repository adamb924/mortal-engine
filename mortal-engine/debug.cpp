#include "debug.h"

#include <QtDebug>
#include <QTextStream>

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
//        qDebug() << output;
//        stream << "T" << QString("\t").repeated(indentLevel);
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
//        qDebug() << output;
//        stream << "T" << QString("\t").repeated(indentLevel);
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
//        qDebug() << "NEWLINE";
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
