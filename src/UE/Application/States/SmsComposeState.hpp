#pragma once

#include "BaseState.hpp"

namespace ue
{


class ConnectedState;
class TalkingState;

class SmsComposeState : public BaseState
{
public:
    SmsComposeState(Context& context);
    
    
    void handleDisconnect() override;
    void handleSms(common::PhoneNumber from, std::string text) override;
    
    
    void composeSms() override;
    void acceptSmsCompose(common::PhoneNumber number, const std::string& text) override;
    void rejectSmsCompose() override;
    
    
    void handleCallRequest(common::PhoneNumber from) override;
    void acceptCallRequest() override;
    void rejectCallRequest() override;
    void handleTimeout() override;
    void handleClose() override;

private:
    void updateNotificationIcon(const std::string& source);
    common::PhoneNumber callingPhoneNumber;
    static const std::chrono::seconds CALL_TIMEOUT;
};

}
