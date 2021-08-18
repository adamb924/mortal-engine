#ifndef MUTUALLYEXCLUSIVEMORPHEMES_H
#define MUTUALLYEXCLUSIVEMORPHEMES_H

#include "abstractnode.h"
#include <QSet>
#include "datatypes/parsing.h"

class MorphemeNode;
class Form;

class MutuallyExclusiveMorphemes : public AbstractNode
{
public:
    explicit MutuallyExclusiveMorphemes(const MorphologicalModel * model);
    ~MutuallyExclusiveMorphemes() override;
    MutuallyExclusiveMorphemes * copy(MorphologyXmlReader *morphologyReader, const QString & idSuffix) const override;

    void addMorpheme( MorphemeNode * m );

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const override;


    void setNext(AbstractNode *next) override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

    const AbstractNode *followingNodeHavingLabel(const QString & targetLabel) const override;

    bool checkHasOptionalCompletionPath() const override;

    void addConstraintsToAllMorphemes(const QSet<const AbstractConstraint *> &constraints);

private:
    QList<Parsing> parsingsUsingThisNode(const Parsing & parsing, Parsing::Flags flags) const override;
    QList<Generation> generateFormsUsingThisNode(const Generation &generation) const override;

private:
    QSet<MorphemeNode*> mMorphemes;
};

#endif // MUTUALLYEXCLUSIVEMORPHEMES_H
