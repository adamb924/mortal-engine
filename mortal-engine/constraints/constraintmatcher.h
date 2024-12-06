#ifndef CONSTRAINTMATCHER_H
#define CONSTRAINTMATCHER_H

class QXmlStreamReader;

namespace ME {

class AbstractConstraint;
class MorphologyXmlReader;
class MorphologicalModel;

typedef AbstractConstraint* (*ConstraintReader)(QXmlStreamReader &, MorphologyXmlReader*);
typedef bool (*ElementMatcher)(QXmlStreamReader &);

class ConstraintMatcher
{
public:
    inline ConstraintMatcher( ElementMatcher m, ConstraintReader r ) : matcher(m), reader(r) {}
    ElementMatcher matcher;
    ConstraintReader reader;
};

} // namespace ME

#endif // CONSTRAINTMATCHER_H
