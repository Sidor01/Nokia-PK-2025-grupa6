#pragma once

#include "IBtsPort.hpp"
#include "Logger/PrefixedLogger.hpp"
#include "ITransport.hpp"
#include "Messages/PhoneNumber.hpp"

namespace ue
{

class BtsPort : public IBtsPort
{
public:
    BtsPort(common::ILogger& logger, common::ITransport& transport, common::PhoneNumber phoneNumber);
    void start(IBtsEventsHandler& handler);
    void stop();

    void sendAttachRequest(common::BtsId) override;
    void sendCallAccept(common::PhoneNumber to) override;
    void sendCallDropped(common::PhoneNumber to) override;
    void sendCallTalk(common::PhoneNumber to, std::string message) override;
    void sendSms(common::PhoneNumber to, const std::string& text) override;
    void sendCallRequest(common::PhoneNumber to) override;

private:
    void handleMessage(BinaryMessage msg);
    void handleDisconnect();

    common::PrefixedLogger logger;
    common::ITransport& transport;
    common::PhoneNumber phoneNumber;

    IBtsEventsHandler* handler = nullptr;
};

}
