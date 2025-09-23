#include "sqlserverstemlist.h"

#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlError>

#include "create-allomorphs/createallomorphs.h"

#include "morphology.h"
#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include <QtDebug>

using namespace ME;

QString SqlServerStemList::XML_DATABASE_NAME = "database-name";

SqlServerStemList::SqlServerStemList(const MorphologicalModel *model)
    : AbstractSqlStemList(model)
{

}

void SqlServerStemList::openSqlServerDatabase(const QString &connectionString, const QString &databaseName)
{
    if(!QSqlDatabase::isDriverAvailable("QODBC"))
    {
        qWarning() << "SqlServerStemList::openDatabase()" << "Driver failed to open. Available drivers:" << QSqlDatabase::drivers();
        return;
    }

    /// if the database is already open, no need to replace that connection
    /// with another one (and trigger a warning message)
    if( QSqlDatabase::database(databaseName).isValid() )
    {
        return;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", databaseName );

    db.setDatabaseName( connectionString );

    if(!db.open())
    {
        qWarning() << "SqlServerStemList::openDatabase()" << "Database failed to open." << connectionString;
        return;
    }
    if( !db.isValid() )
    {
        qWarning() << "SqlServerStemList::openDatabase()" << "Invalid database: " << connectionString;
    }

    QSqlQuery q(db);
    q.setForwardOnly(true);
    if( !q.exec("use " + databaseName + ";") )
    {
        qWarning() << "SqlServerStemList::openDatabase()" << "Could not select database: " << databaseName;
    }

}

void SqlServerStemList::cloneSqlServerDatabase(const QString &databaseName, const QString &newConnectionName)
{
    QSqlDatabase db = QSqlDatabase::cloneDatabase(databaseName, newConnectionName);
    if(!db.open())
    {
        qWarning() << "SqlServerStemList::cloneSqlServerDatabase()" << "Database failed to open.";
        return;
    }
    if( !db.isValid() )
    {
        qWarning() << "SqlServerStemList::cloneSqlServerDatabase()" << "Invalid database: ";
    }

    QSqlQuery q(db);
    q.setForwardOnly(true);
    if( !q.exec("use " + databaseName + ";") )
    {
        qWarning() << "SqlServerStemList::cloneSqlServerDatabase()" << "Could not select database: " << databaseName;
    }
}

QString SqlServerStemList::elementName()
{
    return "sqlserver-stem-list";
}

AbstractNode *SqlServerStemList::readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader, const MorphologicalModel *model)
{
    Q_ASSERT( in.isStartElement() );

    SqlServerStemList* sl = new SqlServerStemList(model);
    sl->readInitialNodeAttributes(in, morphologyReader);

    if( in.attributes().value("accepts-stems").toString() == "true" )
        morphologyReader->recordStemAcceptingNewStems( sl );

    if( in.attributes().value("create-tables").toString() == "false" )
        sl->mCreateTables = false;

    /// the default here is true
    if( in.attributes().value("include-glosses").toString() == "false" )
    {
        sl->setReadGlosses(false);
    }

    morphologyReader->recordStemList( sl );

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName() ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == AbstractSqlStemList::XML_CONNECTION_STRING )
            {
                sl->setConnectionString( in.readElementText() );
            }
            else if( in.name() == XML_DATABASE_NAME )
            {
                sl->setDbName( in.readElementText() );
            }
            else if( in.name() == AbstractStemList::XML_MATCHING_TAG )
            {
                sl->addConditionTag( in.readElementText() );
            }
            else if( in.name() == AbstractNode::XML_ADD_ALLOMORPHS )
            {
                sl->addCreateAllomorphs( morphologyReader->createAllomorphsFromId( in.attributes().value("with").toString() ) );
            }
            else if( in.name() == AbstractSqlStemList::XML_TABLE_PREFIX )
            {
                sl->setTablePrefix( in.readElementText() );
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

bool SqlServerStemList::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

void SqlServerStemList::openDatabase(const QString &connectionString, const QString &databaseName) const
{
    SqlServerStemList::openSqlServerDatabase(connectionString, databaseName);
}

void SqlServerStemList::cloneDatabase(const QString &databaseName, const QString &newConnectionName) const
{
    SqlServerStemList::cloneSqlServerDatabase(databaseName, newConnectionName);
}

QString SqlServerStemList::qCreateStems() const
{
    return "IF OBJECT_ID(N'dbo." + tableStems() + "', N'U') IS NULL BEGIN create table " + tableStems() + " ( _id bigint IDENTITY(1,1) PRIMARY KEY, liftGuid nvarchar(max) COLLATE Latin1_General_100_CI_AS_SC_UTF8 ) END;";
}

QString SqlServerStemList::qCreateAllomorphs() const
{
    return "IF OBJECT_ID(N'dbo." + tableAllomorphs() + "', N'U') IS NULL BEGIN create table " + tableAllomorphs() + " ( _id bigint IDENTITY(1,1) PRIMARY KEY, stem_id bigint, use_in_generations bigint default 1) END;";
}

QString SqlServerStemList::qUpdateAllomorphsA() const
{
    return "ALTER TABLE " + tableAllomorphs() + " ADD portmanteau nvarchar(max) COLLATE Latin1_General_100_CI_AS_SC_UTF8 default null;";
}

QString SqlServerStemList::qCreateForms() const
{
    return "IF OBJECT_ID(N'dbo." + tableForms() + "', N'U') IS NULL BEGIN create table " + tableForms() + " ( _id bigint IDENTITY(1,1) PRIMARY KEY, allomorph_id bigint, Form nvarchar(max) COLLATE Latin1_General_100_CI_AS_SC_UTF8, WritingSystem nvarchar(max) COLLATE Latin1_General_100_CI_AS_SC_UTF8 ) END;";
}

QString SqlServerStemList::qCreateGlosses() const
{
    return "IF OBJECT_ID(N'dbo." + tableGlosses() + "', N'U') IS NULL BEGIN create table " + tableGlosses() + " ( _id bigint IDENTITY(1,1) PRIMARY KEY, stem_id bigint, Form nvarchar(max) COLLATE Latin1_General_100_CI_AS_SC_UTF8, WritingSystem nvarchar(max) COLLATE Latin1_General_100_CI_AS_SC_UTF8 ) END;";
}

QString SqlServerStemList::qCreateTags() const
{
    return "IF OBJECT_ID(N'dbo." + tableTags() + "', N'U') IS NULL BEGIN create table " + tableTags() + " ( _id bigint IDENTITY(1,1) PRIMARY KEY, Label nvarchar(max) COLLATE Latin1_General_100_CI_AS_SC_UTF8 ) END;";
}

QString SqlServerStemList::qCreateTagMembers() const
{
    return "IF OBJECT_ID(N'dbo." + tableTagMembers() + "', N'U') IS NULL BEGIN create table " + tableTagMembers() + " ( tag_id bigint, allomorph_id bigint ) END;";
}

QString SqlServerStemList::qCreateAllomorphsIdx() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'stemIdIdx' AND object_id = OBJECT_ID('" + tableAllomorphs() + "')) BEGIN CREATE INDEX stemIdIdx ON " + tableAllomorphs() + " (stem_id) END;";
}

