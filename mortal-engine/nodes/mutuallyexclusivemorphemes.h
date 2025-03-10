#ifndef MUTUALLYEXCLUSIVEMORPHEMES_H
#define MUTUALLYEXCLUSIVEMORPHEMES_H

#include "abstractnode.h"
#include <QSet>
#include "datatypes/parsing.h"
#include "mortal-engine_global.h"

namespace ME {

class MorphemeNode;
class Form;

class MORTAL_ENGINE_EXPORT MutuallyExclusiveMorphemes : public AbstractNode
{
public:
    explicit MutuallyExclusiveMorphemes(const MorphologicalModel * model);
    ~MutuallyExclusiveMorphemes() override;
    MutuallyExclusiveMorphemes * copy(MorphologyXmlReader *morphologyReader, const NodeId &idSuffix) const override;

    void addMorpheme( MorphemeNode * m );

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const AbstractNode *doNotFollow) const override;


    void setNext(AbstractNode *next) override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

    const AbstractNode *followingNodeHavingLabel(const MorphemeLabel & targetLabel, QHash<const Jump *, int> &jumps) const override;

    void addConstraintsToAllMorphemes(const QSet<const AbstractConstraint *> &constraints);

    QList<const AbstractNode *> availableMorphemeNodes(QHash<const Jump*,int> &jumps) const override;

    QSet<const MorphemeNode *> morphemes() const;

    bool isMutuallyExclusiveMorphemes() const override;

private:
    QList<Parsing> parsingsUsingThisNode(const Parsing & parsing, Parsing::Flags flags) const override;
    QList<Generation> generateFormsUsingThisNode(const Generation &generation) const override;

private:
    QSet<MorphemeNode*> mMorphemes;
};

} // namespace ME

#endif // MUTUALLYEXCLUSIVEMORPHEMES_H
