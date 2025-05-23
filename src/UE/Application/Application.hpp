#pragma once

#include "Logger/PrefixedLogger.hpp"
#include "Messages/PhoneNumber.hpp"
#include "IEventsHandler.hpp"
#include "Context.hpp"

namespace ue
{

using common::PhoneNumber;
using common::ILogger;

class Application : public IEventsHandler
{
public:
    Application(PhoneNumber phoneNumber,
                ILogger& iLogger,
                IBtsPort& bts,
                IUserPort& user,
                ITimerPort& timer);
    ~Application();


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
    
    const SmsDb& getSmsDb() const { return context.smsDb; }
    
    void handleClose();
    
private:
    Context context;
    common::PrefixedLogger logger;

};

}
