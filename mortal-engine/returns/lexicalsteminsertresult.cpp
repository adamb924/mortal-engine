#include "lexicalsteminsertresult.h"

#include <QTextStream>

#include "nodes/abstractstemlist.h"

LexicalStemInsertResult::LexicalStemInsertResult()
{

}

void LexicalStemInsertResult::recordResult(AbstractStemList *asl, bool result)
{
    mInsertResults.insert( asl, result );
}

void LexicalStemInsertResult::append(const LexicalStemInsertResult &other)
{
    mInsertResults.insert( other.mInsertResults );
}

int LexicalStemInsertResult::numberOfInsertions() const
{
    int count = 0;
    QHashIterator<AbstractStemList*,bool> iter(mInsertResults);
    while( iter.hasNext() )
    {
        iter.next();
        if( iter.value() )
            count++;
    }
    return count;
}

int LexicalStemInsertResult::numberOfFailures() const
{
    return numberOfAttempts() - numberOfInsertions();
}

int LexicalStemInsertResult::numberOfAttempts() const
{
    return mInsertResults.count();
}

QHash<AbstractStemList *, bool> LexicalStemInsertResult::insertResults() const
{
    return mInsertResults;
}

QString LexicalStemInsertResult::summary() const
{
    QString dbgString;
    QTextStream dbg(&dbgString);

    dbg << "LexicalStemInsertResult(\n";

    QHashIterator<AbstractStemList*,bool> iter(mInsertResults);
    while( iter.hasNext() )
    {
        iter.next();
        dbg << iter.key()->label().toString() << ", " << ( iter.value() ? "true" : "false" ) << "\n";
    }

    dbg << ")\n";
    return dbgString;
}
