#ifndef TAG_H
#define TAG_H

#include <QString>

class QXmlStreamWriter;
class QDomElement;

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT Tag
{
public:
    /**
     * @brief Construct a new Tag object with the specified \a label
     * 
     * @param label The label of the tag
     */
    explicit Tag(const QString & label);

    QString label() const;

    /**
     * @brief Returns true if the \a other Tag has the same label as this Tag; otherwise returns false.
     * 
     */
    bool operator==(const Tag & other) const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;

    /**
     * @brief Writes an XML representation of the object to the specified writer.
     *
     * @param out The XML stream to be written to
     */
    void serialize(QXmlStreamWriter & out) const;
    void serialize(QDomElement & out) const;

    uint hash() const;

    static QSet<Tag> fromString(const QString & str, const QString & delimiter = ",");

private:
    QString mLabel;
    uint mHash;
};

Q_DECL_EXPORT uint qHash(const Tag & key);

#endif // TAG_H
