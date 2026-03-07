# Define a macro to add a Qt test
macro(add_qt_test TESTNAME)
    # All arguments after the first two are treated as source files
    add_executable(${TESTNAME} ${ARGN} ${TEST_SOURCES})

    target_include_directories(${TESTNAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/test/tests
    )

    target_link_libraries(${TESTNAME} PRIVATE
        yangl_lib
        Qt6::Test
    )

    add_test(NAME ${TESTNAME} COMMAND ${TESTNAME})

endmacro()
