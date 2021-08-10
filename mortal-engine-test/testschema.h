/*!
  \class TestSchema
  \brief An class that represents a set of tests. The class has a list of AbstractTest objects, which are the tests.

  Different TestSchema objects can be associated with different Morphology objects, which is the reason to have TestSchema as a separate data structure.
*/

#ifndef TESTSCHEMA_H
#define TESTSCHEMA_H

#include <QString>
#include <QList>

#include "testharness.h"
#include "abstracttest.h"

class Morphology;

class TestSchema
{
public:
    explicit TestSchema(const QString & label);
    ~TestSchema();

    //! \brief Get the label of the TestSchema.
    QString label() const;

    //! \brief Set the label of the TestSchema.
    void setLabel(const QString &label);

    //! \brief Prints the report from the schema to \a out.
    //! \param out The QTextStream to print the output to.
    //! \param verbosity The level of verbosity to print (see TestHarness::VerbosityLevel)
    //! \param showModel The models are printed if this is set to true.
    void printReport(QTextStream &out, TestHarness::VerbosityLevel verbosity = TestHarness::ErrorsOnly);

    //! \brief Sets the TestSchema to use the Morphology in the XML file located at \a morphologyFile.
    void setMorphologyFile(const QString &morphologyFile);

    //! \brief Add \a test to the TestSchema.
    void addTest(AbstractTest * test);

    //! \brief Get the Morphology object of the TestSchema.
    const Morphology *morphology() const;

    //! \brief Returns filename of the morphology file of the TestSchema.
    QString morphologyFile() const;

    //! \brief Set whether the schema should print the model to the output
    void setShowModel(bool showModel);

    //! \brief Get whether the schema should print the model to the output
    bool showModel() const;

private:
    QString mLabel;
    QString mMorphologyFile;
    QList<AbstractTest*> mTests;
    Morphology * mMorphology;
    bool mShowModel;
};

#endif // TESTSCHEMA_H
