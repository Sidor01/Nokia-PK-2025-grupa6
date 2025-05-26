#pragma once

#include "ITimerPort.hpp"
#include "Logger/PrefixedLogger.hpp"
#include <atomic>
#include <chrono>

namespace ue
{

class TimerPort : public ITimerPort
{
public:
    TimerPort(common::ILogger& logger);
    ~TimerPort();

    void start(ITimerEventsHandler& handler);
    void stop();

    void startTimer(Duration duration) override;
    void stopTimer() override;
    void processTimeoutNow();

private:
    common::PrefixedLogger logger;
    ITimerEventsHandler* handler = nullptr;
    std::atomic<bool> timerActive{false};
    
    std::chrono::steady_clock::time_point startTime;
    Duration timerDuration{0};
};

}
