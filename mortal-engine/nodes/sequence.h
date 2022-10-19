#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "abstractpath.h"

class Sequence : public AbstractPath
{
public:
    explicit Sequence(const MorphologicalModel *model);
    ~Sequence() override;
    Sequence * copy(MorphologyXmlReader *morphologyReader, const QString & idSuffix) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     *
     * @return QString The logging output.
     */
    QString summary(const AbstractNode *doNotFollow) const override;

    bool isSequence() const override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model);
    static bool matchesElement(QXmlStreamReader &in);
};

#endif // SEQUENCE_H
