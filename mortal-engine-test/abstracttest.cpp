#include "abstracttest.h"

AbstractTest::AbstractTest(const Morphology *morphology) : mMorphology(morphology), mShowDebug(false)
{

}

AbstractTest::~AbstractTest()
{

}

bool AbstractTest::fails() const
{
    return !succeeds();
}

void AbstractTest::evaluate()
{
    bool oldShowDebugValue = Morphology::DebugOutput;
    Morphology::DebugOutput = mShowDebug;
    runTest();
    Morphology::DebugOutput = oldShowDebugValue;
}

void AbstractTest::setInput(const Form &input)
{
    mInput = input;
}

void AbstractTest::setLabel(const QString &label)
{
    mLabel = label;
}

QString AbstractTest::summaryStub() const
{
    QString stub;
    if( succeeds() )
    {
        stub += QObject::tr("Success: ");
    }
    else
    {
        stub += QObject::tr("Failure: ");
    }
    if( mLabel.length() > 0 )
    {
        stub += mLabel + ": ";
    }
    return stub;
}

QString AbstractTest::setToString(QSet<Form> forms) const
{
    if( forms.isEmpty() )
    {
        return QObject::tr("(empty)");
    }
    QString result = "(";
    QSetIterator<Form> i(forms);
    while(i.hasNext())
    {
        Form f = i.next();
        result += QObject::tr("%1 (%2)")
                .arg( f.text(),
                      f.writingSystem().abbreviation() );
        if( i.hasNext() )
        {
            result += ", ";
        }
    }
    result += ")";
    return result;
}

QString AbstractTest::setToString(QSet<QString> forms) const
{
    if( forms.count() == 0 )
    {
        return "no match";
    }
    QStringList list = forms.values();
    std::sort( list.begin(), list.end() );
    if( list.length() > 1 )
    {
        return "(" + list.join(", ") + ")";
    }
    else
    {
        return list.join(", ");
    }
}

bool AbstractTest::showDebug() const
{
    return mShowDebug;
}

void AbstractTest::setShowDebug(bool showDebug)
{
    mShowDebug = showDebug;
}
