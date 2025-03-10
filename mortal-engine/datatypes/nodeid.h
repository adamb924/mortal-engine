#ifndef NODEID_H
#define NODEID_H

#include "mortal-engine_global.h"

#include <QString>

namespace ME {

class MORTAL_ENGINE_EXPORT NodeId
{
public:
    NodeId();
    explicit NodeId(const QString & id);

    bool operator==(const NodeId & other) const;
    bool operator!=(const NodeId & other) const;
    NodeId operator +(const NodeId & suffix) const;

    QString toString() const;
    QString summary() const;
    bool isNull() const;

private:
    QString mId;
};

Q_DECL_EXPORT uint qHash(const ME::NodeId &key);

}

#endif // NODEID_H