QString SqlServerStemList::qCreateGlossesIdx() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'glossIdx' AND object_id = OBJECT_ID('" + tableGlosses() + "')) BEGIN CREATE INDEX glossIdx ON " + tableGlosses() + " (stem_id) END;";
}

QString SqlServerStemList::qCreateFormsIdx() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'formIdx' AND object_id = OBJECT_ID('" + tableForms() + "')) BEGIN CREATE INDEX formIdx ON " + tableForms() + " (allomorph_id) END;";
}

QString SqlServerStemList::qCreateTagsIdx1() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'tagIdx' AND object_id = OBJECT_ID('" + tableTagMembers() + "')) BEGIN CREATE INDEX tagIdx ON " + tableTagMembers() + " (allomorph_id) END;";
}

QString SqlServerStemList::qCreateTagsIdx2() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'tagIdxTwo' AND object_id = OBJECT_ID('" + tableTagMembers() + "')) BEGIN CREATE INDEX tagIdxTwo ON " + tableTagMembers() + " (tag_id) END;";
}

QString SqlServerStemList::qSelectStemIdsWithTags(const QString &taglist) const
{
    return "SELECT DISTINCT stem_id, liftGuid from " + tableTagMembers() + " LEFT JOIN " + tableTags() + " ON " + tableTags() + "._id=" + tableTagMembers() + ".tag_id LEFT JOIN " + tableAllomorphs() + " ON " + tableAllomorphs() + "._id=" + tableTagMembers() + ".allomorph_id LEFT JOIN " + tableStems() + " ON " + tableStems() + "._id=" + tableAllomorphs() + ".stem_id WHERE Label IN ("+taglist+");";
}

QString SqlServerStemList::qInsertStem() const
{
    return "INSERT INTO " + tableStems() + " (liftGuid) VALUES (?);";
}

QString SqlServerStemList::qReplaceStem() const
{
    return "REPLACE INTO " + tableStems() + " (_id, liftGuid) VALUES (?, ?);";
}

/// DEBUG BEGIN UNTESTED METHODS

QString SqlServerStemList::qSelectStemIds() const
{
    return "SELECT DISTINCT _id, liftGuid from " + tableStems() + ";";
}

QString SqlServerStemList::qSelectStemsSingleQuery() const
{
    return "SELECT stem_id, liftGuid, " + tableAllomorphs() + "._id AS allomorph_id, use_in_generations, Form, writingsystem, STRING_AGG(label, ','), portmanteau "
              "FROM " + tableStems() +
              " INNER JOIN " + tableAllomorphs() + " ON " + tableStems() + "._id = " + tableAllomorphs() + ".stem_id "
                                 "INNER JOIN " + tableForms() + " ON " + tableAllomorphs() + "._id = " + tableForms() + ".allomorph_id "
                 "INNER JOIN " + tableTagMembers() + " ON " + tableTagMembers() + ".allomorph_id = " + tableForms() + ".allomorph_id "
                 "INNER JOIN " + tableTags() + " ON " + tableTags() + "._id = " + tableTagMembers() + ".tag_id "
                 "GROUP BY " + tableAllomorphs() + ".stem_id, "
                   + tableStems() + ".liftGuid, "
                   + tableAllomorphs() + "._id, "
                   + tableAllomorphs() + ".use_in_generations, "
                   + tableForms() + "._id, "
                   + tableForms() + ".Form, "
                   + tableForms() + ".writingsystem, "
           + tableAllomorphs() + ".portmanteau;";
}

