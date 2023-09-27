#ifndef PORTMANTEAU_H
#define PORTMANTEAU_H

#include "datatypes/morphemesequence.h"
#include "mortal-engine_global.h"

class MorphemeNode;
class AbstractNode;
class Morphology;
class WritingSystem;

#include <QStringList>

class MORTAL_ENGINE_EXPORT Portmanteau
{
public:
    enum Status { Valid, Invalid };

    Portmanteau();
    Portmanteau(const Portmanteau &other);
    Portmanteau &operator=(const Portmanteau & other);

    Portmanteau(const QString & initializationString, const MorphemeNode * parent );
    bool operator==(const Portmanteau & other) const;

    /// this needs to be called after all the nodes are read, so they can be searched
    void initialize();

    Portmanteau::Status status() const;
    bool isValid() const;

    const AbstractNode *next() const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;

    QString morphemeGlosses(const WritingSystem & summaryDisplayWritingSystem, const QString & delimiter) const;

    const AbstractNode *lastNode() const;

    int count() const;

    MorphemeSequence morphemes() const;

private:
    Portmanteau::Status mStatus;
    QString mInitializationString;
    const MorphemeNode * mParent;
    MorphemeSequence mMorphemes;
    QList<const MorphemeNode*> mMorphemeNodes;
};

#endif // PORTMANTEAU_H
