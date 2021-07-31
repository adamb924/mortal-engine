#include "portmanteau.h"

#include "morphology.h"
#include "nodes/abstractnode.h"
#include "nodes/morphemenode.h"

#include <QtDebug>
#include <stdexcept>

Portmanteau::Portmanteau()
    : mStatus(Portmanteau::Invalid),
      mParent(nullptr),
      mLastNode(nullptr)
{

}

Portmanteau::Portmanteau(const QString &initializationString, const MorphemeNode *parent)
     : mStatus(Portmanteau::Valid),
       mInitializationString(initializationString),
       mParent(parent),
       mLastNode(nullptr)
{
}

bool Portmanteau::operator==(const Portmanteau &other) const
{
    return mInitializationString == other.mInitializationString && mParent == other.mParent;
}

void Portmanteau::initialize(const Morphology *morphology)
{
    mMorphemes = mInitializationString.mid(1, mInitializationString.length()-2).split("][");
    if( mMorphemes.count() == 0 )
    {
        std::string is = mInitializationString.toUtf8().constData();
        throw std::runtime_error( "Empty morpheme string, apparently: " + is );
    }

    QList<MorphemeNode *> nodes = morphology->getMorphemeNodeFromLabel( mMorphemes.last() );
    if( nodes.count() == 0 )
    {
        std::string is = mInitializationString.toUtf8().constData();
        std::string label = mMorphemes.last().toUtf8().constData();
        throw std::runtime_error( "Attempting to process the portmanteau string " + is + " but couldn't find following node with this label: " + label );
    }
    else
    {
        if( nodes.count() > 1 )
        {
            qWarning() << "Portmanteau definition with ambiguous label. Strange results may obtain, and may vary from run to run. The label is: " << mMorphemes.last();
        }
        mLastNode = nodes.first();
    }
}

Portmanteau::Status Portmanteau::status() const
{
    return mStatus;
}

bool Portmanteau::isValid() const
{
    return mStatus == Portmanteau::Valid;
}

const AbstractNode *Portmanteau::next() const
{
    return mLastNode->next();
}

QString Portmanteau::label() const
{
    return mMorphemes.join("][");
}

const AbstractNode *Portmanteau::lastNode() const
{
    return mLastNode;
}

int Portmanteau::count() const
{
    return mMorphemes.count();
}

QStringList Portmanteau::morphemes() const
{
    return mMorphemes;
}

QString Portmanteau::summary() const
{
    if( mLastNode != nullptr && mLastNode->next() != nullptr )
    {
        return QString("Portmanteau([%1], Next: %2)").arg( label(), next() == nullptr ? "null" : next()->label() );
    }
    else
    {
        return QString("Portmanteau([%1], Next: null)").arg( label() );
    }
}
