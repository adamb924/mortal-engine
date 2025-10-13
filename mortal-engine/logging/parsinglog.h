#ifndef PARSINGLOG_H
#define PARSINGLOG_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

class QString;
#include <QSet>
#include "datatypes/allomorph.h"

namespace ME {

class Form;
class MorphologicalModel;
class AbstractNode;
class Parsing;
class StemIdentityConstraint;
class MorphemeSequenceConstraint;
class Generation;

class ParsingLog
{
public:
    ParsingLog();
    virtual ~ParsingLog();

    virtual void beginParse(const Form & f) const {}
    virtual void beginGenerate(const WritingSystem & ws, const StemIdentityConstraint & sic, const MorphemeSequenceConstraint & msc, const MorphologicalModel *model) const {}
    virtual void beginModel(const MorphologicalModel *model) const {}
    virtual void beginNode(const AbstractNode *node, const Parsing &parsing) const {}
    virtual void beginNode(const AbstractNode *node, const Generation &generation) const {}

    virtual void begin(const QString & elementName) const {}
    virtual void end() const {}

    virtual void addToStackTrace(Parsing & parsing, const QString & string) const {}

    virtual void summarizeMatchingAllomorphs(QSet<Allomorph> &portmanteau, QSet<Allomorph> &nonPortmanteau ) const {}

    virtual void constraintsSetSatisfactionSummary(const QString & elementName, const Parsing * parsing, const QSet<const AbstractConstraint *> & set, const AbstractNode *node, const Allomorph &allomorph) const {}
    virtual void longDistanceConstraintsSatisfactionSummary(const Parsing * parsing) const {}
    virtual void allomorphMatchSummary(const Parsing * parsing, const Allomorph &allomorph) const {}
    virtual void allomorphConditionMatchSummary(const Parsing * parsing, const Allomorph &allomorph) const {}

    virtual void completed(const Parsing & p) const {}

    virtual void parsingStatus(const Parsing & p) const {}

    virtual void output(const QString & elementName, const QString & information) const {}
    virtual void info(const QString & information) const {}
};

}

#pragma GCC diagnostic pop

#endif // PARSINGLOG_H
