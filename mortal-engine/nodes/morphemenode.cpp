#include "morphemenode.h"

#include <QtDebug>

#include "datatypes/allomorph.h"
#include "datatypes/generation.h"
#include "datatypes/portmanteau.h"

#include "generation-constraints/morphemesequenceconstraint.h"

#include "morphology.h"
#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include "debug.h"

MorphemeNode::MorphemeNode(const MorphologicalModel *model) : AbstractNode(model)
{

}

MorphemeNode::~MorphemeNode()
{
}

void MorphemeNode::addAllomorph(const Allomorph & allomorph)
{
    mAllomorphs.append( allomorph );
}

QList<Parsing> MorphemeNode::parsingsUsingThisNode(const Parsing &parsing, Parsing::Flags flags) const
{
    QList<Parsing> candidates;

    if( Morphology::DebugOutput )
    {
        qInfo() << "In " << label() << "with" << parsing.intermediateSummary();
    }

    QSet<Allomorph> matches = matchingAllomorphs(parsing);

    if( Morphology::DebugOutput )
    {
        qInfo() << label() << " has " << matches.count() << " allomorph matches.";
    }

    QSetIterator<Allomorph> matchesIterator( matches );
    while( matchesIterator.hasNext() )
    {
        Allomorph a = matchesIterator.next();
        Parsing p = parsing;
        p.append( this, a );

        appendIfComplete(candidates, p);
        MAYBE_RETURN_EARLY

        if( hasNext(a, p.writingSystem()) && p.isOngoing() )/// there are further morphemes in the model
        {
            if( Morphology::DebugOutput )
            {
                qInfo() << "Moving from" << label() << " to " << next(a, p.writingSystem())->label() << "having appended" << a.oneLineSummary();
            }
            candidates.append( next(a, p.writingSystem())->possibleParsings( p, flags ) );
            MAYBE_RETURN_EARLY
        }
    }

    /// we now need to check whether any of the candidates have morpheme sequences
    /// that clash with portmanteaux that are available at this node
    /// TODO: this code won't be executed with MAYBE_RETURN_EARLY
    filterOutPortmanteauClashes(candidates, parsing.writingSystem());

    return candidates;
}

QList<Generation> MorphemeNode::generateFormsUsingThisNode(const Generation &generation) const
{
    QList<Generation> candidates;

    /// test whether this node fits the morpheme sequence constraint
    if( ! generation.ableToAppend(label()) )
    {
        if( Morphology::DebugOutput )
        {
            qInfo() << qPrintable("\t") << "Current node does not match the morpheme sequence constraint:" << label();
        }
        return candidates;
    }

    /// at this point we know that the morpheme sequence constraint has been satisfied
    QSet<Allomorph> portmanteaux;
    QSet<Allomorph> nonPortmanteau;
    matchingAllomorphs(generation, portmanteaux, nonPortmanteau );

    if( Morphology::DebugOutput )
    {
        qInfo().noquote() << qPrintable("\t") << QObject::tr("%1 allomorph matches (%2 normal, %3 portmanteau)").arg( portmanteaux.count() + nonPortmanteau.count() ).arg( nonPortmanteau.count() ).arg( portmanteaux.count() );
    }

    QList<Generation> portmanteuGenerations = generateFormsWithAllomorphs(generation, portmanteaux);
    /// only consider non-portmanteau generations if there are no successful portmanteau generations
    /// TODO: is this correct? Could it ever be ambiguous? — For a parsing, yes, but for a generation, no.
    if( portmanteuGenerations.count() > 0 )
    {
        candidates.append( portmanteuGenerations );
    }
    else
    {
        candidates.append( generateFormsWithAllomorphs(generation, nonPortmanteau) );
    }

    return candidates;
}

QList<Allomorph> MorphemeNode::allomorphs() const
{
    return mAllomorphs;
}

bool MorphemeNode::hasForm(const Form &f) const
{
    QListIterator<Allomorph> i(mAllomorphs);
    while( i.hasNext() )
    {
        if( i.next().hasForm(f) )
        {
            return true;
        }
    }
    return false;
}

bool MorphemeNode::isMorphemeNode() const
{
    return true;
}

