#ifndef ABSTRACTSQLSTEMLIST_H
#define ABSTRACTSQLSTEMLIST_H

#include "abstractstemlist.h"

class AbstractSqlStemList : public AbstractStemList
{
public:
    explicit AbstractSqlStemList(const MorphologicalModel * model);

    const static QString DEFAULT_DBNAME;

    QString dbName() const;
    void setReadGlosses(bool newReadGlosses);

    void readStems(const QHash<QString, WritingSystem> &writingSystems) override;

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
};

#endif // ABSTRACTSQLSTEMLIST_H
