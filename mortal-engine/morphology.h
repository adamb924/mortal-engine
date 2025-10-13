/*!
  \class Morphology
  \brief A class representing the morphology of a language. The most important data structure is a list of MorphologicalModel objects. The most important method is possibleParsings, which returns a list of possible parsings for a given input.

  MorphologicalModel is a model of a particular part of a morphology, e.g., nouns or verbs.
*/

#ifndef MORPHOLOGY_H
#define MORPHOLOGY_H

#include "mortal-engine_global.h"
#include "nodes/morphologicalmodel.h"

#include "datatypes/lexicalstem.h"

namespace ME {

class StemIdentityConstraint;
class MorphemeSequenceConstraint;
class AbstractStemList;
class LexicalStemInsertResult;
class XmlParsingLog;

using InputNormalizer = std::function<QString(QString)>;


class MORTAL_ENGINE_EXPORT Morphology
{
public:
    friend class MorphologyXmlReader;
    friend class MorphologyChecker;

    Morphology();
    virtual ~Morphology();
    Morphology(const Morphology &) = delete;
    Morphology &operator=(const Morphology &) = delete;

    /// Functions related to creating an object from XML
    void readXmlFile( const QString & path );
    bool isWellFormed(const Form & form) const;
    void clearData();

    /// Basic access functions
    QList<MorphologicalModel *> morphologicalModels() const;
    WritingSystem writingSystem(const QString & lang) const;
    QHash<QString, WritingSystem> writingSystems() const;

    /// Parsing/generating/transducing functions
    QList<Parsing> possibleParsings(const Form & form, Parsing::Flags flags = Parsing::None) const;
    QSet<Parsing> uniqueParsings(const Form & form, Parsing::Flags flags = Parsing::None) const;
    QList<Parsing> guessStem(const Form & form) const;
    QList<Generation> generateForms(const WritingSystem & ws, StemIdentityConstraint sic, MorphemeSequenceConstraint msc , const MorphologicalModel *model = nullptr) const;
    QList<Generation> generateForms(const WritingSystem & ws, const LexicalStem & stem, const MorphemeSequence & morphemeSequence, const MorphologicalModel *model = nullptr) const;
    QList<Generation> generateForms(const WritingSystem & ws, const Parsing & parsing) const;
    QList<Generation> replaceStemInto(const Form & husk, const LexicalStem kernel, const WritingSystem & outputWs ) const;
    QList<Generation> transduceInto(const Form & form, const WritingSystem & newWs) const;
    Generation getFirstTransduction(const Form & form, const WritingSystem & newWs) const;

    /// Normalization functions
    Form normalize(const Form & f) const;
    void setNormalizationFunction(const WritingSystem & forWs, InputNormalizer n);

    /// Lexicon functions
    QSet<const AbstractStemList *> getMatchingStemLists(const LexicalStem & stem) const;
    LexicalStemInsertResult addLexicalStem(const LexicalStem & stem);
    LexicalStemInsertResult replaceLexicalStem(const LexicalStem & stem);
    LexicalStem * getLexicalStem(qlonglong id) const;
    void removeLexicalStem(qlonglong id);

    /// returns all matching lexicalStems
    QList<LexicalStem *> searchLexicalStems( const Form & formSearchString ) const;
    QList<LexicalStem *> lexicalStems(const Form & form, const QSet<Tag> containingTags = QSet<Tag>(), const QSet<Tag> withoutTags = QSet<Tag>() ) const;
    QList<LexicalStem *> lexicalStems( const Allomorph & allomorph, bool matchConstraints = true ) const;

    /// NB: this function can throw an exception
    LexicalStem * uniqueLexicalStem( const Form & formSearchString ) const;

    /// Model check functions
    void printModelCheck(QTextStream &out) const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     *
     * @return QString The logging output.
     */
    QString summary() const;

    /// Querying the model
    //! \brief Returns a pointer to the node with the given id, or nullptr if it doesn't exist
    AbstractNode* getNodeFromId(const NodeId &id) const;

    //! \brief Returns a list of pointers to nodes with the given label, or an empty list if none exist
    QList<AbstractNode*> getNodesFromLabel(const MorphemeLabel &label) const;
    QList<MorphemeNode*> getMorphemeNodeFromLabel(const MorphemeLabel &label) const;
    MorphologicalModel *getMorphologicalModelFromLabel(const MorphemeLabel &label) const;
    void setNodeId(const NodeId &id, AbstractNode * node);
    QSet<AbstractStemList *> stemLists() const;

    bool isOk() const;

    static void setPath(const QString & path);

    QSet<const AbstractNode *> nodes() const;

    QString morphologyPath() const;

    const ParsingLog *parsingLog() const;

    void setDebugOutput(bool newDebugOutput);

    bool debugOutput() const;

    bool stemDebugOutput() const;
    void setStemDebugOutput(bool newStemDebugOutput);

private:
    QList<MorphologicalModel*> mMorphologicalModels;
    QHash<QString,WritingSystem> mWritingSystems;
    QHash<NodeId,AbstractNode*> mNodesById;
    QMultiHash<MorphemeLabel,AbstractNode*> mNodesByLabel;
    QMultiHash<MorphemeLabel,MorphemeNode*> mMorphemeNodesByLabel;
    QMultiHash<MorphemeLabel,MorphologicalModel*> mMorphologicalModelsByLabel;
    QSet<AbstractNode*> mNodes;
    QSet<AbstractStemList*> mStemAcceptingStemLists;
    QSet<AbstractStemList*> mStemLists;
    QSet<MorphemeNode*> mMorphemeNodes;
    bool mIsOk;
    QHash<WritingSystem,InputNormalizer> mNormalizationFunctions;
    static QRegularExpression endingStemId;
    QString mMorphologyPath;
    const XmlParsingLog * mParsingLog;
    bool mDebugOutput;
    bool mStemDebugOutput;

    static ParsingLog NULL_PARSING_LOG;
};

} // namespace ME

#endif // MORPHOLOGY_H
