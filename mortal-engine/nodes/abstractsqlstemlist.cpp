#include "abstractsqlstemlist.h"

#include <QSqlQuery>
#include <QSqlDriver>
#include <QSqlError>
#include <QXmlStreamReader>
#include <QtDebug>
#include <QElapsedTimer>

#include "datatypes/lexicalstem.h"

using namespace ME;

const QString AbstractSqlStemList::DEFAULT_DBNAME = "SQLITE_STEM_LIST";
QString AbstractSqlStemList::XML_CONNECTION_STRING = "connection-string";
QString AbstractSqlStemList::XML_EXTERNAL_DATABASE = "external-database";
QString AbstractSqlStemList::XML_TABLE_PREFIX = "table-prefix";

QString AbstractSqlStemList::TABLE_STEMS = "Stems";
QString AbstractSqlStemList::TABLE_ALLOMORPHS = "Allomorphs";
QString AbstractSqlStemList::TABLE_FORMS = "Forms";
QString AbstractSqlStemList::TABLE_GLOSSES = "Glosses";
QString AbstractSqlStemList::TABLE_TAGS = "Tags";
QString AbstractSqlStemList::TABLE_TAGMEMBERS = "TagMembers";

QString AbstractSqlStemList::STEM_CONNECTION = "stem-connection";
QString AbstractSqlStemList::ALLOMORPH_CONNECTION = "allomorph-connection";

AbstractSqlStemList::AbstractSqlStemList(const MorphologicalModel *model) :
    AbstractStemList(model),
    mDbName(DEFAULT_DBNAME),
    mReadGlosses(true),
    mCreateTables(true)
{

}

AbstractSqlStemList::~AbstractSqlStemList()
{
    /// 2024-12-31: I'm not actually sure this is necessary.
    foreach(QString connectionName, QSqlDatabase::connectionNames())
    {
        QSqlDatabase::removeDatabase(connectionName);
    }
}

void AbstractSqlStemList::setConnectionString(const QString &connectionString)
{
    /// TODO 2024-12-30: I can't recall why this was ever here. It's poorly done since
    /// mDbName is set elsewhere. I wonder if this supposed to be used for some kind
    /// of multithreading.
    /// https://stackoverflow.com/a/16568641/1447002
    // mDbName = QString("%1_0x%2").arg(DEFAULT_DBNAME).arg( reinterpret_cast<quintptr>(this),
    //                     QT_POINTER_SIZE * 2, 16, QChar('0'));

    openDatabase(connectionString, mDbName);
    if( mCreateTables )
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

    if( mCreateTables )
        createTables();
}

void AbstractSqlStemList::readStemsMultipleQueries(const QHash<QString, WritingSystem> &writingSystems)
{
    QElapsedTimer timer;
    timer.start();

    QSqlDatabase db = QSqlDatabase::database(mDbName);

    openAlternateConnections();

    /// this is where the tag filtering takes place
    QSqlQuery query(db);
    query.setForwardOnly(true);
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

    qInfo().noquote() << QString("Stems read in %1 ms").arg(timer.elapsed());
}

