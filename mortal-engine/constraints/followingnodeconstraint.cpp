#include "followingnodeconstraint.h"

#include <QtDebug>
#include <QXmlStreamReader>
#include <stdexcept>

#include "nodes/abstractnode.h"
#include "datatypes/allomorph.h"
#include "debug.h"

FollowingNodeConstraint::FollowingNodeConstraint() : AbstractConstraint(AbstractConstraint::LocalConstraint), mIdentifier(FollowingNodeConstraint::Null)
{

}

FollowingNodeConstraint::~FollowingNodeConstraint()
{

}

bool FollowingNodeConstraint::matches(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED(parsing)
    Q_UNUSED(node)
    Q_UNUSED(allomorph)

    switch( mIdentifier )
    {
    case FollowingNodeConstraint::Null:
        qWarning() << "FollowingNodeConstraint type is Null, which should not happen.";
        break;
    case FollowingNodeConstraint::Id:
        return node->id() == mIdentifierString;
    case FollowingNodeConstraint::Label:
        return node->label() == mIdentifierString;
    }
    return true;
}

QString FollowingNodeConstraint::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "FollowingNodeConstraint"+suffix+" (";
    switch( mIdentifier )
    {
    case FollowingNodeConstraint::Null:
        dbg << "NULL";
        break;
    case FollowingNodeConstraint::Id:
        dbg << "id: " << mIdentifierString;
        break;
    case FollowingNodeConstraint::Label:
        dbg << "label: " << mIdentifierString;
        break;
    }

    dbg << ")";

    return dbgString;
}

QString FollowingNodeConstraint::elementName()
{
    return "following-node";
}

AbstractConstraint *FollowingNodeConstraint::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_UNUSED(morphologyReader)

    Q_ASSERT( in.isStartElement() );
    FollowingNodeConstraint *c = new FollowingNodeConstraint;

    QXmlStreamAttributes attr = in.attributes();
    if( attr.hasAttribute("id") )
    {
        c->setIdentifier( FollowingNodeConstraint::Id );
        c->setIdentifierString( attr.value("id").toString() );
    }
    else if( attr.hasAttribute("label") )
    {
        c->setIdentifier( FollowingNodeConstraint::Label );
        c->setIdentifierString( attr.value("label").toString() );
    }
    else
    {
        std::string en = elementName().toUtf8().constData();
        throw std::runtime_error( "Error: no id or name attribute in " + en );
    }

    /// get to the end element
    in.readNext();

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return c;
}

bool FollowingNodeConstraint::matchesElement(QXmlStreamReader &in)
{
    return in.name() == elementName();
}

void FollowingNodeConstraint::setIdentifierString(const QString &identifierString)
{
    mIdentifierString = identifierString;
}

void FollowingNodeConstraint::setIdentifier(const Identifier &identifier)
{
    mIdentifier = identifier;
}
