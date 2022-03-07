#ifndef IGNOREFLAG_H
#define IGNOREFLAG_H

#include "datatypes/writingsystem.h"

class Parsing;

class IgnoreFlag
{
public:
    enum Type { IgnoreParsing, IgnoreGeneration };

    IgnoreFlag( Type type, const WritingSystem & ws );
    bool shouldBeIgnored( const Parsing * p ) const;

private:
    Type mType;
    WritingSystem mWritingSystem;
};

#endif // IGNOREFLAG_H
