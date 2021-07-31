/*!
  \class TestHarness
  \brief An class that represents a set of test schemata. The class has a list of TestSchema objects, which themselves contain the individual tests.

  This corresponds to the file test-harness.xml. HarnessXmlReader contains the logic for reading the XML file.
*/

#ifndef TESTHARNESS_H
#define TESTHARNESS_H

#include <QList>

class QTextStream;
class TestSchema;

class TestHarness
{
public:
    friend class HarnessXmlReader;

    enum VerbosityLevel{ AllResults, ErrorsOnly };

    TestHarness();
    ~TestHarness();

    //! \brief Uses HarnessXmlReader to read the XML file specified by \a filename.
    void readTestFile(const QString & filename);

    //! \brief Prints the report for all of the schemata (which contain the tests).
    //! \param out The QTextStream to print the output to.
    //! \param verbosity The level of verbosity to print (see TestHarness::VerbosityLevel)
    //! \param showModel The models are printed if this is set to true.
    void printReport(QTextStream &out, TestHarness::VerbosityLevel verbosity, bool showModel, bool check);

private:
    QList<TestSchema*> mSchemata;
};

#endif // TESTHARNESS_H
