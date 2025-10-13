/*!
  \class Message
  \brief An AbstractTest subclass for printing a message to the output log. This is not a test in the normal sense of the word. It's a convenience.
*/

#ifndef MESSAGE_H
#define MESSAGE_H

#include "abstracttest.h"

namespace ME {

class Message : public AbstractTest
{
public:
    Message(const QString & message, Morphology *morphology);
    ~Message() override;

    //! \brief Returns false (to encourage the message's being displayed.
    bool succeeds() const override;

    //! \brief Returns mMessage.
    QString message() const override;

    QString barebonesOutput() const override;

    //! \brief Empty. Implementation required by AbstractTest.
    void runTest() override;

private:
    QString mMessage;
};

} // namespace ME

#endif // MESSAGE_H
