#include "Application.hpp"
#include "ApplicationEnvironmentFactory.hpp"
#include "Ports/BtsPort.hpp"
#include "Ports/UserPort.hpp"
#include "Ports/TimerPort.hpp"
#include "SmsDb.hpp"
#include <chrono>
#include <QTimer>

int main(int argc, char* argv[])
{
    using namespace ue;
    using namespace std::chrono_literals;

    auto appEnv = ue::createApplicationEnvironment(argc, argv);
    auto& logger = appEnv->getLogger();
    auto& tranport = appEnv->getTransportToBts();
    auto& gui = appEnv->getUeGui();
    auto phoneNumber = appEnv->getMyPhoneNumber();

    BtsPort bts(logger, tranport, phoneNumber);
    UserPort user(logger, gui, phoneNumber);
    TimerPort timer(logger);
    SmsDb smsDb(phoneNumber);
    Application app(phoneNumber, logger, bts, user, timer, smsDb);
    bts.start(app);
    user.start(app);
    timer.start(app);
    
    QTimer *qtTimer = new QTimer();
    qtTimer->setInterval(1000); 
    QObject::connect(qtTimer, &QTimer::timeout, [&timer]() {
        timer.processTimeoutNow();
    });
    qtTimer->start();
    
    gui.setCloseGuard([&app]() {
        app.handleClose();
        return true; 
    });
    
    appEnv->startMessageLoop();
    bts.stop();
    user.stop();
    timer.stop();
    
    delete qtTimer;
}

