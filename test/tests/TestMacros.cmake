# Define a macro to add a Qt test
macro(add_qt_test TESTNAME)
    # All arguments after the first two are treated as source files
    add_executable(${TESTNAME} ${ARGN} ${TEST_SOURCES})

    target_link_libraries(${TESTNAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Network
        Qt6::Qml
        Qt6::Quick
        Qt6::Concurrent
        Qt6::QuickWidgets
        Qt6::Location
        Qt6::Positioning
        Qt6::Test
    )

    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})
    target_include_directories(${TESTNAME} PUBLIC ${CMAKE_SOURCE_DIR}/src/)

endmacro()
