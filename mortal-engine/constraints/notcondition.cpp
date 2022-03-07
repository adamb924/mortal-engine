#include "notcondition.h"

#include <QTextStream>
#include <QtDebug>
#include <QXmlStreamReader>

#include "morphologyxmlreader.h"
#include "datatypes/parsing.h"
#include "debug.h"

NotCondition::NotCondition()
{

}

NotCondition::~NotCondition()
{

}

bool NotCondition::matchesThisConstraint(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    QSetIterator<const AbstractConstraint *> i(mConstraints);
    while( i.hasNext() )
    {
        const AbstractConstraint * ac = i.next();
        if( ac->matches(parsing, node, allomorph) )
        {
            return false;
        }
    }

    return true;
}

QString NotCondition::satisfactionSummary(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    bool result = matches(parsing, node, allomorph);
    QString suffix = result ? "+" : "*";

    dbg << "NotCondition"+suffix+" ( ";
    QSetIterator<const AbstractConstraint *> i(mConstraints);
    while( i.hasNext() )
    {
        dbg << i.next()->satisfactionSummary(parsing,node,allomorph) << " ";
    }
    dbg << ")";

    return dbgString;
}

bool NotCondition::isNot() const
{
    return true;
}

QString NotCondition::elementName()
{
    return "not";
}

AbstractConstraint *NotCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_UNUSED(in)
    Q_UNUSED(morphologyReader)
    Q_ASSERT( in.isStartElement() );
    NotCondition * condition = new NotCondition;
    condition->readCommonAttributes(in);

    in.readNextStartElement();

    QSet<const AbstractConstraint *> constraints = morphologyReader->readConstraints(elementName(), in);
    condition->setConstraints( constraints );

    if( constraints.count() > 1 )
    {
        qWarning() << "Line: " << in.lineNumber() << ". More than one tag in a not condition. Condition will fail if any of the conditions is true. (Wrap the conditions in an <or> element to get rid of this message.)";
    }

    morphologyReader->registerNestedConstraint(condition);

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return condition;
}

bool NotCondition::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

QString NotCondition::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "NotCondition"+suffix+" (" << newline;
    dbg.indent();
    QSetIterator<const AbstractConstraint *> i(mConstraints);
    while( i.hasNext() )
    {
        dbg << i.next()->summary() << newline;
    }
    dbg.unindent();
    dbg << ")";

    return dbgString;
}
