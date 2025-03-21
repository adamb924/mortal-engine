#include "precedingnodeconstraint.h"

#include <QtDebug>
#include <QXmlStreamReader>
#include <stdexcept>
#include "debug.h"
#include "datatypes/parsing.h"
#include "nodes/abstractnode.h"

using namespace ME;

PrecedingNodeConstraint::PrecedingNodeConstraint() : AbstractConstraint(AbstractConstraint::MatchCondition), mIdentifier(Null), mScope(ImmediatelyPreceding)
{

}

bool PrecedingNodeConstraint::matchesThisConstraint(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED(node)
    Q_UNUSED(allomorph)

    /// there has to be a preceding node
    if( parsing->steps().count() < 1 )
    {
        return false;
    }

    switch(mScope)
    {
    case AnyPreceding:
        return matchAnyPreceding(parsing);
    case ImmediatelyPreceding:
        return matchImmediatelyPreceding(parsing);
    }
}

bool PrecedingNodeConstraint::matchImmediatelyPreceding(const Parsing *parsing) const
{
    switch( mIdentifier )
    {
    case PrecedingNodeConstraint::Null:
        return false;
    case PrecedingNodeConstraint::Id:
        return parsing->steps().constLast().lastNodeMatchesId( NodeId(mIdentifierString) );
    case PrecedingNodeConstraint::Label:
        return parsing->steps().constLast().lastNodeMatchesLabel( MorphemeLabel(mIdentifierString) );
    }
}

bool PrecedingNodeConstraint::matchAnyPreceding(const Parsing *parsing) const
{
    switch( mIdentifier )
    {
    case PrecedingNodeConstraint::Null:
        return false;
    case PrecedingNodeConstraint::Id:
        return parsing->steps().constLast().anyNodeMatchesId( NodeId(mIdentifierString) );
    case PrecedingNodeConstraint::Label:
        return parsing->steps().constLast().anyNodeMatchesLabel( MorphemeLabel(mIdentifierString) );
    }
}

QString PrecedingNodeConstraint::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "PrecedingNodeConstraint"+suffix+" (";
    switch( mIdentifier )
    {
    case PrecedingNodeConstraint::Null:
        dbg << "NULL";
        break;
    case PrecedingNodeConstraint::Id:
        dbg << "id: " << mIdentifierString;
        break;
    case PrecedingNodeConstraint::Label:
        dbg << "label: " << mIdentifierString;
        break;
    }

    dbg << ")";

    return dbgString;
}

QString PrecedingNodeConstraint::elementName()
{
    return "preceding-node";
}

AbstractConstraint *PrecedingNodeConstraint::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_UNUSED(morphologyReader)

    Q_ASSERT( in.isStartElement() );
    PrecedingNodeConstraint *c = new PrecedingNodeConstraint;

    QXmlStreamAttributes attr = in.attributes();
    if( attr.hasAttribute("id") )
    {
        c->setIdentifier( PrecedingNodeConstraint::Id );
        c->setIdentifierString( attr.value("id").toString() );
    }
    else if( attr.hasAttribute("label") )
    {
        c->setIdentifier( PrecedingNodeConstraint::Label );
        c->setIdentifierString( attr.value("label").toString() );
    }
    else
    {
        std::string en = elementName().toUtf8().constData();
        throw std::runtime_error( "Error: no id or name attribute in " + en );
    }

    if( attr.hasAttribute("scope") )
    {
        if( attr.value("scope").toString() == "immediately-preceding" )
        {
            c->mScope = ImmediatelyPreceding;
        }
        else if( attr.value("scope").toString() == "any-preceding" )
        {
            c->mScope = AnyPreceding;
        }
    }

    /// get to the end element
    in.readNext();

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return c;
}

bool PrecedingNodeConstraint::matchesElement(QXmlStreamReader &in)
{
    return in.name() == elementName();
}

void PrecedingNodeConstraint::setIdentifierString(const QString &identifierString)
{
    mIdentifierString = identifierString;
}

void PrecedingNodeConstraint::setIdentifier(const PrecedingNodeConstraint::Identifier &identifier)
{
    mIdentifier = identifier;
}
