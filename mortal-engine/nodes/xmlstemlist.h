#ifndef STEMLIST_H
#define STEMLIST_H

#include "abstractstemlist.h"

#include <QSet>

class WritingSystem;

class XmlStemList : public AbstractStemList
{
public:
    explicit XmlStemList(const MorphologicalModel * model);

    void setFilename(const QString &filename);

    void readStems( const QHash<QString,WritingSystem> &writingSystems ) override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);

private:
    void insertStemIntoDataModel( LexicalStem * stem ) override;
    void removeStemFromDataModel( qlonglong id ) override;

    QString mFilename;
};

#endif // STEMLIST_H
