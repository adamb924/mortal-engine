#include "sqlitestemlist.h"

#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlError>

#include "datatypes/lexicalstem.h"
#include "create-allomorphs/createallomorphs.h"

#include "morphology.h"
#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include <QtDebug>

QString SqliteStemList::XML_EXTERNAL_DATABASE = "external-database";

SqliteStemList::SqliteStemList(const MorphologicalModel *model)
    : AbstractSqlStemList(model)
{

}

void SqliteStemList::setDatabasePath(const QString &databasePath)
{
    /// https://stackoverflow.com/a/16568641/1447002
    mDbName = QString("%1_0x%2").arg(DEFAULT_DBNAME).arg( reinterpret_cast<quintptr>(this),
                        QT_POINTER_SIZE * 2, 16, QChar('0'));

    openDatabase(databasePath, mDbName);
    createTables();
}

void SqliteStemList::setExternalDatabase(const QString &dbName)
{
    mDbName = dbName;

    QSqlDatabase db = QSqlDatabase::database(mDbName);
    if( !db.isValid() )
        qWarning() << QString("The database %1 returns false for isValid().").arg(dbName);
    if( !db.isOpen() )
        qWarning() << QString("The database %1 returns false for isOpen().").arg(dbName);

    createTables();
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
                sl->setDatabasePath( in.readElementText() );
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

void SqliteStemList::openDatabase(const QString &filename, const QString &databaseName)
{
    if(!QSqlDatabase::isDriverAvailable("QSQLITE"))
    {
        qWarning() << "SqliteStemList::openDatabase()" << "Driver failed to open.";
        return;
    }

    /// if the database is already open, no need to replace that connection
    /// with another one (and trigger a warning message)
    if( QSqlDatabase::database(databaseName).isValid() )
    {
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", databaseName );

    db.setDatabaseName( filename );
    db.setConnectOptions("QSQLITE_ENABLE_REGEXP");

    if(!db.open())
    {
        qWarning() << "SqliteStemList::openDatabase()" << "Database failed to open." << filename;
        return;
    }
    if( !db.isValid() )
    {
        qWarning() << "SqliteStemList::openDatabase()" << "Invalid database: " << filename;
    }
}
