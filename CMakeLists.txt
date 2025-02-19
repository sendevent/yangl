cmake_minimum_required(VERSION 3.19)

project(yangl LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Concurrent Qml Quick QuickWidgets Location Positioning)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

qt_standard_project_setup()

include(${CMAKE_CURRENT_SOURCE_DIR}/CollectSourceFiles.cmake)

add_subdirectory(app)

option(DISABLE_TESTS "Disable unit tests" OFF)
# if(NOT DISABLE_TESTS)
#     add_subdirectory(test)
# endif()
