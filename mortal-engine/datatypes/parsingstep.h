#ifndef PARSINGSTEP_H
#define PARSINGSTEP_H

class AbstractNode;
class QXmlStreamWriter;
class LexicalStem;
class Morphology;

#include "allomorph.h"
#include "lexicalstem.h"

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT ParsingStep
{
public:
    enum SummaryType { MorphemeGloss, MorphemeForm, MorphemeLabel };

    ParsingStep(const AbstractNode* node, const Allomorph & allomorph);
    ParsingStep(const AbstractNode* node, const Allomorph & allomorph, const LexicalStem &lexicalStem);

    const AbstractNode *node() const;
    const AbstractNode *lastNode(const WritingSystem &ws) const;
    bool lastNodeMatchesId(const QString & id) const;
    bool lastNodeMatchesLabel(const QString & label) const;

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

    QString summaryPortion(ParsingStep::SummaryType type, const WritingSystem & summaryDisplayWritingSystem, const WritingSystem &parsingWritingSystem, const QString & betweenMorphemes) const;

    static ParsingStep readFromXml(QXmlStreamReader &in, const Morphology * morphology, bool &ok, const QString & elementName = "parsing-step");
    static QList<ParsingStep> readListFromXml(QXmlStreamReader &in, const Morphology * morphology, const QString & untilElement, bool &ok, const QString & elementName = "parsing-step");

    static ParsingStep readFromXml(QDomElement parsing, const Morphology * morphology, bool &ok);
    static QList<ParsingStep> readListFromXml(QDomElement list, const Morphology * morphology, bool &ok);

    QString label(const WritingSystem &ws) const;

    void setIsStem(const bool &isStem);

private:
    const AbstractNode* mNode;
    Allomorph mAllomorph;
    LexicalStem mLexicalStem;
    bool mIsStem;
};

#endif // PARSINGSTEP_H
