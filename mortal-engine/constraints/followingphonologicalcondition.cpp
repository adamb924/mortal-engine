#include "followingphonologicalcondition.h"

#include <QHash>
#include <QTextStream>
#include <QRegularExpression>
#include <QtDebug>
#include <QXmlStreamReader>

#include "datatypes/form.h"
#include "datatypes/parsing.h"
#include "datatypes/allomorph.h"
#include "morphologyxmlreader.h"
#include "morphology.h"
#include "debug.h"

FollowingPhonologicalCondition::FollowingPhonologicalCondition() : AbstractConstraint(AbstractConstraint::LocalConstraint)
{

}

FollowingPhonologicalCondition::~FollowingPhonologicalCondition()
{

}

bool FollowingPhonologicalCondition::matches(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED( node )

    /// When considering a candidate allomorph, we need to look at the text of that allomorph
    Form remainder = allomorph.form( parsing->writingSystem() );
    QRegularExpression re = mRegularExpressions.value( parsing->writingSystem() );
    QRegularExpressionMatch match = re.match(remainder.text());
    return match.hasMatch();
}

void FollowingPhonologicalCondition::addRegularExpression(const WritingSystem &ws, const QRegularExpression &re)
{
    mRegularExpressions.insert(ws, re);
}

QString FollowingPhonologicalCondition::elementName()
{
    return "phonological-condition";
}

AbstractConstraint *FollowingPhonologicalCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_ASSERT( in.isStartElement() );
    FollowingPhonologicalCondition *c = new FollowingPhonologicalCondition;
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

bool FollowingPhonologicalCondition::matchesElement(QXmlStreamReader &in)
{
    return in.name() == elementName()
            && in.attributes().value("type").toString() == "following";
}

QString FollowingPhonologicalCondition::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "FollowingPhonologicalCondition"+suffix+" (" << mRegularExpressions.count() << ": ";
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
