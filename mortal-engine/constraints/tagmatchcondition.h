#ifndef TAGMATCHCONDITION_H
#define TAGMATCHCONDITION_H

#include "abstractconstraint.h"
#include "datatypes/tag.h"

#include <QSet>

namespace ME {

class Allomorph;

class TagMatchCondition : public AbstractConstraint
{
public:
    enum Scope { NullScope, AnyPreceding, ImmediatelyPreceding, AnyFollowing, ImmediatelyFollowing, Self };
    enum TagMatchType { NullType, Any, All };

    TagMatchCondition(TagMatchCondition::Scope searchScope, TagMatchCondition::TagMatchType type);
    ~TagMatchCondition() override;

    void addTag(Tag t);
    void addTag(const QString & t);

    void addInteruptTag(Tag t);
    void addInteruptTag(const QString & t);

    bool matchesThisConstraint( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

    bool matchAnyPreceding(const Parsing * parsing) const;
    bool matchImmediatelyPreceding( const Parsing * parsing ) const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const QString & suffix = QString()) const override;

    static QString elementName();
    static AbstractConstraint *readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);

    static TagMatchCondition::Scope scopeFromString(const QString & string);
    static QString scopeString(TagMatchCondition::Scope scope);

    static TagMatchCondition::TagMatchType tagMatchTypeFromString(const QString & string);
    static QString tagMatchTypeString(TagMatchCondition::TagMatchType type);

    static QString XML_TAG_MATCH;
    static QString XML_MATCH_TAG;
    static QString XML_INTERUPTED_BY;

private:
    bool match(const Allomorph &allomorph ) const;
    bool matchInterrupt(const Allomorph &allomorph ) const;

private:
    TagMatchCondition::Scope mSearchScope;
    TagMatchCondition::TagMatchType mTagMatchType;
    QSet<Tag> mTags;
    QSet<Tag> mInterruptTags;
};

} // namespace ME

#endif // TAGMATCHCONDITION_H
