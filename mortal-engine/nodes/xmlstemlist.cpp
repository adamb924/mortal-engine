#include "xmlstemlist.h"

#include <QXmlStreamReader>
#include <QFile>

#include "datatypes/lexicalstem.h"
#include "datatypes/allomorph.h"
#include "datatypes/writingsystem.h"

#include "morphology.h"
#include "morphologyxmlreader.h"
#include <QtDebug>

#include "create-allomorphs/createallomorphs.h"

XmlStemList::XmlStemList(const MorphologicalModel *model) : AbstractStemList(model)
{

}

void XmlStemList::setFilename(const QString &filename)
{
    mFilename = filename;
}

void XmlStemList::readStems(const QHash<QString, WritingSystem> &writingSystems)
{
    QFile file(mFilename);
    LexicalStem * stem = nullptr;
    Allomorph allomorph(Allomorph::Null);

    if( file.open( QFile::ReadOnly ) )
    {
        QXmlStreamReader in(&file);

        while(!in.atEnd())
        {
            in.readNext();

            if( in.isStartElement() )
            {
                if( in.name() == Allomorph::XML_STEM )
                {
                    stem = new LexicalStem();
                }
                else if( in.name() == Allomorph::XML_ALLOMORPH )
                {
                    allomorph = Allomorph(Allomorph::Original);
                    if( in.attributes().hasAttribute(Allomorph::XML_PORTMANTEAU) )
                    {
                        allomorph.setPortmanteau( Portmanteau( in.attributes().value(Allomorph::XML_PORTMANTEAU).toString() ) );
                    }
                }
                else if( in.name() == Allomorph::XML_FORM )
                {
                    /// if we haven't yet read an allomorph tag, we need to create a new allomorph right now
                    if( allomorph.type() == Allomorph::Null )
                    {
                        allomorph = Allomorph(Allomorph::Original);
                    }

                    QXmlStreamAttributes attr = in.attributes();
                    if( attr.hasAttribute("lang") )
                    {
                        WritingSystem ws = writingSystems.value( attr.value("lang").toString() );
                        allomorph.setForm( Form( ws, in.readElementText() ) );
                    }
                }
                else if( in.name() == Allomorph::XML_TAG )
                {
                    allomorph.addTag( in.readElementText() );
                }
            }
            else if( in.isEndElement() )
            {
                if( in.name() == Allomorph::XML_STEM )
                {
                    if( allomorph.type() != Allomorph::Null && match( allomorph ) )
                    {
                        stem->insert( allomorph );
                    }
                    if( stem->allomorphCount() > 0 )
                    {
                        mStems.insert( stem );
                    }
                    allomorph = Allomorph(Allomorph::Null);
                }
                else if (in.name() == Allomorph::XML_ALLOMORPH )
                {
                    if( match( allomorph ) )
                    {
                        stem->insert( allomorph );
                    }
                    allomorph = Allomorph(Allomorph::Null);
                }
            }
        }
    }

    if( mStems.isEmpty() )
    {
        qWarning() << "No stems were read for the node" << debugIdentifier();
    }
}

QString XmlStemList::elementName()
{
    return "stem-list";
}

AbstractNode *XmlStemList::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel *model)
{
    Q_ASSERT( in.isStartElement() );
    XmlStemList * node = new XmlStemList(model);
    node->readInitialNodeAttributes(in, morphologyReader);

    if( in.attributes().value("accepts-stems").toString() == "true" )
        morphologyReader->recordStemAcceptingNewStems(node);

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName() ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == AbstractStemList::XML_FILENAME )
            {
                node->setFilename( in.readElementText() );
            }
            else if( in.name() == AbstractStemList::XML_MATCHING_TAG )
            {
                node->addConditionTag( in.readElementText() );
            }
            else if( CreateAllomorphs::matchesElement(in) )
            {
                node->addCreateAllomorphs( CreateAllomorphs::readFromXml(in, morphologyReader) );
            }
            else if( in.name() == AbstractNode::XML_ADD_ALLOMORPHS )
            {
                node->addCreateAllomorphs( morphologyReader->createAllomorphsFromId( in.attributes().value("with").toString() ) );
            }
        }
    }

    node->readStems( morphologyReader->morphology()->writingSystems() );

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return node;
}

bool XmlStemList::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

void XmlStemList::insertStemIntoDataModel(LexicalStem *stem)
{
    stem->generateAllomorphs(mCreateAllomorphs);
    mStems.insert( stem );
    qWarning() << "An XML stem list does not save new stems added to the model.";
}

void XmlStemList::removeStemFromDataModel(qlonglong id)
{
    QSetIterator<LexicalStem*> i(mStems);
    while( i.hasNext() )
    {
        LexicalStem * ls = i.next();
        if( ls->id() == id )
        {
            mStems.remove(ls);
            break;
        }
    }
    qWarning() << "Stems are not removed from an XML stem list permanently.";
}
