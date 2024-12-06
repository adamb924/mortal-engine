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

using namespace ME;

QString AbstractNode::XML_OPTIONAL = "optional";
QString AbstractNode::XML_ADD_ALLOMORPHS = "add-allomorphs";

AbstractNode::AbstractNode(const MorphologicalModel *model, Type t) :
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

    if( Morphology::DebugOutput )
    {
        /// only add this data if we're in debug mode; no reason to slow down all those deep copies
        p.addToStackTrace( QString("%1, %2").arg(debugIdentifier(), id()) );
    }

    /// TODO keep thinking about this logic. This means that we only stop requiring a node when a
    /// morpheme is appended. At the least the input should be something like next-morpheme-required="true"
    if( nodeCanAppendMorphemes() )
        p.setNextNodeRequired(false);

    if( Morphology::DebugOutput )
    {
        if( next() != nullptr )
            qInfo().noquote() << QString("CURRENT NODE: %1 (NEXT NODE: %2)").arg( debugIdentifier(), next()->debugIdentifier() );
        else
            qInfo().noquote() << QString("CURRENT NODE: %1").arg( debugIdentifier() );
    }

    QList<Parsing> candidates;
    if( optional() && ! nodeRequired )
    {
        /// we want to move to the next node either 1) the parse hasn't been completed, or 2) there
        /// are null morphemes in the model, which we might want to append
        bool shouldTryToContinue = p.isOngoing() || p.isNull() || model()->hasZeroLengthForms();
        if( AbstractNode::hasNext() && shouldTryToContinue )
        {
            if( Morphology::DebugOutput )
            {
                qInfo().noquote() << "Attempting parse without optional node:" << debugIdentifier() << "with" << p.intermediateSummary();
            }
            candidates.append( AbstractNode::next()->possibleParsings( p, flags ) );
            MAYBE_RETURN_EARLY
        }
        else
        {
            if( Morphology::DebugOutput )
            {
                qInfo().noquote() << "No following node here:" << debugIdentifier() << typeToString(type()) << "with" << p.intermediateSummary();
            }
            appendIfComplete(candidates, p);
        }
    }

    if( Morphology::DebugOutput )
    {
        qInfo().noquote() << "Attempting parse with node:" << debugIdentifier() << "with" << p.intermediateSummary();
    }
    candidates.append( parsingsUsingThisNode(p, flags) );

    return candidates;
}

QList<Generation> AbstractNode::generateForms(const Generation &generation) const
{
    if( Morphology::DebugOutput )
    {
        qInfo() << "GENERATION CURRENT NODE:" << debugIdentifier() << id();
        if( next() != nullptr )
        {
            qInfo() << qPrintable("\t") << "NEXT NODE:" << next()->debugIdentifier() << next()->id();
        }
    }

    QList<Generation> candidates;

    if( appendIfComplete(candidates, generation) )
    {
        return candidates;
    }

    if( optional() )
    {
        if( AbstractNode::hasNext() )
        {
            if( Morphology::DebugOutput )
            {
                qInfo() << "Attempting generate without optional node:" << debugIdentifier() << "with" << generation.intermediateSummary();
            }
            candidates.append( AbstractNode::next()->generateForms( generation ) );
        }
        else
        {
            if( Morphology::DebugOutput )
            {
                qInfo() << "No following node here:" << debugIdentifier() << "with" << generation.intermediateSummary();
            }
            appendIfComplete(candidates, generation);
        }

    }

    if( Morphology::DebugOutput )
    {
        qInfo() << "Attempting generation with node:" << debugIdentifier() << "with" << generation.intermediateSummary();
    }
    candidates << generateFormsUsingThisNode(generation);

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
            if( Morphology::DebugOutput )
            {
                qInfo() << "Completed generation:" << generation.oneLineSummary();
            }
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
        if( Morphology::DebugOutput )
        {
            qInfo() << "Completed parsing:" << parsing.form().summary() << parsing.labelSummary() << parsing.summary();
        }
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

const AbstractNode *AbstractNode::next() const
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
    if( ! mId.isEmpty() )
    {
        out.writeAttribute("id", mId);
    }
    out.writeAttribute(AbstractNode::XML_OPTIONAL, (mOptional ? "true" : "false" ) );
    out.writeEndElement(); /// node
}

void AbstractNode::serialize(QDomElement &out) const
{
    out.setAttribute("label", mLabel.toString());
    out.setAttribute("type", typeToString(mType) );
    if( ! mId.isEmpty() )
    {
        out.setAttribute("id", mId);
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
        setId( attr.value("id").toString() );
        morphologyReader->morphology()->setNodeId( id(), this );
    }
}

QString AbstractNode::id() const
{
    return mId;
}

void AbstractNode::setId(const QString &id)
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
        mNext->setNextNodeIfFinalNode( nextNode );
    }
    else
    {
        /// call setNext() instead of setting mNext, because nodes like MEMs
        /// and Paths need to set the next nodes for child nodes, etc.
        setNext( nextNode );
    }
}

const AbstractNode *AbstractNode::followingNodeHavingLabel(const MorphemeLabel &targetLabel) const
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
        return mNext->followingNodeHavingLabel(targetLabel);
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
    if( id().isEmpty() )
        return label().toString();
    else
        return QString("%1[%2]").arg( label().toString(), id() );
}

bool AbstractNode::hasPathToEnd() const
{
    return mHasPathToEnd;
}
