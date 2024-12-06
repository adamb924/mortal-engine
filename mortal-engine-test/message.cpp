#include "message.h"

using namespace ME;

Message::Message(const QString &message, const Morphology *morphology) : AbstractTest(morphology), mMessage(message)
{

}

Message::~Message()
{

}

bool Message::succeeds() const
{
    return false; /// so it will always be printed
}

QString Message::message() const
{
    return mMessage;
}

QString Message::barebonesOutput() const
{
    return mMessage;
}

void Message::runTest()
{

}
