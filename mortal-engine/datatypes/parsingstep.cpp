#include "parsingstep.h"

#include <QXmlStreamWriter>
#include <QDomElement>

#include "nodes/abstractnode.h"
#include "datatypes/portmanteau.h"
#include "morphology.h"
#include "nodes/morphemenode.h"

ParsingStep::ParsingStep(const AbstractNode *node, const Allomorph & allomorph) :
    mNode(node),
    mAllomorph(allomorph),
    mIsStem(false)
{

}

ParsingStep::ParsingStep(const AbstractNode *node, const Allomorph &allomorph, const LexicalStem &lexicalStem) :
    mNode(node),
    mAllomorph(allomorph),
    mLexicalStem(lexicalStem),
    mIsStem(true)
{

}

const AbstractNode *ParsingStep::node() const
{
    return mNode;
}

const AbstractNode *ParsingStep::lastNode(const WritingSystem & ws) const
{
    if( mAllomorph.hasPortmanteau(ws) )
    {
        return mAllomorph.portmanteau().lastNode();
    }
    else
    {
        return mNode;
    }
}

bool ParsingStep::lastNodeMatchesId(const QString &id) const
{
    QSetIterator<WritingSystem> wsIter( mAllomorph.writingSystems() );
    while(wsIter.hasNext())
    {
        const WritingSystem ws = wsIter.next();
        if( lastNode(ws)->id() == id )
        {
            return true;
        }
    }
    return false;
}

bool ParsingStep::lastNodeMatchesLabel(const QString &label) const
{
    QSetIterator<WritingSystem> wsIter( mAllomorph.writingSystems() );
    while(wsIter.hasNext())
    {
        const WritingSystem ws = wsIter.next();
        if( lastNode(ws)->label() == label )
        {
            return true;
        }
    }
    return false;
}

Allomorph ParsingStep::allomorph() const
{
    return mAllomorph;
}

void ParsingStep::serialize(QXmlStreamWriter &out, bool includeGlosses) const
{
    out.writeStartElement("parsing-step");
    mNode->serialize(out);
    mAllomorph.serialize(out);

    if( includeGlosses )
    {
        if( mIsStem ) /// then print the lexical stem glosses
        {
            QHashIterator<WritingSystem, Form> i( mLexicalStem.glosses() );
            while( i.hasNext() )
            {
                i.next();
                i.value().serialize( out, "gloss" );
            }
        }
        else /// print the morpheme node's glosses
        {
            QHashIterator<WritingSystem, Form> i( mNode->glosses() );
            while( i.hasNext() )
            {
                i.next();
                i.value().serialize( out, "gloss" );
            }
        }
    }

    out.writeEndElement(); /// parsing-step
}

void ParsingStep::serialize(QDomElement &out, bool includeGlosses) const
{
    QDomElement nodeEl = out.ownerDocument().createElement("node");
    mNode->serialize(nodeEl);
    out.appendChild(nodeEl);

    QDomElement allEl = out.ownerDocument().createElement("allomorph");
    mAllomorph.serialize(allEl);
    out.appendChild(allEl);


    if( includeGlosses )
    {
        if( mIsStem ) /// then print the lexical stem glosses
        {
            QHashIterator<WritingSystem, Form> i( mLexicalStem.glosses() );
            while( i.hasNext() )
            {
                i.next();
                QDomElement gloss = out.ownerDocument().createElement("gloss");
                i.value().serialize(gloss);
                out.appendChild(gloss);
            }
        }
        else /// print the morpheme node's glosses
        {
            QHashIterator<WritingSystem, Form> i( mNode->glosses() );
            while( i.hasNext() )
            {
                i.next();
                QDomElement gloss = out.ownerDocument().createElement("gloss");
                i.value().serialize(gloss);
                out.appendChild(gloss);
            }
        }
    }

}

bool ParsingStep::isStem() const
{
    return mIsStem;
}

QString ParsingStep::summaryPortion(ParsingStep::SummaryType type, const WritingSystem &summaryDisplayWritingSystem, const WritingSystem &parsingWritingSystem) const
{
    if( isStem() )
    {
        switch(type)
        {
        case ParsingStep::MorphemeForm:
            return mAllomorph.form( summaryDisplayWritingSystem ).text();
        case ParsingStep::MorphemeGloss:
            return mLexicalStem.gloss(summaryDisplayWritingSystem).text();
        case ParsingStep::MorphemeLabel:
            if( mAllomorph.hasPortmanteau(parsingWritingSystem) )
                return mAllomorph.portmanteau().label();
            else
                return mNode->label();
        }
    }
    else /// affix
    {
        switch(type)
        {
        case ParsingStep::MorphemeForm:
            return mAllomorph.form( summaryDisplayWritingSystem ).text();
        case ParsingStep::MorphemeGloss:
            return mNode->gloss(summaryDisplayWritingSystem).text();
        case ParsingStep::MorphemeLabel:
            if( mAllomorph.hasPortmanteau(parsingWritingSystem) )
                return mAllomorph.portmanteau().label();
            else
                return mNode->label();
        }
    }
    return "";
}

