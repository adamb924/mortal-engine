#ifndef MORPHEMENODE_H
#define MORPHEMENODE_H

#include "abstractnode.h"
#include <QList>
#include "datatypes/parsing.h"
#include "datatypes/allomorph.h"
#include "create-allomorphs/createallomorphs.h"

class MorphemeNode : public AbstractNode
{
public:
    explicit MorphemeNode(const MorphologicalModel * model);
    ~MorphemeNode() override;

    void addAllomorph(const Allomorph &allomorph );

    /**
     * @brief Returns a string representation of the MorphemeNode for logging purposes. The function calls the following node as well.
     * 
     * @return QString The logging output.
     */
    QString summary() const override;

    /**
     * @brief Returns a string representation of the MorphemeNode for logging purposes. This method does not display the following function, which makes it appropriate for use in a MEM context.
     *
     * @return QString The logging output.
     */
    QString summaryWithoutFollowing() const;

    QList<Generation> generateFormsWithAllomorphs(const Generation & generation, QSet<Allomorph> &potentialAllomorphs) const;

    QSet<Allomorph> matchingAllomorphs( const Parsing & parsing ) const;
    void matchingAllomorphs(const Generation &generation, QSet<Allomorph> &portmanteau, QSet<Allomorph> &nonPortmanteau ) const;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    /// this lives here because Allomorph is not a node; perhaps should be reconsidered
    static Allomorph readAllomorphTag(MorphemeNode *morpheme, QXmlStreamReader &in, MorphologyXmlReader *morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);

    /// these are special methods for handling portmanteau
    bool hasNext(const Allomorph & appendedAllomorph, const WritingSystem &ws) const;
    const AbstractNode *next(const Allomorph & appendedAllomorph, const WritingSystem & ws) const;

    void initializePortmanteaux(const Morphology *morphology);

    void addCreateAllomorphs( const CreateAllomorphs & ca );
    void generateAllomorphs();

    QList<Allomorph> allomorphs() const;

    bool hasForm(const Form & f) const;

    bool isMorphemeNode() const override;

    bool nodeCanAppendMorphemes() const override;

    void addGloss(const Form & gloss);
    int glossCount() const;

    void addConstraintsToAllAllomorphs(const QSet<const AbstractConstraint *> &constraints);

    bool hasZeroLengthForms() const;

private:
    QList<Parsing> parsingsUsingThisNode(const Parsing & parsing, Parsing::Flags flags) const override;
    void filterOutPortmanteauClashes(QList<Parsing> &candidates, const WritingSystem &ws) const;
    QList<Generation> generateFormsUsingThisNode( const Generation & parsing) const override;

    QList<Allomorph> mAllomorphs;
    QList<CreateAllomorphs> mCreateAllomorphs;
    QMultiHash<WritingSystem, QStringList> mPortmanteauSequences;
};

#endif // MORPHEMENODE_H
