/*!
  \class MorphologicalModel
  \brief A class representing a model of a particular part of a morphology, e.g., nouns or verbs.
*/


#ifndef MORPHOLOGICALMODEL_H
#define MORPHOLOGICALMODEL_H

#include <QList>

#include "abstractpath.h"
#include "../mortal-engine/datatypes/parsing.h"

namespace ME {

class AbstractNode;
class Form;

class MorphologicalModel : public AbstractPath
{
public:
    MorphologicalModel(const Morphology * morphology);
    MorphologicalModel * copy(MorphologyXmlReader *morphologyReader, const NodeId &idSuffix) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const AbstractNode *doNotFollow) const override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model = nullptr);
    static bool matchesElement(QXmlStreamReader &in);

    bool checkHasOptionalCompletionPath() const override;

    bool isModel() const override;

    bool hasZeroLengthForms() const;
    void setHasZeroLengthForms(bool hasNullForms);

    const Morphology *morphology() const;

private:
    bool mHasZeroLengthForms;
    const Morphology * mMorphology;
};

} // namespace ME

#endif // MORPHOLOGICALMODEL_H
