#pragma once

#include "BaseState.hpp"
#include "../Ports/ISmsListViewPort.hpp"

namespace ue
{

class ConnectedState : public BaseState
{
public:
    ConnectedState(Context& context);
    void handleDisconnect() override;
    void handleSms(common::PhoneNumber from, std::string text) override;
    void handleCallRequest(common::PhoneNumber from) override;
    void handleTimeout() override;
    
    void acceptCallRequest() override;
    void rejectCallRequest() override;
    void viewSms() override;
    void selectSms(size_t index) override;
    void closeSmsView() override;
    void composeSms() override;
    void acceptSmsCompose(common::PhoneNumber number, const std::string& text) override;
    void rejectSmsCompose() override;
    

    void dial() override;
    
private:

    void updateNotificationIcon(const std::string& context);
    
    common::PhoneNumber callingPhoneNumber;
    static const std::chrono::seconds CALL_TIMEOUT;
};

}
