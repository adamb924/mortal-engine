#include "writingsystem.h"

#include <QFont>

WritingSystem::WritingSystem() : mName(""), mAbbreviation(""), mFontFamily(""), mFontSize(0), mLayoutDirection(Qt::LeftToRight)
{
}

WritingSystem::WritingSystem(const QString & name, const QString & abbreviation, Qt::LayoutDirection layoutDirection, QString fontFamily, int fontSize)
     : mName(name), mAbbreviation(abbreviation), mFontFamily(fontFamily), mFontSize(fontSize), mLayoutDirection(layoutDirection)
{
}

WritingSystem::WritingSystem(const QString &name, const QString &abbreviation, Qt::LayoutDirection layoutDirection, QString fontFamily, int fontSize, const QString &keyboardCommand)
    : mName(name), mAbbreviation(abbreviation), mFontFamily(fontFamily), mFontSize(fontSize), mLayoutDirection(layoutDirection), mKeyboardCommand(keyboardCommand)
{

}

WritingSystem::WritingSystem(const QString &abbreviation) : mName(""), mAbbreviation(abbreviation), mFontFamily(""), mFontSize(0), mLayoutDirection(Qt::LeftToRight)
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

QFont WritingSystem::font() const
{
    return QFont( mFontFamily, mFontSize );
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

bool WritingSystem::operator==(const WritingSystem & other) const
{
    return mAbbreviation == other.mAbbreviation;
}

bool WritingSystem::operator!=(const WritingSystem & other) const
{
    return mAbbreviation != other.mAbbreviation;
}

bool WritingSystem::operator==(const QString & flexString) const
{
    return mAbbreviation == flexString;
}

WritingSystem& WritingSystem::operator=(const WritingSystem & other)
{
    mName = other.mName;
    mAbbreviation = other.mAbbreviation;
    mLayoutDirection = other.mLayoutDirection;
    mFontFamily = other.mFontFamily;
    mFontSize = other.mFontSize;
    mKeyboardCommand = other.mKeyboardCommand;
    return *this;
}

uint qHash(const WritingSystem & key)
{
    return qHash(key.abbreviation());
}
