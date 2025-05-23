#pragma once

#include "BaseState.hpp"

namespace ue
{

class ConnectingState : public BaseState
{
public:
    ConnectingState(Context& context);
    void handleAttachAccept() override;
    void handleAttachReject() override;
    void handleTimeout() override;
    void handleDisconnect() override;
    void handleSms(common::PhoneNumber from, std::string text) override;
};

}
