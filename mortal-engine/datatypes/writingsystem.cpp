#include "writingsystem.h"

#include <QXmlStreamReader>
#include <QFile>
#include <stdexcept>

QString WritingSystem::XML_WRITING_SYSTEMS = "writing-systems";
QString WritingSystem::XML_WRITING_SYSTEM = "writing-system";


WritingSystem::WritingSystem() : mName(""), mAbbreviation(""), mFontFamily(""), mFontSize(0), mLayoutDirection(Qt::LeftToRight), mHash( qHash(mAbbreviation) )
{
}

WritingSystem::WritingSystem(const QString & name, const QString & abbreviation, Qt::LayoutDirection layoutDirection, QString fontFamily, int fontSize)
     : mName(name), mAbbreviation(abbreviation), mFontFamily(fontFamily), mFontSize(fontSize), mLayoutDirection(layoutDirection), mHash( qHash(mAbbreviation) )
{
}

WritingSystem::WritingSystem(const QString &name, const QString &abbreviation, Qt::LayoutDirection layoutDirection, QString fontFamily, int fontSize, const QString &keyboardCommand)
    : mName(name), mAbbreviation(abbreviation), mFontFamily(fontFamily), mFontSize(fontSize), mLayoutDirection(layoutDirection), mKeyboardCommand(keyboardCommand), mHash( qHash(mAbbreviation) )
{

}

WritingSystem::WritingSystem(const QString &abbreviation) : mName(""), mAbbreviation(abbreviation), mFontFamily(""), mFontSize(0), mLayoutDirection(Qt::LeftToRight), mHash( qHash(mAbbreviation) )
{

}

QString WritingSystem::name() const
{
    return mName;
}

QString WritingSystem::abbreviation() const
{
    return mAbbreviation;
}

Qt::LayoutDirection WritingSystem::layoutDirection() const
{
    return mLayoutDirection;
}

QString WritingSystem::fontFamily() const
{
    return mFontFamily;
}

int WritingSystem::fontSize() const
{
    return mFontSize;
}

bool WritingSystem::isNull() const
{
    return mAbbreviation.isEmpty();
}

QString WritingSystem::keyboardCommand() const
{
    return mKeyboardCommand;
}

void WritingSystem::setKeyboardCommand(const QString &keyboardCommand)
{
    mKeyboardCommand = keyboardCommand;
}

QString WritingSystem::summary() const
{
    QString dbgString;
    QTextStream dbg(&dbgString);
    
    dbg << "WritingSystem(" << name() << ", " << abbreviation() << ", " << fontFamily() << ", " << fontSize() << ", " << keyboardCommand() << ")";

    return dbgString;
}

QHash<QString, WritingSystem> WritingSystem::readWritingSystems(QXmlStreamReader &in)
{
    Q_ASSERT( in.isStartElement() );
    Q_ASSERT( in.name() == XML_WRITING_SYSTEMS );

    /// follow the src attribute to a filename if there is one
    if( in.attributes().hasAttribute("src") )
    {
        QHash<QString, WritingSystem> wss = readWritingSystems( in.attributes().value("src").toString() );
        while( notAtEndOf(XML_WRITING_SYSTEMS, in) )
            in.readNext();
        return wss;
    }

    QHash<QString,WritingSystem> languages;
    while( notAtEndOf(XML_WRITING_SYSTEMS, in) )
    {
        in.readNextStartElement();
        if ( in.name() == XML_WRITING_SYSTEM )
        {
            QXmlStreamAttributes attr = in.attributes();
            if( attr.hasAttribute("name")
                    && attr.hasAttribute("lang")
                    && attr.hasAttribute("font")
                    && attr.hasAttribute("font-size")
                    && attr.hasAttribute("right-to-left") )
            {
                WritingSystem ws( attr.value("name").toString(),
                               attr.value("lang").toString(),
                               attr.value("right-to-left").toString() == "true" ? Qt::RightToLeft : Qt::LeftToRight,
                               attr.value("font").toString(),
                               attr.value("font-size").toInt() );
                if( attr.hasAttribute("keyboard-command") ) {
                    ws.setKeyboardCommand( attr.value("keyboard-command").toString() );
                }
                languages.insert(ws.abbreviation(), ws);
            }
        }
    }

    return languages;
}

QHash<QString, WritingSystem> WritingSystem::readWritingSystems(const QString &path)
{
    QFile file(path);
    if( file.open( QFile::ReadOnly ) )
    {
        QXmlStreamReader in(&file);
        in.readNextStartElement();
        return readWritingSystems(in);
    }
    else
    {
        std::string message = "Can't open file: ";
        message += path.toUtf8().constData();
        throw std::runtime_error( message );
    }
}

bool WritingSystem::notAtEndOf(const QString &tagName, QXmlStreamReader &in)
{
    return !in.atEnd() && !(in.isEndElement() && in.name() == tagName);
}

uint WritingSystem::hash() const
{
    return mHash;
}

bool WritingSystem::operator==(const WritingSystem & other) const
{
    return mHash == other.mHash;
}

bool WritingSystem::operator!=(const WritingSystem & other) const
{
    return mHash != other.mHash;
}

bool WritingSystem::operator==(const QString & flexString) const
{
    return mAbbreviation == flexString;
}

WritingSystem& WritingSystem::operator=(const WritingSystem & other)
{
    mName = other.mName;
    mAbbreviation = other.mAbbreviation;
    mHash = other.mHash;
    mLayoutDirection = other.mLayoutDirection;
    mFontFamily = other.mFontFamily;
    mFontSize = other.mFontSize;
    mKeyboardCommand = other.mKeyboardCommand;
    return *this;
}

uint qHash(const WritingSystem & key)
{
    return key.hash();
}
