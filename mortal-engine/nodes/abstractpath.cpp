#include "abstractpath.h"

#include "datatypes/generation.h"

AbstractPath::AbstractPath(const MorphologicalModel *model) : AbstractNode(model, AbstractNode::PathNodeType), mInitialNode(nullptr)
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

const AbstractNode *AbstractPath::followingNodeHavingLabel(const QString &targetLabel) const
{
    return mInitialNode->followingNodeHavingLabel(targetLabel);
}

bool AbstractPath::checkHasOptionalCompletionPath() const
{
    /// Look at the following nodes. If all of the nodes are optional, return true, otherwise return false.
    if( next() == nullptr )
    {
        return true;
    }
    else
    {
        if( next()->optional() )
        {
            return mInitialNode->checkHasOptionalCompletionPath();
        }
        else
        {
            return false;
        }
    }
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
