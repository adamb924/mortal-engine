#include "abstracttest.h"

#include "debug.h"
#include "datatypes/morphemesequence.h"
#include "harnessxmlreader.h"

#include <QXmlStreamReader>

using namespace ME;

AbstractTest::AbstractTest(Morphology *morphology) : mMorphology(morphology), mShowDebug(false), mShowStemDebug(false)
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
    bool oldShowDebugValue = mMorphology->debugOutput();
    bool oldShowStemDebugValue = mMorphology->stemDebugOutput();

    mMorphology->setDebugOutput(mShowDebug);
    mMorphology->setStemDebugOutput(mShowStemDebug);
    Debug::indentLevel = 0;
    Debug::atBeginning = true;

    runTest();

    mMorphology->setDebugOutput(oldShowDebugValue);
    mMorphology->setStemDebugOutput(oldShowStemDebugValue);
}

void AbstractTest::setInput(const Form &input)
{
    mInput = input;
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

QString AbstractTest::setToString(const QSet<Form> forms) const
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

QString AbstractTest::setToString(const QSet<QString> forms) const
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

QString AbstractTest::setToString(const QSet<MorphemeSequence> forms) const
{
    if( forms.count() == 0 )
    {
        return "no match";
    }
    QStringList list;
    QSetIterator<MorphemeSequence> iter(forms);
    while(iter.hasNext())
    {
        list << iter.next().toString();
    }
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

QString AbstractTest::setToBarebonesString(const QSet<Form> forms) const
{
    if( forms.isEmpty() )
    {
        return QObject::tr("(empty)");
    }
    QString result;
    QSetIterator<Form> i(forms);
    while(i.hasNext())
    {
        Form f = i.next();
        result += f.text();
        if( i.hasNext() )
        {
            result += ", ";
        }
    }
    return result;
}

QString AbstractTest::setToBarebonesString(const QSet<QString> forms) const
{
    if( forms.isEmpty() )
    {
        return QObject::tr("(empty)");
    }
    QString result;
    QSetIterator<QString> i(forms);
    while(i.hasNext())
    {
        result += i.next();
        if( i.hasNext() )
        {
            result += ", ";
        }
    }
    return result;
}

QString AbstractTest::setToBarebonesString(const QSet<MorphemeSequence> forms) const
{
    if( forms.isEmpty() )
    {
        return QObject::tr("(empty)");
    }
    QStringList list;
    QSetIterator<MorphemeSequence> iter(forms);
    while(iter.hasNext())
    {
        list << iter.next().toString();
    }
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

void AbstractTest::setPropertiesFromAttributes(QXmlStreamReader &in)
{
    mLabel = in.attributes().value(HarnessXmlReader::XML_LABEL).toString();
    mShowDebug = in.attributes().value(HarnessXmlReader::XML_DEBUG).toString() == HarnessXmlReader::XML_TRUE;
    mShowStemDebug = in.attributes().value(HarnessXmlReader::XML_STEM_DEBUG).toString() == HarnessXmlReader::XML_TRUE;
}