QString SqlServerStemList::qSelectStemsSingleQueryWithTags(const QString &taglist) const
{
    return "SELECT stem_id, liftGuid," + tableAllomorphs() + "._id AS allomorph_id,use_in_generations,Form,writingsystem,STRING_AGG(label, ','),portmanteau "
           "FROM  "
           + tableStems() + " "
           "INNER JOIN  "
           + tableAllomorphs() + " ON " + tableStems() + "._id = " + tableAllomorphs() + ".stem_id "
           "INNER JOIN  "
           + tableForms() + " ON " + tableAllomorphs() + "._id = " + tableForms() + ".allomorph_id "
           "INNER JOIN  "
           + tableTagMembers() + " ON " + tableTagMembers() + ".allomorph_id = " + tableForms() + ".allomorph_id "
           "INNER JOIN  "
           + tableTags() + " ON " + tableTags() + "._id = " + tableTagMembers() + ".tag_id "
           "WHERE  "
           + tableTagMembers() + ".allomorph_id IN (SELECT allomorph_id FROM " + tableTagMembers() + " WHERE tag_id IN ( " + taglist + " )) "
           "GROUP BY  "
           + tableAllomorphs() + ".stem_id,  "
           + tableStems() + ".liftGuid, "
           + tableAllomorphs() + "._id, "
           + tableAllomorphs() + ".use_in_generations, "
           + tableForms() + "._id, "
           + tableForms() + ".Form, "
           + tableForms() + ".writingsystem, "
           + tableAllomorphs() + ".portmanteau "
           + "ORDER BY "
           + tableAllomorphs() + ".stem_id ASC,  "
           + tableAllomorphs() + "._id ASC, "
           + tableForms() + "._id ASC;";
}

QString SqlServerStemList::qDeleteFromTagMembers() const
{
    return "DELETE from " + tableTagMembers() + " WHERE allomorph_id IN (SELECT _id FROM " + tableAllomorphs() + " WHERE stem_id=?);";
}

QString SqlServerStemList::qDeleteFromForms() const
{
    return "DELETE from " + tableForms() + " WHERE allomorph_id IN (SELECT _id FROM " + tableAllomorphs() + " WHERE stem_id=?);";
}

QString SqlServerStemList::qDeleteFromGlosses() const
{
    return "DELETE FROM " + tableGlosses() + " WHERE stem_id=?;";
}

QString SqlServerStemList::qDeleteFromAllomorphs() const
{
    return "DELETE FROM " + tableAllomorphs() + " WHERE stem_id=?;";
}

QString SqlServerStemList::qDeleteFromStems() const
{
    return "DELETE FROM " + tableStems() + " WHERE _id=?;";
}

QString SqlServerStemList::qSelectAllomorphsFromStemId() const
{
    return "SELECT _id,use_in_generations,portmanteau FROM " + tableAllomorphs() + " WHERE stem_id=?;";
}

QString SqlServerStemList::qSelectGlossesFromStemId() const
{
    return "SELECT Form, WritingSystem FROM " + tableGlosses() + " WHERE stem_id=?;";
}

QString SqlServerStemList::qSelectFormsFromAllomorphId() const
{
    return "SELECT form, writingsystem FROM " + tableForms() + " WHERE allomorph_id=?;";
}

QString SqlServerStemList::qSelectTagLabelsFromAllomorphId() const
{
    return "SELECT label from " + tableTagMembers() + " LEFT JOIN " + tableTags() + " ON " + tableTags() + "._id=" + tableTagMembers() + ".tag_id WHERE allomorph_id=?;";
}

QString SqlServerStemList::qInsertAllomorph() const
{
    return "INSERT INTO " + tableAllomorphs() + " (stem_id,use_in_generations,portmanteau) VALUES (?,?,?);";
}

QString SqlServerStemList::qInsertForm() const
{
    return "INSERT INTO " + tableForms() + " (allomorph_id, form, writingsystem) VALUES (?, ?, ?);";
}

QString SqlServerStemList::qInsertTagMember() const
{
    return "INSERT INTO " + tableTagMembers() + " (tag_id, allomorph_id) VALUES (?, ?);";
}

QString SqlServerStemList::qInsertGloss() const
{
    return "INSERT INTO " + tableGlosses() + " (stem_id, form, writingsystem) VALUES (?, ?, ?);";
}

QString SqlServerStemList::qInsertTag() const
{
    return "INSERT INTO " + tableTags() + " (label) VALUES (?);";
}

QString SqlServerStemList::qSelectTagIdFromLabel() const
{
    return "SELECT _id FROM " + tableTags() + " WHERE label=?;";
}

/// DEBUG END UNTESTED METHODS
