#ifndef MORPHOLOGYCHECKER_H
#define MORPHOLOGYCHECKER_H

class QTextStream;

namespace ME {

class Morphology;

class MorphologyChecker
{
public:
    explicit MorphologyChecker(const Morphology * morphology);

    void printCheck(QTextStream &out) const;

private:
    void duplicateAllomorphFormCheck(QTextStream &out, bool originalsOnly) const;
    void originalDuplicateCheck(QTextStream &out) const;
    void allDuplicateCheck(QTextStream &out) const;

    void allomorphConstraintCheck(QTextStream &out, bool originalsOnly) const;
    void originalConstraintCheck(QTextStream &out) const;
    void allConstraintCheck(QTextStream &out) const;

    void missingGlossCheck(QTextStream &out) const;

    const Morphology * mMorphology;
};

} // namespace ME

#endif // MORPHOLOGYCHECKER_H
