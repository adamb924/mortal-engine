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

#include "datatypes/parsing.h"
#include "mortal-engine_global.h"

class QXmlStreamWriter;

class Form;
class Generation;

class QXmlStreamReader;
class MorphologyXmlReader;
class MorphologicalModel;

#define MAYBE_RETURN_EARLY if( flags & Parsing::OnlyOneResult && candidates.count() > 0 ) { return candidates; }

class MORTAL_ENGINE_EXPORT AbstractNode
{
public:
    enum Type { StemNodeType, MorphemeNodeType, PathNodeType };

    AbstractNode(const MorphologicalModel * model, Type t = AbstractNode::MorphemeNodeType);

    /// Virtual Void Functions
    virtual ~AbstractNode() = 0;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    virtual QString summary() const;
    QString oneLineSummary() const;

    QList<Parsing> possibleParsings( const Parsing & parsing, Parsing::Flags flags) const;
    QList<Generation> generateForms( const Generation & generation ) const;
    bool appendIfComplete(QList<Generation> &candidates, const Generation & generation) const;
    void appendIfComplete(QList<Parsing> &candidates, const Parsing & parsing) const;

    /// Virtual Functions
    virtual void setNext(AbstractNode *next);

    QString label() const;
    void setLabel(const QString &label);

    AbstractNode::Type type() const;
    QString typeString() const;

    static QString typeToString(AbstractNode::Type type);

    const AbstractNode *next() const;
    bool hasNext() const;

    bool optional() const;
    void setOptional(bool optional);


    /**
     * @brief Writes an XML representation of the object to the specified writer.
     *
     * @param out The XML stream to be written to
     */
    void serialize(QXmlStreamWriter & out) const;
    void serialize(QDomElement & out) const;

    void readInitialNodeAttributes(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader);

    QString id() const;
    void setId(const QString &id);

    bool isStemNode() const;

    void setNextNodeIfFinalNode(AbstractNode *nextNode);

    virtual const AbstractNode *followingNodeHavingLabel(const QString & targetLabel) const;

    void calculateModelProperties();
    bool hasPathToEnd() const;


    virtual bool checkHasOptionalCompletionPath() const;
    virtual bool isFork() const;
    virtual bool isSequence() const;
    virtual bool isModel() const;
    virtual bool isMorphemeNode() const;
    virtual bool isJump() const;

    virtual bool nodeCanAppendMorphemes() const;

    const MorphologicalModel *model() const;

    Form gloss(const WritingSystem & ws) const;
    QHash<WritingSystem,Form> glosses() const;

    static QString XML_OPTIONAL;
    static QString XML_ADD_ALLOMORPHS;

protected:
    QHash<WritingSystem,Form> mGlosses;

private:
    virtual QList<Parsing> parsingsUsingThisNode( const Parsing & parsing, Parsing::Flags flags) const = 0;
    virtual QList<Generation> generateFormsUsingThisNode( const Generation & generation ) const = 0;

    const MorphologicalModel * mModel;
    QString mLabel;
    AbstractNode::Type mType;
    AbstractNode* mNext;
    bool mOptional;
    QString mId;
    bool mHasPathToEnd;
};

#endif // ABSTRACTNODE_H