void AbstractSqlStemList::readStemsSingleQuery(const QHash<QString, WritingSystem> &writingSystems)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    /// this is where the tag filtering takes place
    QSqlQuery query(db);
    query.setForwardOnly(true);
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
        qlonglong previousStemId = -1;
        qlonglong previousAllomorphId = -1;
        Allomorph allomorph(Allomorph::Original);
        LexicalStem * stem = nullptr;

        while(query.next())
        {
            /// every row has a new form
            const WritingSystem ws = writingSystems.value( query.value(5).toString() );
            const Form f( ws, query.value(4).toString() );

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
                allomorph.setId(allomorphId);
                allomorph.setTags( Tag::fromString( query.value(6).toString() ) );
                if( query.value(7).toString().length() > 0 )
                    allomorph.setPortmanteau(Portmanteau( query.value(7).toString() ));
            }
            allomorph.setForm( f );

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
            previousAllomorphId = allomorphId;
        }

        /// now at the end, add the allomorph to the stem, and the stem to the list
        if( stem != nullptr && !allomorph.isEmpty() )
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
    query.setForwardOnly(true);

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

    QSqlDatabase db = QSqlDatabase::database(stemConnectionName());
    QSqlQuery query(db);
    query.setForwardOnly(true);

    query.prepare(qSelectAllomorphsFromStemId());
    query.bindValue( 0, stemId );
    if(query.exec())
    {
        while(query.next())
        {
            qlonglong allomorph_id = query.value(0).toLongLong();
            bool useInGenerations =  query.value(1).toLongLong() > 0;
            QString portmanteau = query.value(2).toString();
            ls->insert( allomorphFromId( allomorph_id, writingSystems, useInGenerations, portmanteau ) );
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
    query.finish();

    return ls;
}

Allomorph AbstractSqlStemList::allomorphFromId(qlonglong allomorphId, const QHash<QString, WritingSystem> &writingSystems, bool useInGenerations, const QString &portmanteau)
{
    QSqlDatabase db = QSqlDatabase::database(allomorphConnectionName());
    QSqlQuery query(db);
    query.setForwardOnly(true);

    Allomorph a(Allomorph::Original);
    a.setId(allomorphId);
    a.setUseInGenerations(useInGenerations);
    if( portmanteau.length() > 0 )
        a.setPortmanteau( Portmanteau(portmanteau) );

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
    q.setForwardOnly(true);

    if( !q.exec(qCreateStems()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    if( !q.exec(qCreateAllomorphs()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    /// ignoring errors on this, since it will produce an error when it applies vaccuously
    q.exec(qUpdateAllomorphsA());

    if( !q.exec(qCreateForms()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    if( !q.exec(qCreateGlosses()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    if( !q.exec(qCreateTags()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    if( !q.exec(qCreateTagMembers()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    /// indices
    if( !q.exec(qCreateAllomorphsIdx()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    if( !q.exec(qCreateGlossesIdx()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    if( !q.exec(qCreateFormsIdx()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    if( !q.exec(qCreateTagsIdx1()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }

    if( !q.exec(qCreateTagsIdx2()) )
    {
        qWarning() << "AbstractSqlStemList::createTables()" << q.lastError().text() << q.executedQuery();
        return;
    }
}

void AbstractSqlStemList::addStemToDatabase(LexicalStem *stem)
{
    QSqlDatabase db = QSqlDatabase::database(mDbName);

    qlonglong stem_id = 0;

    db.transaction();

    /// first insert a row into the stems table to get a stem_id
    QSqlQuery stemQuery(db);
    stemQuery.setForwardOnly(true);
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
    allomorphQuery.setForwardOnly(true);
    allomorphQuery.prepare(qInsertAllomorph());
    allomorphQuery.bindValue(0, stem_id ); /// this will be the same for all subsequent calls

    QSqlQuery formQuery(db);
    formQuery.setForwardOnly(true);
    formQuery.prepare(qInsertForm());

    QSqlQuery tagQuery(db);
    tagQuery.setForwardOnly(true);
    tagQuery.prepare(qInsertTagMember());

    QListIterator<Allomorph> ai = stem->allomorphIterator();
    while(ai.hasNext())
    {
        Allomorph a = ai.next();

        /// only add Original allomorphs to the SQL database, not Derived
        if( a.type() == Allomorph::Original )
        {
            allomorphQuery.bindValue(1, a.useInGenerations() ); /// this will be the same for all subsequent calls
            allomorphQuery.bindValue(2, a.portmanteau().morphemes().toString() ); /// this will be the same for all subsequent calls
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
    glossQuery.setForwardOnly(true);
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
    query.setForwardOnly(true);
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

void AbstractSqlStemList::openAlternateConnections() const
{
    if( !QSqlDatabase::database(stemConnectionName()).isOpen() )
        cloneDatabase(mDbName, stemConnectionName());
    if( !QSqlDatabase::database(allomorphConnectionName()).isOpen() )
        cloneDatabase(mDbName, allomorphConnectionName());
}

QString AbstractSqlStemList::stemConnectionName() const
{
    return mDbName + ":" + STEM_CONNECTION;
}

QString AbstractSqlStemList::allomorphConnectionName() const
{
    return mDbName + ":" + ALLOMORPH_CONNECTION;
}

void AbstractSqlStemList::setDbName(const QString &newDbName)
{
    mDbName = newDbName;
}

void AbstractSqlStemList::setTablePrefix(const QString &newTablePrefix)
{
    mTablePrefix = newTablePrefix;
}

QString AbstractSqlStemList::tableStems() const
{
    return mTablePrefix + TABLE_STEMS;
}

QString AbstractSqlStemList::tableAllomorphs() const
{
    return mTablePrefix + TABLE_ALLOMORPHS;
}

QString AbstractSqlStemList::tableForms() const
{
    return mTablePrefix + TABLE_FORMS;
}

QString AbstractSqlStemList::tableGlosses() const
{
    return mTablePrefix + TABLE_GLOSSES;
}

QString AbstractSqlStemList::tableTags() const
{
    return mTablePrefix + TABLE_TAGS;
}

QString AbstractSqlStemList::tableTagMembers() const
{
    return mTablePrefix + TABLE_TAGMEMBERS;
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
        query.setForwardOnly(true);
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
