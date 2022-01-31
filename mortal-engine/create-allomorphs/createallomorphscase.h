#ifndef CREATEALLOMORPHSCASE_H
#define CREATEALLOMORPHSCASE_H

class Allomorph;
class AbstractConstraint;

#include <QSet>
#include <QList>

#include "createallomorphsreplacement.h"
#include "datatypes/tag.h"

class QXmlStreamReader;
class MorphologyXmlReader;
class MorphemeNode;

class CreateAllomorphsCase
{
public:
    enum FormsMode { IncludeAllForms, IncludeOnlyChangedForms };

    CreateAllomorphsCase();

    Allomorph createAllomorph(const Allomorph &input, const QSet<Allomorph> * allomorphs) const;

    static QString elementName();
    static CreateAllomorphsCase readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);

    void addMatchTag(const Tag & t);
    void addNotMatchTag(const Tag & t);
    void addConstraints(const QSet<const AbstractConstraint *> c);
    void addReplacement(const CreateAllomorphsReplacement & r);

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;

    QSet<const AbstractConstraint *> constraints() const;

    void setFormsMode(const FormsMode &formsMode);

    static FormsMode formsModeFromString(const QString & mode);

    void addAddTag( const Tag & t );
    void addRemoveTag( const Tag & t );

    static QString XML_MATCH_TAG;
    static QString XML_WITHOUT_TAG;
    static QString XML_WHEN;
    static QString XML_THEN;
    static QString XML_REPLACE_THIS;
    static QString XML_WITH_THIS;
    static QString XML_ADD_TAG;
    static QString XML_REMOVE_TAG;
    static QString XML_TOLERATE_DUPLICATES;
    static QString XML_TRUE;

    void setTolerateDuplicates(bool tolerateDuplicates);

private:
    QSet<const AbstractConstraint *> mConstraints;
    QList<CreateAllomorphsReplacement> mReplacements;
    QSet<Tag> mMatchTags;
    QSet<Tag> mNotMatchTags;
    FormsMode mFormsMode;
    QSet<Tag> mAddTags;
    QSet<Tag> mRemoveTags;
    bool mTolerateDuplicates;
};

#endif // CREATEALLOMORPHSCASE_H
