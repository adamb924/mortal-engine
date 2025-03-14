#ifndef MORPHEMENODE_H
#define MORPHEMENODE_H

#include "abstractnode.h"
#include <QList>
#include "datatypes/parsing.h"
#include "datatypes/allomorph.h"
#include "create-allomorphs/createallomorphs.h"

namespace ME {

class MorphemeNode : public AbstractNode
{
public:
    explicit MorphemeNode(const MorphologicalModel * model);
    ~MorphemeNode() override;
    MorphemeNode * copy(MorphologyXmlReader *morphologyReader, const NodeId & idSuffix) const override;

    void addAllomorph(const Allomorph &allomorph );

    /**
     * @brief Returns a string representation of the MorphemeNode for logging purposes. The function calls the following node as well.
     * 
     * @return QString The logging output.
     */
    QString summary(const AbstractNode *doNotFollow) const override;

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
    static Allomorph readAllomorphTag(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);

    void initializePortmanteaux();

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

    QList<const AbstractNode *> availableMorphemeNodes(QHash<const Jump*,int> &jumps) const override;

    static QString XML_ALLOMORPH;
    static QString XML_GLOSS;

private:
    QList<Parsing> parsingsUsingThisNode(const Parsing & parsing, Parsing::Flags flags) const override;
    void filterOutPortmanteauClashes(QList<Parsing> &candidates, const WritingSystem &ws) const;
    QList<Generation> generateFormsUsingThisNode( const Generation & parsing) const override;

    QList<Allomorph> mAllomorphs;
    QList<CreateAllomorphs> mCreateAllomorphs;
    QMultiHash<WritingSystem, MorphemeSequence> mPortmanteauSequences;
};

} // namespace ME

#endif // MORPHEMENODE_H
