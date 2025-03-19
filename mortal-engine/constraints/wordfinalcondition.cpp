#include "wordfinalcondition.h"

#include <QtDebug>
#include <QXmlStreamReader>

#include "datatypes/parsing.h"
#include "datatypes/allomorph.h"
#include "datatypes/generation.h"
#include "debug.h"

using namespace ME;

WordFinalCondition::WordFinalCondition() : AbstractConstraint(LocalConstraint)
{

}

WordFinalCondition::~WordFinalCondition()
{

}

bool WordFinalCondition::matchesThisConstraint(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED(node)

    if( parsing->isGeneration() )
    {
        /// this seems hackish, but I'm not sure how else to make it work
        /// for generations the
        const Generation * g = dynamic_cast<const Generation*>( parsing );
        return g->morphemeSequenceConstraint()->remainingMorphemeCount() == 0;
    }
    else
    {
//        WritingSystem ws = parsing->writingSystem();
        /// The test is passed if this allomorph is the same length as the parsing,
        /// i.e., it will be the only morpheme in the parsing
        /// i.e., it will be at the end of the word
        /// 10/8/20: I wonder if this should rather be that the length of the parsing,
        /// plus the length of the allomorph should, should equal the parsing length
        /// does this just require that it be the only morpheme in the word?
        /// 10/21/2020: Adding the null option, because that means that no allomorph is being appended
//        if( ( parsing->steps().count() == 1 && allomorph.type() == Allomorph::Null )
//                || allomorph.form(ws).text().length() == parsing->form().text().length() )
        /// 2020-11-18: Evidentally a tough nut. This is a LocalConstraint, which will be
        /// evaluated when another allomorph is appended to the string. So if that allomorph
        /// is not null, the test fails. If it is null, the test succeeds (because appending the null
        /// allomorph means that we're at the end of the parse)
        if( allomorph.type() == Allomorph::Null )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

QString WordFinalCondition::elementName()
{
    return "word-final";
}

AbstractConstraint *WordFinalCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_UNUSED(morphologyReader)
    Q_ASSERT( in.isStartElement() );
    WordFinalCondition *c = new WordFinalCondition;

    while( !in.isEndElement() )
    {
        in.readNext();
    }

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return c;
}

bool WordFinalCondition::matchesElement(QXmlStreamReader &in)
{
    return in.name() == elementName();
}

QString WordFinalCondition::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "Word Final Constraint" + suffix;

    return dbgString;
}
