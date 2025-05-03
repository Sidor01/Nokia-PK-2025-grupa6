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
    context.bts.sendAttachRequest(btsId);
    context.timer.startTimer(500ms);

    context.setState<ConnectingState>();
}

}