ParsingStep ParsingStep::readFromXml(QXmlStreamReader &in, const Morphology *morphology, const QString &elementName)
{
    Q_ASSERT( in.isStartElement() && in.name() == elementName );

    in.readNextStartElement(); /// get to node
    bool isStem = in.attributes().value("type").toString() == "stem";
    QString label = in.attributes().value("label").toString();

    AbstractNode * node = nullptr;

    /// prefer to use the id
    if( in.attributes().hasAttribute("id") )
    {
        node = morphology->getNodeFromId( in.attributes().value("id").toString() );
        if( node == nullptr )
        {
            qCritical() << "ParsingStep::readFromXml: Null node for id" << in.attributes().value("id").toString();
        }
    }
    else
    {
        QList<AbstractNode *> nodes = morphology->getNodesFromLabel(label);
        if( nodes.size() == 0 )
        {
            qCritical() << "ParsingStep::readFromXml: No node with label:" << label << label.length();
        }
        else if( nodes.size() > 1 )
        {
            qCritical() << "ParsingStep::readFromXml: Multiple nodes with label:" << label << "(choosing first)";
            node = nodes.first();
        }
        else
        {
            node = nodes.first();
        }
    }

    in.readNext(); /// get to 'end' element of <node>
    in.readNextStartElement(); /// get to allomorph

    Allomorph a = Allomorph::readFromXml(in, morphology);
    in.readNext(); /// get past the </allomorph> tag

    /// get to end node
    while( ! in.isEndElement() )
        in.readNext();

    Q_ASSERT( in.isEndElement() && in.name() == elementName );

    if( isStem )
    {
        QList<LexicalStem *> lss = morphology->lexicalStems(a, false);
        if( lss.size() == 0 )
        {
            qCritical() << "ParsingStep::readFromXml: No stem for allomorph:" << a.summary();
            Q_ASSERT(lss.size() != 0);
        }
        else if( lss.size() > 1 )
        {
            qCritical() << "ParsingStep::readFromXml: Multiple nodes for allomorph:" << a.summary() << "(choosing first)";
        }

        return ParsingStep( node, a, * lss.first() );
    }
    else
    {
        return ParsingStep( node, a );
    }
}

QList<ParsingStep> ParsingStep::readListFromXml(QXmlStreamReader &in, const Morphology *morphology, const QString &untilElement, const QString &elementName)
{
    Q_ASSERT( in.isStartElement() && in.name() == untilElement );

    QList<ParsingStep> steps;
    while( !in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == untilElement ) )
    {
        in.readNext();

        if( in.isStartElement() && in.name() == elementName )
        {
            steps << ParsingStep::readFromXml(in, morphology, elementName);
        }
    }

    Q_ASSERT( in.isEndElement() && in.name() == untilElement );
    return steps;

}

ParsingStep ParsingStep::readFromXml(QDomElement parsing, const Morphology *morphology)
{
    QDomNodeList nodes = parsing.elementsByTagName("node");
    Q_ASSERT(!nodes.isEmpty());
    QDomElement nodeElement = nodes.at(0).toElement();

    bool isStem = nodeElement.attribute("type") == "stem";
    QString label = nodeElement.attribute("label");

    AbstractNode * node = nullptr;

    /// prefer to use the id
    if( nodeElement.hasAttribute("id") )
    {
        node = morphology->getNodeFromId( nodeElement.attribute("id") );
        if( node == nullptr )
        {
            qCritical() << "ParsingStep::readFromXml(QDomElement): Null node for id" << nodeElement.attribute("id");
        }
    }
    else
    {
        QList<AbstractNode *> nodes = morphology->getNodesFromLabel(label);
        if( nodes.size() == 0 )
        {
            qCritical() << "ParsingStep::readFromXml(QDomElement): No node with label:" << label << label.length();
        }
        else if( nodes.size() > 1 )
        {
            qCritical() << "ParsingStep::readFromXml(QDomElement): Multiple nodes with label:" << label << "(choosing first)";
            node = nodes.first();
        }
        else
        {
            node = nodes.first();
        }
    }

    QDomNodeList allomorphs = parsing.elementsByTagName("allomorph");
    Q_ASSERT(!allomorphs.isEmpty());
    Allomorph a = Allomorph::readFromXml( allomorphs.at(0).toElement(), morphology);

    if( isStem )
    {
        QList<LexicalStem *> lss = morphology->lexicalStems(a, false);
        if( lss.size() == 0 )
        {
            qCritical() << "ParsingStep::readFromXml(QDomElement): No stem for allomorph:" << a.summary();
            return ParsingStep( node, a );
        }
        else if( lss.size() > 1 )
        {
            qCritical() << "ParsingStep::readFromXml(QDomElement): Multiple stems for allomorph:" << a.summary() << "(choosing first)";
            foreach(LexicalStem * s, lss)
            {
                qCritical() << "\t" << s->summary();
            }
        }

        return ParsingStep( node, a, * lss.first() );
    }
    else
    {
        return ParsingStep( node, a );
    }
}

QList<ParsingStep> ParsingStep::readListFromXml(QDomElement list, const Morphology *morphology)
{
    QList<ParsingStep> parsingSteps;
    QDomNodeList parsingStepElements = list.elementsByTagName("parsing-step");
    for(int i=0; i< parsingStepElements.count(); i++)
    {
        parsingSteps << ParsingStep::readFromXml( parsingStepElements.at(i).toElement(), morphology );
    }
    return parsingSteps;
}

QString ParsingStep::label(const WritingSystem & ws) const
{
    if( mAllomorph.hasPortmanteau(ws) )
    {
        return mAllomorph.portmanteau().label();
    }
    else
    {
        return mNode->label();
    }
}

void ParsingStep::setIsStem(const bool &isStem)
{
    mIsStem = isStem;
}

LexicalStem ParsingStep::lexicalStem() const
{
    return mLexicalStem;
}
