#ifndef ABSTRACTSTEMLIST_H
#define ABSTRACTSTEMLIST_H

#include "mortal-engine_global.h"

#include "abstractnode.h"
#include "datatypes/parsing.h"
#include "datatypes/allomorph.h"
#include "datatypes/lexicalstem.h"
#include "create-allomorphs/createallomorphs.h"

namespace ME {

class LexicalStem;

class MORTAL_ENGINE_EXPORT AbstractStemList : public AbstractNode
{
public:
    explicit AbstractStemList(const MorphologicalModel * model);
    ~AbstractStemList() override;
    AbstractStemList(const AbstractStemList &) = delete;
    AbstractStemList &operator=(const AbstractStemList &) = delete;

    AbstractStemList * copy(MorphologyXmlReader *morphologyReader, const NodeId &idSuffix) const override;

    virtual void readStems( const QHash<QString,WritingSystem> &writingSystems ) = 0;

    /// BEGIN STEM MODIFICATION FUNCTIONS

    //! \brief If any allomorph in \a stem meets the match() condition, add this stem to the database and return true. Otherwise delete the pointer and return false.
    bool addStem( LexicalStem * stem );

    //! \brief Replace a lexical stem having the same mId as \a stem with the data in \a stem
    bool replaceStem(const LexicalStem &stem );

    //!
    LexicalStem *getStem( qlonglong id ) const;

    bool matchesForInsert( const LexicalStem & stem ) const;
    QList<LexicalStem *> stemsFromAllomorph(const Form & form, const QSet<Tag> containingTags = QSet<Tag>(), const QSet<Tag> withoutTags = QSet<Tag>(), bool includeDerivedAllomorphs = false ) const;
    QList<LexicalStem *> stemsFromAllomorph(const Allomorph & allomorph, bool matchConstraints) const;
    LexicalStem * lexicalStem( const LexicalStem & stem ) const;

    bool removeLexicalStem(qlonglong id);

    //// END OF STEM FUNCTIONS

    QList< QPair<Allomorph,LexicalStem> > matchingAllomorphs(const Parsing &parsing) const;

    void addConditionTag(const QString & tag);

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const AbstractNode * doNotFollow) const override;

    void generateAllomorphsFromRules();

    void addCreateAllomorphs(const CreateAllomorphs &createAllomorphs);

    bool someStemContainsForm(const Form & f) const;

    bool nodeCanAppendMorphemes() const override;

    QList<const AbstractNode *> availableMorphemeNodes(QHash<const Jump*,int> &jumps) const override;

    static QString XML_FILENAME;
    static QString XML_MATCHING_TAG;

    QSet<LexicalStem *> stems() const;

    void initializePortmanteaux();

    template<typename T>
    void filterOutPortmanteauClashes(QList<T> &candidates) const;

private:
    QList<Parsing> parsingsUsingThisNode(const Parsing & parsing, Parsing::Flags flags) const override;
    QList<QPair<Allomorph, LexicalStem>> possibleStemForms(const Parsing & parsing) const;
    QList<Generation> generateFormsUsingThisNode(const Generation & generation) const override;

protected:
    virtual void insertStemIntoDataModel( LexicalStem * stem ) = 0;
    virtual void removeStemFromDataModel( qlonglong id ) = 0;

    bool match(const Allomorph &allomorph) const;

    QSet<LexicalStem*> mStems;
    QSet<Tag> mTags;
    QList<CreateAllomorphs> mCreateAllomorphs;
};

} // namespace ME

#endif // ABSTRACTSTEMLIST_H
