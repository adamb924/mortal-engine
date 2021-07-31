#ifndef PORTMANTEAU_H
#define PORTMANTEAU_H

#include "mortal-engine_global.h"

class MorphemeNode;
class AbstractNode;
class Morphology;

#include <QStringList>

class MORTAL_ENGINE_EXPORT Portmanteau
{
public:
    enum Status { Valid, Invalid };

    Portmanteau();
    Portmanteau(const QString & initializationString, const MorphemeNode * parent );
    bool operator==(const Portmanteau & other) const;

    /// this needs to be called after all the nodes are read, so they can be searched
    void initialize(const Morphology * morphology);

    Portmanteau::Status status() const;
    bool isValid() const;

    const AbstractNode *next() const;

    QString label() const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;

    const AbstractNode *lastNode() const;

    int count() const;

    QStringList morphemes() const;

private:
    Portmanteau::Status mStatus;
    QString mInitializationString;
    const MorphemeNode * mParent;
    const AbstractNode * mLastNode;
    QStringList mMorphemes;
};

#endif // PORTMANTEAU_H
