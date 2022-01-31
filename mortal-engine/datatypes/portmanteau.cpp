#include "portmanteau.h"

#include "morphology.h"
#include "nodes/abstractnode.h"
#include "nodes/morphemenode.h"

#include <QtDebug>
#include <stdexcept>

Portmanteau::Portmanteau()
    : mStatus(Portmanteau::Invalid),
      mParent(nullptr)
{

}

Portmanteau::Portmanteau(const QString &initializationString, const MorphemeNode *parent)
     : mStatus(Portmanteau::Valid),
       mInitializationString(initializationString),
       mParent(parent)
{
}

bool Portmanteau::operator==(const Portmanteau &other) const
{
    return mInitializationString == other.mInitializationString && mParent == other.mParent;
}

void Portmanteau::initialize()
{
    mMorphemeNodes.clear();
    mMorphemes = mInitializationString.mid(1, mInitializationString.length()-2).split("][");

    /// portmanteaux must have at least two morphemes
    if( mMorphemes.count() < 2 )
    {
        std::string is = mInitializationString.toUtf8().constData();
        throw std::runtime_error( "A portmanteau must have at least two morphemes: " + is );
    }

    /// the first morpheme must be the same (i.e., have the same label as) the MorphemeNode that this portmanteau belongs to
    if( mMorphemes.first() != mParent->label() )
    {
        std::string is = mInitializationString.toUtf8().constData();
        std::string parent = mParent->label().toUtf8().constData();
        throw std::runtime_error( "The first morpheme of a portmanteau should be the current morpheme: " + is + " but it's " + parent );
    }

    /// the parent MorphemeNode is the first element of the portmanteau
    mMorphemeNodes << mParent;

    /// then add every other morpheme in the portmanteau
    /// this will execute at least once because every portmanteau has to have at least two morphemes (checked above)
    int i=1;
    do
    {
        /// the current node is the (first?) node following the most recently added node node that has the label in question
        const AbstractNode * current = mMorphemeNodes.last()->followingNodeHavingLabel( mMorphemes.at(i) );
        /// fail if no node has that label
        if( current == nullptr )
        {
            std::string is = mInitializationString.toUtf8().constData();
            std::string label = mMorphemes.first().toUtf8().constData();
            throw std::runtime_error( "Attempting to process the portmanteau string " + is + " but the following node with this label could not be found: " + label );
        }
        /// fail if, for some reason the node returned is not a MorphemeNode
        if( !current->isMorphemeNode() )
        {
            std::string is = mInitializationString.toUtf8().constData();
            std::string label = mMorphemes.first().toUtf8().constData();
            throw std::runtime_error( "Attempting to process the portmanteau string " + is + " but the following node with this label is not a MorphemeNode: " + label );
        }
        mMorphemeNodes << static_cast<const MorphemeNode*>(current);

        i++;
    }
    while( i < mMorphemes.count() );

    Q_ASSERT(mMorphemeNodes.count() > 1);
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
    return mMorphemeNodes.last()->AbstractNode::next();
}

QString Portmanteau::label() const
{
    return mMorphemes.join("][");
}

const AbstractNode *Portmanteau::lastNode() const
{
    return mMorphemeNodes.last();
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
    if( mMorphemeNodes.isEmpty() )
    {
        return QString("Portmanteau([%1], not yet initialized)").arg( label() );
    }
    else if( mMorphemeNodes.last()->AbstractNode::next() != nullptr )
    {
        return QString("Portmanteau([%1], Next: %2)").arg( label(), next() == nullptr ? "null" : next()->label() );
    }
    else
    {
        return QString("Portmanteau([%1], Next: null)").arg( label() );
    }
}

QString Portmanteau::morphemeGlosses(const WritingSystem &summaryDisplayWritingSystem, const QString &delimiter) const
{
    QStringList glosses;
    for(int i=0; i<mMorphemeNodes.count(); i++)
    {
        glosses << mMorphemeNodes.at(i)->gloss(summaryDisplayWritingSystem).text();
    }
    return glosses.join(delimiter);
}
