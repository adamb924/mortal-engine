#ifndef XMLPARSINGLOG_H
#define XMLPARSINGLOG_H

#include <QFile>
#include <QXmlStreamWriter>

#include "logging/parsinglog.h"

namespace ME {

class XmlParsingLog : public ParsingLog
{
public:
    XmlParsingLog();
    virtual ~XmlParsingLog() override;

    void setStream(QXmlStreamWriter *stream) override;

    void beginParse(const Form & f) const override;
    void beginGenerate(const WritingSystem & ws, const StemIdentityConstraint & sic, const MorphemeSequenceConstraint & msc, const MorphologicalModel *model) const override;
    void beginModel(const MorphologicalModel *model) const override;
    void beginNode(const AbstractNode *node, const Parsing &parsing) const override;
    void beginNode(const AbstractNode *node, const Generation &generation) const override;

    void begin(const QString & elementName) const override;
    void end() const override;

    void addToStackTrace(Parsing & parsing, const QString & string) const override;

    void summarizeMatchingAllomorphs(QSet<Allomorph> &portmanteaux, QSet<Allomorph> &nonPortmanteaux ) const override;

    void constraintsSetSatisfactionSummary(const QString & elementName, const Parsing * parsing, const QSet<const AbstractConstraint *> & set, const AbstractNode *node, const Allomorph &allomorph) const override;
    void longDistanceConstraintsSatisfactionSummary(const Parsing * parsing) const override;
    void allomorphMatchSummary(const Parsing * parsing, const Allomorph &allomorph) const override;
    void allomorphConditionMatchSummary(const Parsing * parsing, const Allomorph &allomorph) const override;

    void completed(const Parsing & p) const override;

    void parsingStatus(const Parsing & p) const override;

    void output(const QString & elementName, const QString & information) const override;
    void info(const QString & information) const override;

private:
    void writeNodeAttributes(const AbstractNode * node) const;

    QXmlStreamWriter * xml;
};

}

#endif // XMLPARSINGLOG_H
