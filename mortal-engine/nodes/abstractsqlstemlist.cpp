#include "abstractsqlstemlist.h"

#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlError>
#include <QXmlStreamReader>
#include <QtDebug>

#include "datatypes/lexicalstem.h"

const QString AbstractSqlStemList::DEFAULT_DBNAME = "SQLITE_STEM_LIST";
QString AbstractSqlStemList::XML_CONNECTION_STRING = "connection-string";
QString AbstractSqlStemList::XML_EXTERNAL_DATABASE = "external-database";

AbstractSqlStemList::AbstractSqlStemList(const MorphologicalModel *model) :
    AbstractStemList(model),
    mDbName(DEFAULT_DBNAME),
    mReadGlosses(true)
{

}

void AbstractSqlStemList::setConnectionString(const QString &connectionString)
{
    /// https://stackoverflow.com/a/16568641/1447002
    mDbName = QString("%1_0x%2").arg(DEFAULT_DBNAME).arg( reinterpret_cast<quintptr>(this),
                        QT_POINTER_SIZE * 2, 16, QChar('0'));

    openDatabase(connectionString, mDbName);
    createTables();
}

void AbstractSqlStemList::setReadGlosses(bool newReadGlosses)
{
    mReadGlosses = newReadGlosses;
}

QString AbstractSqlStemList::dbName() const
{
    return mDbName;
}

void AbstractSqlStemList::readStems(const QHash<QString, WritingSystem> & writingSystems)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    if( !db.isOpen() )
        return;

    /// this is the default. It's about 50% slower.
    if( mReadGlosses )
    {
        readStemsMultipleQueries(writingSystems);
    }
    else /// this reads the stems about 50% faster. As a side effect it doesn't read the stem glosses.
    {
        readStemsSingleQuery(writingSystems);
    }

    /*
     * It's not nice to have this show up with every run. TODO think about a verbose warning mode.
    if( mStems.isEmpty() )
    {
        qWarning() << "No stems were read for the node" << debugIdentifier();
    */
}

void AbstractSqlStemList::setExternalDatabase(const QString &dbName)
{
    mDbName = dbName;

    QSqlDatabase db = QSqlDatabase::database(mDbName);
    if( !db.isValid() )
        qWarning() << QString("The database %1 returns false for isValid().").arg(dbName);
    if( !db.isOpen() )
        qWarning() << QString("The database %1 returns false for isOpen().").arg(dbName);

    createTables();
}

void AbstractSqlStemList::readStemsMultipleQueries(const QHash<QString, WritingSystem> &writingSystems)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    /// this is where the tag filtering takes place
    QSqlQuery query(db);
    if( mTags.isEmpty() )
    {
        query.prepare(qSelectStemIds());
    }
    else
    {
        const QString inStatement = tagsInSqlList();
        query.prepare( qSelectStemIdsWithTags(inStatement) );
    }

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
        qWarning() << "AbstractSqlStemList::readStemsMultipleQueries()" << query.lastError().text() << query.executedQuery();
    }
}

