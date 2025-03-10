#ifndef PATH_H
#define PATH_H

#include "abstractpath.h"

#include <QSet>

namespace ME {

class Path : public AbstractPath
{
public:
    explicit Path(const MorphologicalModel * model);
    ~Path() override;
    Path * copy(MorphologyXmlReader *morphologyReader, const NodeId &idSuffix) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const AbstractNode *doNotFollow) const override;

    bool checkHasOptionalCompletionPath() const override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);
};

} // namespace ME

#endif // PATH_H
