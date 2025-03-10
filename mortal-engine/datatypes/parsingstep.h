#ifndef PARSINGSTEP_H
#define PARSINGSTEP_H

#include "allomorph.h"
#include "lexicalstem.h"
#include "datatypes/nodeid.h"

#include "mortal-engine_global.h"

class QXmlStreamWriter;

namespace ME {

class AbstractNode;
class LexicalStem;
class Morphology;

class MORTAL_ENGINE_EXPORT ParsingStep
{
public:
    ParsingStep(const AbstractNode* node, const Allomorph & allomorph);
    ParsingStep(const AbstractNode* node, const Allomorph & allomorph, const LexicalStem &lexicalStem);

    const AbstractNode *node() const;
    QList<const AbstractNode *> nodes(const WritingSystem &ws) const;
    const AbstractNode *lastNode(const WritingSystem &ws) const;
    bool lastNodeMatchesId(const NodeId &id) const;
    bool lastNodeMatchesLabel(const class MorphemeLabel & label) const;
    bool anyNodeMatchesId(const NodeId & id) const;
    bool anyNodeMatchesLabel(const class MorphemeLabel & label) const;

    Allomorph allomorph() const;

    LexicalStem lexicalStem() const;

    /**
     * @brief Writes an XML representation of the object to the specified writer.
     *
     * @param out The XML stream to be written to
     */
    void serialize(QXmlStreamWriter & out, bool includeGlosses = false) const;
    void serialize(QDomElement & out, bool includeGlosses) const;

    bool isStem() const;

    static ParsingStep readFromXml(QXmlStreamReader &in, const Morphology * morphology, bool &ok, const QString & elementName = "parsing-step");
    static QList<ParsingStep> readListFromXml(QXmlStreamReader &in, const Morphology * morphology, const QString & untilElement, bool &ok, const QString & elementName = "parsing-step");

    static ParsingStep readFromXml(QDomElement parsing, const Morphology * morphology, bool &ok);
    static QList<ParsingStep> readListFromXml(QDomElement list, const Morphology * morphology, bool &ok);

    MorphemeSequence morphemes(const WritingSystem &ws) const;

    void setIsStem(const bool &isStem);

private:
    const AbstractNode* mNode;
    Allomorph mAllomorph;
    /// these need to be an object rather than a pointer because even LexicalStems that don't exist in the model can be specified in generations
    LexicalStem mLexicalStem;
    bool mIsStem;
};

} // namespace ME

#endif // PARSINGSTEP_H
