#include "tag.h"

#include <QXmlStreamWriter>
#include <QHash>
#include <QDomElement>
#include <QSet>

#include "hashseed.h"

using namespace ME;

Tag::Tag(const QString &label)
    : mLabel(label), mHash( qHash(mLabel, HASH_SEED ) )
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

QSet<Tag> Tag::fromString(const QString &str, const QString & delimiter)
{
    QSet<Tag> returnValue;
    QStringList list = str.split(delimiter);
    for(int i=0; i < list.count(); i++)
        returnValue << Tag( list.at(i) );
    return returnValue;
}

QString Tag::summary() const
{
    return mLabel;
}

uint ME::qHash(const Tag &key)
{
    return key.hash();
}
