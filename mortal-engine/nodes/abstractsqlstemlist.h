#ifndef ABSTRACTSQLSTEMLIST_H
#define ABSTRACTSQLSTEMLIST_H

#include "abstractstemlist.h"

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT AbstractSqlStemList : public AbstractStemList
{
public:
    explicit AbstractSqlStemList(const MorphologicalModel * model);

    const static QString DEFAULT_DBNAME;

    void setConnectionString(const QString &connectionString, const QString &databaseName = QString());

    QString dbName() const;
    void setReadGlosses(bool newReadGlosses);

    void readStems(const QHash<QString, WritingSystem> &writingSystems) override;

    void setExternalDatabase(const QString & dbName);

    static QString XML_EXTERNAL_DATABASE;
    static QString XML_CONNECTION_STRING;
    static QString XML_DATABASE_NAME;

protected:
    /// Queries
    /// These are SQLite queries, and should be overridden in classes using different database engines.
    virtual QString qSelectStemIds() const;
    virtual QString qSelectStemIdsWithTags(const QString & taglist) const;
    virtual QString qSelectStemsSingleQuery() const;
    virtual QString qSelectStemsSingleQueryWithTags(const QString & taglist) const;

    virtual QString qDeleteFromTagMembers() const;
    virtual QString qDeleteFromForms() const;
    virtual QString qDeleteFromGlosses() const;
    virtual QString qDeleteFromAllomorphs() const;
    virtual QString qDeleteFromStems() const;

    virtual QString qSelectAllomorphsFromStemId() const;
    virtual QString qSelectGlossesFromStemId() const;

    virtual QString qSelectFormsFromAllomorphId() const;
    virtual QString qSelectTagLabelsFromAllomorphId() const;

    virtual QString qInsertStem() const;
    virtual QString qInsertAllomorph() const;
    virtual QString qInsertForm() const;
    virtual QString qInsertTagMember() const;
    virtual QString qInsertGloss() const;
    virtual QString qInsertTag() const;

    virtual QString qSelectTagIdFromLabel() const;

    virtual QString qCreateStems() const;
    virtual QString qCreateAllomorphs() const;
    virtual QString qCreateForms() const;
    virtual QString qCreateGlosses() const;
    virtual QString qCreateTags() const;
    virtual QString qCreateTagMembers() const;
    virtual QString qCreateAllomorphsIdx() const;
    virtual QString qCreateGlossesIdx() const;
    virtual QString qCreateFormsIdx() const;
    virtual QString qCreateTagsIdx1() const;
    virtual QString qCreateTagsIdx2() const;

private:
    void readStemsMultipleQueries(const QHash<QString, WritingSystem> &writingSystems);
    void readStemsSingleQuery(const QHash<QString, WritingSystem> &writingSystems);

    void insertStemIntoDataModel( LexicalStem * stem ) override;
    void removeStemFromDataModel( qlonglong id ) override;
    LexicalStem * lexicalStemFromId(qlonglong stemId, const QString &liftGuid, const QHash<QString, WritingSystem> &writingSystems);
    Allomorph allomorphFromId(qlonglong allomorphId, const QHash<QString, WritingSystem> &writingSystems, bool useInGenerations);

    void addStemToDatabase( LexicalStem * stem );
    qlonglong ensureTagInDatabase( const QString & tag );

protected:
    QString mDbName;
    bool mReadGlosses;

    void createTables();
    QString tagsInSqlList() const;
    QString tagIdsInSqlList() const;

    virtual void openDatabase(const QString & connectionString, const QString & databaseName) = 0;
    virtual bool databaseExists(const QString &databaseName) const = 0;
};

#endif // ABSTRACTSQLSTEMLIST_H
