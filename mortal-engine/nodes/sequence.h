#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "abstractpath.h"

class Sequence : public AbstractPath
{
public:
    Sequence(const MorphologicalModel *model);
    ~Sequence() override;

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

#endif // SEQUENCE_H
