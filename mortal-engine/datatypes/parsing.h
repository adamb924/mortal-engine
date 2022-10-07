/**
 * @file parsing.h
 * @author Adam Baker (adam@adambaker.org)
 * @brief A class representing the parsing of a particular Form.
 * @version 0.1
 * @date 2020-11-10
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef PARSING_H
#define PARSING_H

#include <QList>
#include "parsingstep.h"

#include "form.h"

class AbstractNode;
class MorphologicalModel;
class LexicalStem;
class StemIdentityConstraint;
class MorphemeSequenceConstraint;
class AbstractConstraint;
class Jump;
class Morphology;

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT Parsing
{
public:
    enum Status { Null, Ongoing, Failed, Completed };
    enum Flags {
        None = 0,
        GuessStem = 1 << 0,
        OnlyOneResult = 1 << 1
    };

    /**
     * @brief Construct a new Parsing object
     * 
     * @param form The Form to be parsed
     * @param morphologicalModel The model to be used in parsing the \a form
     */
    Parsing();
    Parsing(const Form & form, const MorphologicalModel * morphologicalModel);
    virtual ~Parsing();

    Parsing(const Parsing &other);
    Parsing &operator=(const Parsing &other);
    bool operator==(const Parsing & other) const;

    //! \section Basic data access

    /**
     * @brief Returns the Form that is being parsed.
     * 
     */
    Form form() const;

    /**
     * @brief Returns the WritingSystem of the the Form that is being parsed.
     * 
     */
    WritingSystem writingSystem() const;

    /**
     * @brief Returns the position (0-indexed) of the current position of the parsing.
     * 
    */
    int position() const;


    Status status() const;
    static QString statusToString(Parsing::Status status);
    static Status statusFromString(const QString & status);

    /**
     * @brief Returns the portion of the Form that has been parsed so far.
     * 
     */
    Form parsedSoFar() const;

    /**
     * @brief Returns the portion of the Form that remains to be parsed.
     * 
     */
    Form remainder() const;

    /**
     * @brief Returns the Form, starting from the specified position.
     * 
     */
    Form formFromPosition(int pos) const;

    /**
     * @brief Returns a natural language summary of the current state of the parsing.
     * 
     */
    virtual QString intermediateSummary() const;

    //! \section Parsing logic


    virtual void append(const AbstractNode* node, const Allomorph &allomorph, const LexicalStem &lexicalStem = LexicalStem(), bool isStem = false );

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;
    QString oneLineSummary() const;

    QList<ParsingStep> steps() const;

    bool allomorphMatches(const Allomorph &allomorph) const;
    bool allomorphMatchConditionsSatisfied(const Allomorph &allomorph) const;

    void allomorphConditionMatchStringSummary(const Allomorph &allomorph) const;

    bool atEnd() const;

    bool isNull() const;
    bool isCompleted() const;
    bool isOngoing() const;
    bool hasNotFailed() const;

    QString labelSummary( const WritingSystem & ws = WritingSystem() ) const;
    QString morphemeDelimitedSummary( const WritingSystem & ws ) const;
    QString hypotheticalStyleSummary( const WritingSystem & ws ) const;
    QString interlinearStyleSummary( const WritingSystem & ws = WritingSystem() ) const;
    QString stringSummary( ParsingStep::SummaryType stemType, const WritingSystem & stemWs, ParsingStep::SummaryType affixType, const WritingSystem & affixWs, const QString & beforeMorpheme, const QString & afterMorpheme, const QString & betweenMorphemes ) const;
    static QString parsingListSummary(const QList<Parsing> & list, const WritingSystem & ws = WritingSystem());
    static bool parsingsHaveSameStems( const QList<Parsing> & list );
    static bool parsingsHaveSameMorphologicalSequence( const QList<Parsing> & list );

    QStringList morphemeNames() const;

    //! Returns the text of the Allomorph whose node label is "Stem", or an empty form
    Form stem(const WritingSystem & ws) const;

    bool allConstraintsSatisfied() const;

    /**
     * @brief Writes an XML representation of the object to the specified writer.
     *
     * @param out The XML stream to be written to
     */
    void serialize(QXmlStreamWriter & out, bool includeGlosses = false) const;
    void serialize(QDomElement & out, bool includeGlosses = false) const;

    static Parsing readFromXml(QXmlStreamReader &in, const Morphology * morphology, const QString & elementName = "parsing");
    static QList<Parsing> readListFromXml(QXmlStreamReader &in, const Morphology * morphology, const QString & untilElement, const QString & elementName = "parsing");

    static Parsing readFromXml(QDomElement parsing, const Morphology * morphology);
    static QList<Parsing> readListFromXml(QDomElement list, const Morphology * morphology);

    //! \brief Returns the first non-null LexicalStem pointer in the parsing, or an empty LexicalStem if none is found
    LexicalStem firstLexicalStem() const;
    QList<LexicalStem> lexicalStems() const;
    bool containsStem(const LexicalStem & stem) const;
    bool containsStem(qlonglong stemId) const;
    Allomorph firstLexicalStemAllomorph() const;
    Allomorph firstHypotheticalAllomorph() const;

    const MorphologicalModel *morphologicalModel() const;

    virtual bool isGeneration() const;

    /// return true if the parsing contains, as a sequence, any of the morpheme sequences
    /// represented by the given portmanteau list; otherwise returns false
    bool hasPortmanteauClash(const QStringList & morphemes, const WritingSystem & ws ) const;
    bool hasPortmanteauClash(const QMultiHash<WritingSystem, QStringList> &portmanteaux, const WritingSystem & ws ) const;

    void incrementJumpCounter(const Jump * jump);
    bool jumpPermitted(const Jump * jump) const;

    bool nextNodeRequired() const;
    void setNextNodeRequired(bool nextNodeRequired);

    void addToStackTrace(const QString & str);

    void setSteps(const QList<ParsingStep> &steps);

    bool hasHypotheticalStem() const;

    uint hash() const;

    static int MAXIMUM_JUMPS;

protected:
    void setPosition(int position);

    void addLocalConstraints(const QSet<const AbstractConstraint *> &newConstraints);
    void addLongDistanceConstraints(const QSet<const AbstractConstraint *> &newConstraints);

    void setStatus(const Status &status);

    /// make these private to discourage finding partial matches
    bool allomorphMatchesSegmentally(const Allomorph &allomorph) const;

    bool constraintsSetSatisfied(const QSet<const AbstractConstraint *> & set, const AbstractNode *node, const Allomorph &allomorph) const;
    QString constraintsSetSatisfactionSummary(const QSet<const AbstractConstraint *> & set, const AbstractNode *node, const Allomorph &allomorph) const;

    bool longDistanceConstraintsSatisfied() const;
    QString longDistanceConstraintsSatisfactionSummary() const;


    QList<ParsingStep> mSteps;
    Form mForm;
    int mPosition;

    QSet<const AbstractConstraint *> mLocalConstraints;
    QSet<const AbstractConstraint *> mLongDistanceConstraints;

    void calculateHash();

private:
    Status mStatus;
    const MorphologicalModel * mMorphologicalModel;
    QString mMessage;
    QHash<const Jump*,int> mJumpCounts;
    bool mNextNodeRequired;
    QStringList mStackTrace;
    uint mHash;
};

Q_DECL_EXPORT uint qHash(const Parsing & key);

Q_DECL_EXPORT bool parsingLessThanStepwise(const Parsing &p1, const Parsing &p2);

#endif // PARSING_H
