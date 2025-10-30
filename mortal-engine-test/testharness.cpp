#include "testharness.h"

#include <QObject>

#include "testschema.h"
#include "harnessxmlreader.h"

using namespace ME;

TestHarness::TestHarness()
{

}

TestHarness::~TestHarness()
{
    qDeleteAll(mSchemata);
}

void TestHarness::readTestFile(const QString &filename)
{
    HarnessXmlReader reader(this);
    reader.readTestFile(filename);
}

void TestHarness::printReport(QTextStream &out, VerbosityLevel verbosity, bool showModel, bool check, const NodeId &inspectId)
{
    foreach(TestSchema *ts, mSchemata)
    {
        if( verbosity != TestHarness::BareBones )
            out << QObject::tr("Schema: %1 [%2]\n").arg(ts->label(), ts->morphologyFile());
        if( showModel || ts->showModel() )
        {
            out << ts->morphology()->summary() << Qt::endl << Qt::endl;
        }
        if( !inspectId.isNull() )
        {
            AbstractNode * node = ts->morphology()->getNodeFromId(inspectId);
            if( node == nullptr )
            {
                out << "Could not find node with ID: " << inspectId.toString();
            }
            else
            {
               out << node->summary(node->next()) << Qt::endl << Qt::endl;
            }
        }
        if( check )
        {
            out << Qt::endl;
            ts->morphology()->printModelCheck(out);
            out << Qt::endl;
        }
        ts->printReport(out, verbosity);
        out << Qt::endl << Qt::endl;
    }
}
