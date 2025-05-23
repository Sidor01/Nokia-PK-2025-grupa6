#include "ConnectedState.hpp"
#include "NotConnectedState.hpp"
#include "TalkingState.hpp"
#include "DialState.hpp"
#include "../Ports/ISmsListViewPort.hpp"
#include "../Ports/UserPort.hpp"
#include "UeGui/ICallMode.hpp"
#include <sstream>
#include <chrono>
#include <thread>
namespace ue
{

const std::chrono::seconds ConnectedState::CALL_TIMEOUT{30};

ConnectedState::ConnectedState(Context &context)
    : BaseState(context, "ConnectedState")
{
    try {
        context.user.clearIncomingCallText();
        context.user.clearOutgoingCallText();
    } catch (const std::bad_cast&) {
        logger.logError("Failed to cast IUserPort to UserPort to clear call history");
    }
    
    context.user.showConnected();
    
    updateNotificationIcon("Entering ConnectedState");
}

void ConnectedState::handleDisconnect() {
    context.setState<NotConnectedState>();
}

void ConnectedState::handleSms(common::PhoneNumber from, std::string text)
{
    logger.logInfo("Received SMS from: ", from, ", text: ", text);
    

    context.smsDb.addSms(from, text);
    

    context.user.showNewSms(true);
}

void ConnectedState::handleCallRequest(common::PhoneNumber from)
{
    logger.logInfo("Received call request from: ", from);
    callingPhoneNumber = from;
    
    context.timer.startTimer(CALL_TIMEOUT);
    
    logger.logInfo("Showing call request UI for caller: ", from);
    context.user.showCallRequest(from);
}

void ConnectedState::handleTimeout()
{
    logger.logInfo("Call request timeout - auto-rejecting call from: ", callingPhoneNumber);
    
    context.bts.sendCallDropped(callingPhoneNumber);
    
    context.user.showConnected();
}

void ConnectedState::acceptCallRequest()
{
    logger.logInfo("User accepted call from: ", callingPhoneNumber);
    
    context.timer.stopTimer();
    
    logger.logInfo("Sending CallAccept message to: ", callingPhoneNumber);
    context.bts.sendCallAccept(callingPhoneNumber);
    
    logger.logInfo("Changing to TalkingState with peer: ", callingPhoneNumber);
    context.setState<TalkingState>(callingPhoneNumber);
}

void ConnectedState::rejectCallRequest()
{
    logger.logInfo("User rejected call from: ", callingPhoneNumber);
    
    context.timer.stopTimer();
    
    context.bts.sendCallDropped(callingPhoneNumber);
    
    context.user.showConnected();
}

void ConnectedState::dial()
{
    logger.logInfo("User initiated dialing");
    
    context.setState<DialState>();
}

void ConnectedState::viewSms()
{
    logger.logInfo("User requested to view SMS list");
    
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

void ConnectedState::selectSms(size_t index)
{
    logger.logInfo("User selected SMS at index: ", index);
    

    const auto& smsList = context.smsDb.getAllSms();
    

    if (index < smsList.size())
    {

        const auto& sms = smsList[index];
        
        if (sms.isSent) {
            context.user.showSentSmsContent(to_string(sms.to), sms.text);
        } else {
            context.user.showSmsContent(to_string(sms.from), sms.text);
        }
        
        if (!sms.isRead)
        {
            context.smsDb.markAsRead(index);
            
            updateNotificationIcon("selectSms");
        }
    }
    else
    {
        logger.logError("Invalid SMS index: ", index);
        viewSms();
    }
}

void ConnectedState::closeSmsView()
{
    logger.logInfo("User closed SMS view");
    
    context.user.showConnected();
    
    updateNotificationIcon("closeSmsView");
}

void ConnectedState::composeSms()
{
    logger.logInfo("User requested to compose an SMS");
    

    context.user.showSmsComposeView();
}

void ConnectedState::acceptSmsCompose(common::PhoneNumber number, const std::string& text)
{
    logger.logInfo("User accepted SMS composition to: ", number, ", text: ", text);
    

    context.bts.sendSms(number, text);
    

    context.smsDb.addSentSms(context.phoneNumber, number, text);
    
    context.user.showConnected();
    
    updateNotificationIcon("acceptSmsCompose");
}

void ConnectedState::rejectSmsCompose()
{
    logger.logInfo("User rejected SMS composition");
    
    context.user.showConnected();

    updateNotificationIcon("rejectSmsCompose");
}

void ConnectedState::updateNotificationIcon(const std::string& source)
{
    bool hasUnread = context.smsDb.hasUnreadSms();
    logger.logInfo("Updating SMS notification icon (", source, "): ", (hasUnread ? "visible" : "hidden"));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    context.user.showNewSms(hasUnread);
}

}