bool MorphemeNode::nodeCanAppendMorphemes() const
{
    return true;
}

void MorphemeNode::addGloss(const Form &gloss)
{
    mGlosses.insert( gloss.writingSystem(), gloss );
}

int MorphemeNode::glossCount() const
{
    return mGlosses.count();
}

void MorphemeNode::addConstraintsToAllAllomorphs(const QSet<const AbstractConstraint *> &constraints)
{
    for(int i=0; i< mAllomorphs.count(); i++)
    {
        mAllomorphs[i].addConstraints( constraints );
    }
}

bool MorphemeNode::hasZeroLengthForms() const
{
    for(int i=0; i< mAllomorphs.count(); i++)
    {
        if( mAllomorphs.at(i).hasZeroLengthForms() )
        {
            return true;
        }
    }
    return false;
}

QList<Generation> MorphemeNode::generateFormsWithAllomorphs(const Generation &generation, QSet<Allomorph> &potentialAllomorphs) const
{
    QList<Generation> candidates;

    QSetIterator<Allomorph> i(potentialAllomorphs);
    while( i.hasNext() ) /// if there is a matching allomorph
    {
        Allomorph a = i.next();
        Generation g = generation;
        g.append( this, a );

        if( hasNext(a, g.writingSystem()) && g.isOngoing() )
        {
            if( Morphology::DebugOutput )
            {
                qInfo() << "Moving from" << label() << " to " << next(a, g.writingSystem())->label() << "having appended" << a.oneLineSummary();
            }
            candidates.append( next(a, g.writingSystem())->generateForms(g) );
        }
        else
        {
            appendIfComplete(candidates, g);
        }
    }
    return candidates;
}

QSet<Allomorph> MorphemeNode::matchingAllomorphs(const Parsing &parsing) const
{
    QSet<Allomorph> matches;

    /// check all allomorphs
    QListIterator<Allomorph> i(mAllomorphs);
    while( i.hasNext() )
    {
        Allomorph a = i.next();
        if( parsing.allomorphMatches(a) )
        {
            matches << a;
        }
    }

    return matches;
}

void MorphemeNode::matchingAllomorphs(const Generation &generation, QSet<Allomorph> &portmanteaux, QSet<Allomorph> &nonPortmanteau) const
{
    /// check all allomorphs
    QListIterator<Allomorph> i(mAllomorphs);
    while( i.hasNext() )
    {
        Allomorph a = i.next();
        if( a.hasForm(generation.writingSystem()) && generation.allomorphMatchConditionsSatisfied(a) )
        {
            if( a.hasPortmanteau(generation.writingSystem()) )
            {
                if( generation.morphemeSequenceConstraint()->matchesPortmanteau( a.portmanteau() ) )
                {
                    portmanteaux << a;
                }
            }
            else
            {
                nonPortmanteau << a;
            }
        }
    }
}

QString MorphemeNode::elementName()
{
    return "morpheme";
}

AbstractNode *MorphemeNode::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel * model)
{
    Q_ASSERT( in.isStartElement() );
    MorphemeNode* morpheme = new MorphemeNode(model);
    morpheme->readInitialNodeAttributes(in, morphologyReader);

    /// store any morpheme-level constraints
    QSet<const AbstractConstraint*> morphemeLevelConstraints;

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName() ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == "optional" )
            {
                morpheme->setOptional(true);
            }
            else if( in.name() == "allomorph" )
            {
                morpheme->addAllomorph( readAllomorphTag(morpheme, in, morphologyReader) );
            }
            else if( in.name() == "add-allomorphs" )
            {
                morpheme->addCreateAllomorphs( morphologyReader->createAllomorphsFromId( in.attributes().value("with").toString() ) );
            }
            else if( in.name() == "gloss" )
            {
                morpheme->addGloss( Form::readFromXml(in, morphologyReader->morphology(), "gloss") );
            }
            else if ( morphologyReader->currentNodeMatchesConstraint(in) )
            {
                morphemeLevelConstraints << morphologyReader->tryToReadConstraint(in);
            }
            else
            {
                qWarning() << "Unexpected tag when reading MorphemeNode" << in.name() << "on line" << in.lineNumber();
            }
        }
    }

    /// now, add any morpheme-level constraints to every allomorph
    morpheme->addConstraintsToAllAllomorphs(morphemeLevelConstraints);

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return morpheme;
}

