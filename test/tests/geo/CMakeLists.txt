set(TEST_RCC_SOURCES ${CMAKE_SOURCE_DIR}/src/rsc.qrc)
qt_add_resources(TEST_RCC_SOURCES ${APP_RCC})

list(APPEND TEST_SOURCES ${TEST_RCC_SOURCES})

add_qt_test(Test_CoordinatesResolver
    testcoordinatesresolver.cpp
)
