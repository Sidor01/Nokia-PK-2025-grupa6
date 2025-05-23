#include "Application.hpp"
#include "States/NotConnectedState.hpp"

namespace ue
{

Application::Application(common::PhoneNumber phoneNumber,
                         common::ILogger &iLogger,
                         IBtsPort &bts,
                         IUserPort &user,
                         ITimerPort &timer)
    : context{iLogger, bts, user, timer, phoneNumber, SmsDb(phoneNumber)},
    logger(iLogger, "[APP] ")
{
    logger.logInfo("Started");
    context.setState<NotConnectedState>(true);
}

Application::~Application()
{
    logger.logInfo("Stopped");
}

void Application::handleTimeout()
{
    context.state->handleTimeout();
}

void Application::handleSib(common::BtsId btsId)
{
    context.state->handleSib(btsId);
}

void Application::handleAttachAccept()
{
    context.state->handleAttachAccept();
}

void Application::handleAttachReject()
{
    context.state->handleAttachReject();
}

void Application::handleDisconnect()
{
    context.state->handleDisconnect();
}

void Application::handleSms(common::PhoneNumber from, std::string text)
{
    logger.logInfo("SMS received from: ", from);
    context.state->handleSms(from, text);
}

void Application::handleCallRequest(common::PhoneNumber from)
{
    logger.logInfo("Call request received from: ", from);
    context.state->handleCallRequest(from);
}

void Application::handleCallAccepted(common::PhoneNumber from)
{
    logger.logInfo("Call accepted by: ", from);
    context.state->handleCallAccepted(from);
}

void Application::handleCallDropped(common::PhoneNumber from)
{
    logger.logInfo("Call dropped by: ", from);
    context.state->handleCallDropped(from);
}

void Application::handleCallTalk(common::PhoneNumber from, std::string message)
{
    logger.logInfo("Call talk message from: ", from);
    context.state->handleCallTalk(from, message);
}

void Application::acceptCallRequest()
{
    logger.logInfo("User accepted incoming call");
    context.state->acceptCallRequest();
}

void Application::rejectCallRequest()
{
    logger.logInfo("User rejected incoming call");
    context.state->rejectCallRequest();
}

void Application::dial()
{
    logger.logInfo("User initiated dialing");
    context.state->dial();
}

void Application::acceptDialing(common::PhoneNumber number)
{
    logger.logInfo("User accepted dialing to: ", number);
    context.state->acceptDialing(number);
}

void Application::rejectDialing()
{
    logger.logInfo("User rejected dialing");
    context.state->rejectDialing();
}

void Application::viewSms()
{
    logger.logInfo("User requested to view SMS");
    context.state->viewSms();
}

void Application::selectSms(size_t index)
{
    logger.logInfo("User selected SMS at index: ", index);
    context.state->selectSms(index);
}

void Application::closeSmsView()
{
    logger.logInfo("User closed SMS view");
    context.state->closeSmsView();
}

void Application::composeSms()
{
    logger.logInfo("User requested to compose SMS");
    context.state->composeSms();
}

void Application::acceptSmsCompose(common::PhoneNumber number, const std::string& text)
{
    logger.logInfo("User accepted SMS composition to: ", number);
    context.state->acceptSmsCompose(number, text);
}

void Application::rejectSmsCompose()
{
    logger.logInfo("User rejected SMS composition");
    context.state->rejectSmsCompose();
}

void Application::handleClose()
{
    logger.logInfo("Application close requested");
    context.state->handleClose();
}

}
