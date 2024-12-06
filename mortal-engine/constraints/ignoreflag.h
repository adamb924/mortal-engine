#ifndef IGNOREFLAG_H
#define IGNOREFLAG_H

#include "datatypes/writingsystem.h"

namespace ME {

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

} // namespace ME

#endif // IGNOREFLAG_H
