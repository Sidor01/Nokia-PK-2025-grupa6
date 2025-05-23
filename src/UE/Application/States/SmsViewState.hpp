#pragma once

#include "BaseState.hpp"
#include "../Ports/ISmsListViewPort.hpp"

namespace ue
{


class ConnectedState;
class TalkingState;

class SmsViewState : public BaseState
{
public:
    SmsViewState(Context& context);
    
    
    void handleDisconnect() override;
    void handleSms(common::PhoneNumber from, std::string text) override;
    
    
    void viewSms() override;
    void selectSms(size_t index) override;
    void closeSmsView() override;
    void composeSms() override;
    
    
    void handleCallRequest(common::PhoneNumber from) override;
    void acceptCallRequest() override;
    void rejectCallRequest() override;
    void handleTimeout() override;
    void handleClose() override;

private:
    void updateNotificationIcon(const std::string& source);
    common::PhoneNumber callingPhoneNumber;
    static const std::chrono::seconds CALL_TIMEOUT;
    
    bool viewingSpecificSms = false;  
    size_t currentSmsIndex = 0;       
};

}
