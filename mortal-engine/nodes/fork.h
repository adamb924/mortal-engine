#ifndef FORK_H
#define FORK_H

#include "abstractnode.h"
#include "path.h"

namespace ME {

class Fork : public AbstractNode
{
public:
    explicit Fork(const MorphologicalModel * model);
    ~Fork() override;
    Fork * copy(MorphologyXmlReader *morphologyReader, const NodeId &idSuffix) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const AbstractNode *doNotFollow) const override;

    void setNext(AbstractNode *next) override;

    void addPath(Path * p);

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

    const AbstractNode *followingNodeHavingLabel(const MorphemeLabel & targetLabel, QHash<const Jump *, int> &jumps) const override;

    bool checkHasOptionalCompletionPath() const override;

    bool isFork() const override;

    QList<const AbstractNode *> availableMorphemeNodes(QHash<const Jump*,int> &jumps) const override;

private:
    QList<Parsing> parsingsUsingThisNode(const Parsing & parsing, Parsing::Flags flags) const override;
    QList<Generation> generateFormsUsingThisNode( const Generation & generation) const override;

private:
    QList<Path*> mPaths;
};

} // namespace ME

#endif // FORK_H
