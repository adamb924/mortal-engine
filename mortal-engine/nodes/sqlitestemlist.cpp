#include "sqlitestemlist.h"

#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlError>

#include "create-allomorphs/createallomorphs.h"

#include "morphology.h"
#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include <QtDebug>

using namespace ME;

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
            else if( in.name() == AbstractSqlStemList::XML_TABLE_PREFIX )
            {
                sl->setTablePrefix( in.readElementText() );
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

void SqliteStemList::openDatabase(const QString &connectionString, const QString &databaseName) const
{
    SqliteStemList::openSqliteDatabase(connectionString, databaseName);
}

void SqliteStemList::cloneDatabase(const QString &databaseName, const QString &newConnectionName) const
{
    SqliteStemList::cloneSqliteDatabase(databaseName, newConnectionName);
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

void SqliteStemList::cloneSqliteDatabase(const QString &databaseName, const QString &newConnectionName)
{
    QSqlDatabase db = QSqlDatabase::cloneDatabase(databaseName, newConnectionName);
    if(!db.open())
    {
        qWarning() << "SqliteStemList::cloneSqlServerDatabase()" << "Database failed to open.";
        return;
    }
    if( !db.isValid() )
    {
        qWarning() << "SqliteStemList::cloneSqlServerDatabase()" << "Invalid database: ";
    }
}

QString SqliteStemList::qSelectStemIds() const
{
    return "SELECT DISTINCT _id, liftGuid from " + tableStems() + ";";
}

QString SqliteStemList::qSelectStemIdsWithTags(const QString &taglist) const
{
    return "SELECT DISTINCT stem_id, liftGuid from " + tableTagMembers() + " LEFT JOIN " + tableTags() + ", " + tableAllomorphs() + "," + tableStems() + " ON " + tableTags() + "._id=" + tableTagMembers() + ".tag_id AND " + tableAllomorphs() + "._id=" + tableTagMembers() + ".allomorph_id AND " + tableStems() + "._id=" + tableAllomorphs() + ".stem_id WHERE Label IN ("+taglist+");";
}

QString SqliteStemList::qSelectStemsSingleQuery() const
{
    return "SELECT stem_id, liftGuid," + tableAllomorphs() + "._id AS allomorph_id,use_in_generations,Form,writingsystem,group_concat(label),portmanteau "
                                                             "FROM " + tableStems() + ", " + tableAllomorphs() + ", " + tableForms() + ", " + tableTags() + ", " + tableTagMembers() + " "
                                                                                                                             "ON "
                                                                                                                             "" + tableStems() + "._id=" + tableAllomorphs() + ".stem_id "
                                                          "AND " + tableAllomorphs() + "._id=" + tableForms() + ".allomorph_id "
                                                          "AND " + tableTags() + "._id=" + tableTagMembers() + ".tag_id "
                                                         "AND " + tableTagMembers() + ".allomorph_id=" + tableForms() + ".allomorph_id "
                                                                   "GROUP BY " + tableForms() + "._id "
           + "ORDER BY "
           + tableAllomorphs() + ".stem_id ASC,  "
           + tableAllomorphs() + "._id ASC, "
           + tableForms() + "._id ASC;";
}

QString SqliteStemList::qSelectStemsSingleQueryWithTags(const QString &taglist) const
{
    return "SELECT stem_id, liftGuid,allomorphs._id AS allomorph_id,use_in_generations,Form,writingsystem,group_concat(label),portmanteau "
           "FROM " + tableStems() + ", " + tableAllomorphs() + ", " + tableForms() + ", " +  tableTags()  + ", " + tableTagMembers() + " "
                                                                                                                                     "ON "
                                                                                                                                     "" + tableStems() + "._id=" + tableAllomorphs() + ".stem_id "
                                                          "AND " + tableAllomorphs() + "._id=" + tableForms() + ".allomorph_id "
                                                          "AND " + tableTags() + "._id=" + tableTagMembers() + ".tag_id "
                                                         "AND " + tableTagMembers() + ".allomorph_id=" + tableForms() + ".allomorph_id "
                                                                   "WHERE " + tableTagMembers() + ".allomorph_id IN (SELECT allomorph_id FROM " + tableTagMembers() + " WHERE tag_id IN ( " + taglist + " ) ) "
                                                                                                                                       "GROUP BY " + tableForms() + "._id "
           + "ORDER BY "
           + tableAllomorphs() + ".stem_id ASC,  "
           + tableAllomorphs() + "._id ASC, "
           + tableForms() + "._id ASC;";
}

QString SqliteStemList::qDeleteFromTagMembers() const
{
    return "DELETE from " + tableTagMembers() + " WHERE allomorph_id IN (SELECT _id FROM " + tableAllomorphs() + " WHERE stem_id=?);";
}

QString SqliteStemList::qDeleteFromForms() const
{
    return "DELETE from " + tableForms() + " WHERE allomorph_id IN (SELECT _id FROM " + tableAllomorphs() + " WHERE stem_id=?);";
}

QString SqliteStemList::qDeleteFromGlosses() const
{
    return "DELETE FROM " + tableGlosses() + " WHERE stem_id=?;";
}

QString SqliteStemList::qDeleteFromAllomorphs() const
{
    return "DELETE FROM " + tableAllomorphs() + " WHERE stem_id=?;";
}

QString SqliteStemList::qDeleteFromStems() const
{
    return "DELETE FROM " + tableStems() + " WHERE _id=?;";
}

QString SqliteStemList::qSelectAllomorphsFromStemId() const
{
    return "SELECT _id,use_in_generations,portmanteau FROM " + tableAllomorphs() + " WHERE stem_id=?;";
}

QString SqliteStemList::qSelectGlossesFromStemId() const
{
    return "SELECT Form, WritingSystem FROM " + tableGlosses() + " WHERE stem_id=?;";
}

QString SqliteStemList::qSelectFormsFromAllomorphId() const
{
    return "SELECT form, writingsystem FROM " + tableForms() + " WHERE allomorph_id=?;";
}

QString SqliteStemList::qSelectTagLabelsFromAllomorphId() const
{
    return "SELECT label from " + tableTagMembers() + " LEFT JOIN Tags ON " + tableTags() + "._id=" + tableTagMembers() + ".tag_id WHERE allomorph_id=?;";
}

QString SqliteStemList::qInsertStem() const
{
    return "INSERT INTO " + tableStems() + " (_id, liftGuid) VALUES (null, ?);";
}

QString SqliteStemList::qReplaceStem() const
{
    return "INSERT OR REPLACE INTO " + tableStems() + " (_id, liftGuid) VALUES (?, ?);";
}

QString SqliteStemList::qInsertAllomorph() const
{
    return "INSERT INTO " + tableAllomorphs() + " (stem_id,use_in_generations,portmanteau) VALUES (?,?,?);";
}

QString SqliteStemList::qInsertForm() const
{
    return "INSERT INTO " + tableForms() + " (allomorph_id, form, writingsystem) VALUES (?, ?, ?);";
}

QString SqliteStemList::qInsertTagMember() const
{
    return "INSERT INTO " + tableTagMembers() + " (tag_id, allomorph_id) VALUES (?, ?);";
}

QString SqliteStemList::qInsertGloss() const
{
    return "INSERT INTO " + tableGlosses() + " (stem_id, form, writingsystem) VALUES (?, ?, ?);";
}

QString SqliteStemList::qInsertTag() const
{
    return "INSERT INTO " + tableTags() + " (label) VALUES (?);";
}

QString SqliteStemList::qSelectTagIdFromLabel() const
{
    return "SELECT _id FROM " + tableTags() + " WHERE label=?;";
}

QString SqliteStemList::qCreateStems() const
{
    return "create table if not exists " + tableStems() + " ( _id integer primary key autoincrement, liftGuid text  );";
}

QString SqliteStemList::qCreateAllomorphs() const
{
    return "create table if not exists " + tableAllomorphs() + " ( _id integer primary key autoincrement, stem_id integer, use_in_generations integer default 1, portmanteau text default null);";
}

QString SqliteStemList::qUpdateAllomorphsA() const
{
    return "ALTER TABLE " + tableAllomorphs() + " ADD COLUMN portmanteau text default null;";
}

QString SqliteStemList::qCreateForms() const
{
    return "create table if not exists " + tableForms() + " ( _id integer primary key autoincrement, allomorph_id integer, Form text, WritingSystem text );";
}

QString SqliteStemList::qCreateGlosses() const
{
    return "create table if not exists " + tableGlosses() + " ( _id integer primary key autoincrement, stem_id integer, Form text, WritingSystem text );";
}

QString SqliteStemList::qCreateTags() const
{
    return "create table if not exists " + tableTags() + " ( _id integer primary key autoincrement, Label text );";
}

QString SqliteStemList::qCreateTagMembers() const
{
    return "create table if not exists " + tableTagMembers() + " ( tag_id integer, allomorph_id );";
}

QString SqliteStemList::qCreateAllomorphsIdx() const
{
    return "CREATE INDEX IF NOT EXISTS stemIdIdx ON " + tableAllomorphs() + " (stem_id);";
}

QString SqliteStemList::qCreateGlossesIdx() const
{
    return "CREATE INDEX IF NOT EXISTS glossIdx ON " + tableGlosses() + " (stem_id);";
}

QString SqliteStemList::qCreateFormsIdx() const
{
    return "CREATE INDEX IF NOT EXISTS formIdx ON " + tableForms() + " (allomorph_id);";
}

QString SqliteStemList::qCreateTagsIdx1() const
{
    return "CREATE INDEX IF NOT EXISTS tagIdx ON " + tableTagMembers() + " (allomorph_id);";
}

QString SqliteStemList::qCreateTagsIdx2() const
{
    return "CREATE INDEX IF NOT EXISTS tagIdxTwo ON " + tableTagMembers() + " (tag_id);";
}
