#include "tag.h"

#include <QXmlStreamWriter>
#include <QHash>
#include <QDomElement>

Tag::Tag(const QString &label)
    : mLabel(label)
{

}

QString Tag::label() const
{
    return mLabel;
}

bool Tag::operator==(const Tag &other) const
{
    return mLabel == other.label();
}

void Tag::serialize(QXmlStreamWriter &out) const
{
    out.writeTextElement("tag", mLabel);
}

void Tag::serialize(QDomElement &out) const
{
    QDomText textNode = out.ownerDocument().createTextNode( mLabel );
    out.appendChild(textNode);
}

QString Tag::summary() const
{
    return mLabel;
}

uint qHash(const Tag &key)
{
    return qHash( key.label() );
}
