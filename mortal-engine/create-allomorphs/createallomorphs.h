#ifndef CREATEALLOMORPHS_H
#define CREATEALLOMORPHS_H

class Allomorph;
class QXmlStreamReader;
class MorphologyXmlReader;

#include "createallomorphscase.h"

#include <QSet>

class CreateAllomorphs
{
public:
    enum OtherwiseMode { Default, None, Override };

    CreateAllomorphs();

    QSet<Allomorph> generateAllomorphs(const Allomorph &original) const;

    void setOtherwiseMode(OtherwiseMode mode);

    void addCase( CreateAllomorphsCase c );

    bool hasCases() const;

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

    QString id() const;
    void setId(const QString &id);

    void setOtherwiseOverride(const CreateAllomorphsCase &otherwiseOverride);

    QString overrideModeAsString() const;

private:
    Allomorph createOtherwiseAllomorph(const Allomorph &original, QSet<Allomorph> allomorphs ) const;

    QList<CreateAllomorphsCase> mCases;
    CreateAllomorphsCase mOtherwiseOverride;
    OtherwiseMode mOtherwiseMode;
    QString mId;
};

#endif // CREATEALLOMORPHS_H
