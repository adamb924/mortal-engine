#ifndef CREATEALLOMORPHSREPLACEMENT_H
#define CREATEALLOMORPHSREPLACEMENT_H

#include <QHash>
#include <QRegularExpression>
#include "datatypes/writingsystem.h"
#include "datatypes/tag.h"

#include <QSet>

class Form;

class CreateAllomorphsReplacement
{
public:
    CreateAllomorphsReplacement( const QList<Form> & replaceThis, const QList<Form> & withThis );

    void setReplaceThis(const QList<Form> & replaceThis);
    void setWithThis(const QList<Form> & withThis);

    Form perform( const Form & subject, bool &changed ) const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;

private:
    QHash<WritingSystem,QRegularExpression> mReplaceThis;
    QHash<WritingSystem,QString> mWithThis;
    WritingSystem mWritingSystem;
    QSet<Tag> mAddTags;
    QSet<Tag> mRemoveTags;
};

#endif // CREATEALLOMORPHSREPLACEMENT_H
