#include "SmsViewState.hpp"
#include "ConnectedState.hpp"
#include "TalkingState.hpp"
#include "SmsComposeState.hpp"
#include "NotConnectedState.hpp"
#include "../Ports/ISmsListViewPort.hpp"
#include <sstream>
#include <chrono>
#include <thread>

namespace ue
{

const std::chrono::seconds SmsViewState::CALL_TIMEOUT{30};

SmsViewState::SmsViewState(Context &context)
    : BaseState(context, "SmsViewState")
{
    logger.logInfo("Entered SMS view state");
    
    
    auto* smsListViewPort = dynamic_cast<ISmsListViewPort*>(&context.user);
    if (smsListViewPort) {
        smsListViewPort->setSmsList(context.smsDb.getAllSms());
        
        smsListViewPort->setSelectSmsCallback([this](size_t index) {
            this->selectSms(index);
        });
    } else {
        logger.logError("Failed to cast UserPort to ISmsListViewPort");
    }
    
    context.user.showSmsList();
    
   
    updateNotificationIcon("Entering SmsViewState");
}

void SmsViewState::handleDisconnect() 
{
    logger.logInfo("Connection to BTS dropped while viewing SMS/SMS list");
    
    if (receivingCallRequest) {
        logger.logInfo("Connection dropped while receiving call request from: ", callingPhoneNumber, 
                      " - cannot inform peer UE, going to NotConnected immediately");
        context.timer.stopTimer();
        receivingCallRequest = false;
    }
    
    context.setState<NotConnectedState>();
}

void SmsViewState::handleSms(common::PhoneNumber from, std::string text)
{
    logger.logInfo("Received SMS from: ", from, ", text: ", text);
    
    context.smsDb.addSms(from, text);
    context.user.showNewSms(true);
    
    
    if (!viewingSpecificSms) {
        viewSms();
    } else {
        logger.logInfo("Received SMS while viewing specific SMS content - not refreshing view");
    }
}

void SmsViewState::viewSms()
{
    logger.logInfo("Refreshing SMS list view");
    
    
    viewingSpecificSms = false;
    
    updateNotificationIcon("viewSms");

    auto* smsListViewPort = dynamic_cast<ISmsListViewPort*>(&context.user);
    if (smsListViewPort) {
        smsListViewPort->setSmsList(context.smsDb.getAllSms());
        
        smsListViewPort->setSelectSmsCallback([this](size_t index) {
            this->selectSms(index);
        });
    } else {
        logger.logError("Failed to cast UserPort to ISmsListViewPort");
    }
    
    context.user.showSmsList();
}

void SmsViewState::selectSms(size_t index)
{
    logger.logInfo("User selected SMS at index: ", index);
    
    const auto& smsList = context.smsDb.getAllSms();
    
    if (index < smsList.size()) {
        
        viewingSpecificSms = true;
        currentSmsIndex = index;
        
        const auto& sms = smsList[index];
        
        if (sms.isSent) {
            context.user.showSentSmsContent(to_string(sms.to), sms.text);
        } else {
            context.user.showSmsContent(to_string(sms.from), sms.text);
        }
        
        if (!sms.isRead) {
            context.smsDb.markAsRead(index);
            
            updateNotificationIcon("selectSms");
        }
    } else {
        logger.logError("Invalid SMS index: ", index);
        viewingSpecificSms = false;
        viewSms();
    }
}

void SmsViewState::closeSmsView()
{
    logger.logInfo("User closed SMS view");
    
    
    viewingSpecificSms = false;
  
    context.setState<ConnectedState>();
}

void SmsViewState::composeSms()
{
    logger.logInfo("User requested to compose an SMS from SMS view state");
    
    context.setState<SmsComposeState>();
}


void SmsViewState::handleCallRequest(common::PhoneNumber from)
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
    
    logger.logInfo("Received call request from: ", from, " - interrupting SMS viewing");
    callingPhoneNumber = from;
    receivingCallRequest = true;  
    
    viewingSpecificSms = false;
    
    context.timer.startTimer(CALL_TIMEOUT);
    
    logger.logInfo("Showing call request UI for caller: ", from);
    context.user.showCallRequest(from);
}

void SmsViewState::handleCallDropped(common::PhoneNumber from)
{
    if (receivingCallRequest && from == callingPhoneNumber) {
        logger.logInfo("Call dropped by caller: ", from, " - returning to SMS view state");
        
        context.timer.stopTimer();
        receivingCallRequest = false;
        
        if (viewingSpecificSms) {
            const auto& smsList = context.smsDb.getAllSms();
            if (currentSmsIndex < smsList.size()) {
                const auto& sms = smsList[currentSmsIndex];
                if (sms.isSent) {
                    context.user.showSentSmsContent(to_string(sms.to), sms.text);
                } else {
                    context.user.showSmsContent(to_string(sms.from), sms.text);
                }
            } else {
                viewingSpecificSms = false;
                viewSms();
            }
        } else {
            viewSms();
        }
    } else if (receivingCallRequest) {
        logger.logInfo("Received call dropped from unexpected number: ", from, " while receiving call from: ", callingPhoneNumber, " - ignoring");
    } else {
        logger.logInfo("Received call dropped from: ", from, " but not expecting any call - ignoring");
    }
}

void SmsViewState::acceptCallRequest()
{
    logger.logInfo("User accepted call from: ", callingPhoneNumber);
    
    context.timer.stopTimer();
    receivingCallRequest = false;  
    
    logger.logInfo("Sending CallAccept message to: ", callingPhoneNumber);
    context.bts.sendCallAccept(callingPhoneNumber);
    
    logger.logInfo("Changing to TalkingState with peer: ", callingPhoneNumber);
    context.setState<TalkingState>(callingPhoneNumber);
}

void SmsViewState::rejectCallRequest()
{
    logger.logInfo("User rejected call from: ", callingPhoneNumber);
    
    context.timer.stopTimer();
    receivingCallRequest = false; 
    
    context.bts.sendCallDropped(callingPhoneNumber);
    
    logger.logInfo("Not returning to SMS view after call rejection");
    context.setState<ConnectedState>();
}

void SmsViewState::handleTimeout()
{
    logger.logInfo("Call request timeout - auto-rejecting call from: ", callingPhoneNumber);
    
    context.bts.sendCallDropped(callingPhoneNumber);
    
    receivingCallRequest = false;  
    
    logger.logInfo("Not returning to SMS view after call timeout");
    context.setState<ConnectedState>();
}

void SmsViewState::updateNotificationIcon(const std::string& source)
{
    bool hasUnread = context.smsDb.hasUnreadSms();
    logger.logInfo("Updating SMS notification icon (", source, "): ", (hasUnread ? "visible" : "hidden"));
    
    context.user.showNewSms(hasUnread);
}

void SmsViewState::handleClose()
{
    logger.logInfo("User closes UE while in SmsViewState");
    
    if (receivingCallRequest) {
        logger.logInfo("UE closing while receiving call request from: ", callingPhoneNumber, " - sending CallDropped");
        context.timer.stopTimer();
        context.bts.sendCallDropped(callingPhoneNumber);
        receivingCallRequest = false;
    }
    
    logger.logInfo("UE closing immediately without waiting");
}

}
