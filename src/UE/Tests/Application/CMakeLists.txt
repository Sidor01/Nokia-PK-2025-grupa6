project(UeApplicationUT)
cmake_minimum_required(VERSION 3.12)

aux_source_directory(. SRC_LIST)
aux_source_directory(Mocks SRC_LIST)
aux_source_directory(Ports SRC_LIST)
list(REMOVE_ITEM SRC_FILES "${CMAKE_CURRENT_SOURCE_DIR}/ViewSmsTestSuite.cpp")
list(APPEND SRC_LIST CallTestSuite.cpp)
include_directories(${COMMON_DIR}/Tests)
include_directories(${UE_DIR}/Tests)

add_executable(${PROJECT_NAME} ${SRC_LIST})
target_link_libraries(${PROJECT_NAME} UeApplication)
target_link_libraries(${PROJECT_NAME} CommonUtMocks)
target_link_gtest()





