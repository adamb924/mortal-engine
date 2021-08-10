#include "tagmatchcondition.h"

#include <QXmlStreamReader>

#include "datatypes/tag.h"
#include "datatypes/allomorph.h"
#include "datatypes/parsing.h"
#include "datatypes/parsingstep.h"
#include "debug.h"

QString TagMatchCondition::XML_TAG_MATCH = "tag-match";
QString TagMatchCondition::XML_MATCH_TAG = "match-tag";
QString TagMatchCondition::XML_INTERUPTED_BY = "interrupted-by";

TagMatchCondition::TagMatchCondition(TagMatchCondition::Scope searchScope, TagMatchCondition::TagMatchType type) :
    AbstractConstraint(AbstractConstraint::MatchCondition),
    mSearchScope(searchScope),
    mTagMatchType(type)
{

}

TagMatchCondition::~TagMatchCondition()
{

}

void TagMatchCondition::addTag(const QString &t)
{
    mTags.insert( Tag(t) );
}

void TagMatchCondition::addInteruptTag(Tag t)
{
    mInterruptTags.insert( t );
}

void TagMatchCondition::addInteruptTag(const QString &t)
{
    mInterruptTags.insert( Tag(t) );
}

bool TagMatchCondition::match(const Allomorph &allomorph) const
{
    if( mTags.count() == 0 )
    {
        return true;
    }

    /// if it's a hypothetical allomorph, give it pass on the tag condition
    if( allomorph.isHypothetical() )
    {
        return true;
    }

    switch( mTagMatchType )
    {
    case TagMatchCondition::All:
        return allomorph.tags().contains(mTags);
    case TagMatchCondition::Any:
        return mTags.intersects(allomorph.tags());
    case TagMatchCondition::NullType:
        qWarning() << "TagMatchCondition should not have type NullType.";
    }
    return false;
}

bool TagMatchCondition::matchInterrupt(const Allomorph &allomorph) const
{
    return mInterruptTags.intersects(allomorph.tags());
}

bool TagMatchCondition::matches(const Parsing *parsing, const AbstractNode *node, const Allomorph &allomorph) const
{
    Q_UNUSED(node)
    switch( mSearchScope )
    {
    case TagMatchCondition::AnyPreceding:
        return matchAnyPreceding(parsing);
    case TagMatchCondition::ImmediatelyPreceding:
        return matchImmediatelyPreceding(parsing);
    case TagMatchCondition::AnyFollowing:
        /// the only difference here is that TagMatchCondition::matches will be called at the end of the derivation
        return matchAnyPreceding(parsing);
    case TagMatchCondition::ImmediatelyFollowing:
        return match(allomorph);
    case TagMatchCondition::Self:
        return match( allomorph );
    case TagMatchCondition::NullScope:
        qWarning() << "TagMatchCondition should not have scope NullScope.";
    }
    return false;
}

bool TagMatchCondition::matchAnyPreceding(const Parsing *parsing) const
{
    if( mInterruptTags.isEmpty() )
    {
        foreach( ParsingStep ps, parsing->steps() )
        {
            if( match(ps.allomorph()) )
            {
                return true;
            }
        }
        return false;
    }
    else
    {
        bool precedingMatchWithoutInterrupt = false;
        QListIterator<ParsingStep> i( parsing->steps() );
        while(i.hasNext())
        {
            ParsingStep ps = i.next();
            if( match(ps.allomorph()) )
            {
                precedingMatchWithoutInterrupt = true;
            }
            if( matchInterrupt(ps.allomorph()) )
            {
                precedingMatchWithoutInterrupt = false;
            }
        }
        return precedingMatchWithoutInterrupt;
    }
}

