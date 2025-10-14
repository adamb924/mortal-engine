/**
 * @file form.h
 * @author Adam Baker (adam@adambaker.org)
 * @brief A class that provides a language-sensitive data type, a pairing of a QString and a WritingSystem. An optional ID (qlonglong) can be specified as well, but this is not required, and the object comparison functions ignore the id() value.
 * @version 0.1
 * @date 2020-10-26
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef FORM_H
#define FORM_H

#include "mortal-engine_global.h"

#include "writingsystem.h"

class QXmlStreamWriter;
class QDomElement;
class QDomDocument;
class QXmlStreamReader;


namespace ME {

class Parsing;
class MorphologyXmlReader;
class Morphology;

class MORTAL_ENGINE_EXPORT Form
{
public:
    /**
     * @brief Construct a new Form object with an empty WritingSystem and text
     * 
     */
    Form();

    /**
     * @brief Construct a new Form object with the given WritingSystem and text
     * 
     * @param ws The writing system
     * @param text The text of the form
     */
    Form(const WritingSystem & ws, const QString & text);

    /**
     * @brief Construct a new Form object with the given WritingSystem, text, and id
     * 
     * @param ws The writing system
     * @param text The text of the form
     * @param id The id of the form
     */
    Form(const WritingSystem & ws, const QString & text, qlonglong id);

    /**
     * @brief Equality operator
     * 
     * @param other The other Allomorph
     * @return true The text and WritingSystem of \a other are the same.
     * @return false Either the text, WritingSystem, or both of \a other are different.
     */
    bool operator==(const Form & other) const;

    /**
     * @brief Inequality operator
     * 
     * @param other The other Allomorph
     * @return true Either the text, WritingSystem, or both of \a other are different.
     * @return false The text and WritingSystem of \a other are the same.
     */
    bool operator!=(const Form & other) const;

    /**
     * @brief If the WritingSystem of \a other matches, the text of \a other is appended to the text of this object. Otherwise no change is made.
     * 
     * @param other The Form to be appended.
     * @return Form A copy of the object
     */
    Form operator+=(const Form & other);

    /**
     * @brief The text of \a other is appended to the text of this object.
     *
     * @param other The text to be appended.
     * @return Form A copy of the object
     */
    Form operator+=(const QString & other);

    /**
     * @brief Returns the writing system of the Form
     * 
     * @return WritingSystem 
     */
    WritingSystem writingSystem() const;

    /**
     * @brief Returns the text of the Form
     * 
     * @return QString 
     */
    QString text() const;

    /**
     * @brief Set the text of the Form. (Note that QString is WritingSystem-agnostic.)
     * 
     * @param text 
     */
    void setText(const QString &text);

    /**
     * @brief A convenience passthrough to access QString::mid of the mText member.
     * 
     * @param position The starting position (0-indexed; see QString::mid())
     * @param n The number of characters to return
     * @return Form A form containing the substring.
     */
    Form mid(int position, int n = -1) const;

    /**
     * @brief A convenience passthrough to access QString::length of the mText member.
     * 
     * @return int The length of the Form's mText member.
     */
    int  length() const;

    /**
     * @brief Return the ID of the Form (mId)
     * 
     * @return qlonglong The Form's ID
     */
    qlonglong id() const;

    /**
     * @brief Set the ID of the Form (mId)
     * 
     * @param id The new ID
     */
    void setId(const qlonglong &id);

    /**
     * @brief Return true if the Form's text has nothing but non-word, whitespace, and digit characters; otherwise returns false. The regular expression is The Form's text matches the regular expression ^[\\W\\s\\d]+$. The QRegularExpression::UseUnicodePropertiesOption is used.
     * 
     * @return true The Form's text matches the regular expression ^[\\W\\s\\d]+$
     * @return false The Form's text does not match the regular expression ^[\\W\\s\\d]+$
     */
    bool isWhitespaceAndNonWordCharacters() const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const QString & label = "Form") const;

    /**
     * @brief Writes an XML element to \a out representing the Form.
                e.g., <form lang="writing-system-abbreviation">the text</form>
            Or if \a elementName is, e.g., set to "myElement":
                <myElement lang="writing-system-abbreviation">the text</myElement>
            The mId attribute is serialized if it is not equal to -1.
                e.g., <form lang="writing-system-abbreviation" id="3478">the text</form>
     * 
     * @param out The output stream
     * @param elementName The name of the target element.
     */
    void serialize(QXmlStreamWriter & out, const QString & elementName = QString("form") ) const;
    void serialize(QDomElement & out) const;

    /**
     * @brief Generates a QDomElement representing the Form. The element is the same as in Form::serialize
     * 
     * @param document The document for which the element is generated.
     * @param elementName The name of the target element.
     * @return QDomElement The newly created element
     */
    QDomElement toElement(QDomDocument *document, const QString & elementName = QString("form"));

    /**
     * @brief Reads a Form element from \a in, and constructs a new Form element. \a in is assumed to be positioned
            on the starting element (with name \a elementName). After the function is called, \a in will be positioned
            on the ending element  (with name \a elementName).
     * 
     * @param in The XML stream reader
     * @param morphology Pointer to Morphology, which provides needed accessory functions.
     * @param elementName The name of the Form element; default is "form".
     * @return Form A newly constructed Form object.
     */
    static Form readFromXml(QXmlStreamReader &in, const Morphology *morphology, const QString & elementName = "form");

    static Form readFromXml(QDomElement element, const Morphology *morphology);

    /**
     * @brief Returns a list of Form elements read from \a in. \a in is assumed to be position on a starting element with name
                \a untilElement. All elements named \a elementName are read as Form objects. After the function is called, \a in
                will be position on an end element named \a untilElement.

                For instance,
                \code{cpp}
                Form::readFromXml(in, morphology, "myElement", "container")
                \endcode
                \code{cpp}
                    <container>
                        <myElement lang="...">...</form>
                        <myElement lang="...">...</form>
                        <myElement lang="...">...</form>
                    </container>
                \endcode
     * 
     * @param in The XML stream reader
     * @param morphology Pointer to Morphology, which provides needed accessory functions.
     * @param untilElement The name of the container element.
     * @param elementName The name of the Form element; default is "form".
     * @return QList<Form> A list of newly constructed Form objects.
     */
    static QList<Form> readListFromXml(QXmlStreamReader &in, const Morphology *morphology, const QString & untilElement, const QString & elementName = "form");

    uint hash() const;

private:
    void calculateHash();

private:
    WritingSystem mWritingSystem;
    QString mText;
    qlonglong mId;
    uint mHash;
};

Q_DECL_EXPORT uint qHash(const ME::Form &key);

Q_DECL_EXPORT uint qHash(const std::pair<const ME::WritingSystem&, const ME::Form&>& pair);

} // namespace ME

#endif // FORM_H
