#ifndef PORTMANTEAU_H
#define PORTMANTEAU_H

#include "datatypes/morphemesequence.h"
#include "mortal-engine_global.h"

#include <QStringList>

namespace ME {

class MorphemeNode;
class AbstractNode;
class Morphology;
class WritingSystem;

class MORTAL_ENGINE_EXPORT Portmanteau
{
public:
    enum Status { Valid, Invalid };

    Portmanteau();
    Portmanteau(const Portmanteau &other);
    Portmanteau &operator=(const Portmanteau & other);

    Portmanteau(const QString & initializationString);
    bool operator==(const Portmanteau & other) const;

    /// this needs to be called after all the nodes are read, so they can be searched
    bool initialize(const AbstractNode * parent, QHash<MorphemeLabel, const AbstractNode *> &cache);

    Portmanteau::Status status() const;
    bool isValid() const;

    const AbstractNode *next() const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;

    const AbstractNode *lastNode() const;

    int count() const;

    MorphemeSequence morphemes() const;

    QList<const AbstractNode *> nodes() const;

private:
    const AbstractNode * getFollowingNode(const AbstractNode * startingFrom, const MorphemeLabel & label, QHash<MorphemeLabel, const AbstractNode *> &cache) const;

    Portmanteau::Status mStatus;
    QString mInitializationString;
    MorphemeSequence mMorphemes;
    QList<const AbstractNode*> mNodes;
};

} // namespace ME

#endif // PORTMANTEAU_H
