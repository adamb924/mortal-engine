#include "abstractnode.h"

#include <QtDebug>

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QRegularExpression>
#include <QDomElement>

#include "morphologyxmlreader.h"
#include "morphology.h"
#include "datatypes/generation.h"
#include "logging/parsinglog.h"

using namespace ME;

QString AbstractNode::XML_OPTIONAL = "optional";
QString AbstractNode::XML_ADD_ALLOMORPHS = "add-allomorphs";

AbstractNode::AbstractNode(const Morphology *morphology, const MorphologicalModel *model, Type t) :
    mMorphology(morphology),
    mModel(model),
    mType(t),
    mNext(nullptr),
    mOptional(false),
    mHasPathToEnd(false)
{
}

QList<Parsing> AbstractNode::possibleParsings(const Parsing &parsing, Parsing::Flags flags) const
{
    bool nodeRequired = parsing.nextNodeRequired();
    Parsing p = parsing;

    /// Parsing::addToStackTrace is only actually called in the XmlParsingLog implementation
    parsingLog()->addToStackTrace( p, QString("%1, %2").arg(debugIdentifier(), id().toString()) );

    /// TODO keep thinking about this logic. This means that we only stop requiring a node when a
    /// morpheme is appended. At the least the input should be something like next-morpheme-required="true"
    if( nodeCanAppendMorphemes() )
        p.setNextNodeRequired(false);

    parsingLog()->beginNode(this, p);

    QList<Parsing> candidates;
    candidates.append( parsingsUsingThisNode(p, flags) );

    parsingLog()->end(); /// beginNode

    if( optional() && ! nodeRequired )
    {
        /// we want to move to the next node either 1) the parse hasn't been completed, or 2) there
        /// are null morphemes in the model, which we might want to append
        bool shouldTryToContinue = p.isOngoing() || p.isNull() || model()->hasZeroLengthForms();
        if( AbstractNode::hasNext() && shouldTryToContinue )
        {
            candidates.append( AbstractNode::next()->possibleParsings( p, flags ) );
            MAYBE_RETURN_EARLY
        }
        else
        {
            appendIfComplete(candidates, p);
        }
    }

    return candidates;
}

QList<Generation> AbstractNode::generateForms(const Generation &generation) const
{
    parsingLog()->beginNode(this, generation);

    QList<Generation> candidates;

    if( appendIfComplete(candidates, generation) )
    {
        return candidates;
    }

    if( optional() )
    {
        if( AbstractNode::hasNext() )
        {
            parsingLog()->begin("without-this-node");
            candidates.append( AbstractNode::next()->generateForms( generation ) );
            parsingLog()->end(); /// without-this-node
        }
        else
        {
            appendIfComplete(candidates, generation);
        }

    }

    parsingLog()->begin("with-this-node");
    candidates << generateFormsUsingThisNode(generation);
    parsingLog()->end(); /// with-this-node

    parsingLog()->end();

    return candidates;

}

bool AbstractNode::appendIfComplete(QList<Generation> &candidates, const Generation &generation) const
{
    bool mscHasNoMoreMorphemes = generation.morphemeSequenceConstraint()->hasNoMoreMorphemes();

    if( mscHasNoMoreMorphemes )
    {
        Generation g = generation;
        g.setCompleteIfAllConstraintsSatisfied();
        if( g.isCompleted() )
        {
            parsingLog()->completed(generation);
            candidates.append( g );
            return true;
        }
    }
    return false;
}

void AbstractNode::appendIfComplete(QList<Parsing> &candidates, const Parsing &parsing) const
{
    if( parsing.isCompleted() &&  parsing.allConstraintsSatisfied() ) /// the parsing only succeeds if there's nothing left to parse
    {
        parsingLog()->completed(parsing);
        candidates.append(parsing);
    }
}

AbstractNode::~AbstractNode()
{

}

MorphemeLabel AbstractNode::label() const
{
    return mLabel;
}

void AbstractNode::setLabel(const MorphemeLabel &label)
{
    mLabel = label;
}

AbstractNode::Type AbstractNode::type() const
{
    return mType;
}

QString AbstractNode::typeString() const
{
    return AbstractNode::typeToString( mType );
}

QString AbstractNode::typeToString(AbstractNode::Type type)
{
    switch( type ) {
    case AbstractNode::StemNodeType:
        return "stem";
    case AbstractNode::MorphemeNodeType:
        return "morpheme";
    case AbstractNode::PathNodeType:
        return "path";
    }
    return "null";
}

AbstractNode *AbstractNode::next() const
{
    return mNext;
}

bool AbstractNode::hasNext() const
{
    return mNext != nullptr;
}

QList<const AbstractNode *> AbstractNode::nextMorphemeNodes() const
{
    QHash<const Jump*,int> jumpCounts;
    if( mNext == nullptr )
    {
        return QList<const AbstractNode *>();
    }
    else
    {
        return mNext->availableMorphemeNodes(jumpCounts);
    }
}

bool AbstractNode::hasNext(const Allomorph &appendedAllomorph, const WritingSystem &ws) const
{
    if( appendedAllomorph.hasPortmanteau(ws) )
    {
        return appendedAllomorph.portmanteau().next() != nullptr;
    }
    else
    {
        return AbstractNode::next() != nullptr;
    }
}

