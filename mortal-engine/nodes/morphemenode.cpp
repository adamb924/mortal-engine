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

using namespace ME;

QString MorphemeNode::XML_ALLOMORPH = "allomorph";
QString MorphemeNode::XML_GLOSS = "gloss";

MorphemeNode::MorphemeNode(const MorphologicalModel *model) : AbstractNode(model)
{

}

MorphemeNode::~MorphemeNode()
{
}

MorphemeNode *MorphemeNode::copy(MorphologyXmlReader *morphologyReader, const QString &idSuffix) const
{
    MorphemeNode * m = new MorphemeNode( model() );

    /// copy AbstractNode properties
    m->setLabel( label() );
    /// mType will be set by the constructor
    /// mNext will be set by the constructor
    m->setOptional( optional() );
    if( !id().isEmpty() )
    {
        m->setId( id() + idSuffix );
    }
    /// mHasPathToEnd should be calculated automatically
    m->mGlosses = mGlosses;

    /// copy local data
    m->mAllomorphs = mAllomorphs;
    m->mCreateAllomorphs = mCreateAllomorphs;
    m->mPortmanteauSequences = mPortmanteauSequences;

    morphologyReader->registerNode(m);

    return m;
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
        qInfo().noquote() << "In " << debugIdentifier() << "with" << parsing.intermediateSummary();
    }

    QSet<Allomorph> matches = matchingAllomorphs(parsing);

    if( Morphology::DebugOutput )
    {
        qInfo().noquote() << debugIdentifier() << " has " << matches.count() << " allomorph matches.";
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
                qInfo().noquote() << "Moving from" << debugIdentifier() << " to " << next(a, p.writingSystem())->debugIdentifier() << "having appended" << a.oneLineSummary();
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
            qInfo() << qPrintable("\t") << "Current node does not match the morpheme sequence constraint. The line above this one should have more information.";
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
        QSetIterator<Allomorph> matchesIterator( nonPortmanteau );
        while( matchesIterator.hasNext() )
        {
            qInfo().noquote() << "\t\t" << matchesIterator.next().oneLineSummary();
        }
        matchesIterator = QSetIterator<Allomorph>( portmanteaux );
        while( matchesIterator.hasNext() )
        {
            qInfo().noquote() << "\t\t" << matchesIterator.next().oneLineSummary();
        }
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
    return static_cast<int>( mGlosses.count() );
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

QList<const AbstractNode *> MorphemeNode::availableMorphemeNodes(QHash<const Jump *, int> &jumps) const
{
    QList<const AbstractNode *> list;
    list << this;

    /// move on to the next node if this one is optional
    if( optional() && AbstractNode::next() != nullptr )
    {
        list.append( AbstractNode::next()->availableMorphemeNodes(jumps) );
    }

    return list;
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
                qInfo().noquote() << "Moving from" << debugIdentifier() << " to " << next(a, g.writingSystem())->debugIdentifier() << "having appended" << a.oneLineSummary();
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
        if( a.useInGenerations() /// this allows the user to disable generations for particular allomorphs (e.g., weird portmanteaux)
                && a.hasForm(generation.writingSystem())
                && generation.allomorphMatchConditionsSatisfied(a) )
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
            if( in.name() == AbstractNode::XML_OPTIONAL )
            {
                morpheme->setOptional(true);
            }
            else if( in.name() == XML_ALLOMORPH )
            {
                morpheme->addAllomorph( readAllomorphTag(in, morphologyReader) );
            }
            else if( in.name() == AbstractNode::XML_ADD_ALLOMORPHS )
            {
                morpheme->addCreateAllomorphs( morphologyReader->createAllomorphsFromId( in.attributes().value("with").toString() ) );
            }
            else if( in.name() == XML_GLOSS )
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
Allomorph MorphemeNode::readAllomorphTag(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_ASSERT( in.isStartElement() );
    Allomorph allomorph(Allomorph::Original);

    if( in.attributes().hasAttribute("portmanteau") )
    {
        allomorph.setPortmanteau( Portmanteau( in.attributes().value("portmanteau").toString() ) );
    }

    if( in.attributes().hasAttribute( Allomorph::XML_USE_IN_GENERATIONS) )
    {
        allomorph.setUseInGenerations( in.attributes().value(Allomorph::XML_USE_IN_GENERATIONS) == Allomorph::XML_TRUE );
    }


    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_ALLOMORPH ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == Allomorph::XML_FORM )
            {
                QXmlStreamAttributes attr = in.attributes();
                if( attr.hasAttribute("lang") )
                {
                    WritingSystem ws = morphologyReader->morphology()->writingSystem( attr.value("lang").toString() );
                    allomorph.setForm( Form( ws, in.readElementText() ) );
                }
            }
            else if( in.name() == Allomorph::XML_TAG )
            {
                allomorph.addTag( in.readElementText() );
            }
            else
            {
                allomorph.addConstraint( morphologyReader->tryToReadConstraint(in) );
            }
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == XML_ALLOMORPH );
    return allomorph;
}

bool MorphemeNode::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

void MorphemeNode::initializePortmanteaux()
{
    QHash<MorphemeLabel,const AbstractNode *> cache;
    for( const auto& allomorph : mAllomorphs )
    {
        const auto& writingSystems = allomorph.writingSystems();
        for (const auto& ws : writingSystems)
        {
            if( allomorph.hasPortmanteau(ws) )
            {
                allomorph.portmanteau().initialize(this, cache);
                /// also keep a list of the portmanteau for later
                mPortmanteauSequences.insert(ws, allomorph.portmanteau().morphemes() );
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
    Debug::atBeginning = false;

    dbg << "MorphemeNode(" << newline;
    dbg.indent();
    dbg << "Label: " << label().toString() << newline;
    dbg << "ID: " << id() << newline;
    dbg << "Type: " << AbstractNode::typeToString(type()) << newline;
    dbg << "Optional: " << (optional() ? "true" : "false" ) << newline;
    dbg << "Has optional completion path: " << ( hasPathToEnd() ? "true" : "false" ) << newline;

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

    Debug::atBeginning = true;

    return dbgString;
}

QString MorphemeNode::summary(const AbstractNode *doNotFollow) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << summaryWithoutFollowing();

    if( AbstractNode::hasNext() && AbstractNode::next() != doNotFollow )
    {
        dbg << AbstractNode::next()->summary();
    }

    return dbgString;
}
