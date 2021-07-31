#ifndef PATH_H
#define PATH_H

#include "abstractpath.h"

#include <QSet>

class Path : public AbstractPath
{
public:
    explicit Path(const MorphologicalModel * model);
    ~Path() override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);
};

#endif // PATH_H
