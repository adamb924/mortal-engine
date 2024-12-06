#include "orcondition.h"

#include <QTextStream>
#include <QtDebug>

#include "morphologyxmlreader.h"
#include "datatypes/allomorph.h"

#include <QXmlStreamReader>
#include "debug.h"

using namespace ME;

OrCondition::OrCondition()
{

}

OrCondition::~OrCondition()
{

}

bool OrCondition::matchesThisConstraint(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    bool returnValue = false;
    QSetIterator<const AbstractConstraint *> i(mConstraints);
    while( i.hasNext() )
    {
        const AbstractConstraint * c = i.next();
        if( c->matches(parsing, node, allomorph) )
        {
            returnValue = true;
            break;
        }
    }
    return returnValue;
}

QString OrCondition::satisfactionSummary(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    bool result = matches(parsing, node, allomorph);
    QString suffix = result ? "+" : "*";

    dbg << "OrCondition"+suffix+" ( ";
    QSetIterator<const AbstractConstraint *> i(mConstraints);
    while( i.hasNext() )
    {
        dbg << i.next()->satisfactionSummary(parsing,node,allomorph) << " ";
    }
    dbg << ")";

    return dbgString;
}

QString OrCondition::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "OrCondition"+suffix+" (" << newline;
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

QString OrCondition::elementName()
{
    return "or";
}

AbstractConstraint *OrCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_ASSERT( in.isStartElement() );
    OrCondition * condition = new OrCondition;
    condition->readCommonAttributes(in);

    in.readNextStartElement();

    QSet<const AbstractConstraint *> constraints = morphologyReader->readConstraints(elementName(), in);
    condition->setConstraints( constraints );

    morphologyReader->registerNestedConstraint(condition);

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return condition;
}

bool OrCondition::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}
