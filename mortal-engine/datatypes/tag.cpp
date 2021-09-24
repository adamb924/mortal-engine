#include "tag.h"

#include <QXmlStreamWriter>
#include <QHash>
#include <QDomElement>

Tag::Tag(const QString &label)
    : mLabel(label), mHash( qHash(mLabel, static_cast<uint>(qGlobalQHashSeed())) )
{

}

QString Tag::label() const
{
    return mLabel;
}

bool Tag::operator==(const Tag &other) const
{
    return mHash == other.hash();
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

uint Tag::hash() const
{
    return mHash;
}

QString Tag::summary() const
{
    return mLabel;
}

uint qHash(const Tag &key)
{
    return key.hash();
}
