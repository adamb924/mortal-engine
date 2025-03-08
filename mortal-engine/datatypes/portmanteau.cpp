#include "portmanteau.h"

#include "nodes/abstractnode.h"

#include <QtDebug>
#include <stdexcept>

using namespace ME;

Portmanteau::Portmanteau()
    : mStatus(Portmanteau::Invalid)
{

}

Portmanteau::Portmanteau(const Portmanteau & other)
    : mStatus(other.mStatus),
      mInitializationString(other.mInitializationString),
      mMorphemes(other.mMorphemes),
    mNodes(other.mNodes)
{

}

Portmanteau &Portmanteau::operator=(const Portmanteau &other)
{
    mStatus = other.mStatus;
    mInitializationString = other.mInitializationString;
    mMorphemes = other.mMorphemes;
    mNodes = other.mNodes;
    return *this;
}

Portmanteau::Portmanteau(const QString &initializationString)
     : mStatus(Portmanteau::Valid),
       mInitializationString(initializationString)
{
}

bool Portmanteau::operator==(const Portmanteau &other) const
{
    return mInitializationString == other.mInitializationString;
}

bool Portmanteau::initialize(const AbstractNode *parent, QHash<MorphemeLabel, const AbstractNode *> &cache)
{
    mNodes.clear();

    if( mInitializationString.isEmpty() )
        return false;

    mMorphemes = MorphemeSequence::fromString(mInitializationString);

    /// portmanteaux must have at least two morphemes
    if( mMorphemes.count() < 2 )
    {
        std::string is = mInitializationString.toUtf8().constData();
        throw std::runtime_error( "A portmanteau must have at least two morphemes: " + is );
    }

    /// the first morpheme must be the same (i.e., have the same label as) the MorphemeNode that this portmanteau belongs to
    if( mMorphemes.first() != parent->label() )
    {
        std::string is = mInitializationString.toUtf8().constData();
        std::string parentLabel = parent->label().toString().toUtf8().constData();
        throw std::runtime_error( "The first morpheme of a portmanteau should be the current morpheme: " + is + " but it's " + parentLabel );
    }

    /// the parent AbstractNode is the first element of the portmanteau
    mNodes << parent;

    /// then add every other morpheme in the portmanteau
    /// this will execute at least once because every portmanteau has to have at least two morphemes (checked above)
    int i=1;
    do
    {
        /// the current node is the (first?) node following the most recently added node node that has the label in question
        const AbstractNode * startingFrom = mNodes.last(); /// i.e., either the first node or the last one added in a previous loop iteration
        const AbstractNode * current = getFollowingNode(startingFrom, mMorphemes.at(i), cache);

        /// fail if no node has that label
        if( current == nullptr )
        {
            std::string is = mInitializationString.toUtf8().constData();
            std::string startingLabel = mMorphemes.first().toString().toUtf8().constData();
            std::string targetLabel = mMorphemes.at(i).toString().toUtf8().constData();
            throw std::runtime_error( "Attempting to process the portmanteau string " + is + " but the node '" + targetLabel + "' could not be found following '" + startingLabel + "'." );
        }
        mNodes << current;

        i++;
    }
    while( i < mMorphemes.count() );

    Q_ASSERT(mNodes.count() > 1);

    return mMorphemes.count() > 1;
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
    return mNodes.last()->AbstractNode::next();
}

const AbstractNode *Portmanteau::lastNode() const
{
    return mNodes.last();
}

int Portmanteau::count() const
{
    return mMorphemes.count();
}

MorphemeSequence Portmanteau::morphemes() const
{
    return mMorphemes;
}

QList<const AbstractNode *> Portmanteau::nodes() const
{
    return mNodes;
}

const AbstractNode *Portmanteau::getFollowingNode(const AbstractNode *startingFrom, const MorphemeLabel &label, QHash<MorphemeLabel, const AbstractNode *> &cache) const
{
    /// if the cache contains the label already, return the cached value
    if( cache.contains(label) )
    {
        return cache.value(label);
    }
    else
    {
        /// For models with lots of jumps and high number of
        /// maximum jumps, this search can be quite expensive.
        /// Therefore try limiting to one jump and search with
        /// that; if it fails, search with all allowed jumps.

        /// Store the user-defined value and temporarily set to 1
        int originalMaximumJumps = Parsing::MAXIMUM_JUMPS;
        Parsing::MAXIMUM_JUMPS = 1;

        /// try with just one jump permitted (to each node)
        QHash<const Jump *, int> jumps;
        const AbstractNode * current = startingFrom->followingNodeHavingLabel(label, jumps);

        /// restore the user-supplied value
        Parsing::MAXIMUM_JUMPS = originalMaximumJumps;

        /// try again if we need
        if( current == nullptr )
        {
            jumps.clear();
            current = startingFrom->followingNodeHavingLabel(label, jumps);
        }

        /// Save the search result in the cache
        cache.insert(label, current);
        return current;
    }
}

QString Portmanteau::summary() const
{
    if( mNodes.isEmpty() )
    {
        return QString("Portmanteau(%1, not yet initialized but having initialization string %2)").arg( mMorphemes.toString() ).arg(mInitializationString);
    }
    else if( mNodes.last()->AbstractNode::next() != nullptr )
    {
        return QString("Portmanteau(%1, Next: %2)").arg( mMorphemes.toString(), next() == nullptr ? "null" : next()->label().toString() );
    }
    else
    {
        return QString("Portmanteau(%1, Next: null)").arg( mMorphemes.toString() );
    }
}
