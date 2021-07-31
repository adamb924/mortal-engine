/*!
  \class MorphologicalModel
  \brief A class representing a model of a particular part of a morphology, e.g., nouns or verbs.
*/


#ifndef MORPHOLOGICALMODEL_H
#define MORPHOLOGICALMODEL_H

#include <QList>

class AbstractNode;
class Form;

#include "abstractpath.h"
#include "../mortal-engine/datatypes/parsing.h"

class MorphologicalModel : public AbstractPath
{
public:
    MorphologicalModel();

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const override;

    static QString elementName();
    static AbstractNode *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader, const MorphologicalModel * model = nullptr);
    static bool matchesElement(QXmlStreamReader &in);

    bool checkHasOptionalCompletionPath() const override;

    bool isModel() const override;

    bool hasZeroLengthForms() const;
    void setHasZeroLengthForms(bool hasNullForms);

private:
    bool mHasZeroLengthForms;

};

#endif // MORPHOLOGICALMODEL_H
