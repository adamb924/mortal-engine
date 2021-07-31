#ifndef POINTERTOCONSTRAINT_H
#define POINTERTOCONSTRAINT_H

#include "abstractconstraint.h"

class PointerToConstraint : public AbstractConstraint
{
public:
    PointerToConstraint();
    ~PointerToConstraint() override;

    bool matches( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;
    QString satisfactionSummary( const Parsing * parsing, const AbstractNode *node, const Allomorph &allomorph ) const override;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary(const QString & suffix = QString()) const override;

    void setTheConstraint(const AbstractConstraint *theConstraint);

    static QString elementName();
    static AbstractConstraint *readFromXml(QXmlStreamReader &in, MorphologyXmlReader *morphologyReader);
    static bool matchesElement(QXmlStreamReader &in);

    QString targetId() const;
    void setTargetId(const QString &targetId);

    const AbstractConstraint *theConstraint() const;

    bool isMatchCondition() const override;
    bool isLocalConstraint() const override;
    bool isLongDistanceConstraint() const override;


private:
    QString mTargetId;
    const AbstractConstraint * mTheConstraint;
};

#endif // POINTERTOCONSTRAINT_H
