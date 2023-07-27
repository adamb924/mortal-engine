#include "sqlserverstemlist.h"

#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlError>

#include "create-allomorphs/createallomorphs.h"

#include "morphology.h"
#include "morphologyxmlreader.h"
#include <QXmlStreamReader>
#include <QtDebug>

SqlServerStemList::SqlServerStemList(const MorphologicalModel *model)
    : AbstractSqlStemList(model)
{

}

void SqlServerStemList::openSqlServerDatabase(const QString &connectionString, const QString &databaseName)
{
    if(!QSqlDatabase::isDriverAvailable("QODBC"))
    {
        qWarning() << "SqlServerStemList::openDatabase()" << "Driver failed to open.";
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

    /// the default here is true
    if( in.attributes().value("include-glosses").toString() == "false" )
    {
        qCritical() << "The include-glosses=\"false\" is giving errors. Do not use it until it is fixed.";
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

bool SqlServerStemList::matchesElement(QXmlStreamReader &in)
{
    return in.isStartElement() && in.name() == elementName();
}

void SqlServerStemList::openDatabase(const QString &connectionString, const QString &databaseName)
{
    SqlServerStemList::openSqlServerDatabase(connectionString, databaseName);
}

QString SqlServerStemList::qCreateStems() const
{
    return "IF OBJECT_ID(N'dbo.Stems', N'U') IS NULL BEGIN create table Stems ( _id bigint IDENTITY(1,1) PRIMARY KEY, liftGuid varchar(max) ) END;";
}

QString SqlServerStemList::qCreateAllomorphs() const
{
    return "IF OBJECT_ID(N'dbo.Allomorphs', N'U') IS NULL BEGIN create table Allomorphs ( _id bigint IDENTITY(1,1) PRIMARY KEY, stem_id bigint, use_in_generations bigint default 1) END;";
}

QString SqlServerStemList::qCreateForms() const
{
    return "IF OBJECT_ID(N'dbo.Forms', N'U') IS NULL BEGIN create table Forms ( _id bigint IDENTITY(1,1) PRIMARY KEY, allomorph_id bigint, Form varchar(max), WritingSystem varchar(max) ) END;";
}

QString SqlServerStemList::qCreateGlosses() const
{
    return "IF OBJECT_ID(N'dbo.Glosses', N'U') IS NULL BEGIN create table Glosses ( _id bigint IDENTITY(1,1) PRIMARY KEY, stem_id bigint, Form varchar(max), WritingSystem varchar(max) ) END;";
}

QString SqlServerStemList::qCreateTags() const
{
    return "IF OBJECT_ID(N'dbo.Tags', N'U') IS NULL BEGIN create table Tags ( _id bigint IDENTITY(1,1) PRIMARY KEY, Label varchar(max) ) END;";
}

QString SqlServerStemList::qCreateTagMembers() const
{
    return "IF OBJECT_ID(N'dbo.TagMembers', N'U') IS NULL BEGIN create table TagMembers ( tag_id bigint, allomorph_id bigint ) END;";
}

QString SqlServerStemList::qCreateAllomorphsIdx() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'stemIdIdx' AND object_id = OBJECT_ID('Allomorphs')) BEGIN CREATE INDEX stemIdIdx ON Allomorphs (stem_id) END;";
}

QString SqlServerStemList::qCreateGlossesIdx() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'glossIdx' AND object_id = OBJECT_ID('Glosses')) BEGIN CREATE INDEX glossIdx ON Glosses (stem_id) END;";
}

QString SqlServerStemList::qCreateFormsIdx() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'formIdx' AND object_id = OBJECT_ID('Forms')) BEGIN CREATE INDEX formIdx ON Forms (allomorph_id) END;";
}

QString SqlServerStemList::qCreateTagsIdx1() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'tagIdx' AND object_id = OBJECT_ID('TagMembers')) BEGIN CREATE INDEX tagIdx ON TagMembers (allomorph_id) END;";
}

QString SqlServerStemList::qCreateTagsIdx2() const
{
    return "IF NOT EXISTS(SELECT * FROM sys.indexes WHERE name = 'tagIdxTwo' AND object_id = OBJECT_ID('TagMembers')) BEGIN CREATE INDEX tagIdxTwo ON TagMembers (tag_id) END;";
}

QString SqlServerStemList::qSelectStemIdsWithTags(const QString &taglist) const
{
    return "SELECT DISTINCT stem_id, liftGuid from TagMembers LEFT JOIN Tags ON Tags._id=TagMembers.tag_id LEFT JOIN Allomorphs ON Allomorphs._id=TagMembers.allomorph_id LEFT JOIN Stems ON Stems._id=Allomorphs.stem_id WHERE Label IN ("+taglist+");";
}

QString SqlServerStemList::qInsertStem() const
{
    return "INSERT INTO stems (liftGuid) VALUES (?);";
}
