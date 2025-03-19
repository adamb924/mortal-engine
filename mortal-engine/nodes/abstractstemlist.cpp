#include "abstractstemlist.h"

#include "datatypes/lexicalstem.h"
#include "generation-constraints/stemidentityconstraint.h"
#include "datatypes/generation.h"
#include "morphology.h"

#include "debug.h"

using namespace ME;

QString AbstractStemList::XML_FILENAME = "filename";
QString AbstractStemList::XML_MATCHING_TAG = "matching-tag";


AbstractStemList::AbstractStemList(const MorphologicalModel *model) : AbstractNode(model, AbstractNode::StemNodeType)
{

}

AbstractStemList::~AbstractStemList()
{
    qDeleteAll(mStems);
}

AbstractStemList *AbstractStemList::copy(MorphologyXmlReader *morphologyReader, const NodeId &idSuffix) const
{
    Q_UNUSED(morphologyReader)
    Q_UNUSED(idSuffix)
    qCritical() << "Don't try to copy stem lists. The program is probably going to crash now.";
    return nullptr;
}

bool AbstractStemList::addStem(LexicalStem *stem)
{
    bool shouldInsert = matchesForInsert(*stem);

    if( shouldInsert )
    {
        LexicalStem * newStem = new LexicalStem( * stem );
        newStem->initializePortmanteaux(this);
        insertStemIntoDataModel( newStem );
    }

    return shouldInsert;
}

bool AbstractStemList::replaceStem(const LexicalStem & stem)
{
    bool removed = removeLexicalStem(stem.id());
    if( removed )
    {
        LexicalStem * newStem = new LexicalStem(stem);
        newStem->initializePortmanteaux(this);
        mStems.insert(newStem);
        insertStemIntoDataModel(newStem);
    }
    return removed;
}

LexicalStem *AbstractStemList::getStem(qlonglong id) const
{
    QSetIterator<LexicalStem*> i(mStems);
    while( i.hasNext() )
    {
        LexicalStem * current = i.next();
        if( current->id() == id )
        {
            return current;
        }
    }
    return nullptr;
}

bool AbstractStemList::matchesForInsert(const LexicalStem &stem) const
{
    QListIterator<Allomorph> iter = stem.allomorphIterator();
    while( iter.hasNext() )
    {
        Allomorph a = iter.next();
        if( match( a ) )
        {
            return true;
        }
    }
    return false;
}

QList<LexicalStem *> AbstractStemList::stemsFromAllomorph(const Form &form, const QSet<Tag> containingTags, const QSet<Tag> withoutTags, bool includeDerivedAllomorphs) const
{
    QList<LexicalStem *> stems;
    QSetIterator<LexicalStem*> i(mStems);
    while( i.hasNext() )
    {
        LexicalStem * current = i.next();
        if( current->hasAllomorph( form, containingTags, withoutTags, includeDerivedAllomorphs ) )
        {
            stems << current;
        }
    }
    return stems;
}

QList<LexicalStem *> AbstractStemList::stemsFromAllomorph(const Allomorph &allomorph, bool matchConstraints) const
{
    QList<LexicalStem *> stems;
    QSetIterator<LexicalStem*> i(mStems);
    while( i.hasNext() )
    {
        LexicalStem * current = i.next();
        if( current->hasAllomorph( allomorph, matchConstraints ) )
        {
            stems << current;
        }
    }
    return stems;
}

LexicalStem *AbstractStemList::lexicalStem(const LexicalStem &stem) const
{
    QSetIterator<LexicalStem*> i(mStems);
    while( i.hasNext() )
    {
        LexicalStem * current = i.next();
        if( *current == stem )
        {
            return current;
        }
    }
    return nullptr;
}

bool AbstractStemList::removeLexicalStem(qlonglong id)
{
    QSetIterator<LexicalStem*> i(mStems);
    while( i.hasNext() )
    {
        LexicalStem * current = i.next();
        if( current->id() == id )
        {            
            mStems.remove(current);
            removeStemFromDataModel(id);
            return true;
        }
    }
    return false;
}

