#include "testschema.h"

#include <QTextStream>
#include <QElapsedTimer>

#include "../mortal-engine/morphology.h"

using namespace ME;

TestSchema::TestSchema(const QString &label) : mLabel(label), mMorphology(nullptr), mShowModel(false)
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
        if( verbosity == TestHarness::BareBones )
        {
            out << t->barebonesOutput() << "\n";
        }
        else if( verbosity == TestHarness::AllResults || t->fails() )
        {
            out << t->message() << "\n";
        }
    }
}

void TestSchema::setMorphologyFile(const QString &morphologyFile)
{
    mMorphologyFile = morphologyFile;
    mMorphology = new Morphology;

    QElapsedTimer timer;
    timer.start();

    try {
        mMorphology->readXmlFile(mMorphologyFile);
    } catch (const std::runtime_error &e) {
        qCritical() << e.what() << "(" << mMorphologyFile << ")";
    }

    qInfo().noquote() << QString("Mortal Engine initialized in %1 ms").arg(timer.elapsed());
}

void TestSchema::addTest(AbstractTest *test)
{
    mTests.append(test);
}

Morphology *TestSchema::morphology() const
{
    return mMorphology;
}

QString TestSchema::morphologyFile() const
{
    return mMorphologyFile;
}

void TestSchema::setShowModel(bool showModel)
{
    mShowModel = showModel;
}

bool TestSchema::showModel() const
{
    return mShowModel;
}
