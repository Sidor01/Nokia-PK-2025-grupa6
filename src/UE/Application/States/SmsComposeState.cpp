#include "SmsComposeState.hpp"
#include "ConnectedState.hpp"
#include "TalkingState.hpp"
#include "NotConnectedState.hpp"
#include <chrono>
#include <thread>

namespace ue
{

const std::chrono::seconds SmsComposeState::CALL_TIMEOUT{60};

SmsComposeState::SmsComposeState(Context &context)
    : BaseState(context, "SmsComposeState")
{
    logger.logInfo("Entered SMS compose state");
    
    
    context.user.showSmsComposeView();
}

void SmsComposeState::handleDisconnect() 
{
    logger.logInfo("Connection to BTS dropped while sending SMS");
    
    if (receivingCallRequest) {
        logger.logInfo("Connection dropped while receiving call request from: ", callingPhoneNumber, 
                      " - cannot inform peer UE, going to NotConnected immediately");
        context.timer.stopTimer();
        receivingCallRequest = false;
    }
    
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
    if (receivingCallRequest) {
        if (from == callingPhoneNumber) {
            logger.logInfo("Received subsequent call request from same caller: ", from, " - ignoring as already processing call request from this peer");
        } else {
            logger.logInfo("Received subsequent call request from: ", from, " while already processing call request from: ", callingPhoneNumber, " - dropping new request");
            context.bts.sendCallDropped(from);
        }
        return;
    }
    
    logger.logInfo("Received call request from: ", from, " - interrupting SMS composition");
    callingPhoneNumber = from;
    receivingCallRequest = true;  
    
    context.timer.startTimer(CALL_TIMEOUT);
    
    logger.logInfo("Showing call request UI for caller: ", from);
    context.user.showCallRequest(from);
}

void SmsComposeState::handleCallDropped(common::PhoneNumber from)
{
    if (receivingCallRequest && from == callingPhoneNumber) {
        logger.logInfo("Call dropped by caller: ", from, " - returning to SMS compose state");
        
        context.timer.stopTimer();
        receivingCallRequest = false;
        
        context.user.showSmsComposeView();
    } else if (receivingCallRequest) {
        logger.logInfo("Received call dropped from unexpected number: ", from, " while receiving call from: ", callingPhoneNumber, " - ignoring");
    } else {
        logger.logInfo("Received call dropped from: ", from, " but not expecting any call - ignoring");
    }
}

void SmsComposeState::acceptCallRequest()
{
    logger.logInfo("User accepted call from: ", callingPhoneNumber);
    
    context.timer.stopTimer();
    receivingCallRequest = false;  
    
    logger.logInfo("Sending CallAccept message to: ", callingPhoneNumber);
    context.bts.sendCallAccept(callingPhoneNumber);
    
    logger.logInfo("Changing to TalkingState with peer: ", callingPhoneNumber);
    context.setState<TalkingState>(callingPhoneNumber);
}

void SmsComposeState::rejectCallRequest()
{
    logger.logInfo("User rejected call from: ", callingPhoneNumber);
    
    context.timer.stopTimer();
    receivingCallRequest = false;  
    
    context.bts.sendCallDropped(callingPhoneNumber);
    
    logger.logInfo("Not returning to SMS compose after call rejection");
    context.setState<ConnectedState>();
}

void SmsComposeState::handleTimeout()
{
    logger.logInfo("Call request timeout - auto-rejecting call from: ", callingPhoneNumber);
    
    context.bts.sendCallDropped(callingPhoneNumber);
    
    receivingCallRequest = false;  
    
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
    logger.logInfo("User closes UE while in SmsComposeState");
    
    if (receivingCallRequest) {
        logger.logInfo("UE closing while receiving call request from: ", callingPhoneNumber, " - sending CallDropped");
        context.timer.stopTimer();
        context.bts.sendCallDropped(callingPhoneNumber);
        receivingCallRequest = false;
    }
    
    logger.logInfo("UE closing immediately without waiting");
}

}
