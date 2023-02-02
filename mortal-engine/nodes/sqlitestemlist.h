#ifndef Q_OS_WASM

#ifndef SQLITESTEMLIST_H
#define SQLITESTEMLIST_H

#include "abstractsqlstemlist.h"

#include "mortal-engine_global.h"

class MORTAL_ENGINE_EXPORT SqliteStemList : public AbstractSqlStemList
{
public:
    explicit SqliteStemList(const MorphologicalModel * model);

    void setDatabasePath(const QString &databasePath);
    void setExternalDatabase(const QString & dbName);

    static void openDatabase(const QString & filename, const QString & databaseName);

    /// element-reading code
    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

    static QString XML_EXTERNAL_DATABASE;
};

#endif // SQLITESTEMLIST_H

#else

class SqliteStemList;

#endif // Q_OS_WASM