const AbstractNode *AbstractNode::next(const Allomorph &appendedAllomorph, const WritingSystem &ws) const
{
    if( appendedAllomorph.hasPortmanteau(ws) )
    {
        return appendedAllomorph.portmanteau().next();
    }
    else
    {
        return AbstractNode::next();
    }
}

const ParsingLog *AbstractNode::parsingLog() const
{
    return mMorphology->parsingLog();
}

void AbstractNode::setNext(AbstractNode *next)
{
    mNext = next;
}

bool AbstractNode::optional() const
{
    return mOptional;
}

void AbstractNode::setOptional(bool optional)
{
    mOptional = optional;
}

QString AbstractNode::summary(const AbstractNode *doNotFollow) const
{
    Q_UNUSED(doNotFollow)
    QString dbgString;
    QTextStream dbg(&dbgString);

    dbg << "AbstractNode(";
    dbg << "Label: " << mLabel.toString() << ", ";
    dbg << "Type: " << AbstractNode::typeToString(mType) << ", ";
    dbg << "Optional: " << (mOptional ? "true" : "false" ) << ")";

    return dbgString;
}

QString AbstractNode::oneLineSummary() const
{
    return summary().replace(QRegularExpression("\\s+")," ").trimmed();
}

void AbstractNode::serialize(QXmlStreamWriter &out) const
{
    out.writeStartElement("node");
    out.writeAttribute("label", mLabel.toString());
    out.writeAttribute("type", typeToString(mType) );
    if( ! mId.isNull() )
    {
        out.writeAttribute("id", mId.toString());
    }
    out.writeAttribute(AbstractNode::XML_OPTIONAL, (mOptional ? "true" : "false" ) );
    out.writeEndElement(); /// node
}

void AbstractNode::serialize(QDomElement &out) const
{
    out.setAttribute("label", mLabel.toString());
    out.setAttribute("type", typeToString(mType) );
    if( ! mId.isNull() )
    {
        out.setAttribute("id", mId.toString());
    }
    out.setAttribute(AbstractNode::XML_OPTIONAL, (mOptional ? "true" : "false" ) );
}

void AbstractNode::readInitialNodeAttributes(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    QXmlStreamAttributes attr = in.attributes();
    if( attr.hasAttribute("label") )
        setLabel( MorphemeLabel( attr.value("label").toString() ) );

    if( attr.hasAttribute("id") )
    {
        setId( NodeId(attr.value("id").toString()) );
        morphologyReader->morphology()->setNodeId( id(), this );
    }
}

NodeId AbstractNode::id() const
{
    return mId;
}

void AbstractNode::setId(const NodeId &id)
{
    mId = id;
}

bool AbstractNode::isStemNode() const
{
    return mType == AbstractNode::StemNodeType;
}

void AbstractNode::setNextNodeIfFinalNode(AbstractNode *nextNode)
{
    if( hasNext() )
    {
        next()->setNextNodeIfFinalNode( nextNode );
    }
    else
    {
        /// call setNext() instead of setting mNext, because nodes like MEMs
        /// and Paths need to set the next nodes for child nodes, etc.
        setNext( nextNode );
    }
}

const AbstractNode *AbstractNode::followingNodeHavingLabel(const MorphemeLabel &targetLabel, QHash<const Jump *, int> &jumps) const
{
    if( label() == targetLabel )
    {
        return this;
    }
    else if ( next() == nullptr )
    {
        return nullptr;
    }
    else
    {
        return mNext->followingNodeHavingLabel(targetLabel, jumps);
    }
}

void AbstractNode::calculateModelProperties()
{
    if( mHasPathToEnd == false ) /// it the property has been set manually (e.g., by a container node), leave it to that
    {
        mHasPathToEnd = checkHasOptionalCompletionPath();
    }
}

bool AbstractNode::checkHasOptionalCompletionPath() const
{
    /// Look at the following nodes. If all of the nodes are optional, return true, otherwise return false.
    if( next() == nullptr )
    {
        return true;
    }
    else
    {
        /// forks need to be evalated, because they might not be optional
        /// although there would be some path that offers a completion path
        if( next()->optional() || next()->isFork() || next()->isSequence() || next()->isJump() )
        {
            return next()->checkHasOptionalCompletionPath();
        }
        else
        {
            return false;
        }
    }
}

bool AbstractNode::isFork() const
{
    return false;
}

bool AbstractNode::isSequence() const
{
    return false;
}

bool AbstractNode::isModel() const
{
    return false;
}

bool AbstractNode::isMorphemeNode() const
{
    return false;
}

bool AbstractNode::isMutuallyExclusiveMorphemes() const
{
    return false;
}

bool AbstractNode::isJump() const
{
    return false;
}

bool AbstractNode::nodeCanAppendMorphemes() const
{
    return false;
}

const MorphologicalModel *AbstractNode::model() const
{
    return mModel;
}

Form AbstractNode::gloss(const WritingSystem &ws) const
{
    return mGlosses.value(ws, Form(ws,""));
}

QHash<WritingSystem, Form> AbstractNode::glosses() const
{
    return mGlosses;
}

QString AbstractNode::debugIdentifier() const
{
    if( id().isNull() )
        return label().toString();
    else
        return QString("%1[%2]").arg( label().toString(), id().toString() );
}

bool AbstractNode::hasPathToEnd() const
{
    return mHasPathToEnd;
}
