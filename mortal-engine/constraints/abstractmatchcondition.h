#ifndef ABSTRACTMATCHCONDITION_H
#define ABSTRACTMATCHCONDITION_H

#include <QString>

class Allomorph;
class Parsing;
class QXmlStreamReader;

#include "abstractconstraint.h"

#include "datatypes/writingsystem.h"

class MorphologyXmlReader;

class AbstractMatchCondition : public AbstractConstraint
{
public:
    AbstractMatchCondition();
    virtual ~AbstractMatchCondition() override = 0;

    virtual bool matches( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override = 0;

    static QString elementName();
    static AbstractMatchCondition *readFromXml(QXmlStreamReader &in, MorphologyXmlReader * morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);
};

#endif // ABSTRACTMATCHCONDITION_H
