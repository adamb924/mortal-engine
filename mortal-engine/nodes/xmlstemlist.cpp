#include "xmlstemlist.h"

#include <QXmlStreamReader>
#include <QFile>

#include "datatypes/lexicalstem.h"
#include "datatypes/allomorph.h"
#include "datatypes/writingsystem.h"
#include "datatypes/parsing.h"

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
    Allomorph allomorph(Allomorph::Original);

    if( file.open( QFile::ReadOnly ) )
    {
        QXmlStreamReader in(&file);

        while(!in.atEnd())
        {
            in.readNext();

            if( in.tokenType() == QXmlStreamReader::StartElement )
            {
                if( in.name() == "stem" )
                {
                    allomorph = Allomorph(Allomorph::Original);
                }
                else if( in.name() == "form" )
                {
                    QXmlStreamAttributes attr = in.attributes();
                    if( attr.hasAttribute("lang") )
                    {
                        WritingSystem ws = writingSystems.value( attr.value("lang").toString() );
                        allomorph.setForm( Form( ws, in.readElementText() ) );
                    }
                }
                else if( in.name() == "tag" )
                {
                    allomorph.addTag( in.readElementText() );
                }
            }
            if( in.tokenType() == QXmlStreamReader::EndElement && in.name() == "stem" )
            {
                if( match( allomorph ) )
                {
                    mStems.insert( new LexicalStem(allomorph) );
                }
            }
        }
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
            if( in.name() == "filename" )
            {
                node->setFilename( in.readElementText() );
            }
            else if( in.name() == "matching-tag" )
            {
                node->addConditionTag( in.readElementText() );
            }
            else if( CreateAllomorphs::matchesElement(in) )
            {
                node->setCreateAllomorphs( CreateAllomorphs::readFromXml(in, morphologyReader) );
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
