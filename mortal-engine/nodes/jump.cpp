#include "jump.h"

#include "datatypes/generation.h"

#include "morphology.h"
#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include <QtDebug>
#include "debug.h"

Jump::Jump(const MorphologicalModel *model) : AbstractNode(model), mNodeTarget(nullptr), mTargetNodeRequired(false)
{
    setOptional(true);
}

Jump::~Jump()
{

}

Jump *Jump::copy(MorphologyXmlReader *morphologyReader, const QString &idSuffix) const
{
    Jump * j = new Jump( model() );

    /// copy AbstractNode properties
    j->setLabel( label() );
    /// mType will be set by the constructor
    /// mNext will be set by the constructor
    j->setOptional( optional() );
    if( !id().isEmpty() )
    {
        j->setId( id() + idSuffix );
    }
    /// mHasPathToEnd should be calculated automatically
    j->mGlosses = mGlosses;

    /// set local properties
    j->setNodeTarget( mNodeTarget );
    j->setTargetId( targetId() );
    j->setTargetNodeRequired( mTargetNodeRequired );

    morphologyReader->registerNode(j);

    return j;
}

QString Jump::summary(const AbstractNode *doNotFollow) const
{
    Q_UNUSED(doNotFollow)

    QString dbgString;
    Debug dbg(&dbgString);
    Debug::atBeginning = false;

    dbg << QObject::tr("Jump(ID: %1, Pointer: %2, optional: %3, target node required: %4)")
            .arg(mTargetId,
                 mNodeTarget == nullptr ? "null" : "not null",
                 optional() ? "true" : "false",
                 mTargetNodeRequired ? "true" : "false" );

    Debug::atBeginning = true;
    return dbgString;
}

QList<Parsing> Jump::parsingsUsingThisNode(const Parsing &parsing, Parsing::Flags flags) const
{
    if( parsing.jumpPermitted(this) )
    {
        Parsing p = parsing;
        p.incrementJumpCounter(this);
        p.setNextNodeRequired( mTargetNodeRequired );
        if( Morphology::DebugOutput )
        {
            qInfo() << "Jumping to:" << mNodeTarget->label() << mNodeTarget->id();
        }
        return mNodeTarget->possibleParsings( p, flags );
    }
    else
    {
        return QList<Parsing>();
    }
}

QList<Generation> Jump::generateFormsUsingThisNode(const Generation &generation) const
{
    if( generation.jumpPermitted(this) )
    {
        Generation g = generation;
        g.incrementJumpCounter(this);
        return mNodeTarget->generateForms(g);
    }
    else
    {
        return QList<Generation>();
    }
}

void Jump::setTargetNodeRequired(bool targetNodeRequired)
{
    mTargetNodeRequired = targetNodeRequired;
}

bool Jump::isJump() const
{
    return true;
}

QList<const AbstractNode *> Jump::availableMorphemeNodes(QHash<const Jump *, int> &jumps) const
{
    QList<const AbstractNode *> list;

    if( jumps.value(this,0) < Parsing::MAXIMUM_JUMPS )
    {
        jumps[ this ] = jumps.value(this,0) + 1;
        list.append( mNodeTarget->availableMorphemeNodes(jumps) );

        /// move on to the next node if this one is optional
        if( optional() && AbstractNode::next() != nullptr )
        {
            list.append( AbstractNode::next()->availableMorphemeNodes(jumps) );
        }
    }

    return list;
}

void Jump::setNodeTarget(const AbstractNode *nodeTarget)
{
    mNodeTarget = nodeTarget;
}

void Jump::setTargetId(const QString &targetId)
{
    mTargetId = targetId;
}

QString Jump::targetId() const
{
    return mTargetId;
}

QString Jump::elementName()
{
    return "jump";
}

AbstractNode *Jump::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel * model)
{
    Q_UNUSED(morphologyReader)
    Q_ASSERT( in.isStartElement() );
    Jump * j = new Jump(model);
    j->readInitialNodeAttributes(in, morphologyReader);
    j->setTargetId( in.attributes().value("to").toString() );

    if( in.attributes().value(AbstractNode::XML_OPTIONAL).toString() == "false" )
    {
        j->setOptional(false);
    }

    if( in.attributes().value("target-node-required").toString() == "true" )
    {
        j->setTargetNodeRequired(true);
    }

    morphologyReader->registerJump(j);

    while( ! in.isEndElement() )
        in.readNext();

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return j;
}

bool Jump::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

const AbstractNode *Jump::followingNodeHavingLabel(const QString &targetLabel) const
{
    /// if the target node is in the same model as the current node
    /// we've got a circularity situation. in that case, just return
    /// the answer for this node
    if( mNodeTarget->model() == model() )
    {
        return AbstractNode::followingNodeHavingLabel(targetLabel);
    }
    else
    {
        return mNodeTarget->followingNodeHavingLabel(targetLabel);
    }
}

bool Jump::checkHasOptionalCompletionPath() const
{
    /// if the target node is in the same model as the current node
    /// we've got a circularity situation. in that case, assume that
    /// there's a path to the end
    if( mNodeTarget->model() == model() )
    {
        return true;
    }
    else
    {
        /// there is only an optional completion path if the target node
        /// has such a path, *and* if the target node is not required.
        return mNodeTarget->checkHasOptionalCompletionPath() && !mTargetNodeRequired;
    }
}
