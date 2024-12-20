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

    /// element-reading code
    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

protected:
    void openDatabase(const QString & connectionString, const QString & databaseName) override;

    QString qCreateStems() const override;
    QString qCreateAllomorphs() const override;
    QString qCreateForms() const override;
    QString qCreateGlosses() const override;
    QString qCreateTags() const override;
    QString qCreateTagMembers() const override;
    QString qCreateAllomorphsIdx() const override;
    QString qCreateGlossesIdx() const override;
    QString qCreateFormsIdx() const override;
    QString qCreateTagsIdx1() const override;
    QString qCreateTagsIdx2() const override;

    QString qSelectStemIdsWithTags(const QString & taglist) const override;
    QString qInsertStem() const override;
};

} // namespace ME

#endif // SQLSERVERSTEMLIST_H