void AbstractSqlStemList::readStemsSingleQuery(const QHash<QString, WritingSystem> &writingSystems)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    /// this is where the tag filtering takes place
    QSqlQuery query(db);
    if( mTags.isEmpty() )
    {
        query.prepare(qSelectStemsSingleQuery());
    }
    else
    {
        const QString inStatement = tagIdsInSqlList();
        query.prepare(qSelectStemsSingleQueryWithTags(inStatement));
    }

    if(query.exec())
    {
        qlonglong previousAllomorphId = -1;
        Allomorph allomorph(Allomorph::Original);
        qlonglong previousStemId = -1;
        LexicalStem * stem = nullptr;

        while(query.next())
        {
            /// every row has a new form
            const WritingSystem ws = writingSystems.value( query.value(5).toString() );
            const Form f( ws, query.value(4).toString() );

            const qlonglong stemId = query.value(0).toLongLong();
            if( stemId != previousStemId )
            {
                /// first save the previous LexicalStem
                if( stem != nullptr )
                {
                    mStems << stem;
                }
                /// create a new LexicalStem
                stem = new LexicalStem;
                stem->setId(stemId);
            }
            previousStemId = stemId;

            const qlonglong allomorphId = query.value(2).toLongLong();
            if( allomorphId != previousAllomorphId )
            {
                /// first, save the previous allomorph, if it has forms
                if( ! allomorph.isEmpty() )
                {
                    stem->insert( allomorph );
                }
                /// then reset the allomorph object, and add this form
                allomorph = Allomorph(Allomorph::Original);
                allomorph.setTags( Tag::fromString( query.value(6).toString() ) );
            }
            allomorph.setForm( f );
            previousAllomorphId = allomorphId;
        }

        /// now at the end, add the allomorph to the stem, and the stem to the list
        if( stem != nullptr )
        {
            stem->insert( allomorph );
            mStems << stem;
        }
    }
    else
    {
        qWarning() << "AbstractSqlStemList::readStemsSingleQuery()" << query.lastError().text() << query.executedQuery();
    }
}

void AbstractSqlStemList::insertStemIntoDataModel(LexicalStem *stem)
{
    stem->generateAllomorphs(mCreateAllomorphs);
    mStems.insert(stem);
    addStemToDatabase(stem);
}

void AbstractSqlStemList::removeStemFromDataModel(qlonglong id)
{
    if( id == -1 )
    {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database(mDbName);

    db.transaction();

    QSqlQuery query(db);

    query.prepare(qDeleteFromTagMembers());
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "AbstractSqlStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    query.prepare(qDeleteFromForms());
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "AbstractSqlStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    query.prepare(qDeleteFromGlosses());
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "AbstractSqlStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    query.prepare(qDeleteFromAllomorphs());
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "AbstractSqlStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    query.prepare(qDeleteFromStems());
    query.bindValue(0, id );
    if( !query.exec() )
    {
        qWarning() << "AbstractSqlStemList::removeStemFromDataModel()" << query.lastError().text() << query.executedQuery();
        db.rollback();
        return;
    }

    db.commit();
}

LexicalStem *AbstractSqlStemList::lexicalStemFromId(qlonglong stemId, const QString & liftGuid, const QHash<QString, WritingSystem> &writingSystems)
{
    LexicalStem * ls = new LexicalStem;
    ls->setId(stemId);
    if( !liftGuid.isEmpty() )
        ls->setLiftGuid( liftGuid );

    QSqlDatabase db = QSqlDatabase::database(mDbName);
    QSqlQuery query(db);

    query.prepare(qSelectAllomorphsFromStemId());
    query.bindValue( 0, stemId );
    if(query.exec())
    {
        while(query.next())
        {
            qlonglong allomorph_id = query.value(0).toLongLong();
            bool useInGenerations =  query.value(1).toLongLong() > 0;
            ls->insert( allomorphFromId( allomorph_id, writingSystems, useInGenerations ) );
        }
    }
    else
    {
        qWarning() << "AbstractSqlStemList::lexicalStemFromId()" << query.lastError().text() << query.executedQuery();
    }
    query.finish();

    query.prepare(qSelectGlossesFromStemId());
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
        qWarning() << "AbstractSqlStemList::readStems()" << query.lastError().text() << query.executedQuery();
    }

    return ls;
}

Allomorph AbstractSqlStemList::allomorphFromId(qlonglong allomorphId, const QHash<QString, WritingSystem> &writingSystems, bool useInGenerations)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);
    QSqlQuery query(db);

    Allomorph a(Allomorph::Original);
    a.setId(allomorphId);
    a.setUseInGenerations(useInGenerations);

    query.prepare(qSelectFormsFromAllomorphId());
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
        qWarning() << "AbstractSqlStemList::allomorphFromId()" << query.lastError().text() << query.executedQuery();
    }
    query.finish();

    query.prepare(qSelectTagLabelsFromAllomorphId());
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
        qWarning() << "AbstractSqlStemList::allomorphFromId()" << query.lastError().text() << query.executedQuery();
    }

    return a;
}

