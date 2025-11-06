#ifndef CREATEALLOMORPHS_H
#define CREATEALLOMORPHS_H

class QXmlStreamReader;

#include "createallomorphscase.h"

#include <QSet>

namespace ME {

class Allomorph;
class MorphologyXmlReader;

class CreateAllomorphs
{
public:
    enum OtherwiseMode { Default, None, Override };

    CreateAllomorphs();

    QSet<Allomorph> generateAllomorphs(const Allomorph &original) const;

    static QString elementName();
    static CreateAllomorphs readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);
    static QHash<QString, CreateAllomorphs> readListFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const QString & untilElement);

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;

private:
    void setOtherwiseMode(OtherwiseMode mode);
    void addCase( CreateAllomorphsCase c );
    void setOtherwiseOverride(const CreateAllomorphsCase &otherwiseOverride);
    QString overrideModeAsString() const;
    Allomorph createOtherwiseAllomorph(const Allomorph &original, QSet<Allomorph> allomorphs ) const;

    QString id() const;
    void setId(const QString &id);

    static QString XML_NO_OTHERWISE;
    static QString XML_OVERRIDE_OTHERWISE;

    QList<CreateAllomorphsCase> mCases;
    CreateAllomorphsCase mOtherwiseOverride;
    OtherwiseMode mOtherwiseMode;
    QString mId;
};

} // namespace ME

#endif // CREATEALLOMORPHS_H
