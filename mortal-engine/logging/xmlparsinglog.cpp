#include "xmlparsinglog.h"

#include "messages.h"
#include "constraints/abstractlongdistanceconstraint.h"
#include "datatypes/form.h"
#include "datatypes/generation.h"
#include "nodes/abstractnode.h"
#include "nodes/morphologicalmodel.h"
#include "generation-constraints/stemidentityconstraint.h"
#include "generation-constraints/morphemesequenceconstraint.h"

using namespace ME;

XmlParsingLog::XmlParsingLog()
{
}

XmlParsingLog::~XmlParsingLog()
{
}

void XmlParsingLog::setStream(QXmlStreamWriter *stream)
{
    xml = stream;
}

void ME::XmlParsingLog::beginParse(const Form &f) const
{
    xml->writeStartElement("parsing");
    xml->writeAttribute("form",f.text());
    xml->writeAttribute("writing-system",f.writingSystem().abbreviation());
}

void ME::XmlParsingLog::end() const
{
    xml->writeEndElement();
}

void XmlParsingLog::addToStackTrace(Parsing &parsing, const QString &string) const
{
    parsing.addToStackTrace( string );
}

void XmlParsingLog::beginGenerate(const WritingSystem &ws, const StemIdentityConstraint &sic, const MorphemeSequenceConstraint &msc, const MorphologicalModel *model) const
{
    xml->writeStartElement("generation");
    if( model != nullptr )
        xml->writeAttribute("model",model->label().toString());
    xml->writeAttribute("writing-system",ws.abbreviation());
    xml->writeTextElement("stem-identity-constraint", sic.oneLineSummary());
    xml->writeTextElement("morpheme-sequence-constraint", msc.oneLineSummary());
}

void ME::XmlParsingLog::beginModel(const MorphologicalModel *model) const
{
    xml->writeStartElement("model");
    writeNodeAttributes(model);
}

void XmlParsingLog::beginNode(const AbstractNode *node, const Parsing & parsing) const
{
    xml->writeStartElement("at-node");
    writeNodeAttributes(node);
    if( node->optional() )
    {
        xml->writeStartElement("optional");
        xml->writeEndElement();
    }

    xml->writeStartElement("status");
    xml->writeAttribute("parsed", parsing.parsedSoFar().text() );
    xml->writeAttribute("parsed-as", parsing.labelSummary() );
    xml->writeAttribute("remaining", parsing.remainder().text() );
    xml->writeEndElement(); /// status
}

void XmlParsingLog::beginNode(const AbstractNode *node, const Generation &generation) const
{
    xml->writeStartElement("at-node");
    writeNodeAttributes(node);
    if( node->optional() )
    {
        xml->writeStartElement("optional");
        xml->writeEndElement();
    }

    xml->writeStartElement("status");
    xml->writeAttribute("generated", generation.form().text() );
    xml->writeAttribute("from", generation.morphemeSequenceConstraint()->originalSequence().toString() );
    xml->writeAttribute("summary", generation.morphemeDelimitedSummary( generation.form().writingSystem() ) );

    xml->writeEndElement(); /// status
}

void XmlParsingLog::begin(const QString &elementName) const
{
    xml->writeStartElement(elementName);
}

void XmlParsingLog::summarizeMatchingAllomorphs(QSet<Allomorph> &portmanteaux, QSet<Allomorph> &nonPortmanteaux) const
{
    xml->writeStartElement("allomorph");
    info( QObject::tr("%1 allomorph matches (%2 normal, %3 portmanteau)").arg( portmanteaux.count() + nonPortmanteaux.count() ).arg( nonPortmanteaux.count() ).arg( portmanteaux.count() ) );
    if( nonPortmanteaux.count() > 0 )
    {
        xml->writeStartElement("non-portmanteau");
        QSetIterator<Allomorph> matchesIterator( nonPortmanteaux );
        while( matchesIterator.hasNext() )
        {
            xml->writeTextElement("match", matchesIterator.next().oneLineSummary());
        }
        xml->writeEndElement(); /// non-portmanteau-matches
    }

    if( portmanteaux.count() > 0 )
    {
        xml->writeStartElement("portmanteau");
        QSetIterator<Allomorph> matchesIterator( portmanteaux );
        while( matchesIterator.hasNext() )
        {
            xml->writeTextElement("match", matchesIterator.next().oneLineSummary());
        }
        xml->writeEndElement(); /// portmanteau-matches
    }

    xml->writeEndElement(); /// allomorph
}

