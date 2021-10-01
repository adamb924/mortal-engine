#ifndef ABSTRACTPATH_H
#define ABSTRACTPATH_H

#include "abstractnode.h"

class AbstractPath : public AbstractNode
{
public:
    explicit AbstractPath(const MorphologicalModel * model);
    ~AbstractPath() override;

    AbstractNode *initialNode() const;
    void setInitialNode(AbstractNode *initialNode);

    void setNext(AbstractNode *nextNode) override;

    const AbstractNode *followingNodeHavingLabel(const QString & targetLabel) const override;

    bool checkHasOptionalCompletionPath() const override;

    QList<const AbstractNode *> availableMorphemeNodes(QHash<const Jump*,int> &jumps) const override;

private:
    QList<Parsing> parsingsUsingThisNode(const Parsing & parsing, Parsing::Flags flags) const override;
    QList<Generation> generateFormsUsingThisNode( const Generation & parsing) const override;

protected:
    AbstractNode* mInitialNode;
};

#endif // ABSTRACTPATH_H
