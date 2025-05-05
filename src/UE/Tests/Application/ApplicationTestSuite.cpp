#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Application.hpp"
#include "Mocks/ILoggerMock.hpp"
#include "Mocks/IBtsPortMock.hpp"
#include "Mocks/IUserPortMock.hpp"
#include "Mocks/ITimerPortMock.hpp"
#include "Messages/PhoneNumber.hpp"
#include <memory>

namespace ue
{
using namespace ::testing;
using namespace std::chrono_literals;

class ApplicationTestSuite : public Test
{
protected:
    const common::PhoneNumber PHONE_NUMBER{112};
    const common::BtsId BTS_ID{22};
    NiceMock<common::ILoggerMock> loggerMock;
    StrictMock<IBtsPortMock> btsPortMock;
    StrictMock<IUserPortMock> userPortMock;
    StrictMock<ITimerPortMock> timerPortMock;

    Application objectUnderTest{PHONE_NUMBER,
                                loggerMock,
                                btsPortMock,
                                userPortMock,
                                timerPortMock};
};

struct ApplicationNotConnectedTestSuite : ApplicationTestSuite
{
    void shallHandleSibMessage()
    {
        EXPECT_CALL(btsPortMock, sendAttachRequest(BTS_ID));
        EXPECT_CALL(timerPortMock, startTimer(500ms));
        EXPECT_CALL(userPortMock, showConnecting());

        objectUnderTest.handleSib(BTS_ID);
    }
};

TEST_F(ApplicationNotConnectedTestSuite, shallHandleSibMessage)
{
    shallHandleSibMessage();
}

TEST_F(ApplicationNotConnectedTestSuite, shallHandleSmsInNotConnectedState)
{
    const common::PhoneNumber SENDER_NUMBER{123};
    const std::string MESSAGE = "Test SMS";

    // In not connected state, should show connecting and store SMS
    EXPECT_CALL(userPortMock, showNotConnected());
    EXPECT_CALL(userPortMock, showNewSms());

    objectUnderTest.handleSms(SENDER_NUMBER, MESSAGE);
}

struct ApplicationConnectingTestSuite : ApplicationNotConnectedTestSuite
{
    ApplicationConnectingTestSuite()
    {
        shallHandleSibMessage();
    }
};

TEST_F(ApplicationConnectingTestSuite, shallHandleAttachAccept)
{
    InSequence seq;  // Enforce strict order of expectations
    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showConnected());
    objectUnderTest.handleAttachAccept();
}

TEST_F(ApplicationConnectingTestSuite, shallHandleAttachReject)
{
    InSequence seq;
    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showNotConnected());
    objectUnderTest.handleAttachReject();
}

TEST_F(ApplicationConnectingTestSuite, shallHandleTimeout)
{
    EXPECT_CALL(userPortMock, showNotConnected());
    objectUnderTest.handleTimeout();
}

struct ApplicationConnectedTestSuite : ApplicationConnectingTestSuite
{
    ApplicationConnectedTestSuite()
    {
        InSequence seq;
        EXPECT_CALL(timerPortMock, stopTimer());
        EXPECT_CALL(userPortMock, showConnected());
        objectUnderTest.handleAttachAccept();
    }
};

}
