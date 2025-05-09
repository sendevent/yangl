cmake_minimum_required(VERSION 3.31)

project(yangl LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    Network
    Qml
    Quick
    Concurrent
    QuickWidgets
    Location
    Positioning
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

include(${CMAKE_CURRENT_SOURCE_DIR}/CollectSourceFiles.cmake)

add_subdirectory(src)

target_link_libraries(${PROJECT_NAME} PRIVATE
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
)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src/)

option(DISABLE_TESTS "Disable unit tests" ON)
if(NOT DISABLE_TESTS)
    add_subdirectory(test)
endif()

