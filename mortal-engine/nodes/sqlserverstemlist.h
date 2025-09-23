#ifndef SQLSERVERSTEMLIST_H
#define SQLSERVERSTEMLIST_H

#include "abstractsqlstemlist.h"

#include "mortal-engine_global.h"

namespace ME {

class MORTAL_ENGINE_EXPORT SqlServerStemList : public AbstractSqlStemList
{
public:
    explicit SqlServerStemList(const MorphologicalModel * model);
    SqlServerStemList();

    static void openSqlServerDatabase(const QString & connectionString, const QString & databaseName);
    static void cloneSqlServerDatabase(const QString & databaseName, const QString & newConnectionName);

    /// element-reading code
    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

protected:
    void openDatabase(const QString & connectionString, const QString & databaseName) const override;
    void cloneDatabase(const QString & databaseName, const QString & newConnectionName) const override;

    QString qSelectStemIds() const override;
    QString qSelectStemIdsWithTags(const QString & taglist) const override;
    QString qSelectStemsSingleQuery() const override;
    QString qSelectStemsSingleQueryWithTags(const QString & taglist) const override;

    QString qDeleteFromTagMembers() const override;
    QString qDeleteFromForms() const override;
    QString qDeleteFromGlosses() const override;
    QString qDeleteFromAllomorphs() const override;
    QString qDeleteFromStems() const override;

    QString qSelectAllomorphsFromStemId() const override;
    QString qSelectGlossesFromStemId() const override;

    QString qSelectFormsFromAllomorphId() const override;
    QString qSelectTagLabelsFromAllomorphId() const override;

    QString qInsertStem() const override;
    QString qReplaceStem() const override;
    QString qInsertAllomorph() const override;
    QString qInsertForm() const override;
    QString qInsertTagMember() const override;
    QString qInsertGloss() const override;
    QString qInsertTag() const override;

    QString qSelectTagIdFromLabel() const override;

    QString qCreateStems() const override;
    QString qCreateAllomorphs() const override;
    QString qUpdateAllomorphsA() const override;
    QString qCreateForms() const override;
    QString qCreateGlosses() const override;
    QString qCreateTags() const override;
    QString qCreateTagMembers() const override;
    QString qCreateAllomorphsIdx() const override;
    QString qCreateGlossesIdx() const override;
    QString qCreateFormsIdx() const override;
    QString qCreateTagsIdx1() const override;
    QString qCreateTagsIdx2() const override;

private:
    static QString XML_DATABASE_NAME;

};

} // namespace ME

#endif // SQLSERVERSTEMLIST_H
