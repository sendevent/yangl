cmake_minimum_required(VERSION 3.10)

set_target_properties(${TESTNAME} PROPERTIES AUTOUIC OFF)

set(TEST_SOURCES ${ARGN} ${APP_SOURCES})

list(FILTER TEST_SOURCES EXCLUDE REGEX ".*\\.ui$")
list(FILTER TEST_SOURCES EXCLUDE REGEX ".*ui_.*\\.h$")
list(FILTER TEST_SOURCES EXCLUDE REGEX ".*moc_.*\\.cpp$")
list(FILTER TEST_SOURCES EXCLUDE REGEX "main\\.cpp$")

enable_testing()

# Add test utility first
add_subdirectory(test_fake_status)

# Then unit tests
add_subdirectory(tests)
