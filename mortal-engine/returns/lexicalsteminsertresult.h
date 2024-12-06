#ifndef LEXICALSTEMINSERTRESULT_H
#define LEXICALSTEMINSERTRESULT_H

#include <QHash>

#include "mortal-engine_global.h"

namespace ME {

class AbstractStemList;

class MORTAL_ENGINE_EXPORT LexicalStemInsertResult
{
public:
    LexicalStemInsertResult();

    void recordResult( AbstractStemList * asl, bool result );
    void append( const LexicalStemInsertResult & other );

    int numberOfInsertions() const;
    int numberOfFailures() const;
    int numberOfAttempts() const;

    QHash<AbstractStemList *, bool> insertResults() const;

    /**
     * @brief Returns a string representation of the Form for logging purposes.
     * 
     * @return QString The logging output.
     */
    QString summary() const;

private:
    QHash<AbstractStemList*,bool> mInsertResults;
};

#endif // LEXICALSTEMINSERTRESULT_H

} // namespace ME
