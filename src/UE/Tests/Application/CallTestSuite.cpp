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

class CallTestSuite : public Test
{
protected:
    const common::PhoneNumber PHONE_NUMBER{42};
    NiceMock<common::ILoggerMock> loggerMock;
    NiceMock<IBtsPortMock> btsPortMock;
    NiceMock<IUserPortMock> userPortMock;
    NiceMock<ITimerPortMock> timerPortMock;
    NiceMock<ISmsDbMock> smsDbMock;

    Application objectUnderTest;

    CallTestSuite()
        : objectUnderTest(PHONE_NUMBER,
                         loggerMock,
                         btsPortMock,
                         userPortMock,
                         timerPortMock,
                         smsDbMock)
    {
        EXPECT_CALL(userPortMock, showConnecting());
        EXPECT_CALL(btsPortMock, sendAttachRequest(common::BtsId{1}));
        objectUnderTest.handleSib(common::BtsId{1});

        EXPECT_CALL(userPortMock, showConnected());
        objectUnderTest.handleAttachAccept();
    }
};

// Tests for interactions with Receiving Call Request

TEST_F(CallTestSuite, ShallCloseImmediatelyWhenUserClosesWhileReceivingCallRequest)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    const common::PhoneNumber callerNumber{167};

    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallRequest(callerNumber));

    objectUnderTest.handleCallRequest(callerNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, sendCallDropped(callerNumber));

    objectUnderTest.handleClose();
}

TEST_F(CallTestSuite, ShallGoToNotConnectedStateImmediatelyWhenBtsConnectionDroppedWhileReceivingCallRequest)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    const common::PhoneNumber callerNumber{168};

    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallRequest(callerNumber));

    objectUnderTest.handleCallRequest(callerNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showNotConnected());
    EXPECT_CALL(btsPortMock, sendCallDropped(_)).Times(0);

    objectUnderTest.handleDisconnect();

    EXPECT_CALL(userPortMock, showConnecting());
    EXPECT_CALL(btsPortMock, sendAttachRequest(common::BtsId{2}));
    objectUnderTest.handleSib(common::BtsId{2});
}

TEST_F(CallTestSuite, ShallStoreAndNotInterruptReceivingCallRequestWhenReceivingSms)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    const common::PhoneNumber callerNumber{169};

    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallRequest(callerNumber));

    objectUnderTest.handleCallRequest(callerNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    const common::PhoneNumber smsNumber{246};
    const std::string smsText = "Test message received during call request";

    EXPECT_CALL(smsDbMock, addSms(smsNumber, smsText));

    EXPECT_CALL(userPortMock, showNewSms(_))
        .Times(AnyNumber());

    EXPECT_CALL(userPortMock, showCallRequest(_))
        .Times(0);

    objectUnderTest.handleSms(smsNumber, smsText);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, sendCallAccept(callerNumber));

    objectUnderTest.acceptCallRequest();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showConnected());

    objectUnderTest.handleCallDropped(callerNumber);

    std::vector<Sms> expectedSmsDb = {
        Sms{smsNumber, smsText}
    };

    EXPECT_CALL(smsDbMock, getAllSms())
        .Times(AtLeast(1))
        .WillRepeatedly(ReturnRef(expectedSmsDb));
    EXPECT_CALL(smsDbMock, hasUnreadSms())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(userPortMock, setSmsList(Ref(expectedSmsDb)))
        .Times(AtLeast(1));
    EXPECT_CALL(userPortMock, setSelectSmsCallback(_))
        .Times(AtLeast(1));
    EXPECT_CALL(userPortMock, showSmsList())
        .Times(AtLeast(1));

    objectUnderTest.viewSms();
}

TEST_F(CallTestSuite, ShallDropSubsequentCallRequests)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    const common::PhoneNumber firstCallerNumber{170};

    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallRequest(firstCallerNumber));

    objectUnderTest.handleCallRequest(firstCallerNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    const common::PhoneNumber secondCallerNumber{171};

    EXPECT_CALL(btsPortMock, sendCallDropped(secondCallerNumber));

    EXPECT_CALL(userPortMock, showCallRequest(_)).Times(0);
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(0);

    objectUnderTest.handleCallRequest(secondCallerNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(btsPortMock, sendCallDropped(_)).Times(0);
    EXPECT_CALL(userPortMock, showCallRequest(_)).Times(0);
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(0);

    objectUnderTest.handleCallRequest(firstCallerNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, sendCallAccept(firstCallerNumber));

    objectUnderTest.acceptCallRequest();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showConnected());

    objectUnderTest.handleCallDropped(firstCallerNumber);
}

// Tests for interactions with Sending Call Request

