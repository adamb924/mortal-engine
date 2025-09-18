/*!
  \class AbstractNode
  \brief An abstract class that represents a node in a morpheme processing algorithm. The most important method is possibleParsings, which other nodes will call in order to determine what the possible parsings are, given this node and what follows it.

  The class is subclassed variously by by other files in the /morpheme-nodes/ folder.

  MorphologicalModel is an important subclass of AbstractNode.
*/

#ifndef ABSTRACTNODE_H
#define ABSTRACTNODE_H

#include <QString>
#include <QMap>

#include "datatypes/morphemelabel.h"
#include "datatypes/nodeid.h"
#include "datatypes/parsing.h"
#include "mortal-engine_global.h"

class QXmlStreamWriter;
class QXmlStreamReader;

#define MAYBE_RETURN_EARLY if( flags & ME::Parsing::OnlyOneResult && candidates.count() > 0 ) { return candidates; }

namespace ME {

class Form;
class Generation;
class MorphologyXmlReader;
class MorphologicalModel;

class MORTAL_ENGINE_EXPORT AbstractNode
{
public:
    enum Type { StemNodeType, MorphemeNodeType, PathNodeType };

    AbstractNode(const MorphologicalModel * model, Type t = AbstractNode::MorphemeNodeType);
    AbstractNode(const AbstractNode &) = delete;
    AbstractNode &operator=(const AbstractNode &) = delete;

    /// Virtual Void Functions
    virtual ~AbstractNode() = 0;

    /**
     * @brief Returns a deep copy of the object.
     */
    virtual AbstractNode * copy(MorphologyXmlReader *morphologyReader, const NodeId & idSuffix) const = 0;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    virtual QString summary(const AbstractNode * doNotFollow = nullptr) const;
    QString oneLineSummary() const;

    QList<Parsing> possibleParsings( const Parsing & parsing, Parsing::Flags flags) const;
    QList<Generation> generateForms( const Generation & generation ) const;
    bool appendIfComplete(QList<Generation> &candidates, const Generation & generation) const;
    void appendIfComplete(QList<Parsing> &candidates, const Parsing & parsing) const;

    /// Virtual Functions
    virtual void setNext(AbstractNode *next);

    virtual MorphemeLabel label() const;
    void setLabel(const MorphemeLabel &label);

    AbstractNode::Type type() const;
    QString typeString() const;

    static QString typeToString(AbstractNode::Type type);

    virtual AbstractNode *next() const;
    virtual bool hasNext() const;

    virtual bool optional() const;
    void setOptional(bool optional);


    /**
     * @brief Writes an XML representation of the object to the specified writer.
     *
     * @param out The XML stream to be written to
     */
    void serialize(QXmlStreamWriter & out) const;
    void serialize(QDomElement & out) const;

    void readInitialNodeAttributes(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader);

    virtual NodeId id() const;
    void setId(const NodeId &id);

    bool isStemNode() const;

    void setNextNodeIfFinalNode(AbstractNode *nextNode);

    /**
     * @brief Returns the first node in the same model following this node that has the label \a targetLabel, or nullptr if it is not found.
     *
     * @param targetLabel The node label to match
     *
     * This function is used in processing portmanteau strings. The fact that it does not
     * search for a node outside the current model implies that portmanteau can only be
     * valid for nodes within the same model.
     */
    virtual const AbstractNode *followingNodeHavingLabel(const MorphemeLabel & targetLabel, QHash<const Jump *, int> &jumps) const;

    void calculateModelProperties();
    bool hasPathToEnd() const;


    virtual bool checkHasOptionalCompletionPath() const;
    virtual bool isFork() const;
    virtual bool isSequence() const;
    virtual bool isModel() const;
    virtual bool isMorphemeNode() const;
    virtual bool isMutuallyExclusiveMorphemes() const;
    virtual bool isJump() const;

    virtual bool nodeCanAppendMorphemes() const;

    const MorphologicalModel *model() const;

    Form gloss(const WritingSystem & ws) const;
    QHash<WritingSystem,Form> glosses() const;

    static QString XML_OPTIONAL;
    static QString XML_ADD_ALLOMORPHS;

    virtual QString debugIdentifier() const;

    /// NB: in these function names "morpheme nodes" refers to morphemes that can contribute segments
    QList<const AbstractNode*> nextMorphemeNodes() const;
    virtual QList<const AbstractNode*> availableMorphemeNodes(QHash<const Jump*,int> &jumps) const = 0;

    /// these are special methods for handling portmanteau
    bool hasNext(const Allomorph & appendedAllomorph, const WritingSystem &ws) const;
    const AbstractNode *next(const Allomorph & appendedAllomorph, const WritingSystem & ws) const;

protected:
    QHash<WritingSystem,Form> mGlosses;

private:
    virtual QList<Parsing> parsingsUsingThisNode( const Parsing & parsing, Parsing::Flags flags) const = 0;
    virtual QList<Generation> generateFormsUsingThisNode( const Generation & generation ) const = 0;

    const MorphologicalModel * mModel;
    MorphemeLabel mLabel;
    AbstractNode::Type mType;
    AbstractNode* mNext;
    bool mOptional;
    NodeId mId;
    bool mHasPathToEnd;
};

} // namespace ME

#endif // ABSTRACTNODE_H
