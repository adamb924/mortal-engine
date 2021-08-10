#include "phonologicalcondition.h"

#include <QHash>
#include <QTextStream>
#include <QRegularExpression>
#include <QtDebug>
#include <QXmlStreamReader>

#include "datatypes/parsing.h"
#include "morphology.h"
#include "morphologyxmlreader.h"
#include "debug.h"

PhonologicalCondition::PhonologicalCondition() : AbstractConstraint(AbstractConstraint::MatchCondition)
{

}

PhonologicalCondition::~PhonologicalCondition()
{

}

bool PhonologicalCondition::matches(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED(node)
    Q_UNUSED(allomorph)
    Form soFar = parsing->parsedSoFar();
    QRegularExpression re = mRegularExpressions.value( parsing->writingSystem() );
    QRegularExpressionMatch match = re.match(soFar.text());
    return match.hasMatch();
}

void PhonologicalCondition::addRegularExpression(const WritingSystem &ws, const QRegularExpression &re)
{
    mRegularExpressions.insert(ws, re);
}

QString PhonologicalCondition::elementName()
{
    return "phonological-condition";
}

AbstractConstraint *PhonologicalCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_ASSERT( in.isStartElement() );
    PhonologicalCondition *c = new PhonologicalCondition;
    c->readId(in);

    while( !in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName() ) )
    {
        in.readNext();
        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == AbstractConstraint::XML_MATCH_EXPRESSION )
            {
                /// this'll all get optimized by the compiler I assume
                WritingSystem ws = morphologyReader->morphology()->writingSystem( in.attributes().value("lang").toString() );
                c->addRegularExpression( ws, QRegularExpression( in.readElementText() ) );
            }
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return c;
}

bool PhonologicalCondition::matchesElement(QXmlStreamReader &in)
{
    return in.name() == elementName()
            && in.attributes().value("type").toString() == "preceding";
}

QString PhonologicalCondition::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "PhonologicalCondition" + suffix + " (" << mRegularExpressions.count() << ": ";
    QListIterator<WritingSystem> i( mRegularExpressions.keys() );
    while( i.hasNext() )
    {
        WritingSystem ws = i.next();
        dbg << mRegularExpressions.value(ws).pattern() << " (" << ws.abbreviation() << ")";
        if( i.hasNext() )
        {
            dbg << "; ";
        }
    }
    dbg << ")";

    return dbgString;
}
