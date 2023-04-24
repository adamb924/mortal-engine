#ifndef JUMP_H
#define JUMP_H

#include "abstractnode.h"

class Jump : public AbstractNode
{
public:
    explicit Jump(const MorphologicalModel * model);
    ~Jump() override;
    Jump * copy(MorphologyXmlReader *morphologyReader, const QString &idSuffix) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const AbstractNode *doNotFollow) const override;


    void setNodeTarget(const AbstractNode *nodeTarget);

    void setTargetId(const QString &targetId);

    QString targetId() const;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

    const AbstractNode *followingNodeHavingLabel(const QString & targetLabel) const override;

    bool checkHasOptionalCompletionPath() const override;

    void setTargetNodeRequired(bool targetNodeRequired);

    bool isJump() const override;

    QList<const AbstractNode *> availableMorphemeNodes(QHash<const Jump*,int> &jumps) const override;

    virtual QSet<MorphemeSequence> possibleMorphemeSequences(const QSet<MorphemeSequence> given, QHash<const Jump *, int> &jumps) const override;

private:
    QList<Parsing> parsingsUsingThisNode(const Parsing & parsing, Parsing::Flags flags) const override;
    QList<Generation> generateFormsUsingThisNode( const Generation & parsing) const override;

private:
    const AbstractNode * mNodeTarget;
    QString mTargetId;
    bool mTargetNodeRequired;
};

#endif // JUMP_H