TEST_F(CallTestSuite, ShallCloseImmediatelyWhenUserClosesWhileDialing)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    EXPECT_CALL(btsPortMock, sendCallRequest(_)).Times(0);

    objectUnderTest.handleClose();
}

TEST_F(CallTestSuite, ShallSendCallDroppedWhenUserClosesAfterSendingCallRequest)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    const common::PhoneNumber recipientNumber{200};

    EXPECT_CALL(btsPortMock, sendCallRequest(recipientNumber));
    EXPECT_CALL(timerPortMock, startTimer(_));

    EXPECT_CALL(userPortMock, showCallView(recipientNumber));

    objectUnderTest.acceptDialing(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, sendCallDropped(recipientNumber));

    objectUnderTest.handleClose();
}

TEST_F(CallTestSuite, ShallGoToNotConnectedStateImmediatelyWhenBtsConnectionDroppedWhileSendingCallRequest)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    const common::PhoneNumber recipientNumber{198};

    EXPECT_CALL(btsPortMock, sendCallRequest(recipientNumber));
    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallView(recipientNumber));

    objectUnderTest.acceptDialing(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showNotConnected());
    EXPECT_CALL(btsPortMock, sendCallDropped(_)).Times(0);

    objectUnderTest.handleDisconnect();

    EXPECT_CALL(userPortMock, showConnecting());
    EXPECT_CALL(btsPortMock, sendAttachRequest(common::BtsId{3}));
    objectUnderTest.handleSib(common::BtsId{3});
}

TEST_F(CallTestSuite, ShallStoreAndNotInterruptSendingCallRequestWhenReceivingSms)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    const common::PhoneNumber recipientNumber{199};

    EXPECT_CALL(btsPortMock, sendCallRequest(recipientNumber));
    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallView(recipientNumber));

    objectUnderTest.acceptDialing(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    const common::PhoneNumber smsNumber{147};
    const std::string smsText = "Test SMS received during sending call request";

    EXPECT_CALL(smsDbMock, addSms(smsNumber, smsText));

    EXPECT_CALL(userPortMock, showNewSms(_))
        .Times(AnyNumber());

    EXPECT_CALL(userPortMock, showCallView(_))
        .Times(0);

    objectUnderTest.handleSms(smsNumber, smsText);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    EXPECT_CALL(timerPortMock, stopTimer());

    objectUnderTest.handleCallAccepted(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showConnected());

    objectUnderTest.handleCallDropped(recipientNumber);

    std::vector<Sms> expectedSmsDb = {
        Sms{smsNumber, smsText}
    };

    EXPECT_CALL(smsDbMock, getAllSms())
        .Times(AtLeast(1))
        .WillRepeatedly(ReturnRef(expectedSmsDb));
    EXPECT_CALL(smsDbMock, hasUnreadSms())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(userPortMock, setSmsList(Ref(expectedSmsDb)))
        .Times(AtLeast(1));
    EXPECT_CALL(userPortMock, setSelectSmsCallback(_))
        .Times(AtLeast(1));
    EXPECT_CALL(userPortMock, showSmsList())
        .Times(AtLeast(1));

    objectUnderTest.viewSms();
}

TEST_F(CallTestSuite, ShallInterruptDialingWhenReceivingCallRequest)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    const common::PhoneNumber callerNumber{190};

    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallRequest(callerNumber));

    objectUnderTest.handleCallRequest(callerNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    const common::PhoneNumber recipientNumber{195};

    EXPECT_CALL(btsPortMock, sendCallRequest(recipientNumber)).Times(0);
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(0);
    EXPECT_CALL(userPortMock, showCallView(recipientNumber)).Times(0);

    objectUnderTest.acceptDialing(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, sendCallAccept(callerNumber));

    objectUnderTest.acceptCallRequest();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showConnected());

    objectUnderTest.handleCallDropped(callerNumber);
}

TEST_F(CallTestSuite, ShallSendCallDroppedWhenReceivingCallRequestAfterSendingCallRequest)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    const common::PhoneNumber recipientNumber{191};

    EXPECT_CALL(btsPortMock, sendCallRequest(recipientNumber));
    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallView(recipientNumber));

    objectUnderTest.acceptDialing(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    const common::PhoneNumber callerNumber{175};

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, sendCallDropped(recipientNumber));
    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallRequest(callerNumber));

    objectUnderTest.handleCallRequest(callerNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, sendCallAccept(callerNumber));

    objectUnderTest.acceptCallRequest();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showConnected());

    objectUnderTest.handleCallDropped(callerNumber);
}

// Tests for interactions with Call(Talking)

