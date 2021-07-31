#include "testschema.h"

#include <QTextStream>

#include "../mortal-engine/morphology.h"

TestSchema::TestSchema(const QString &label) : mLabel(label), mMorphology(nullptr)
{

}

TestSchema::~TestSchema()
{
    qDeleteAll(mTests);
    if( mMorphology != nullptr )
    {
        delete mMorphology;
    }
}

QString TestSchema::label() const
{
    return mLabel;
}

void TestSchema::setLabel(const QString &label)
{
    mLabel = label;
}

void TestSchema::printReport(QTextStream &out, TestHarness::VerbosityLevel verbosity)
{
    foreach (AbstractTest * t, mTests)
    {
        if( verbosity == TestHarness::AllResults || t->fails() )
        {
            out << t->message() << "\n";
        }
    }
}

void TestSchema::setMorphologyFile(const QString &morphologyFile)
{
    mMorphologyFile = morphologyFile;
    mMorphology = new Morphology;
    mMorphology->readXmlFile(mMorphologyFile);
}

void TestSchema::addTest(AbstractTest *test)
{
    mTests.append(test);
}

const Morphology *TestSchema::morphology() const
{
    return mMorphology;
}

QString TestSchema::morphologyFile() const
{
    return mMorphologyFile;
}
