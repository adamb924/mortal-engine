#include "ignoreflag.h"

#include "datatypes/parsing.h"

IgnoreFlag::IgnoreFlag(Type type, const WritingSystem &ws) : mType(type), mWritingSystem(ws)
{

}

bool IgnoreFlag::shouldBeIgnored(const Parsing *p) const
{
    if( p->writingSystem() == mWritingSystem
            && ( ( mType == IgnoreFlag::IgnoreGeneration && p->isGeneration() )
                 || ( mType == IgnoreFlag::IgnoreParsing && !p->isGeneration() ) ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}
