/**
 * @file writingsystem.h
 * @author Adam Baker (adam@adambaker.org)
 * @brief A data class holding data about a writing system.
 * Note that the equality operator for this class is defined so that two WritingSystem objects are identical if they have the same mAbbreviation value. Similarly, a WritingSystem object can be compared directly to a QString.
 * @version 0.1
 * @date 2020-10-26
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef WRITINGSYSTEM_H
#define WRITINGSYSTEM_H

#include <Qt>
#include <QString>
#include <QHash>
#include <QtDebug>

class QXmlStreamReader;

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT WritingSystem
{
public:
    /**
     * @brief Construct a new Writing System object, which is null.
     * 
     */
    WritingSystem();

    /**
     * @brief Construct a new Writing System object with the given values
     */
    WritingSystem(const QString & name, const QString & abbreviation, Qt::LayoutDirection layoutDirection, QString fontFamily, int fontSize);

    /**
     * @brief Construct a new Writing System object with the given values
     */
    WritingSystem(const QString & name, const QString & abbreviation, Qt::LayoutDirection layoutDirection, QString fontFamily, int fontSize, const QString & keyboardCommand);

    /**
     * @brief Construct a new Writing System object with the given \a abbreviation. Constructing WritingSystem objects this way is not ideal, but sometimes it is necessary/helpful.
     */
    explicit WritingSystem(const QString & abbreviation);

    /**
     * @brief Returns the name of the WritingSystem.
     * 
     * @return QString the name of the WritingSystem
     */
    QString name() const;

    /**
     * @brief Returns the abbreviation of the WritingSystem.
     * 
     * @return QString the abbreviation of the WritingSystem
     */
    QString abbreviation() const;

    /**
     * @brief Returns the layout direction of the WritingSystem.
     * 
     * @return Qt::LayoutDirection the layout direction of the WritingSystem
     */
    Qt::LayoutDirection layoutDirection() const;

    /**
     * @brief Returns true if the abbreviations of the two WritingSystems are identical, otherwise returns false.
     */
    bool operator==(const WritingSystem & other) const;

    /**
     * @brief Returns true if the abbreviations of the two WritingSystems are different, otherwise returns false.
     */
    bool operator!=(const WritingSystem & other) const;

    /**
     * @brief Returns true if the abbreviations of the WritingSystem is identical to \a flexString, otherwise returns false.
     */
    bool operator==(const QString & flexString) const;

    /**
     * @brief Performs a deep copy of \a other and returns a copy of this object.
     */
    WritingSystem& operator=(const WritingSystem & other);

    /**
     * @brief Returns the preferred font family for displaying text with the WritingSystem.
     * 
     * @return QString The font family.
     */
    QString fontFamily() const;

    /**
     * @brief Returns the preferred font size for displaying text with the WritingSystem.
     * 
     * @return int Font size in points (pt).
     */
    int fontSize() const;

    /**
     * @brief Returns true if the mAbbreviation element is the empty string, otherwise returns false.
     * 
     * @return true The mAbbreviation element is the empty string.
     * @return false The mAbbreviation element is not the empty string.
     */
    bool isNull() const;

    /**
     * @brief Returns the keyboard command associated with this WritingSystem. This is a command can be used to switch the user's input method to one appropriate to the WritingSystem.
     * 
     * @return QString The keyboard command for the WritingSystem, or an empty string.
     */
    QString keyboardCommand() const;

    /**
     * @brief Set the keyboard command associated with this WritingSystem. This command should be able to be called to switch the user's input method to one appropriate to the WritingSystem.
     * 
     * @return QString The keyboard command for the WritingSystem, or an empty string.
     */
    void setKeyboardCommand(const QString &keyboardCommand);

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;

    static QHash<QString,WritingSystem> readWritingSystems(QXmlStreamReader &in);
    static QHash<QString,WritingSystem> readWritingSystems(const QString & path);
    /// convenience method
    static bool notAtEndOf(const QString & tagName, QXmlStreamReader &in);

    /// XML Strings
    static QString XML_WRITING_SYSTEMS;
    static QString XML_WRITING_SYSTEM;

    uint hash() const;

private:
    QString mName, mAbbreviation, mFontFamily;
    int mFontSize;
    Qt::LayoutDirection mLayoutDirection;
    QString mKeyboardCommand;
    uint mHash;
};

Q_DECL_EXPORT uint qHash(const WritingSystem & key);

#endif // WRITINGSYSTEM_H
