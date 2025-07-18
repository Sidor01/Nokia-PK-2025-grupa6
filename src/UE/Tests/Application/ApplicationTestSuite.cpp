#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Application.hpp"
#include "Mocks/ILoggerMock.hpp"
#include "Mocks/IBtsPortMock.hpp"
#include "Mocks/IUserPortMock.hpp"
#include "Mocks/ITimerPortMock.hpp"
#include "Mocks/ISmsDbMock.hpp"
#include "Messages/PhoneNumber.hpp"
#include "Messages/BtsId.hpp"
#include <memory>

namespace ue
{
using namespace ::testing;

class ApplicationTestSuite : public Test
{
protected:
    const common::PhoneNumber PHONE_NUMBER{42}; // test specific number, don't use in production
    NiceMock<common::ILoggerMock> loggerMock;
    NiceMock<IBtsPortMock> btsPortMock;
    NiceMock<IUserPortMock> userPortMock;
    NiceMock<ITimerPortMock> timerPortMock;
    NiceMock<ISmsDbMock> smsDbMock;

    Application objectUnderTest;
    
    ApplicationTestSuite()
        : objectUnderTest(PHONE_NUMBER, 
                         loggerMock,
                         btsPortMock,
                         userPortMock,
                         timerPortMock,
                         smsDbMock)
    {}
};

struct ApplicationNotConnectedTestSuite : ApplicationTestSuite
{};

TEST_F(ApplicationNotConnectedTestSuite, todo)
{
}

TEST_F(ApplicationNotConnectedTestSuite, ShallStopTimerAndCloseUeWhenClosingInConnectingState)
{
    EXPECT_CALL(userPortMock, showConnecting());
    EXPECT_CALL(btsPortMock, sendAttachRequest(common::BtsId{1}));
    objectUnderTest.handleSib(common::BtsId{1});
    
    EXPECT_CALL(timerPortMock, stopTimer()).Times(1);
    
    objectUnderTest.handleClose();
}

TEST_F(ApplicationNotConnectedTestSuite, ShallStoreNewSibDataWhileInConnectingState)
{
    EXPECT_CALL(userPortMock, showConnecting());
    EXPECT_CALL(btsPortMock, sendAttachRequest(common::BtsId{1})).Times(1);
    objectUnderTest.handleSib(common::BtsId{1});
    
    EXPECT_CALL(btsPortMock, sendAttachRequest(common::BtsId{2})).Times(0);
    objectUnderTest.handleSib(common::BtsId{2});

    EXPECT_CALL(userPortMock, showConnected());
    EXPECT_CALL(timerPortMock, stopTimer());
    objectUnderTest.handleAttachAccept();
}

TEST_F(ApplicationNotConnectedTestSuite, ShallBreakAttachProcedureWhenConnectionToBtsDroppingWhileConnecting)
{
    EXPECT_CALL(userPortMock, showConnecting());
    EXPECT_CALL(btsPortMock, sendAttachRequest(common::BtsId{1}));
    objectUnderTest.handleSib(common::BtsId{1});
    
    EXPECT_CALL(timerPortMock, stopTimer()).Times(1);
    EXPECT_CALL(userPortMock, showNotConnected());
    objectUnderTest.handleDisconnect();

    EXPECT_CALL(userPortMock, showConnecting());
    EXPECT_CALL(btsPortMock, sendAttachRequest(common::BtsId{2}));
    objectUnderTest.handleSib(common::BtsId{2});
}

// Tests related to SMS viewing have been moved to ViewSmsTestSuite.cpp

// ViewSmsTestSuite has been moved to its own file: ViewSmsTestSuite.cpp
}