QList<Parsing> AbstractStemList::parsingsUsingThisNode(const Parsing &parsing, Parsing::Flags flags) const
{
    QList<Parsing> candidates;

    QList< QPair<Allomorph, LexicalStem> > allomorphMatches = matchingAllomorphs(parsing);

    /// if the parsing is suppose to guess the stem, we should try all possible parsings
    /// but if the parsing already has a hypothetical stem, we shouldn't try to find another
    if( flags & Parsing::GuessStem
            && !parsing.hasHypotheticalStem() )
    {
        allomorphMatches.append( possibleStemForms(parsing) );
    }
    if( Morphology::DebugOutput )
    {
        qInfo() << debugIdentifier() << "has" << allomorphMatches.count() << "candidate stem matches.";
    }

    QListIterator< QPair<Allomorph, LexicalStem> > ai(allomorphMatches);
    while(ai.hasNext())
    {
        QPair<Allomorph, LexicalStem> pair = ai.next();
        Allomorph a = pair.first;
        LexicalStem ls = pair.second;
        Parsing p = parsing;
        p.append(this, a, ls, true);

        if( Morphology::DebugOutput && p.hasNotFailed() )
        {
            qInfo().noquote() << "\n\nStem match:" << a.oneLineSummary();
        }

        /// we want to move to the next node either 1) the parse hasn't been completed, or 2) there
        /// are null morphemes in the model, which we might want to append
        bool shouldTryToContinue = p.isOngoing() || model()->hasZeroLengthForms();
        if( hasNext(a, p.writingSystem()) && shouldTryToContinue ) /// more morphemes remain in the model
        {
            candidates.append( next(a, p.writingSystem())->possibleParsings( p, flags ) );
        }
        else /// there's nothing more to match; we have a successful, completed parse
        {
            appendIfComplete(candidates, p);
        }
    }

    filterOutPortmanteauClashes<Parsing>(candidates);

    return candidates;
}

QList<Generation> AbstractStemList::generateFormsUsingThisNode(const Generation &generation) const
{
    QList<Generation> candidates;

    /// if there is a stem identity constraint, then we know what stem to try
    if( generation.stemIdentityConstraint()->hasStemRequirement() )
    {
        LexicalStem s = generation.stemIdentityConstraint()->currentLexicalStem();
        /// check all the allomorphs of the stem
        QListIterator<Allomorph> ai = s.allomorphIterator();
        while(ai.hasNext())
        {
            Allomorph a = ai.next();
            /// make sure that the Allomorph has a form for the generation's writing system
            if( a.useInGenerations() && a.hasForm( generation.writingSystem() ) && generation.allomorphMatchConditionsSatisfied(a) ) /// this just checks for match conditions (e.g., tags)
            {
                if( Morphology::DebugOutput )
                {
                    qInfo().noquote() << "GENERATION Stem Match:" << a.oneLineSummary();
                }

                Generation g = generation;
                g.append(this, a, s, true);

                g.stemIdentityConstraint()->resolveCurrentStemRequirement();

                appendIfComplete( candidates, g );

                if( g.isOngoing() && hasNext() ) /// more morphemes remain in the model
                {
                    if( Morphology::DebugOutput )
                    {
                        qInfo() << "GENERATION Moving on, having appended:" << a.oneLineSummary();
                    }
                    candidates.append( next()->generateForms( g ) );
                }
            }
        }
    }

    filterOutPortmanteauClashes<Generation>(candidates);

    return candidates;
}

QList<QPair<Allomorph, LexicalStem> > AbstractStemList::matchingAllomorphs(const Parsing &parsing) const
{
    QList<QPair<Allomorph, LexicalStem> > list;

    /// disable debug output for the stem search, unless StemDebugOutput
    /// has been explicitly enabled
    bool oldDebugValue = Morphology::DebugOutput;
    if( Morphology::StemDebugOutput == false )
        Morphology::DebugOutput = false;

    /// cycle through each form
    foreach( LexicalStem *s, mStems )
    {
        QListIterator<Allomorph> ai = s->allomorphIterator();
        while(ai.hasNext())
        {
            Allomorph a = ai.next();
            /// stems should never be null morphemes, so we can check for that
            if( a.form( parsing.writingSystem() ).text().length() > 0 && parsing.allomorphMatches( a ) )
            {
                list << QPair<Allomorph, LexicalStem>( a, * s );
            }
        }
    }

    if( Morphology::StemDebugOutput == false )
        Morphology::DebugOutput = oldDebugValue;

    return list;
}