void XmlParsingLog::constraintsSetSatisfactionSummary(const QString &elementName, const Parsing * parsing, const QSet<const AbstractConstraint *> &set, const AbstractNode *node, const Allomorph &allomorph) const
{
    xml->writeStartElement(elementName);
    xml->writeAttribute("count", QString::number( set.count() ) );

    QSetIterator<const AbstractConstraint *> i( set );
    while( i.hasNext() )
    {
        const AbstractConstraint * c = i.next();
        if( c->matches(parsing, node, allomorph) )
        {
            xml->writeTextElement("satisfied", c->satisfactionSummary(parsing,node,allomorph) );
        }
        else
        {
            xml->writeTextElement("failed", c->satisfactionSummary(parsing,node,allomorph) );
        }
    }

    xml->writeEndElement(); /// elementName
}

void XmlParsingLog::longDistanceConstraintsSatisfactionSummary(const Parsing *parsing) const
{
    xml->writeStartElement("long-distance");
    xml->writeAttribute("count", QString::number( parsing->longDistanceConstraints().count() ) );

    QSetIterator<const AbstractConstraint *> i( parsing->longDistanceConstraints() );
    while( i.hasNext() )
    {
        const AbstractLongDistanceConstraint * c = i.next()->toLongDistanceConstraint();
        if( ! c->satisfied(parsing) )
        {
            xml->writeTextElement("satisfied", c->summary() );
        }
        else
        {
            xml->writeTextElement("failed", c->summary() );
        }
    }

    xml->writeEndElement(); /// long-distance-constraints
}

void XmlParsingLog::allomorphMatchSummary(const Parsing *parsing, const Allomorph &allomorph) const
{
    if( !allomorph.hasForm( parsing->writingSystem() ) )
        return;

    bool segmentalMatch = parsing->allomorphMatchesSegmentally(allomorph);
    bool conditionMatch = parsing->allomorphMatchConditionsSatisfied(allomorph);

    xml->writeStartElement("allomorph");
    xml->writeAttribute("form", allomorph.form( parsing->writingSystem() ).text());
    xml->writeAttribute("segmental-match", segmentalMatch ? "true" : "false");
    xml->writeAttribute("condition-match", conditionMatch ? "true" : "false");

    // if( !conditionMatch )
    allomorphConditionMatchSummary(parsing, allomorph);

    xml->writeEndElement(); /// allomorph
}

void XmlParsingLog::allomorphConditionMatchSummary(const Parsing *parsing, const Allomorph &allomorph) const
{
    if( allomorph.matchConditions().count() > 0 )
    {
        xml->writeStartElement("conditions");

        foreach(const AbstractConstraint * c, allomorph.matchConditions() )
        {
            if( c->matches(parsing, nullptr, allomorph ) )
            {
                xml->writeTextElement("satisfied", c->satisfactionSummary(parsing, nullptr, allomorph));
            }
            else
            {
                xml->writeTextElement("failed", c->satisfactionSummary(parsing, nullptr, allomorph));
            }
        }

        xml->writeEndElement(); /// conditions
    }
}

void XmlParsingLog::completed(const Parsing &p) const
{
    xml->writeStartElement("completed-parsing");
    xml->writeTextElement("form", p.form().summary() );
    xml->writeTextElement("label-summary", p.labelSummary() );
    xml->writeTextElement("complete-summary", p.oneLineSummary() );
    xml->writeEndElement(); /// completed-parsing
}

void XmlParsingLog::parsingStatus(const Parsing &p) const
{
    xml->writeStartElement("parsing");
    xml->writeAttribute("status", Parsing::statusToString(p.status()) );
    if( p.status() == Parsing::Completed )
    {
        xml->writeStartElement("trace");
        foreach(const QString & str, p.stackTrace())
        {
            xml->writeTextElement("node", str);
        }
        xml->writeEndElement(); /// trace
    }
    xml->writeEndElement(); /// parsing
}

void XmlParsingLog::output(const QString &elementName, const QString &information) const
{
    xml->writeTextElement(elementName,information);
}

void XmlParsingLog::info(const QString &information) const
{
    xml->writeTextElement("info",information);
}

void XmlParsingLog::writeNodeAttributes(const AbstractNode *node) const
{
    if( ! node->label().isNull() )
        xml->writeAttribute("label",node->label().toString());
    if( ! node->id().isNull() )
        xml->writeAttribute("id",node->id().toString());
}
