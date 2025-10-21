#include "copynode.h"

#include <QString>
#include <QXmlStreamReader>

#include "morphology.h"
#include "morphologyxmlreader.h"

#include "datatypes/generation.h"
#include "debug.h"

using namespace ME;

CopyNode::CopyNode(const MorphologicalModel *model) : AbstractNode(model->morphology(), model), mCopy(nullptr)
{

}

CopyNode::~CopyNode()
{

}

AbstractNode *CopyNode::copy(MorphologyXmlReader *morphologyReader, const NodeId &idSuffix) const
{
    Q_UNUSED(morphologyReader)
    Q_UNUSED(idSuffix)
    qCritical() << "Don't try to copy copy-of nodes. The program is probably going to crash now.";
    return nullptr;
}

QList<Parsing> CopyNode::parsingsUsingThisNode(const Parsing &parsing, Parsing::Flags flags) const
{
    return mCopy->possibleParsings(parsing, flags);
}

QList<Generation> CopyNode::generateFormsUsingThisNode(const Generation &generation) const
{
    return mCopy->generateForms(generation);
}

const AbstractNode *CopyNode::followingNodeHavingLabel(const MorphemeLabel &targetLabel, QHash<const Jump *, int> &jumps) const
{
    return mCopy->followingNodeHavingLabel(targetLabel, jumps);
}

bool CopyNode::checkHasOptionalCompletionPath() const
{
    return mCopy->checkHasOptionalCompletionPath();
}

NodeId CopyNode::id() const
{
    return mCopy->id();
}

MorphemeLabel CopyNode::label() const
{
    return mCopy->label();
}

QString CopyNode::elementName()
{
    return "copy-of";
}

AbstractNode *CopyNode::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel *model)
{
    Q_ASSERT( in.isStartElement() );
    CopyNode * copy = new CopyNode(model);

    if( in.attributes().hasAttribute("target-id") )
    {
        const NodeId id(in.attributes().value("target-id").toString());
        AbstractNode * targetNode = morphologyReader->morphology()->getNodeFromId( id );
        if( targetNode == nullptr )
        {
            qCritical() << "No preceding node with id " << id.toString() << "found.";
            return copy;
        }
        else
        {
            copy->setCopy( targetNode->copy(morphologyReader, NodeId(in.attributes().value("id-suffix").toString()) ) );
        }
    }
    else
    {
        qCritical() << "You must specify a target id.";
        return copy;
    }

    while( ! in.isEndElement() )
        in.readNext();

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );

    return copy;
}

bool CopyNode::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

void CopyNode::setCopy(AbstractNode *copy)
{
    mCopy = copy;
}

void CopyNode::setNext(AbstractNode *next)
{
    mCopy->setNext(next);
}

AbstractNode *CopyNode::next() const
{
    return mCopy->next();
}

bool CopyNode::hasNext() const
{
    return mCopy->hasNext();
}

QString CopyNode::summary(const AbstractNode *doNotFollow) const
{
    return mCopy->summary(doNotFollow);
}

bool CopyNode::optional() const
{
    return mCopy->optional();
}

QSet<const AbstractNode *> CopyNode::availableMorphemeNodes(QHash<const Jump *, int> &jumps) const
{
    QSet<const AbstractNode *> set;
    set.unite( mCopy->availableMorphemeNodes(jumps) );

    /// move on to the next node if this one is optional
    if( optional() && AbstractNode::next() != nullptr )
    {
        set.unite( AbstractNode::next()->availableMorphemeNodes(jumps) );
    }

    return set;
}
