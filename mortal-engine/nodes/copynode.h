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
