#include "BaseState.hpp"

namespace ue
{

BaseState::BaseState(Context &context, const std::string &name)
    : context(context),
      logger(context.logger, "[" + name + "]")
{
    logger.logDebug("entry");
}

BaseState::~BaseState()
{
    logger.logDebug("exit");
}

void BaseState::handleTimeout()
{
    logger.logError("Uexpected: handleTimeout");
}

void BaseState::handleSib(common::BtsId btsId)
{
    logger.logError("Uexpected: handleSib: ", btsId);
}

void BaseState::handleAttachAccept()
{
    logger.logError("Uexpected: handleAttachAccept");
}

void BaseState::handleAttachReject()
{
    logger.logError("Uexpected: handleAttachReject");
}

void BaseState::handleDisconnect() {
    logger.logError("Unexpected: handleDisconnect");
}

void BaseState::handleSms(common::PhoneNumber from, std::string text)
{
    logger.logError("Uexpected: handleSms");
}

void BaseState::handleCallRequest(common::PhoneNumber from)
{
    logger.logError("Unexpected: handleCallRequest from: ", from);
}

void BaseState::handleCallAccepted(common::PhoneNumber from)
{
    logger.logError("Unexpected: handleCallAccepted from: ", from);
}

void BaseState::handleCallDropped(common::PhoneNumber from)
{
    logger.logError("Unexpected: handleCallDropped from: ", from);
}

void BaseState::handleCallTalk(common::PhoneNumber from, std::string message)
{
    logger.logError("Unexpected: handleCallTalk from: ", from, " message: ", message);
}

void BaseState::acceptCallRequest()
{
    logger.logError("Unexpected: acceptCallRequest");
}

void BaseState::rejectCallRequest()
{
    logger.logError("Unexpected: rejectCallRequest");
}

void BaseState::dial()
{
    logger.logError("Unexpected: dial");
}

void BaseState::acceptDialing(common::PhoneNumber number)
{
    logger.logError("Unexpected: acceptDialing with number: ", number);
}

void BaseState::rejectDialing()
{
    logger.logError("Unexpected: rejectDialing");
}

}