void AbstractStemList::addConditionTag(const QString &tag)
{
    mTags.insert( Tag(tag) );
}

QString AbstractStemList::summary(const AbstractNode *doNotFollow) const
{
    QString dbgString;
    Debug dbg(&dbgString);
    Debug::atBeginning = false;
    dbg << "StemList(" << Debug::endl;
    dbg.indent();
    dbg << "Label: " << label().toString() << Debug::endl;
    dbg << "Type: " << AbstractNode::typeToString(type()) << Debug::endl;
    dbg << "Optional: " << (optional() ? "true" : "false" ) << Debug::endl;
    dbg << "Has optional completion path: " << ( hasPathToEnd() ? "true" : "false" ) << Debug::endl;

    dbg << mStems.count() << " stems(s)" << Debug::endl;

    foreach( LexicalStem* s, mStems )
    {
        dbg << s->summary() << newline;
    }

    foreach( CreateAllomorphs ca, mCreateAllomorphs )
    {
        if( ca.hasCases() )
        {
            dbg << ca.summary() << newline;
        }
    }

    dbg.unindent();

    dbg << ")" << newline;

    if( hasNext() && next() != doNotFollow )
    {
        dbg << next()->summary(doNotFollow);
    }

    Debug::atBeginning = true;

    return dbgString;
}

bool AbstractStemList::match(const Allomorph &allomorph) const
{
    return allomorph.tags().contains(mTags);
}

QSet<LexicalStem *> AbstractStemList::stems() const
{
    return mStems;
}

void AbstractStemList::initializePortmanteaux()
{
    QSetIterator<LexicalStem*> i(mStems);
    while( i.hasNext() )
    {
        LexicalStem *stem = i.next();
        stem->initializePortmanteaux(this);
    }
}

template<typename T>
void AbstractStemList::filterOutPortmanteauClashes(QList<T> &candidates) const
{
    for(int i=0; i<candidates.count(); i++)
    {
        if( candidates.at(i).hasLexicalItemPortmanteauClash() )
        {
            candidates.removeAt(i);
            i--;
        }
    }
}

void AbstractStemList::addCreateAllomorphs(const CreateAllomorphs &createAllomorphs)
{
    mCreateAllomorphs << createAllomorphs;
}

bool AbstractStemList::someStemContainsForm(const Form &f) const
{
    QSetIterator<LexicalStem*> i(mStems);
    while(i.hasNext())
    {
        if( i.next()->hasAllomorphWithForm(f) )
            return true;
    }
    return false;
}

bool AbstractStemList::nodeCanAppendMorphemes() const
{
    return true;
}

QList<const AbstractNode *> AbstractStemList::availableMorphemeNodes(QHash<const Jump *, int> &jumps) const
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

QList<QPair<Allomorph, LexicalStem> > AbstractStemList::possibleStemForms(const Parsing &parsing) const
{
    QList<QPair<Allomorph, LexicalStem> > allomorphMatches;
    Form f = parsing.formFromPosition( parsing.position() );
    int maxLength = f.length();
    /// i=1 means that the stem must be at least one character long
    /// i<=maxlength means that the stem can be the entire remainder;
    /// it's up to the morphological model to determine whether the stem can be final in the word
    for(int i=1; i<=maxLength; i++)
    {
        Allomorph hypotheticalAllomorph( parsing.form().mid( parsing.position(), i ), Allomorph::Hypothetical );
        /// NB: hypothetical stems have no LexicalItem pointer
        allomorphMatches << QPair<Allomorph, LexicalStem>( hypotheticalAllomorph, LexicalStem() );
    }
    return allomorphMatches;
}

void AbstractStemList::generateAllomorphsFromRules()
{
    QSetIterator<LexicalStem*> stemIterator(mStems);
    
    while( stemIterator.hasNext() )
    {
        LexicalStem * stem = stemIterator.next();
        stem->generateAllomorphs( mCreateAllomorphs );
    }
}