void AbstractSqlStemList::createTables()
{
    QSqlQuery q(QSqlDatabase::database(mDbName));

    if( !q.exec(qCreateStems()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec(qCreateAllomorphs()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec(qCreateForms()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec(qCreateGlosses()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec(qCreateTags()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec(qCreateTagMembers()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    /// indices
    if( !q.exec(qCreateAllomorphsIdx()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec(qCreateGlossesIdx()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec(qCreateFormsIdx()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec(qCreateTagsIdx1()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();

    if( !q.exec(qCreateTagsIdx2()) )
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
}

void AbstractSqlStemList::addStemToDatabase(LexicalStem *stem)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    qlonglong stem_id = 0;

    db.transaction();

    /// first insert a row into the stems table to get a stem_id
    QSqlQuery stemQuery(db);
    stemQuery.prepare(qInsertStem());
    stemQuery.bindValue(0, stem->liftGuid() );
    if(stemQuery.exec())
    {
        stem_id = stemQuery.lastInsertId().toLongLong();
        stem->setId(stem_id);
    }
    else
    {
        qWarning() << "AbstractSqlStemList::addStemToDatabase()" << stemQuery.lastError().text() << stemQuery.executedQuery();
        db.rollback();
        return;
    }

    QSqlQuery allomorphQuery(db);
    allomorphQuery.prepare(qInsertAllomorph());
    allomorphQuery.bindValue(0, stem_id ); /// this will be the same for all subsequent calls

    QSqlQuery formQuery(db);
    formQuery.prepare(qInsertForm());

    QSqlQuery tagQuery(db);
    tagQuery.prepare(qInsertTagMember());

    QListIterator<Allomorph> ai = stem->allomorphIterator();
    while(ai.hasNext())
    {
        Allomorph a = ai.next();

        /// only add Original allomorphs to the SQL database, not Derived
        if( a.type() == Allomorph::Original )
        {
            allomorphQuery.bindValue(1, a.useInGenerations() ); /// this will be the same for all subsequent calls
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
                    qWarning() << "AbstractSqlStemList::addStemToDatabase()" << formQuery.lastError().text() << formQuery.executedQuery();
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
                    qWarning() << "AbstractSqlStemList::addStemToDatabase()" << tagQuery.lastError().text() << tagQuery.executedQuery();
                    db.rollback();
                    return;
                }
            }
        }
    }

    QSqlQuery glossQuery(db);
    glossQuery.prepare(qInsertGloss());
    glossQuery.bindValue(0, stem_id ); /// this will be the same for all subsequent calls
    QHashIterator<WritingSystem, Form> gi( stem->glosses() );
    while( gi.hasNext() )
    {
        gi.next();
        glossQuery.bindValue(1, gi.value().text() );
        glossQuery.bindValue(2, gi.key().abbreviation() );
        if(!glossQuery.exec())
        {
            qWarning() << "AbstractSqlStemList::addStemToDatabase()" << glossQuery.lastError().text() << glossQuery.executedQuery();
            db.rollback();
            return;
        }
    }

    db.commit();
}

qlonglong AbstractSqlStemList::ensureTagInDatabase(const QString &tag)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    QSqlQuery query(db);
    query.prepare(qSelectTagIdFromLabel());
    query.bindValue(0, tag);
    if(query.exec())
    {
        if( query.first() )
        {
            return query.value(0).toLongLong();
        }
        else
        {
            query.prepare(qInsertTag());
            query.bindValue(0, tag);
            if( query.exec() )
            {
                return query.lastInsertId().toLongLong();
            }
            else
            {
                qWarning() << "AbstractSqlStemList::readStems()" << query.lastError().text() << query.executedQuery();
                return 0;
            }
        }
    }
    else
    {
        qWarning() << "AbstractSqlStemList::readStems()" << query.lastError().text() << query.executedQuery();
        return 0;
    }
}

QString AbstractSqlStemList::tagsInSqlList() const
{
    if( mTags.count() == 0 )
    {
        return QString();
    }
    else /// it has at least one
    {
        QStringList asList;
        QSetIterator<Tag> i( mTags );
        while( i.hasNext() )
        {
            asList << i.next().label();
        }
        const QString ret("'" + asList.join("','") + "'");
        return ret;
    }
}

QString AbstractSqlStemList::tagIdsInSqlList() const
{
    if( mTags.count() == 0 )
    {
        return QString();
    }
    else /// it has at least one
    {
        QSqlDatabase db = QSqlDatabase::database(mDbName);
        QSqlQuery query(db);
        query.prepare(qSelectTagIdFromLabel());

        QStringList ids;
        QSetIterator<Tag> i( mTags );
        while( i.hasNext() )
        {
            query.bindValue( 0, i.next().label() );
            if( !query.exec() )
            {
                qWarning() << "AbstractSqlStemList::tagIdsInSqlList()" << query.lastError().text() << query.executedQuery();
                break;
            }
            if( query.next() )
            {
                ids << query.value(0).toString();
            }
        }
        return ids.join(",");
    }
}

QString AbstractSqlStemList::qSelectStemIds() const
{
    return "SELECT DISTINCT _id, liftGuid from Stems;";
}

QString AbstractSqlStemList::qSelectStemIdsWithTags(const QString &taglist) const
{
    return "SELECT DISTINCT stem_id, liftGuid from TagMembers LEFT JOIN Tags, Allomorphs,Stems ON Tags._id=TagMembers.tag_id AND Allomorphs._id=TagMembers.allomorph_id AND Stems._id=Allomorphs.stem_id WHERE Label IN ("+taglist+");";
}

QString AbstractSqlStemList::qSelectStemsSingleQuery() const
{
    return "SELECT stem_id, liftGuid,allomorphs._id AS allomorph_id,use_in_generations,Form,writingsystem,group_concat(label) "
           "FROM Stems, Allomorphs, Forms, Tags, TagMembers "
           "ON "
               "Stems._id=Allomorphs.stem_id "
               "AND Allomorphs._id=Forms.allomorph_id "
               "AND Tags._id=TagMembers.tag_id "
               "AND TagMembers.allomorph_id=Forms.allomorph_id "
           "GROUP BY Forms._id;";
}

QString AbstractSqlStemList::qSelectStemsSingleQueryWithTags(const QString &taglist) const
{
    return "SELECT stem_id, liftGuid,allomorphs._id AS allomorph_id,use_in_generations,Form,writingsystem,group_concat(label) "
                          "FROM Stems, Allomorphs, Forms, Tags, TagMembers "
                          "ON "
                              "Stems._id=Allomorphs.stem_id "
                              "AND Allomorphs._id=Forms.allomorph_id "
                              "AND Tags._id=TagMembers.tag_id "
                              "AND TagMembers.allomorph_id=Forms.allomorph_id "
                              "WHERE TagMembers.allomorph_id IN (SELECT allomorph_id FROM TagMembers WHERE tag_id IN ( " + taglist + " ) ) "
                              "GROUP BY Forms._id;";
}

QString AbstractSqlStemList::qDeleteFromTagMembers() const
{
    return "DELETE from TagMembers WHERE allomorph_id IN (SELECT _id FROM Allomorphs WHERE stem_id=?);";
}

QString AbstractSqlStemList::qDeleteFromForms() const
{
    return "DELETE from forms WHERE allomorph_id IN (SELECT _id FROM Allomorphs WHERE stem_id=?);";
}

QString AbstractSqlStemList::qDeleteFromGlosses() const
{
    return "DELETE FROM Glosses WHERE stem_id=?;";
}

QString AbstractSqlStemList::qDeleteFromAllomorphs() const
{
    return "DELETE FROM allomorphs WHERE stem_id=?;";
}

QString AbstractSqlStemList::qDeleteFromStems() const
{
    return "DELETE FROM stems WHERE _id=?;";
}

QString AbstractSqlStemList::qSelectAllomorphsFromStemId() const
{
    return "SELECT _id,use_in_generations FROM Allomorphs WHERE stem_id=?;";
}

QString AbstractSqlStemList::qSelectGlossesFromStemId() const
{
    return "SELECT Form, WritingSystem FROM Glosses WHERE stem_id=?;";
}

QString AbstractSqlStemList::qSelectFormsFromAllomorphId() const
{
    return "SELECT form, writingsystem FROM Forms WHERE allomorph_id=?;";
}

QString AbstractSqlStemList::qSelectTagLabelsFromAllomorphId() const
{
    return "SELECT label from TagMembers LEFT JOIN Tags ON Tags._id=TagMembers.tag_id WHERE allomorph_id=?;";
}

QString AbstractSqlStemList::qInsertStem() const
{
    return "INSERT INTO stems (_id, liftGuid) VALUES (null, ?);";
}

QString AbstractSqlStemList::qInsertAllomorph() const
{
    return "INSERT INTO allomorphs (stem_id,use_in_generations) VALUES (?,?);";
}

QString AbstractSqlStemList::qInsertForm() const
{
    return "INSERT INTO forms (allomorph_id, form, writingsystem) VALUES (?, ?, ?);";
}

QString AbstractSqlStemList::qInsertTagMember() const
{
    return "INSERT INTO TagMembers (tag_id, allomorph_id) VALUES (?, ?);";
}

QString AbstractSqlStemList::qInsertGloss() const
{
    return "INSERT INTO Glosses (stem_id, form, writingsystem) VALUES (?, ?, ?);";
}

QString AbstractSqlStemList::qInsertTag() const
{
    return "INSERT INTO tags (label) VALUES (?);";
}

QString AbstractSqlStemList::qSelectTagIdFromLabel() const
{
    return "SELECT _id FROM tags WHERE label=?;";
}

QString AbstractSqlStemList::qCreateStems() const
{
    return "create table if not exists Stems ( _id integer primary key autoincrement, liftGuid text  );";
}

QString AbstractSqlStemList::qCreateAllomorphs() const
{
    return "create table if not exists Allomorphs ( _id integer primary key autoincrement, stem_id integer, use_in_generations integer default 1);";
}

QString AbstractSqlStemList::qCreateForms() const
{
    return "create table if not exists Forms ( _id integer primary key autoincrement, allomorph_id integer, Form text, WritingSystem text );";
}

QString AbstractSqlStemList::qCreateGlosses() const
{
    return "create table if not exists Glosses ( _id integer primary key autoincrement, stem_id integer, Form text, WritingSystem text );";
}

QString AbstractSqlStemList::qCreateTags() const
{
    return "create table if not exists Tags ( _id integer primary key autoincrement, Label text );";
}

QString AbstractSqlStemList::qCreateTagMembers() const
{
    return "create table if not exists TagMembers ( tag_id integer, allomorph_id );";
}

QString AbstractSqlStemList::qCreateAllomorphsIdx() const
{
    return "CREATE INDEX IF NOT EXISTS stemIdIdx ON Allomorphs (stem_id);";
}

QString AbstractSqlStemList::qCreateGlossesIdx() const
{
    return "CREATE INDEX IF NOT EXISTS glossIdx ON Glosses (stem_id);";
}

QString AbstractSqlStemList::qCreateFormsIdx() const
{
    return "CREATE INDEX IF NOT EXISTS formIdx ON Forms (allomorph_id);";
}

QString AbstractSqlStemList::qCreateTagsIdx1() const
{
    return "CREATE INDEX IF NOT EXISTS tagIdx ON TagMembers (allomorph_id);";
}

QString AbstractSqlStemList::qCreateTagsIdx2() const
{
    return "CREATE INDEX IF NOT EXISTS tagIdxTwo ON TagMembers (tag_id);";
}
