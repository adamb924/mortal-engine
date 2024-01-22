#include "portmanteau.h"

#include "nodes/abstractnode.h"
#include "nodes/morphemenode.h"

#include <QtDebug>
#include <stdexcept>

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

void Portmanteau::initialize(const AbstractNode *parent)
{
    mNodes.clear();
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

    /// the parent MorphemeNode is the first element of the portmanteau
    mNodes << parent;

    /// then add every other morpheme in the portmanteau
    /// this will execute at least once because every portmanteau has to have at least two morphemes (checked above)
    int i=1;
    do
    {
        /// the current node is the (first?) node following the most recently added node node that has the label in question
        const AbstractNode * current = mNodes.last()->followingNodeHavingLabel( mMorphemes.at(i) );
        /// fail if no node has that label
        if( current == nullptr )
        {
            std::string is = mInitializationString.toUtf8().constData();
            std::string label = mMorphemes.first().toString().toUtf8().constData();
            throw std::runtime_error( "Attempting to process the portmanteau string " + is + " but the following node with this label could not be found: " + label );
        }
        /// fail if, for some reason the node returned is not a MorphemeNode
        if( !current->isMorphemeNode() )
        {
            std::string is = mInitializationString.toUtf8().constData();
            std::string label = mMorphemes.first().toString().toUtf8().constData();
            throw std::runtime_error( "Attempting to process the portmanteau string " + is + " but the following node with this label is not a MorphemeNode: " + label );
        }
        mNodes << static_cast<const MorphemeNode*>(current);

        i++;
    }
    while( i < mMorphemes.count() );

    Q_ASSERT(mNodes.count() > 1);
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

QString Portmanteau::summary() const
{
    if( mNodes.isEmpty() )
    {
        return QString("Portmanteau(%1, not yet initialized)").arg( mMorphemes.toString() );
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

QString Portmanteau::morphemeGlosses(const WritingSystem &summaryDisplayWritingSystem, const QString &delimiter) const
{
    QStringList glosses;
    for(int i=0; i<mNodes.count(); i++)
    {
        glosses << mNodes.at(i)->gloss(summaryDisplayWritingSystem).text();
    }
    return glosses.join(delimiter);
}
