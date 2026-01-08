add_qt_test(Test_NordVpnWrapper
    testnordvpnwrapper.cpp
    ../../actions/testaction.cpp
    ../../actions/testaction.h
)
target_include_directories(Test_NordVpnWrapper PRIVATE ${CMAKE_SOURCE_DIR}/test/tests)
