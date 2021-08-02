#ifndef MORPHOLOGYXMLREADER_H
#define MORPHOLOGYXMLREADER_H

#include "mortal-engine_global.h"

#include <QString>
#include <QSet>
#include <QHash>

class Morphology;
class AbstractNode;
class AbstractPath;

class QXmlStreamReader;
class Allomorph;
class QXmlStreamAttributes;
class WritingSystem;
class Parsing;

class AbstractStemList;
class PointerToConstraint;
class AbstractNode;
class AbstractNestedConstraint;
class AbstractConstraint;
class MorphologicalModel;
class MorphemeNode;
class Jump;

#include "create-allomorphs/createallomorphs.h"
#include "nodes/nodematcher.h"
#include "constraints/constraintmatcher.h"

class MORTAL_ENGINE_EXPORT MorphologyXmlReader
{
public:
    explicit MorphologyXmlReader(Morphology * morphology);

    void readXmlFile( const QString & path );

    Morphology *morphology() const;

    AbstractNode* readNodes(QXmlStreamReader &in, const QString &untilEndOf, const MorphologicalModel *currentModel);

    QSet<const AbstractConstraint *> readConstraints(const QString &untilEndOf, QXmlStreamReader &in);

    void recordStemAcceptingNewStems( AbstractStemList* stemList ) const;
    void recordStemList( AbstractStemList* stemList ) const;

    AbstractConstraint* tryToReadConstraint(QXmlStreamReader &in);
    bool currentNodeMatchesConstraint(QXmlStreamReader &in) const;

    CreateAllomorphs createAllomorphsFromId(const QString & id) const;

    void registerJump( Jump * j );
    void registerPointer( PointerToConstraint * pointer);
    void registerNode( AbstractNode * node) const;
    void registerNestedConstraint(const AbstractNestedConstraint *c);

    QHash<QString,WritingSystem> writingSystems() const;

private:
    void parseXml(const QString &path );

    void populateNodeHashes();
    /// NB: this currently only checks MorphemeNodes
    void checkForNonUniqueIdsAndLabels();

    //! These get a special function because it provides a list rather than a linked list
    void readMorphologicalModels(QXmlStreamReader &in);

    void readSharedConditions(QXmlStreamReader &in);
    void readSharedCreateAllomorphs(QXmlStreamReader &in);

    AbstractNode* tryToReadMorphemeNode(QXmlStreamReader &in, const MorphologicalModel *currentModel);

    void fillInJumpPointers();
    void fillInConstraintPointers();
    void generateAllomorphsFromRules();
    void parsePortmanteaux(); /// real plural or pseudo?
    void calculateModelProperties();
    void checkNestedConstraintConsistency();

    /// convenience method
    static bool notAtEndOf(const QString & tagName, QXmlStreamReader &in);

    template <class T> void registerConstraintMatcher();
    template <class T> void registerNodeMatcher();

private:
    Morphology * mMorphology;
    QHash<QString, const AbstractConstraint *> mConstraintsById;
    QHash<QString,CreateAllomorphs> mCreateAllomorphsById;

    /// Lists of classes that can be read
    QList<NodeMatcher> mNodeMatchers;
    QList<ConstraintMatcher> mConstraintMatchers;

    QSet<Jump*> mJumps;
    QSet<PointerToConstraint *> mPointers;
    QSet<AbstractNode*> mStemNodes;
    QSet<const AbstractNestedConstraint*> mNestedConstraints;
};

#endif // MORPHOLOGYXMLREADER_H
