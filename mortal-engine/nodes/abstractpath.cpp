#include "abstractpath.h"

#include "datatypes/generation.h"

using namespace ME;

AbstractPath::AbstractPath(const Morphology *morphology, const MorphologicalModel *model) : AbstractNode(morphology, model, AbstractNode::PathNodeType), mInitialNode(nullptr)
{

}

AbstractPath::~AbstractPath()
{

}

AbstractNode *AbstractPath::initialNode() const
{
    return mInitialNode;
}

void AbstractPath::setInitialNode(AbstractNode *initialNode)
{
    mInitialNode = initialNode;
}

void AbstractPath::setNext(AbstractNode *nextNode)
{
    if( mInitialNode != nullptr )
    {
        mInitialNode->setNextNodeIfFinalNode(nextNode);
    }
    /// we also need to set the next node for the
    /// Path node itself, in case it is optional
    /// and can be skipped
    AbstractNode::setNext(nextNode);
}

const AbstractNode *AbstractPath::followingNodeHavingLabel(const MorphemeLabel &targetLabel, QHash<const Jump *, int> &jumps) const
{
    return mInitialNode->followingNodeHavingLabel(targetLabel, jumps);
}

bool AbstractPath::checkHasOptionalCompletionPath() const
{
    /// Look at the following nodes. If all of the nodes are optional, return true, otherwise return false.
    /// if the following node is null, we're at the end of the model anyway, and the answer is "yes"
    if( next() == nullptr )
    {
        return true;
    }
    else
    {
        /// if this node is optional, then there is an optional completion path iff the following node has an optional completion path
        if( optional() )
        {
            return next()->checkHasOptionalCompletionPath();
        }
        /// otherwise, we need to check if there is an optional completion path for the first node
        else
        {
            if( mInitialNode->optional() )
            {
                return mInitialNode->checkHasOptionalCompletionPath();
            }
            else
            {
                return false;
            }
        }
    }
}

QSet<const AbstractNode *> AbstractPath::availableMorphemeNodes(QHash<const Jump *, int> &jumps) const
{
    QSet<const AbstractNode *> set;
    set.unite( mInitialNode->availableMorphemeNodes(jumps) );

    /// move on to the next node if this one is optional
    if( optional() && AbstractNode::next() != nullptr )
    {
        set.unite( AbstractNode::next()->availableMorphemeNodes(jumps) );
    }

    return set;
}

QList<Parsing> AbstractPath::parsingsUsingThisNode(const Parsing &parsing, Parsing::Flags flags) const
{
    if( mInitialNode == nullptr )
    {
        return QList<Parsing>();
    }
    else
    {
        return mInitialNode->possibleParsings(parsing, flags);
    }
}

QList<Generation> AbstractPath::generateFormsUsingThisNode(const Generation &parsing) const
{
    if( mInitialNode == nullptr )
    {
        return QList<Generation>();
    }
    else
    {
        return mInitialNode->generateForms(parsing);
    }
}
