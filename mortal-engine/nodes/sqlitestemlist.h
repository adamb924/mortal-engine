#ifndef Q_OS_WASM

#ifndef SQLITESTEMLIST_H
#define SQLITESTEMLIST_H

#include "abstractsqlstemlist.h"

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT SqliteStemList : public AbstractSqlStemList
{
public:
    explicit SqliteStemList(const MorphologicalModel * model);

    static void openSqliteDatabase(const QString & filename, const QString & databaseName);

    /// element-reading code
    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

protected:
    void openDatabase(const QString & connectionString, const QString & databaseName) override;
    bool databaseExists(const QString &databaseName) const;
};

#endif // SQLITESTEMLIST_H

#else

class SqliteStemList;

#endif // Q_OS_WASM
