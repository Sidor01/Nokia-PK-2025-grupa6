#include "ConnectingState.hpp"
#include "ConnectedState.hpp"
#include "NotConnectedState.hpp"

namespace ue
{

ConnectingState::ConnectingState(Context &context)
    : BaseState(context, "ConnectingState")
{
    context.user.showConnecting();
}

void ConnectingState::handleAttachAccept(){
    context.timer.stopTimer();
    context.setState<ConnectedState>();
}

void ConnectingState::handleAttachReject(){
    context.timer.stopTimer();
    context.setState<NotConnectedState>();
}

void ConnectingState::handleTimeout(){
    context.setState<NotConnectedState>();
}

void ConnectingState::handleDisconnect() {
    logger.logInfo("Connection to BTS dropped while connecting");
    context.timer.stopTimer(); 
    context.setState<NotConnectedState>();
}

void ConnectingState::handleSms(common::PhoneNumber from, std::string text)
{
}

void ConnectingState::handleSib(common::BtsId btsId)
{
    logger.logInfo("Received SIB message while connecting, btsId: ", btsId);
    
    
    context.currentBtsId = btsId;
    
    
    logger.logInfo("Stored SIB data, remaining in Connecting state (not restarting procedure)");
}

void ConnectingState::handleClose()
{
    logger.logInfo("User closed UE while connecting to BTS");
    
    context.timer.stopTimer();
    
    
}
}
