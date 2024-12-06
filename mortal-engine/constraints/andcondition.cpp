#include "andcondition.h"

#include <QTextStream>
#include <QtDebug>
#include <QXmlStreamReader>

#include "morphologyxmlreader.h"
#include "debug.h"

using namespace ME;

AndCondition::AndCondition()
{

}

AndCondition::~AndCondition()
{

}

bool AndCondition::matchesThisConstraint(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    QSetIterator<const AbstractConstraint *> i(mConstraints);
    while( i.hasNext() )
    {
        if( ! i.next()->matches(parsing, node, allomorph) )
        {
            return false;
        }
    }
    return true;
}

QString AndCondition::elementName()
{
    return "and";
}

AbstractConstraint *AndCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_ASSERT( in.isStartElement() );
    AndCondition * condition = new AndCondition;
    condition->readCommonAttributes(in);

    in.readNextStartElement();

    QSet<const AbstractConstraint *> constraints = morphologyReader->readConstraints(elementName(), in);
    condition->setConstraints( constraints );

    morphologyReader->registerNestedConstraint(condition);

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return condition;
}

bool AndCondition::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

QString AndCondition::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "AndCondition"+suffix+" (" << newline;
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

QString AndCondition::satisfactionSummary(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    bool result = matches(parsing, node, allomorph);
    QString suffix = result ? "+" : "*";

    dbg << "AndCondition"+suffix+" ( ";
    QSetIterator<const AbstractConstraint *> i(mConstraints);
    while( i.hasNext() )
    {
        dbg << i.next()->satisfactionSummary(parsing,node,allomorph) << " ";
    }
    dbg << ")";

    return dbgString;
}
