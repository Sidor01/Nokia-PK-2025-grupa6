#pragma once

#include "IEventsHandler.hpp"
#include "Logger/ILogger.hpp"
#include "SmsDb.hpp"
#include <memory>

namespace ue
{

struct Context
{
    common::ILogger& logger;
    IBtsPort& bts;
    IUserPort& user;
    ITimerPort& timer;
    common::PhoneNumber phoneNumber;
    SmsDb smsDb;
    std::unique_ptr<IEventsHandler> state{};

    Context(common::ILogger& logger, IBtsPort& bts, IUserPort& user, 
            ITimerPort& timer, common::PhoneNumber number, SmsDb initialSmsDb = {})
        : logger(logger), bts(bts), user(user), timer(timer), 
          phoneNumber(number), smsDb(initialSmsDb) {}

    template <typename State, typename ...Arg>
    void setState(Arg&& ...arg)
    {
        state = std::make_unique<State>(*this, std::forward<Arg>(arg)...);
    }
};

}
