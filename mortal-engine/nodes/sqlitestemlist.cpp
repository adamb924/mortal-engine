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

QString SqliteStemList::DEFAULT_DBNAME = "SQLITE_STEM_LIST";

SqliteStemList::SqliteStemList(const MorphologicalModel *model) : AbstractStemList(model), mDbName(DEFAULT_DBNAME)
{

}

void SqliteStemList::setDatabasePath(const QString &databasePath)
{
    mDatabasePath = databasePath;
    mDbName = DEFAULT_DBNAME;
    openDatabase(mDatabasePath, mDbName);
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

void SqliteStemList::readStems(const QHash<QString, WritingSystem> & writingSystems)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    if( !db.isOpen() )
        return;

    /// this is where the tag filtering takes place
    QString inStatement = tagsInSqliteList();

    QSqlQuery query(db);
    query.prepare("SELECT DISTINCT stem_id, liftGuid from TagMembers LEFT JOIN Tags, Allomorphs,Stems ON Tags._id=TagMembers.tag_id AND Allomorphs._id=TagMembers.allomorph_id AND Stems._id=Allomorphs.stem_id WHERE Label IN ("+inStatement+");");
    if(query.exec())
    {
        while(query.next())
        {
            const qlonglong stemId = query.value(0).toLongLong();
            const QString liftGuid = query.value(1).toString();
            mStems << lexicalStemFromId(stemId, liftGuid, writingSystems);
        }
    }
    else
    {
        qWarning() << "SqliteStemList::readStems()" << query.lastError().text() << query.executedQuery();
    }
}

LexicalStem *SqliteStemList::lexicalStemFromId(qlonglong stemId, const QString & liftGuid, const QHash<QString, WritingSystem> &writingSystems)
{
    LexicalStem * ls = new LexicalStem;
    ls->setId(stemId);
    if( !liftGuid.isEmpty() )
        ls->setLiftGuid( liftGuid );

    QSqlDatabase db = QSqlDatabase::database(mDbName);
    QSqlQuery query(db);

    query.prepare("SELECT _id FROM Allomorphs WHERE stem_id=?;");
    query.bindValue( 0, stemId );
    if(query.exec())
    {
        while(query.next())
        {
            qlonglong allomorph_id = query.value(0).toLongLong();
            ls->insert( allomorphFromId( allomorph_id, writingSystems ) );
        }
    }
    else
    {
        qWarning() << "SqliteStemList::lexicalStemFromId()" << query.lastError().text() << query.executedQuery();
    }
    query.finish();

    query.prepare("SELECT Form, WritingSystem FROM Glosses WHERE stem_id=?;");
    query.bindValue( 0, stemId );
    if(query.exec())
    {
        while(query.next())
        {
            QString form = query.value(0).toString();
            QString ws = query.value(1).toString();

            ls->setGloss( Form( writingSystems.value( ws ), form ) );
        }
    }
    else
    {
        qWarning() << "SqliteStemList::readStems()" << query.lastError().text() << query.executedQuery();
    }

    return ls;
}

Allomorph SqliteStemList::allomorphFromId(qlonglong allomorphId, const QHash<QString, WritingSystem> &writingSystems)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);
    QSqlQuery query(db);

    Allomorph a(Allomorph::Original);
    a.setId(allomorphId);

    query.prepare("SELECT form, writingsystem FROM Forms WHERE allomorph_id=?;");
    query.bindValue(0, allomorphId);
    if(query.exec())
    {
        while(query.next())
        {
            WritingSystem ws = writingSystems.value( query.value(1).toString() );
            a.setForm( Form( ws, query.value(0).toString() ) );
        }
    }
    else
    {
        qWarning() << "SqliteStemList::allomorphFromId()" << query.lastError().text() << query.executedQuery();
    }
    query.finish();

    query.prepare("SELECT label from TagMembers LEFT JOIN Tags ON Tags._id=TagMembers.tag_id WHERE allomorph_id=?;");
    query.bindValue(0, allomorphId);
    if(query.exec())
    {
        while(query.next())
        {
            QString label = query.value(0).toString();
            a.addTag( label );
        }
    }
    else
    {
        qWarning() << "SqliteStemList::allomorphFromId()" << query.lastError().text() << query.executedQuery();
    }

    return a;
}

