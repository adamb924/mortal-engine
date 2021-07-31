#include "createallomorphsreplacement.h"

#include "datatypes/form.h"
#include "debug.h"

CreateAllomorphsReplacement::CreateAllomorphsReplacement(const QList<Form> &replaceThis, const QList<Form> &withThis )
{
    setReplaceThis(replaceThis);
    setWithThis(withThis);
}

void CreateAllomorphsReplacement::setReplaceThis(const QList<Form> &replaceThis)
{
    foreach(Form f, replaceThis)
    {
        mReplaceThis.insert( f.writingSystem(), QRegularExpression( f.text() ) );
    }
}

void CreateAllomorphsReplacement::setWithThis(const QList<Form> &withThis)
{
    foreach(Form f, withThis)
    {
        mWithThis.insert( f.writingSystem(), f.text() );
    }
}

Form CreateAllomorphsReplacement::perform(const Form &subject, bool &changed) const
{
    WritingSystem ws = subject.writingSystem();
    if( mReplaceThis.contains(ws) && mWithThis.contains(ws))
    {
        Form newForm = subject;
        newForm.setText( subject.text().replace(
                             mReplaceThis.value(ws),
                             mWithThis.value(ws)
                             ) );
        changed = newForm != subject;
        return newForm;
    }
    else
    {
        changed = false;
        return subject;
    }
}

QString CreateAllomorphsReplacement::summary() const
{
    QString dbgString;
    Debug dbg(&dbgString);
    dbg << "CreateAllomorphsReplacement(" << newline;

    dbg.indent();

    dbg << "replace-this( ";
    QHashIterator<WritingSystem,QRegularExpression> ri(mReplaceThis);
    while( ri.hasNext() )
    {
        ri.next();
        dbg << Form( ri.key(), ri.value().pattern() ).summary();
        if( ri.hasNext() )
        {
            dbg << ", ";
        }
    }
    dbg << " )" << newline << "with-this( ";

    QHashIterator<WritingSystem,QString> wi(mWithThis);
    while( wi.hasNext() )
    {
        wi.next();
        dbg << Form( wi.key(), wi.value() ).summary();
        if( wi.hasNext() )
        {
            dbg << ", ";
        }
    }

    dbg << ")";

    dbg.unindent();

    dbg << ")";

    return dbgString;
}
