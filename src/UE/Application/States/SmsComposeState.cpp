#include "SmsComposeState.hpp"
#include "ConnectedState.hpp"
#include "TalkingState.hpp"
#include "NotConnectedState.hpp"
#include <chrono>
#include <thread>

namespace ue
{

const std::chrono::seconds SmsComposeState::CALL_TIMEOUT{30};

SmsComposeState::SmsComposeState(Context &context)
    : BaseState(context, "SmsComposeState")
{
    logger.logInfo("Entered SMS compose state");
    
    
    context.user.showSmsComposeView();
}

void SmsComposeState::handleDisconnect() 
{
    logger.logInfo("Connection to BTS dropped while sending SMS");
    context.setState<NotConnectedState>();
}

void SmsComposeState::handleSms(common::PhoneNumber from, std::string text)
{
    logger.logInfo("Received SMS from: ", from, ", text: ", text);
    
    context.smsDb.addSms(from, text);
    
    
    context.user.showNewSms(true);
}

void SmsComposeState::composeSms()
{
    logger.logInfo("User requested to compose an SMS");
    
    
    context.user.showSmsComposeView();
}

void SmsComposeState::acceptSmsCompose(common::PhoneNumber number, const std::string& text)
{
    logger.logInfo("User accepted SMS composition to: ", number, ", text: ", text);
    
   
    context.bts.sendSms(number, text);
    
    
    context.smsDb.addSentSms(context.phoneNumber, number, text);
    
  
    context.setState<ConnectedState>();
}

void SmsComposeState::rejectSmsCompose()
{
    logger.logInfo("User rejected SMS composition");
    
    
    context.setState<ConnectedState>();
}



void SmsComposeState::handleCallRequest(common::PhoneNumber from)
{
    logger.logInfo("Received call request from: ", from, " - interrupting SMS composition");
    callingPhoneNumber = from;
    
    
    context.timer.startTimer(CALL_TIMEOUT);
    
    logger.logInfo("Showing call request UI for caller: ", from);
    context.user.showCallRequest(from);
}

void SmsComposeState::acceptCallRequest()
{
    logger.logInfo("User accepted call from: ", callingPhoneNumber);
    
    context.timer.stopTimer();
    
    logger.logInfo("Sending CallAccept message to: ", callingPhoneNumber);
    context.bts.sendCallAccept(callingPhoneNumber);
    
    logger.logInfo("Changing to TalkingState with peer: ", callingPhoneNumber);
    context.setState<TalkingState>(callingPhoneNumber);
}

void SmsComposeState::rejectCallRequest()
{
    logger.logInfo("User rejected call from: ", callingPhoneNumber);
    
    context.timer.stopTimer();
    
    context.bts.sendCallDropped(callingPhoneNumber);
    
    
    logger.logInfo("Not returning to SMS compose after call rejection");
    context.setState<ConnectedState>();
}

void SmsComposeState::handleTimeout()
{
    logger.logInfo("Call request timeout - auto-rejecting call from: ", callingPhoneNumber);
    
    context.bts.sendCallDropped(callingPhoneNumber);
    
    
    logger.logInfo("Not returning to SMS compose after call timeout");
    context.setState<ConnectedState>();
}

void SmsComposeState::updateNotificationIcon(const std::string& source)
{
    bool hasUnread = context.smsDb.hasUnreadSms();
    logger.logInfo("Updating SMS notification icon (", source, "): ", (hasUnread ? "visible" : "hidden"));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    context.user.showNewSms(hasUnread);
}

void SmsComposeState::handleClose()
{
    logger.logInfo("User closes UE while composing SMS - closing immediately");
}

}