void SqliteStemList::createTables()
{
    QSqlQuery q(QSqlDatabase::database(mDbName));

    if( !q.exec("create table if not exists Stems ( _id integer primary key autoincrement, liftGuid text  );") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec("create table if not exists Allomorphs ( _id integer primary key autoincrement, stem_id integer);") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec("create table if not exists Forms ( _id integer primary key autoincrement, allomorph_id integer, Form text, WritingSystem text );") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec("create table if not exists Glosses ( _id integer primary key autoincrement, stem_id integer, Form text, WritingSystem text );") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec("create table if not exists Tags ( _id integer primary key autoincrement, Label text );") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec("create table if not exists TagMembers ( tag_id integer, allomorph_id );") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();

    /// indices
    if( !q.exec("CREATE INDEX IF NOT EXISTS stemIdIdx ON Allomorphs (stem_id);") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec("CREATE INDEX IF NOT EXISTS glossIdx ON Glosses (stem_id);") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec("CREATE INDEX IF NOT EXISTS formIdx ON Forms (allomorph_id);") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec("CREATE INDEX IF NOT EXISTS tagIdx ON TagMembers (allomorph_id);") )
        qWarning() << "SqliteStemList::createTables()" << q.lastError().text() << q.executedQuery();
}

void SqliteStemList::addStemToDatabase(LexicalStem *stem)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    qlonglong stem_id = 0;

    db.transaction();

    /// first insert a row into the stems table to get a stem_id
    QSqlQuery stemQuery(db);
    stemQuery.prepare("INSERT INTO stems (_id, liftGuid) VALUES (null, ?);");
    stemQuery.bindValue(0, stem->liftGuid() );
    if(stemQuery.exec())
    {
        stem_id = stemQuery.lastInsertId().toLongLong();
        stem->setId(stem_id);
    }
    else
    {
        qWarning() << "SqliteStemList::addStemToDatabase()" << stemQuery.lastError().text() << stemQuery.executedQuery();
        db.rollback();
        return;
    }

    QSqlQuery allomorphQuery(db);
    allomorphQuery.prepare("INSERT INTO allomorphs (stem_id) VALUES (?);");
    allomorphQuery.bindValue(0, stem_id ); /// this will be the same for all subsequent calls

    QSqlQuery formQuery(db);
    formQuery.prepare("INSERT INTO forms (allomorph_id, form, writingsystem) VALUES (?, ?, ?);");

    QSqlQuery tagQuery(db);
    tagQuery.prepare("INSERT INTO TagMembers (tag_id, allomorph_id) VALUES (?, ?);");

    QSetIterator<Allomorph> ai = stem->allomorphIterator();
    while(ai.hasNext())
    {
        Allomorph a = ai.next();

        /// only add Original allomorphs to the SQL database, not Derived
        if( a.type() == Allomorph::Original )
        {
            allomorphQuery.exec(); /// using previously bound values
            qlonglong allomorph_id = allomorphQuery.lastInsertId().toLongLong();

            /// add each form to the Forms table
            QHash<WritingSystem, Form> forms = a.forms();
            QHashIterator<WritingSystem, Form> fi(forms);
            while( fi.hasNext() )
            {
                fi.next();

                /// stem_id bound above
                formQuery.bindValue(0, allomorph_id);
                formQuery.bindValue(1, fi.value().text() );
                formQuery.bindValue(2, fi.key().abbreviation() );

                if(!formQuery.exec())
                {
                    qWarning() << "SqliteStemList::addStemToDatabase()" << formQuery.lastError().text() << formQuery.executedQuery();
                    db.rollback();
                    return;
                }
            }

            /// associate the allomorph with its tags
            QSetIterator<Tag> ti( a.tags() );
            while( ti.hasNext() )
            {
                qlonglong tag_id = ensureTagInDatabase( ti.next().label() );
                tagQuery.bindValue(0, tag_id);
                tagQuery.bindValue(1, allomorph_id);
                if(!tagQuery.exec())
                {
                    qWarning() << "SqliteStemList::addStemToDatabase()" << tagQuery.lastError().text() << tagQuery.executedQuery();
                    db.rollback();
                    return;
                }
            }
        }
    }

    QSqlQuery glossQuery(db);
    glossQuery.prepare("INSERT INTO Glosses (stem_id, form, writingsystem) VALUES (?, ?, ?);");
    glossQuery.bindValue(0, stem_id ); /// this will be the same for all subsequent calls
    QHashIterator<WritingSystem, Form> gi( stem->glosses() );
    while( gi.hasNext() )
    {
        gi.next();
        glossQuery.bindValue(1, gi.value().text() );
        glossQuery.bindValue(2, gi.key().abbreviation() );
        if(!glossQuery.exec())
        {
            qWarning() << "SqliteStemList::addStemToDatabase()" << glossQuery.lastError().text() << glossQuery.executedQuery();
            db.rollback();
            return;
        }
    }

    db.commit();
}

