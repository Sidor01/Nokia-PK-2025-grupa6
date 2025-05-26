#include "TimerPort.hpp"
#include <functional>
#include <utility>

namespace ue
{

TimerPort::TimerPort(common::ILogger &logger)
    : logger(logger, "[TIMER PORT]")
{
}

TimerPort::~TimerPort()
{
}

void TimerPort::start(ITimerEventsHandler &handler)
{
    logger.logDebug("Started");
    this->handler = &handler;
}

void TimerPort::stop()
{
    logger.logDebug("Stopped");
    handler = nullptr;
}

void TimerPort::startTimer(Duration duration)
{
    logger.logDebug("Start timer: ", duration.count(), "ms");
    startTime = std::chrono::steady_clock::now();
    timerDuration = duration;
    timerActive = true;
}

void TimerPort::stopTimer()
{
    logger.logDebug("Stop timer");
    timerActive = false;
}

void TimerPort::processTimeoutNow()
{
    if (timerActive && handler) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<Duration>(now - startTime);
        
        if (elapsed >= timerDuration) {
            logger.logDebug("Timer expired after ", elapsed.count(), "ms, notifying handler");
            timerActive = false;
            handler->handleTimeout();
        } else {
            logger.logDebug("Timer check: ", elapsed.count(), "ms of ", timerDuration.count(), "ms elapsed");
        }
    }
}

}