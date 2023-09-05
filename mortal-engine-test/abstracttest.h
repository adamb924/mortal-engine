/*!
  \class AbstractTest
  \brief An abstract class for providing a test, which is a run of the morphological engine with a given target.

  Current subclasses of this class are ParsingTest, RecognitionTest, TransductionTest, and Message.

  A TestSchema object contains a list of these objects.
*/

#ifndef ABSTRACTTEST_H
#define ABSTRACTTEST_H

#include <QString>

#include "datatypes/form.h"
#include "morphology.h"

class Morphology;

class AbstractTest
{
public:
    AbstractTest(const Morphology * morphology);
    virtual ~AbstractTest();

    //! \brief Return true if the test succeeds, otherwise returns false.
    virtual bool succeeds() const = 0;

    //! \brief Covenience function, which returns the negated value of suceeds().
    bool fails() const;

    //! \brief Returns a message about the test (e.g., a plain text summary of what happened and whether that was correct). Whether the test succeeds or fails is entirely implementation-dependent.
    virtual QString message() const = 0;

    virtual QString barebonesOutput() const = 0;

    //! \brief Evaluate the function using the input provided by setInput().
    void evaluate();
    virtual void runTest() = 0;

    //! \brief Sets the input
    void setInput(const Form &input);

    //! \brief A simple success or failure message. Implementations of message() can use this method as a way of ensuring uniformity of presentation.
    QString summaryStub() const;

    void setPropertiesFromAttributes(QXmlStreamReader &in);

protected:
    //! \brief Converts a set of QString objects to a comma-delimited list
    QString setToString( const QSet<Form> forms ) const;
    QString setToString( const QSet<QString> forms ) const;
    QString setToString( const QSet<MorphemeSequence> forms ) const;

    QString setToBarebonesString( const QSet<Form> forms ) const;
    QString setToBarebonesString( const QSet<QString> forms ) const;
    QString setToBarebonesString( const QSet<MorphemeSequence> forms ) const;

    const Morphology * mMorphology;
    Form mInput;
    QString mLabel;
    bool mShowDebug;
    bool mShowStemDebug;
};

#endif // ABSTRACTTEST_H
