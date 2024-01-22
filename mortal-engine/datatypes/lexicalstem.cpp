#include "lexicalstem.h"

#include "allomorph.h"
#include "create-allomorphs/createallomorphs.h"
#include "debug.h"

LexicalStem::LexicalStem() : mId(-1), mOriginalAllomorph(Allomorph::Null)
{

}

LexicalStem::LexicalStem(const Allomorph & allomorph) : mId(-1), mOriginalAllomorph(Allomorph::Null)
{
    mAllomorphs << allomorph;
    if( allomorph.isOriginal() )
        mOriginalAllomorph = allomorph;
}

LexicalStem::LexicalStem(const LexicalStem &other) : mOriginalAllomorph(other.mOriginalAllomorph)
{
    /// make a deep copy of the Allomorphs since those are pointers
    mAllomorphs.clear();
    QListIterator<Allomorph> ai( other.mAllomorphs );
    while( ai.hasNext() )
    {
        mAllomorphs << ai.next();
    }

    mGlosses = other.mGlosses;
    mId = other.mId;
}

LexicalStem &LexicalStem::operator=(const LexicalStem &other)
{
    /// make a deep copy of the Allomorphs since those are pointers
    mAllomorphs.clear();
    QListIterator<Allomorph> ai( other.mAllomorphs );
    while( ai.hasNext() )
    {
        mAllomorphs << ai.next();
    }

    mGlosses = other.mGlosses;
    mId = other.mId;
    mOriginalAllomorph = other.mOriginalAllomorph;
    return *this;
}

bool LexicalStem::operator==(const LexicalStem &other) const
{
    /// NB (TODO?) a lexical stem matches iff it has identical allomorphs
    return mAllomorphs == other.mAllomorphs;
}

bool LexicalStem::operator==(qlonglong stemId) const
{
    return mId == stemId;
}

void LexicalStem::insert(const Allomorph & allomorph)
{
    mAllomorphs << allomorph;
    if( allomorph.isOriginal() )
        mOriginalAllomorph = allomorph;
}

void LexicalStem::remove(const Allomorph &allomorph)
{
    mAllomorphs.removeOne(allomorph);
}

QListIterator<Allomorph> LexicalStem::allomorphIterator() const
{
    return QListIterator<Allomorph>(mAllomorphs);
}

void LexicalStem::generateAllomorphs(const CreateAllomorphs &ca)
{
    QSet<Allomorph> replacementAllomorphs;
    QListIterator<Allomorph> i(mAllomorphs);
    while( i.hasNext() )
    {
        Allomorph a = i.next();
        if( a.type() == Allomorph::Original )
        {
            replacementAllomorphs.unite( ca.generateAllomorphs( a ) );
        }
    }
    mAllomorphs = QList<Allomorph>(replacementAllomorphs.begin(),replacementAllomorphs.end());
}

void LexicalStem::generateAllomorphs(const QList<CreateAllomorphs> &cas)
{
    for(int i=0; i < cas.count(); i++ )
    {
        QSet<Allomorph> newAllomorphs;
        QListIterator<Allomorph> iter(mAllomorphs);
        while( iter.hasNext() )
        {
            newAllomorphs.unite( cas.at(i).generateAllomorphs( iter.next() ) );
        }
        mAllomorphs = QList<Allomorph>(newAllomorphs.begin(), newAllomorphs.end());
    }
}

bool LexicalStem::hasAllomorph(const Allomorph & allomorph, bool matchConstraints) const
{
    QListIterator<Allomorph> i(mAllomorphs);
    if( matchConstraints )
    {
        while( i.hasNext() )
        {
            if( i.next() == allomorph )
                return true;
        }
    }
    else
    {
        while( i.hasNext() )
        {
            Allomorph a = i.next();
            if( a.forms() == allomorph.forms() && a.tags() == allomorph.tags() )
                return true;
        }
    }
    return false;
}

bool LexicalStem::hasAllomorph(const Form &form, const QSet<Tag> containingTags, const QSet<Tag> withoutTags, bool includeDerivedAllomorphs) const
{
    QListIterator<Allomorph> i(mAllomorphs);
    while( i.hasNext() )
    {
        Allomorph a = i.next();
        if( a.hasForm( form )
                && a.tags().contains( containingTags )
                && !a.tags().intersects( withoutTags )
                && ( includeDerivedAllomorphs || a.type() != Allomorph::Derived ) )
            return true;
    }
    return false;
}

bool LexicalStem::hasAllomorphWithForm(const Form &form) const
{
    QListIterator<Allomorph> i(mAllomorphs);
    while( i.hasNext() )
    {
        Allomorph a = i.next();
        if( a.hasForm( form ) )
            return true;
    }
    return false;
}

QHash<WritingSystem, Form> LexicalStem::glosses() const
{
    return mGlosses;
}

bool LexicalStem::hasGloss(const WritingSystem &ws) const
{
    return mGlosses.contains(ws);
}

Form LexicalStem::gloss(const WritingSystem &ws) const
{
    return mGlosses.value( ws, Form(ws, "") );
}

void LexicalStem::setGloss(const Form &gloss)
{
    mGlosses.insert( gloss.writingSystem(), gloss );
}

qlonglong LexicalStem::id() const
{
    return mId;
}

void LexicalStem::setId(const qlonglong &id)
{
    mId = id;
}

bool LexicalStem::isEmpty() const
{
    return mAllomorphs.count() == 0;
}

int LexicalStem::allomorphCount() const
{
    return mAllomorphs.count();
}

QListIterator<Allomorph> LexicalStem::allomorphs() const
{
    return mAllomorphs;
}

QString LexicalStem::liftGuid() const
{
    return mLiftGuid;
}

void LexicalStem::setLiftGuid(const QString &liftGuid)
{
    mLiftGuid = liftGuid;
}

Allomorph LexicalStem::displayAllomorph() const
{
    return mOriginalAllomorph;
}

void LexicalStem::initializePortmanteaux(const AbstractNode *parent)
{
    Q_UNUSED(parent)
}

QString LexicalStem::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);

    if( mId == -1 )
        dbg << QString("LexicalStem(%1 allomorph(s),").arg(mAllomorphs.count()) << newline;
    else
        dbg << QString("LexicalStem(ID: %1, %2 allomorph(s),").arg(mId).arg(mAllomorphs.count()) << newline;

    dbg.indent();

    QListIterator<Allomorph> i(mAllomorphs);
    while( i.hasNext() )
    {
        dbg << i.next().summary();
        if( i.hasNext() )
        {
            dbg << newline;
        }
    }

    dbg << newline;
    if( mGlosses.count() == 0 )
    {
        dbg << QString("No glosses") << newline;
    }
    else
    {
        foreach( Form f, mGlosses )
        {
            dbg << f.summary() << newline;
        }
    }
    dbg.unindent();
    dbg << ")";

    return dbgString;
}

QString LexicalStem::oneLineSummary() const
{
    return summary().replace(QRegularExpression("\\s+")," ").trimmed();
}

uint qHash(const LexicalStem &key)
{
    /// 2021-09-22: This hash is very rarely used, and never in this code,
    /// so I am not going to store a hash value.
    uint hash = qHash( key.glosses(), static_cast<uint>(qGlobalQHashSeed()) )  ^ qHash(key.id(), static_cast<uint>(qGlobalQHashSeed()));
    QListIterator<Allomorph> i = key.allomorphIterator();
    while( i.hasNext() )
        hash ^= qHash(i.next());
    return hash;
}
