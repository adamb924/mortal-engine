#include "abstractnestedconstraint.h"

#include <QtDebug>
#include <stdexcept>

using namespace ME;

AbstractNestedConstraint::AbstractNestedConstraint() : AbstractConstraint(AbstractConstraint::Nested)
{

}

void AbstractNestedConstraint::setConstraints(const QSet<const AbstractConstraint *> &constraints)
{
    mConstraints = constraints;
}

void AbstractNestedConstraint::addConstraint(const AbstractConstraint *c)
{
    mConstraints << c;
}

bool AbstractNestedConstraint::isMatchCondition() const
{
    foreach( const AbstractConstraint * ac, mConstraints )
    {
        if( ! ac->isMatchCondition() )
        {
            return false;
        }
    }
    return true;
}

bool AbstractNestedConstraint::isLocalConstraint() const
{
    foreach( const AbstractConstraint * ac, mConstraints )
    {
        if( ! ac->isLocalConstraint() )
        {
            return false;
        }
    }
    return true;
}

bool AbstractNestedConstraint::isLongDistanceConstraint() const
{
    foreach( const AbstractConstraint * ac, mConstraints )
    {
        if( ! ac->isLongDistanceConstraint() )
        {
            return false;
        }
    }
    return true;
}

int AbstractNestedConstraint::count() const
{
    return mConstraints.count();
}

QSet<const AbstractConstraint *> AbstractNestedConstraint::children() const
{
    return mConstraints;
}

void AbstractNestedConstraint::checkIfConstraintsAreOfSameType() const
{
    if( mConstraints.count() == 0 ) return;

    bool isTheFirst = true;
    bool isMC=false, isLC=false, isLDC=false;
    QSetIterator<const AbstractConstraint*> i(mConstraints);
    while(i.hasNext())
    {
        const AbstractConstraint* c = i.next();

        /// first check to see if the constraint is itself a nested constraint with inconsistencies
        if( c->type() == AbstractConstraint::Nested )
        {
            dynamic_cast<const AbstractNestedConstraint*>(c)->checkIfConstraintsAreOfSameType();
        }

        if( isTheFirst )
        {
            isMC = c->isMatchCondition();
            isLC = c->isLocalConstraint();
            isLDC = c->isLongDistanceConstraint();
            isTheFirst = false;
        }
        else
        {
            if( isMC != c->isMatchCondition() || isLC != c->isLocalConstraint() || isLDC != c->isLongDistanceConstraint() )
            {
                std::string s = summary().toUtf8().constData();
                throw std::runtime_error( "Error in nested constraint. All daughter constraints must have the same type. " + s );
            }
        }
    }
}
