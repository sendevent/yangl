qt_add_resources(TEST_GEO_RCC ${CMAKE_SOURCE_DIR}/src/rsc.qrc)

add_qt_test(Test_CoordinatesResolver
    testcoordinatesresolver.cpp
    ${TEST_GEO_RCC}
)
