#ifndef ABSTRACTPATH_H
#define ABSTRACTPATH_H

#include "abstractnode.h"

namespace ME {

class MORTAL_ENGINE_EXPORT AbstractPath : public AbstractNode
{
public:
    explicit AbstractPath(const MorphologicalModel * model);
    ~AbstractPath() override;

    AbstractNode *initialNode() const;
    void setInitialNode(AbstractNode *initialNode);

    void setNext(AbstractNode *nextNode) override;

    const AbstractNode *followingNodeHavingLabel(const MorphemeLabel &targetLabel) const override;

    bool checkHasOptionalCompletionPath() const override;

    QList<const AbstractNode *> availableMorphemeNodes(QHash<const Jump*,int> &jumps) const override;

private:
    QList<Parsing> parsingsUsingThisNode(const Parsing & parsing, Parsing::Flags flags) const override;
    QList<Generation> generateFormsUsingThisNode( const Generation & parsing) const override;

protected:
    AbstractNode* mInitialNode;
};

} // namespace ME

#endif // ABSTRACTPATH_H
