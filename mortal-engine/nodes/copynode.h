/**
 * @file copynode.h
 * @author Adam Baker (adam@adambaker.org)
 * @brief A node that copies another node in the model.
 * The key to understanding the logic of this model is to recognize that the
 * node created a deep copy of whatever node is targeted and stores it in
 * mCopy. (This happens in CopyNode::readFromXml.) All of the functions
 * that access the node data just pass the call on to mCopy.
 * @version 0.1
 * @date 2020-10-26
 *
 *
 */

#ifndef COPYNODE_H
#define COPYNODE_H

#include "abstractnode.h"

class QString;
class QXmlStreamReader;


namespace ME {

class AbstractNode;
class MorphologyXmlReader;
class MorphologicalModel;

class CopyNode : public AbstractNode
{
public:
    explicit CopyNode(const MorphologicalModel * model);
    ~CopyNode() override;
    AbstractNode * copy(MorphologyXmlReader *morphologyReader, const QString & idSuffix) const override;

    QList<Parsing> parsingsUsingThisNode( const Parsing & parsing, Parsing::Flags flags) const override;
    QList<Generation> generateFormsUsingThisNode( const Generation & generation ) const override;

    const AbstractNode *followingNodeHavingLabel(const MorphemeLabel & targetLabel, QHash<const Jump *, int> &jumps) const override;
    bool checkHasOptionalCompletionPath() const override;

    QString id() const override;
    MorphemeLabel label() const override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

    void setCopy(AbstractNode *copy);
    void setNext(AbstractNode *next) override;

    AbstractNode *next() const override;
    bool hasNext() const override;

    QString summary(const AbstractNode *doNotFollow) const override;

    bool optional() const override;

    QList<const AbstractNode *> availableMorphemeNodes(QHash<const Jump*,int> &jumps) const override;

private:
    AbstractNode * mCopy;
};

} // namespace ME

#endif // COPYNODE_H
