add_qt_test(Test_CLI_CALL
    testclicall.cpp
    ../actions/testaction.cpp
    ../actions/testaction.h
)
target_include_directories(Test_CLI_CALL PUBLIC ${CMAKE_SOURCE_DIR}/test/tests)

add_qt_test(Test_CLI_CALLER
    testclicaller.cpp
    ../actions/testaction.cpp
    ../actions/testaction.h
)
target_include_directories(Test_CLI_CALLER PUBLIC ${CMAKE_SOURCE_DIR}/test/tests)

# message(STATUS "Includes for Test_CLI_CALLER:")
# get_target_property(INCLUDES Test_CLI_CALLER INCLUDE_DIRECTORIES)
# message(STATUS "${INCLUDES}")
