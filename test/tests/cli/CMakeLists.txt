add_qt_test(Test_CLICall
    testclicall.cpp
    ../actions/testaction.cpp
    ../actions/testaction.h
)
target_include_directories(Test_CLICall PUBLIC ${CMAKE_SOURCE_DIR}/test/tests)

add_qt_test(Test_CLICaller
    testclicaller.cpp
    ../actions/testaction.cpp
    ../actions/testaction.h
)
target_include_directories(Test_CLICaller PUBLIC ${CMAKE_SOURCE_DIR}/test/tests)

