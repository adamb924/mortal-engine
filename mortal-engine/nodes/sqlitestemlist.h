#ifndef SQLITESTEMLIST_H
#define SQLITESTEMLIST_H

#include "abstractstemlist.h"

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT SqliteStemList : public AbstractStemList
{
public:
    explicit SqliteStemList(const MorphologicalModel * model);

    static QString DEFAULT_DBNAME;

    void setDatabasePath(const QString &databasePath);
    void setExternalDatabase(const QString & dbName);

    void readStems(const QHash<QString, WritingSystem> &writingSystems) override;

    QString dbName() const;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

    static void openDatabase(const QString & filename, const QString & databaseName);

private:
    void insertStemIntoDataModel( LexicalStem * stem ) override;
    void removeStemFromDataModel( qlonglong id ) override;
    LexicalStem * lexicalStemFromId(qlonglong stemId, const QString &liftGuid, const QHash<QString, WritingSystem> &writingSystems);
    Allomorph allomorphFromId(qlonglong allomorphId, const QHash<QString, WritingSystem> &writingSystems);

    QString tagsInSqliteList() const;

    void createTables();
    void addStemToDatabase( LexicalStem * stem );
    qlonglong ensureTagInDatabase( const QString & tag );

    QString mDatabasePath;
    QString mDbName;
};

#endif // SQLITESTEMLIST_H
