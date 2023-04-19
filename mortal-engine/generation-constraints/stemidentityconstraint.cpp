#include "stemidentityconstraint.h"

#include "datatypes/lexicalstem.h"
#include "nodes/abstractnode.h"

StemIdentityConstraint::StemIdentityConstraint() : AbstractGenerationConstraint(AbstractGenerationConstraint::StemIdentityConstraint)
{

}

StemIdentityConstraint::StemIdentityConstraint(const QList<LexicalStem> &lexicalStems) : AbstractGenerationConstraint(AbstractGenerationConstraint::StemIdentityConstraint), mLexicalStems(lexicalStems)
{

}

StemIdentityConstraint::StemIdentityConstraint(const StemIdentityConstraint &other) : AbstractGenerationConstraint(AbstractGenerationConstraint::StemIdentityConstraint), mLexicalStems(other.mLexicalStems)
{
}

StemIdentityConstraint &StemIdentityConstraint::operator=(const StemIdentityConstraint &other)
{
    mLexicalStems = other.mLexicalStems;
    return *this;
}

const LexicalStem StemIdentityConstraint::currentLexicalStem() const
{
    if( mLexicalStems.count() > 0 )
        return mLexicalStems.first();
    else
        return LexicalStem();
}

void StemIdentityConstraint::addLexicalStem(const LexicalStem &lexicalStem)
{
    mLexicalStems << lexicalStem;
}

void StemIdentityConstraint::resolveCurrentStemRequirement()
{
    mLexicalStems.removeAt(0);
}

bool StemIdentityConstraint::hasStemRequirement() const
{
    return mLexicalStems.count() > 0;
}

bool StemIdentityConstraint::satisfied(const AbstractNode *node, const Allomorph & allomorph) const
{
    /// the constraint is satisfied trivially if it's not a proposal to add a stem
    if( node->type() != AbstractNode::StemNodeType )
        return true;

    if( mLexicalStems.isEmpty() )
    {
        qWarning() << "StemIdentityConstraint failing because there are no stems in the constraint. This is probably not desired behavior.";
        return false;
    }

    /// we're satisfied as long as the required lexical stem contains this allomorph (which means has its same forms, and the same tags)
    return mLexicalStems.first().hasAllomorph(allomorph);
}

QString StemIdentityConstraint::summary() const
{
    QString dbgString;
    QTextStream dbg(&dbgString);

    dbg << "StemIdentityConstraint (";
    QListIterator<LexicalStem> i(mLexicalStems);
    while( i.hasNext() )
    {
        dbg << i.next().summary();
        if( i.hasNext() )
        {
            dbg << ", ";
        }
    }
    dbg << ")";

    return dbgString;
}