qlonglong SqliteStemList::ensureTagInDatabase(const QString &tag)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    QSqlQuery query(db);
    query.prepare("SELECT _id FROM tags WHERE label=?;");
    query.bindValue(0, tag);
    if(query.exec())
    {
        if( query.first() )
        {
            return query.value(0).toLongLong();
        }
        else
        {
            query.prepare("INSERT INTO tags (label) VALUES (?);");
            query.bindValue(0, tag);
            if( query.exec() )
            {
                return query.lastInsertId().toLongLong();
            }
            else
            {
                qWarning() << "SqliteStemList::readStems()" << query.lastError().text() << query.executedQuery();
                return 0;
            }
        }
    }
    else
    {
        qWarning() << "SqliteStemList::readStems()" << query.lastError().text() << query.executedQuery();
        return 0;
    }
}

QString SqliteStemList::dbName() const
{
    return mDbName;
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

    morphologyReader->recordStemList( sl );

    while(!in.atEnd() && !(in.tokenType() == QXmlStreamReader::EndElement && in.name() == elementName() ) )
    {
        in.readNext();

        if( in.tokenType() == QXmlStreamReader::StartElement )
        {
            if( in.name() == "filename" )
            {
                sl->setDatabasePath( in.readElementText() );
            }
            else if( in.name() == "external-database" )
            {
                sl->setExternalDatabase( in.readElementText() );
            }
            else if( in.name() == "matching-tag" )
            {
                sl->addConditionTag( in.readElementText() );
            }
            else if( CreateAllomorphs::matchesElement(in) )
            {
                sl->setCreateAllomorphs( CreateAllomorphs::readFromXml(in, morphologyReader) );
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

void SqliteStemList::insertStemIntoDataModel(LexicalStem *stem)
{
    stem->generateAllomorphs(mCreateAllomorphs);
    mStems.insert(stem);
    addStemToDatabase(stem);
}

void SqliteStemList::removeStemFromDataModel(qlonglong id)
{
    if( id == -1 )
    {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(mDbName);

    db.transaction();

    QSqlQuery query(db);

    query.prepare("DELETE from TagMembers WHERE allomorph_id IN (SELECT _id FROM Allomorphs WHERE stem_id=?);");
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "SqliteStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    query.prepare("DELETE from forms WHERE allomorph_id IN (SELECT _id FROM Allomorphs WHERE stem_id=?);");
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "SqliteStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    query.prepare("DELETE FROM Glosses WHERE stem_id=?;");
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "SqliteStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    query.prepare("DELETE FROM allomorphs WHERE stem_id=?;");
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "SqliteStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    query.prepare("DELETE FROM stems WHERE _id=?;");
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "SqliteStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    db.commit();
}

QString SqliteStemList::tagsInSqliteList() const
{
    if( mTags.count() == 0 )
    {
        return "()";
    }
    else /// it has at least one
    {
        QString ret = "(";
        QSetIterator<Tag> i( mTags );
        /// before this was a call to label(), it was an automatic cast to QString, which is also the label
        ret += "'" + i.next().label() + "'";
        while( i.hasNext() )
        {
            ret += ",'" + i.next().label() + "'";
        }
        ret += ")";
        return ret;
    }
}
