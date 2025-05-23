#include "NotConnectedState.hpp"
#include "ConnectingState.hpp"

namespace ue
{

NotConnectedState::NotConnectedState(Context &context, bool firstState)
    : BaseState(context, "NotConnectedState")
{
    if(!firstState) {
        context.user.showNotConnected();
    }
}

void NotConnectedState::handleSib(common::BtsId btsId)
{
    using namespace std::chrono_literals;
    
    
    context.currentBtsId = btsId;
    
    
    context.bts.sendAttachRequest(btsId);
    context.timer.startTimer(500ms);

    context.setState<ConnectingState>();
}
void NotConnectedState::handleSms(common::PhoneNumber from, std::string text)
{
}
}
