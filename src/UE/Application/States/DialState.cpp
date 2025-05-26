#include "DialState.hpp"
#include "ConnectedState.hpp"
#include "TalkingState.hpp"
#include "NotConnectedState.hpp"

namespace ue
{

const std::chrono::seconds DialState::CALL_REQUEST_TIMEOUT{62};
const std::chrono::seconds DialState::CALL_RESPONSE_TIMEOUT{60};

DialState::DialState(Context &context)
    : BaseState(context, "DialState")
{
    context.user.showDialView();
}

void DialState::handleTimeout()
{
    if (receivingCallRequest) {
        logger.logInfo("Call request timeout - auto-rejecting call from: ", callingPhoneNumber);
        context.bts.sendCallDropped(callingPhoneNumber);
        receivingCallRequest = false;
        
        context.setState<ConnectedState>();
    } else {
        logger.logInfo("Call request timeout - call dropped");
        context.user.showPeerUserNotAvailable(recipientPhoneNumber);
        context.setState<ConnectedState>();
    }
}

void DialState::handleDisconnect()
{
    logger.logInfo("Connection to BTS dropped while dialing");
    context.timer.stopTimer();
    context.setState<NotConnectedState>();
}

void DialState::handleCallRequest(common::PhoneNumber from)
{
    logger.logInfo("Received call request from: ", from, " - interrupting outgoing call request to: ", recipientPhoneNumber);
    
    callingPhoneNumber = from;
    receivingCallRequest = true;
    
    context.timer.stopTimer();
    
    if (callRequestSent) {
        logger.logInfo("Outgoing call request was already sent - sending CallDropped to: ", recipientPhoneNumber);
        context.bts.sendCallDropped(recipientPhoneNumber);
    }
    
    context.timer.startTimer(CALL_RESPONSE_TIMEOUT);
    
    logger.logInfo("Showing call request UI for caller: ", from);
    context.user.showCallRequest(from);
}

void DialState::acceptDialing(common::PhoneNumber recipientNumber)
{
    // WARNING! commented lines in this method because they
    // break cancelling Dial (making it impossible to cancel call request)

    //if (receivingCallRequest) {
    //    logger.logInfo("Ignoring outgoing call request to: ", recipientNumber, " because we're already receiving a call from: ", callingPhoneNumber);
    //    return;
    //}
    
    logger.logInfo("Sending call request to: ", recipientNumber);
    
    recipientPhoneNumber = recipientNumber;
    
    context.timer.startTimer(CALL_REQUEST_TIMEOUT);
    
    logger.logInfo("About to send CallRequest message to recipient: ", recipientPhoneNumber);
    
    context.bts.sendCallRequest(recipientPhoneNumber);

    //context.user.showCallView(recipientPhoneNumber);
    
    callRequestSent = true;
    
    logger.logInfo("CallRequest message sent, waiting for response...");
}

void DialState::rejectDialing()
{
    logger.logInfo("User canceled call request");
    
    context.timer.stopTimer();
    
    if (recipientPhoneNumber != common::PhoneNumber{}) {
        context.bts.sendCallDropped(recipientPhoneNumber);
    }
    
    context.setState<ConnectedState>();
}

void DialState::handleCallAccepted(common::PhoneNumber from)
{
    if (!receivingCallRequest && from == recipientPhoneNumber) {
        logger.logInfo("Received CallAccepted message from: ", from);
        logger.logInfo("Call accepted by: ", from);
        
        context.timer.stopTimer();
        
        logger.logInfo("Changing to TalkingState with peer: ", recipientPhoneNumber);
        context.setState<TalkingState>(recipientPhoneNumber);
    } else {
        logger.logError("Call accepted from unexpected number: ", from);
    }
}

void DialState::handleCallDropped(common::PhoneNumber from)
{
    if (receivingCallRequest && from == callingPhoneNumber) {
        logger.logInfo("Call dropped by caller: ", from, " - returning to connected state");
        
        context.timer.stopTimer();
        receivingCallRequest = false;
        
        context.setState<ConnectedState>();
    }
    else if (!receivingCallRequest && from == recipientPhoneNumber) {
        logger.logInfo("Call dropped by: ", from);
        
        context.timer.stopTimer();
        
        context.user.showPeerUserNotAvailable(from);
        context.setState<ConnectedState>();
    } else {
        logger.logError("Call dropped from unknown number: ", from);
    }
}

void DialState::acceptCallRequest()
{
    if (receivingCallRequest) {
        logger.logInfo("User accepted call from: ", callingPhoneNumber);
        
        context.timer.stopTimer();
        receivingCallRequest = false;
        
        logger.logInfo("Sending CallAccept message to: ", callingPhoneNumber);
        context.bts.sendCallAccept(callingPhoneNumber);
        
        logger.logInfo("Changing to TalkingState with peer: ", callingPhoneNumber);
        context.setState<TalkingState>(callingPhoneNumber);
    } else {
        logger.logError("Unexpected: acceptCallRequest when not receiving a call");
    }
}

void DialState::rejectCallRequest()
{
    if (receivingCallRequest) {
        logger.logInfo("User rejected call from: ", callingPhoneNumber);
        
        context.timer.stopTimer();
        receivingCallRequest = false;
        
        context.bts.sendCallDropped(callingPhoneNumber);
        
        context.setState<ConnectedState>();
    } else {
        logger.logError("Unexpected: rejectCallRequest when not receiving a call");
    }
}

void DialState::handleClose()
{
    logger.logInfo("User closes UE while in DialState");
    
    context.timer.stopTimer();
    
    if (receivingCallRequest) {
        logger.logInfo("UE closing while receiving call request from: ", callingPhoneNumber, " - sending CallDropped");
        context.bts.sendCallDropped(callingPhoneNumber);
    }
    
    if (recipientPhoneNumber != common::PhoneNumber{} && !receivingCallRequest) {
        logger.logInfo("UE closing while call request was sent to: ", recipientPhoneNumber, " - sending CallDropped");
        context.bts.sendCallDropped(recipientPhoneNumber);
    }
    
    logger.logInfo("UE closing immediately without waiting");
}

void DialState::handleSms(common::PhoneNumber from, std::string text)
{
    logger.logInfo("Received SMS from: ", from, ", text: ", text);
    
    context.smsDb.addSms(from, text);
    
    context.user.showNewSms(true);
}
}
