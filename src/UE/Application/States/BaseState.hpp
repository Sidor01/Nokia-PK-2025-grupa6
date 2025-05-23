#pragma once

#include "IEventsHandler.hpp"
#include "Logger/PrefixedLogger.hpp"
#include "Context.hpp"

namespace ue
{

class BaseState : public IEventsHandler
{
public:
    BaseState(Context& context, const std::string& name);
    ~BaseState() override;


    void handleTimeout() override;


    void handleSib(common::BtsId btsId) override;
    void handleAttachAccept() override;
    void handleAttachReject() override;
    void handleDisconnect() override;
    void handleSms(common::PhoneNumber from, std::string text) override;
    void handleCallRequest(common::PhoneNumber from) override;
    void handleCallAccepted(common::PhoneNumber from) override;
    void handleCallDropped(common::PhoneNumber from) override;
    void handleCallTalk(common::PhoneNumber from, std::string message) override;
    

    void acceptCallRequest() override;
    void rejectCallRequest() override;
    void dial() override;
    void acceptDialing(common::PhoneNumber number) override;
    void rejectDialing() override;
    void viewSms() override;
    void selectSms(size_t index) override;
    void closeSmsView() override;
    void composeSms() override;
    void acceptSmsCompose(common::PhoneNumber number, const std::string& text) override;
    void rejectSmsCompose() override;

protected:
    Context& context;
    common::PrefixedLogger logger;
};

}
