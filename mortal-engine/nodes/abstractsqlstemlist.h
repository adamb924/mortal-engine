#ifndef ABSTRACTSQLSTEMLIST_H
#define ABSTRACTSQLSTEMLIST_H

#include "abstractstemlist.h"

#include "mortal-engine_global.h"

namespace ME {

class MORTAL_ENGINE_EXPORT AbstractSqlStemList : public AbstractStemList
{
public:
    explicit AbstractSqlStemList(const MorphologicalModel * model);

    const static QString DEFAULT_DBNAME;

    void setConnectionString(const QString &connectionString);

    QString dbName() const;
    void setReadGlosses(bool newReadGlosses);

    void readStems(const QHash<QString, WritingSystem> &writingSystems) override;

    void setExternalDatabase(const QString & dbName);

    static QString XML_EXTERNAL_DATABASE;
    static QString XML_CONNECTION_STRING;
    static QString XML_TABLE_PREFIX;

    void setTablePrefix(const QString &newTablePrefix);

    void setDbName(const QString &newDbName);

protected:
    /// Table names
    QString tableStems() const;
    QString tableAllomorphs() const;
    QString tableForms() const;
    QString tableGlosses() const;
    QString tableTags() const;
    QString tableTagMembers() const;

    /// Queries
    virtual QString qSelectStemIds() const = 0;
    virtual QString qSelectStemIdsWithTags(const QString & taglist) const = 0;
    virtual QString qSelectStemsSingleQuery() const = 0;
    virtual QString qSelectStemsSingleQueryWithTags(const QString & taglist) const = 0;

    virtual QString qDeleteFromTagMembers() const = 0;
    virtual QString qDeleteFromForms() const = 0;
    virtual QString qDeleteFromGlosses() const = 0;
    virtual QString qDeleteFromAllomorphs() const = 0;
    virtual QString qDeleteFromStems() const = 0;

    virtual QString qSelectAllomorphsFromStemId() const = 0;
    virtual QString qSelectGlossesFromStemId() const = 0;

    virtual QString qSelectFormsFromAllomorphId() const = 0;
    virtual QString qSelectTagLabelsFromAllomorphId() const = 0;

    virtual QString qInsertStem() const = 0;
    virtual QString qInsertAllomorph() const = 0;
    virtual QString qInsertForm() const = 0;
    virtual QString qInsertTagMember() const = 0;
    virtual QString qInsertGloss() const = 0;
    virtual QString qInsertTag() const = 0;

    virtual QString qSelectTagIdFromLabel() const = 0;

    /// create database tables
    virtual QString qCreateStems() const = 0;
    virtual QString qCreateAllomorphs() const = 0;
    virtual QString qUpdateAllomorphsA() const = 0;
    virtual QString qCreateForms() const = 0;
    virtual QString qCreateGlosses() const = 0;
    virtual QString qCreateTags() const = 0;
    virtual QString qCreateTagMembers() const = 0;

    /// create database indices
    virtual QString qCreateAllomorphsIdx() const = 0;
    virtual QString qCreateGlossesIdx() const = 0;
    virtual QString qCreateFormsIdx() const = 0;
    virtual QString qCreateTagsIdx1() const = 0;
    virtual QString qCreateTagsIdx2() const = 0;


private:
    void readStemsMultipleQueries(const QHash<QString, WritingSystem> &writingSystems);
    void readStemsSingleQuery(const QHash<QString, WritingSystem> &writingSystems);

    void insertStemIntoDataModel( LexicalStem * stem ) override;
    void removeStemFromDataModel( qlonglong id ) override;
    LexicalStem * lexicalStemFromId(qlonglong stemId, const QString &liftGuid, const QHash<QString, WritingSystem> &writingSystems);
    Allomorph allomorphFromId(qlonglong allomorphId, const QHash<QString, WritingSystem> &writingSystems, bool useInGenerations, const QString & portmanteau);

    void addStemToDatabase( LexicalStem * stem );
    qlonglong ensureTagInDatabase( const QString & tag );

    /// prevent subclasses from accessing these, so that they have to use the table name functions
    QString mTablePrefix;
    static QString TABLE_STEMS;
    static QString TABLE_ALLOMORPHS;
    static QString TABLE_FORMS;
    static QString TABLE_GLOSSES;
    static QString TABLE_TAGS;
    static QString TABLE_TAGMEMBERS;

protected:
    QString mDbName;
    bool mReadGlosses;

    void createTables();
    QString tagsInSqlList() const;
    QString tagIdsInSqlList() const;

    virtual void openDatabase(const QString & connectionString, const QString & databaseName) = 0;
};

} // namespace ME

#endif // ABSTRACTSQLSTEMLIST_H