bool TagMatchCondition::matchImmediatelyPreceding(const Parsing *parsing) const
{
    if( parsing->steps().count() == 0 )
    {
        return false;
    }

    ParsingStep ps = parsing->steps().constLast();
    if( match( ps.allomorph() ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

QString TagMatchCondition::elementName()
{
    return "tag-match";
}

AbstractConstraint *TagMatchCondition::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader)
{
    Q_UNUSED(morphologyReader)
    Q_ASSERT( in.isStartElement() );

    QXmlStreamAttributes attr = in.attributes();

    TagMatchCondition::TagMatchType type = tagMatchTypeFromString( attr.value("type").toString() );
    TagMatchCondition::Scope scope =  scopeFromString( attr.value("scope").toString() );

    TagMatchCondition * tmc = new TagMatchCondition(scope, type);
    tmc->readId(in);
    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == XML_TAG_MATCH ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == XML_MATCH_TAG )
            {
                tmc->addTag( in.readElementText() );
            }
            else if( in.name() == XML_INTERUPTED_BY )
            {
                tmc->addInteruptTag( in.readElementText() );
            }
        }
    }

    /// set the constraint type based on the scope
    switch( scope )
    {
    case TagMatchCondition::AnyPreceding:
    case TagMatchCondition::ImmediatelyPreceding:
    case TagMatchCondition::Self:
        tmc->setType( AbstractConstraint::MatchCondition );
        break;
    case TagMatchCondition::AnyFollowing:
        tmc->setType( AbstractConstraint::LongDistanceConstraint );
        break;
    case TagMatchCondition::ImmediatelyFollowing:
        tmc->setType( AbstractConstraint::LocalConstraint );
        break;
    case TagMatchCondition::NullScope:
        qWarning() << "TagMatchCondition should not have scope NullScope.";
    }

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );

    return tmc;
}

bool TagMatchCondition::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

TagMatchCondition::Scope TagMatchCondition::scopeFromString(const QString &string)
{
    if( string == "immediately-preceding" )
    {
        return TagMatchCondition::ImmediatelyPreceding;
    }
    else if( string == "any-preceding" )
    {
        return TagMatchCondition::AnyPreceding;
    }
    else if( string == "immediately-following" )
    {
        return TagMatchCondition::ImmediatelyFollowing;
    }
    else if( string == "any-following" )
    {
        return TagMatchCondition::AnyFollowing;
    }
    else if( string == "self" )
    {
        return TagMatchCondition::Self;
    }
    else
    {
        return TagMatchCondition::NullScope;
    }
}

QString TagMatchCondition::scopeString(TagMatchCondition::Scope scope)
{
    switch( scope )
    {
    case TagMatchCondition::AnyPreceding:
        return "any-preceding";
    case TagMatchCondition::ImmediatelyPreceding:
        return "immediately-preceding";
    case TagMatchCondition::AnyFollowing:
        return "any-following";
    case TagMatchCondition::ImmediatelyFollowing:
        return "immediately-following";
    case TagMatchCondition::NullScope:
        return "null-scope";
    case TagMatchCondition::Self:
        return "self";
    }
    return "null-scope";
}

TagMatchCondition::TagMatchType TagMatchCondition::tagMatchTypeFromString(const QString &string)
{
    if( string == "all" )
    {
        return TagMatchCondition::All;
    }
    else if( string == "any" )
    {
        return TagMatchCondition::Any;
    }
    else
    {
        return TagMatchCondition::NullType;
    }
}

QString TagMatchCondition::tagMatchTypeString(TagMatchCondition::TagMatchType type)
{
    switch( type )
    {
    case TagMatchCondition::Any:
        return "any";
    case TagMatchCondition::All:
        return "all";
    case TagMatchCondition::NullType:
        return "null-type";
    }
    return "null-type";
}

QString TagMatchCondition::summary(const QString &suffix) const
{
    QString dbgString;
    Debug dbg(&dbgString);

    dbg << "TagMatchCondition"+suffix+" (" << scopeString( mSearchScope ) << ", " << tagMatchTypeString( mTagMatchType ) << "; ";
    QStringList tagList;
    foreach( Tag t, mTags )
    {
        tagList << t.label();
    }
    dbg << tagList.join(", ");

    if( !mInterruptTags.isEmpty() )
    {
        QStringList interruptTagList;
        foreach( Tag t, mInterruptTags )
        {
            interruptTagList << t.label();
        }
        dbg << "; Interrupted by: " << interruptTagList.join(", ");
    }
    dbg << ")";

    return dbgString;
}

void TagMatchCondition::addTag(Tag t)
{
    mTags.insert(t);
}
