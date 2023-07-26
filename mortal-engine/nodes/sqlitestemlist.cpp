#include "sqlitestemlist.h"

#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlError>

#include "create-allomorphs/createallomorphs.h"

#include "morphology.h"
#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include <QtDebug>

SqliteStemList::SqliteStemList(const MorphologicalModel *model)
    : AbstractSqlStemList(model)
{

}

QString SqliteStemList::elementName()
{
    return "sqlite-stem-list";
}

AbstractNode *SqliteStemList::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel *model)
{
    Q_ASSERT( in.isStartElement() );
    SqliteStemList* sl = new SqliteStemList(model);
    sl->readInitialNodeAttributes(in, morphologyReader);

    if( in.attributes().value("accepts-stems").toString() == "true" )
        morphologyReader->recordStemAcceptingNewStems( sl );

    /// the default here is true
    if( in.attributes().value("include-glosses").toString() == "false" )
        sl->setReadGlosses(false);

    morphologyReader->recordStemList( sl );

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName() ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == AbstractStemList::XML_FILENAME )
            {
                sl->setConnectionString( in.readElementText() );
            }
            else if( in.name() == XML_EXTERNAL_DATABASE )
            {
                sl->setExternalDatabase( in.readElementText() );
            }
            else if( in.name() == AbstractStemList::XML_MATCHING_TAG )
            {
                sl->addConditionTag( in.readElementText() );
            }
            else if( in.name() == AbstractNode::XML_ADD_ALLOMORPHS )
            {
                sl->addCreateAllomorphs( morphologyReader->createAllomorphsFromId( in.attributes().value("with").toString() ) );
            }
            else if( CreateAllomorphs::matchesElement(in) )
            {
                sl->addCreateAllomorphs( CreateAllomorphs::readFromXml(in, morphologyReader) );
            }
        }
    }

    sl->readStems( morphologyReader->morphology()->writingSystems() );

    Q_ASSERT( in.isEndElement() && in.name() == elementName() );
    return sl;
}

bool SqliteStemList::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

void SqliteStemList::openDatabase(const QString &connectionString, const QString &databaseName)
{
    SqliteStemList::openSqliteDatabase(connectionString, databaseName);
}

void SqliteStemList::openSqliteDatabase(const QString &connectionString, const QString &databaseName)
{
    if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        qWarning() << "SqliteStemList::openDatabase()" << "The QSQLITE driver is not available.";
        return;
    }

    /// if the database is already open, no need to replace that connection
    /// with another one (and trigger a warning message)
    if( QSqlDatabase::database(databaseName).isValid() )
    {
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", databaseName );

    db.setDatabaseName( connectionString );
    db.setConnectOptions("QSQLITE_ENABLE_REGEXP");

    if(!db.open())
    {
        qWarning() << "SqliteStemList::openDatabase()" << "Database failed to open." << connectionString;
        return;
    }
    if( !db.isValid() )
    {
        qWarning() << "SqliteStemList::openDatabase()" << "Invalid database: " << connectionString;
    }
}
