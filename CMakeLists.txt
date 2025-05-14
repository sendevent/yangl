cmake_minimum_required(VERSION 3.31)

include(YanglVersionGen.cmake)
include(CollectSourceFiles.cmake)

project(yangl
  VERSION ${YANGL_VERSION}
  DESCRIPTION "Yet Another NordVPN GUI for Linux: A lightweight, unofficial system tray app for NordVPN client on Linux"
  HOMEPAGE_URL "https://github.com/sendevent/yangl/"
  LANGUAGES CXX
)

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