TEST_F(CallTestSuite, ShallSendCallDroppedAndCloseImmediatelyWhenUserClosesWhileTalking)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    const common::PhoneNumber recipientNumber{180};

    EXPECT_CALL(btsPortMock, sendCallRequest(recipientNumber));
    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallView(recipientNumber));

    objectUnderTest.acceptDialing(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());

    objectUnderTest.handleCallAccepted(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(btsPortMock, sendCallDropped(recipientNumber));

    objectUnderTest.handleClose();
}

TEST_F(CallTestSuite, ShallGoToNotConnectedStateImmediatelyWhenBtsConnectionDroppedWhileHavingCall)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    const common::PhoneNumber recipientNumber{181};

    EXPECT_CALL(btsPortMock, sendCallRequest(recipientNumber));
    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallView(recipientNumber));

    objectUnderTest.acceptDialing(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());

    objectUnderTest.handleCallAccepted(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());
    EXPECT_CALL(userPortMock, showNotConnected());
    EXPECT_CALL(btsPortMock, sendCallDropped(_)).Times(0);

    objectUnderTest.handleDisconnect();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(userPortMock, showConnecting());
    EXPECT_CALL(btsPortMock, sendAttachRequest(common::BtsId{4}));
    objectUnderTest.handleSib(common::BtsId{4});
}

TEST_F(CallTestSuite, ShallStoreAndNotInterruptTalkingWhenReceivingSms)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    const common::PhoneNumber recipientNumber{182};

    EXPECT_CALL(btsPortMock, sendCallRequest(recipientNumber));
    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallView(recipientNumber));

    objectUnderTest.acceptDialing(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    EXPECT_CALL(timerPortMock, stopTimer());

    objectUnderTest.handleCallAccepted(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    const common::PhoneNumber smsNumber{148};
    const std::string smsText = "Test SMS received during active call";

    EXPECT_CALL(smsDbMock, addSms(smsNumber, smsText));

    EXPECT_CALL(userPortMock, showNewSms(_))
        .Times(AnyNumber());

    EXPECT_CALL(userPortMock, showCallView(_))
        .Times(0);

    objectUnderTest.handleSms(smsNumber, smsText);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    EXPECT_CALL(userPortMock, showConnected());

    objectUnderTest.handleCallDropped(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);
    testing::Mock::VerifyAndClearExpectations(&smsDbMock);

    std::vector<Sms> expectedSmsDb = {
        Sms{smsNumber, smsText}
    };

    EXPECT_CALL(smsDbMock, getAllSms())
        .Times(AtLeast(1))
        .WillRepeatedly(ReturnRef(expectedSmsDb));
    EXPECT_CALL(smsDbMock, hasUnreadSms())
        .Times(AtLeast(1))
        .WillRepeatedly(Return(true));

    EXPECT_CALL(userPortMock, setSmsList(Ref(expectedSmsDb)))
        .Times(AtLeast(1));
    EXPECT_CALL(userPortMock, setSelectSmsCallback(_))
        .Times(AtLeast(1));
    EXPECT_CALL(userPortMock, showSmsList())
        .Times(AtLeast(1));

    objectUnderTest.viewSms();
}

TEST_F(CallTestSuite, ShallDropNewCallRequestWhileTalking)
{
    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);


    EXPECT_CALL(userPortMock, showDialView());
    objectUnderTest.dial();

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);

    const common::PhoneNumber recipientNumber{190};

    EXPECT_CALL(btsPortMock, sendCallRequest(recipientNumber));
    EXPECT_CALL(timerPortMock, startTimer(_));
    EXPECT_CALL(userPortMock, showCallView(recipientNumber));

    objectUnderTest.acceptDialing(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(timerPortMock, stopTimer());

    objectUnderTest.handleCallAccepted(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    const common::PhoneNumber newCallerNumber{191};

    EXPECT_CALL(btsPortMock, sendCallDropped(newCallerNumber));


    EXPECT_CALL(userPortMock, showCallRequest(_)).Times(0);
    EXPECT_CALL(userPortMock, showCallView(newCallerNumber)).Times(0);
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(0);

    objectUnderTest.handleCallRequest(newCallerNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(btsPortMock, sendCallDropped(_)).Times(0);
    EXPECT_CALL(userPortMock, showCallRequest(_)).Times(0);
    EXPECT_CALL(timerPortMock, startTimer(_)).Times(0);

    objectUnderTest.handleCallRequest(recipientNumber);

    testing::Mock::VerifyAndClearExpectations(&userPortMock);
    testing::Mock::VerifyAndClearExpectations(&btsPortMock);
    testing::Mock::VerifyAndClearExpectations(&timerPortMock);

    EXPECT_CALL(btsPortMock, sendCallDropped(recipientNumber));
    EXPECT_CALL(userPortMock, showConnected());

    objectUnderTest.rejectCallRequest();
}
}  // namespace ue
