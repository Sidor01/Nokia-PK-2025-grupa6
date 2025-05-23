project(UeApplication)
cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

aux_source_directory(. SRC_LIST)
aux_source_directory(Ports SRC_LIST)
aux_source_directory(States SRC_LIST)

add_library(${PROJECT_NAME} ${SRC_LIST}
    Ports/BtsPort.cpp Ports/BtsPort.hpp Ports/IBtsPort.hpp Ports/ITimerPort.hpp Ports/IUserPort.hpp Ports/ISmsListViewPort.hpp Ports/TimerPort.cpp Ports/TimerPort.hpp Ports/UserPort.cpp Ports/UserPort.hpp 
    States/BaseState.cpp States/BaseState.hpp 
    States/ConnectedState.cpp States/ConnectedState.hpp 
    States/ConnectingState.cpp States/ConnectingState.hpp 
    States/NotConnectedState.cpp States/NotConnectedState.hpp 
    States/TalkingState.cpp States/TalkingState.hpp
    States/DialState.cpp States/DialState.hpp
    States/SmsViewState.cpp States/SmsViewState.hpp
    States/SmsComposeState.cpp States/SmsComposeState.hpp
    SmsDb.cpp SmsDb.hpp
    Application.cpp Application.hpp CMakeLists.txt Context.hpp IEventsHandler.hpp)
target_link_libraries(${PROJECT_NAME} Common)
