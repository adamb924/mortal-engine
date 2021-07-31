#include "pointertoconstraint.h"

#include <QTextStream>
#include <QtDebug>
#include <QXmlStreamReader>

#include "morphologyxmlreader.h"
#include "abstractlongdistanceconstraint.h"
#include "abstractnestedconstraint.h"
#include "debug.h"

PointerToConstraint::PointerToConstraint() : AbstractConstraint(Pointer), mTheConstraint(nullptr)
{

}

PointerToConstraint::~PointerToConstraint()
{

}

bool PointerToConstraint::matches(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    if( mTheConstraint == nullptr )
    {
        qCritical() << "PointerToConstraint::matches: mTheConstraint == nullptr. Returning false, which is at least 50-50.";
        return false;
    }
    switch( mTheConstraint->type() )
    {
    case AbstractConstraint::MatchCondition:
        return mTheConstraint->matches(parsing, node, allomorph);
    case AbstractConstraint::LocalConstraint:
        return mTheConstraint->matches(parsing, node, allomorph);
    case AbstractConstraint::LongDistanceConstraint:
        return mTheConstraint->toLongDistanceConstraint()->matches(parsing, node, allomorph);
    case AbstractConstraint::Nested:
        return mTheConstraint->toNestedConstraint()->matches(parsing, node, allomorph);
    case AbstractConstraint::Pointer:
        return false;
    }

    return mTheConstraint->matches(parsing, node, allomorph);
}

void PointerToConstraint::setTheConstraint(const AbstractConstraint *theConstraint)
{
    mTheConstraint = theConstraint;
}

QString PointerToConstraint::elementName()
{
    return "condition";
}

AbstractConstraint *PointerToConstraint::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_ASSERT( in.isStartElement() );
    PointerToConstraint * condition = new PointerToConstraint;

    QString id = in.attributes().value("id").toString();
    condition->setTargetId( id );
    morphologyReader->registerPointer( condition );

    /// read to 'end' of self-closing tag
    in.readNext();

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return condition;
}

bool PointerToConstraint::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

QString PointerToConstraint::targetId() const
{
    return mTargetId;
}

void PointerToConstraint::setTargetId(const QString &targetId)
{
    mTargetId = targetId;
}

const AbstractConstraint *PointerToConstraint::theConstraint() const
{
    return mTheConstraint;
}

bool PointerToConstraint::isMatchCondition() const
{
    return mTheConstraint->isMatchCondition();
}

bool PointerToConstraint::isLocalConstraint() const
{
    return mTheConstraint->isLocalConstraint();
}

bool PointerToConstraint::isLongDistanceConstraint() const
{
    return mTheConstraint->isLongDistanceConstraint();
}

QString PointerToConstraint::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    if( mTheConstraint == nullptr )
        dbg << "Pointer to " << mTargetId << ": NULL (error!)";
    else
        dbg << "Pointer to " << mTargetId + ": " << mTheConstraint->summary(suffix);

    return dbgString;
}

QString PointerToConstraint::satisfactionSummary(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    if( mTheConstraint == nullptr )
        dbg << "Pointer to NULL (error!)";
    else
        dbg << "Pointer to " << mTheConstraint->satisfactionSummary(parsing, node, allomorph);

    return dbgString;
}
