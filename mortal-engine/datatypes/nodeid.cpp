#include "nodeid.h"

#include <QHash>
#include "hashseed.h"

using namespace ME;

NodeId::NodeId() {}

NodeId::NodeId(const QString &id) : mId(id) {}

bool NodeId::operator==(const NodeId &other) const
{
    return mId == other.mId;
}

bool NodeId::operator!=(const NodeId &other) const
{
    return mId != other.mId;
}

NodeId NodeId::operator+(const NodeId &suffix) const
{
    return NodeId( mId + suffix.toString() );
}

QString NodeId::toString() const
{
    return mId;
}

QString NodeId::summary() const
{
    return QString("NodeId(%1)").arg(mId);
}

bool NodeId::isNull() const
{
    return mId.isEmpty();
}

uint ME::qHash(const NodeId &key)
{
    return qHash(key.toString(), HASH_SEED);
}