/// TODO probably this belongs in the Allomorph class
Allomorph MorphemeNode::readAllomorphTag(MorphemeNode* morpheme, QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_ASSERT( in.isStartElement() );
    Allomorph allomorph(Allomorph::Original);

    if( in.attributes().hasAttribute("portmanteau") )
    {
        allomorph.setPortmanteau( Portmanteau( in.attributes().value("portmanteau").toString(), morpheme ) );
    }

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == "allomorph") )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == "form" )
            {
                QXmlStreamAttributes attr = in.attributes();
                if( attr.hasAttribute("lang") )
                {
                    WritingSystem ws = morphologyReader->morphology()->writingSystem( attr.value("lang").toString() );
                    allomorph.setForm( Form( ws, in.readElementText() ) );
                }
            }
            else if( in.name() == "tag" )
            {
                allomorph.addTag( in.readElementText() );
            }
            else
            {
                allomorph.addConstraint( morphologyReader->tryToReadConstraint(in) );
            }
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == "allomorph" );
    return allomorph;
}

bool MorphemeNode::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

bool MorphemeNode::hasNext(const Allomorph &appendedAllomorph, const WritingSystem & ws) const
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

const AbstractNode *MorphemeNode::next(const Allomorph &appendedAllomorph, const WritingSystem & ws) const
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

void MorphemeNode::initializePortmanteaux(const Morphology *morphology)
{
    for(int i=0; i<mAllomorphs.count(); i++)
    {
        QSetIterator<WritingSystem> wsIter( mAllomorphs.at(i).writingSystems() );
        while( wsIter.hasNext() )
        {
            const WritingSystem ws = wsIter.next();
            if( mAllomorphs.at(i).hasPortmanteau(ws) )
            {
                mAllomorphs[i].portmanteau().initialize(morphology);
                /// also keep a list of the portmanteau for later
                mPortmanteauSequences.insert(ws, mAllomorphs.at(i).portmanteau().morphemes() );
            }
        }
    }
}

void MorphemeNode::addCreateAllomorphs(const CreateAllomorphs &ca)
{
    mCreateAllomorphs << ca;
}

void MorphemeNode::generateAllomorphs()
{
    for(int i=0; i < mCreateAllomorphs.count(); i++ )
    {
        QSet<Allomorph> newAllomorphs;
        for(int j=0; j < mAllomorphs.count(); j++ )
        {
            newAllomorphs.unite( mCreateAllomorphs.at(i).generateAllomorphs( mAllomorphs.at(j) ) );
        }
        mAllomorphs = newAllomorphs.values();
    }
}

void MorphemeNode::filterOutPortmanteauClashes(QList<Parsing> &candidates, const WritingSystem & ws) const
{
    if( mPortmanteauSequences.count() > 0 ) /// little efficiency
    {
        for(int i=0; i<candidates.count(); i++)
        {
            if( candidates.at(i).hasPortmanteauClash(mPortmanteauSequences, ws) )
            {
                candidates.removeAt(i);
                i--;
            }
        }
    }
}

QString MorphemeNode::summaryWithoutFollowing() const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "MorphemeNode(" << newline;
    dbg.indent();
    dbg << "Label: " << label() << newline;
    dbg << "Type: " << AbstractNode::typeToString(type()) << newline;
    dbg << "Optional: " << (optional() ? "true" : "false" ) << newline;
    dbg << "Has path to end: " << ( hasPathToEnd() ? "true" : "false" ) << newline;

    dbg << mAllomorphs.count() << " allomorph(s)," << newline;
    foreach( Allomorph a, mAllomorphs )
    {
        dbg << a.summary() << newline;
    }
    foreach( Form gloss, mGlosses )
    {
        dbg << gloss.summary("Gloss") << newline;
    }
    dbg.unindent();
    dbg << ")" << newline;

    return dbgString;
}

QString MorphemeNode::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << summaryWithoutFollowing();

    if( AbstractNode::hasNext() )
    {
        dbg << AbstractNode::next()->summary();
    }

    return dbgString;
}
