#ifndef ABSTRACTMATCHCONDITION_H
#define ABSTRACTMATCHCONDITION_H

#include <QString>

#include "abstractconstraint.h"
#include "mortal-engine_global.h"

#include "datatypes/writingsystem.h"

class QXmlStreamReader;

namespace ME {

class Allomorph;
class Parsing;
class MorphologyXmlReader;

class MORTAL_ENGINE_EXPORT AbstractMatchCondition : public AbstractConstraint
{
public:
    AbstractMatchCondition();
    virtual ~AbstractMatchCondition() override = 0;

    virtual bool matchesThisConstraint( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override = 0;

    static QString elementName();
    static AbstractMatchCondition *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);
};

} // namespace ME

#endif // ABSTRACTMATCHCONDITION_H
