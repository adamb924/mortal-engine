#ifndef NODEMATCHER_H
#define NODEMATCHER_H

class AbstractNode;
class QXmlStreamReader;
class MorphologyXmlReader;
class MorphologicalModel;

typedef AbstractNode* (*MorphemeNodeReader)(QXmlStreamReader &, MorphologyXmlReader*, const MorphologicalModel*);
typedef bool (*ElementMatcher)(QXmlStreamReader &);

class NodeMatcher {
public:
    inline NodeMatcher( ElementMatcher m, MorphemeNodeReader r ) : matcher(m), reader(r) {}
    ElementMatcher matcher;
    MorphemeNodeReader reader;
};

#endif // NODEMATCHER_H
