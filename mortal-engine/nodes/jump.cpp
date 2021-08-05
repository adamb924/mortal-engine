#include "jump.h"

#include "datatypes/generation.h"

#include "morphology.h"
#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include <QtDebug>

Jump::Jump(const MorphologicalModel *model) : AbstractNode(model), mNodeTarget(nullptr), mTargetNodeRequired(false)
{
    setOptional(true);
}

Jump::~Jump()
{

}

QString Jump::summary() const
{
    return QObject::tr("Jump(ID: %1, Pointer: %2, optional: %3, target node required: %4)")
            .arg(mTargetId,
                 mNodeTarget == nullptr ? "null" : "not null",
                 optional() ? "true" : "false",
                 mTargetNodeRequired ? "true" : "false" );
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

    if( in.attributes().value("optional").toString() == "false" )
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
        return mNodeTarget->checkHasOptionalCompletionPath();
    }
}
