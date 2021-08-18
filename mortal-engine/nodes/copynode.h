#ifndef COPYNODE_H
#define COPYNODE_H

#include "abstractnode.h"

class QString;
class AbstractNode;
class QXmlStreamReader;
class MorphologyXmlReader;
class MorphologicalModel;

class CopyNode : public AbstractNode
{
public:
    CopyNode(const MorphologicalModel * model);
    ~CopyNode() override;
    AbstractNode * copy(MorphologyXmlReader *morphologyReader, const QString & idSuffix) const override;

    QList<Parsing> parsingsUsingThisNode( const Parsing & parsing, Parsing::Flags flags) const override;
    QList<Generation> generateFormsUsingThisNode( const Generation & generation ) const override;

    const AbstractNode *followingNodeHavingLabel(const QString & targetLabel) const override;
    bool checkHasOptionalCompletionPath() const override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

    void setCopy(AbstractNode *copy);
    void setNext(AbstractNode *next) override;

    QString summary() const override;

    bool optional() const override;

private:
    AbstractNode * mCopy;
};

#endif // COPYNODE_H
