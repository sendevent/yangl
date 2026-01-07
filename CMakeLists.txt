cmake_minimum_required(VERSION 3.22)

include(YanglVersionGen.cmake)
include(CollectSourceFiles.cmake)

project(yangl
  VERSION ${YANGL_VERSION}
  DESCRIPTION "Yet Another NordVPN GUI for Linux: A lightweight, unofficial system tray app for NordVPN client on Linux"
  HOMEPAGE_URL "https://github.com/sendevent/yangl/"
  LANGUAGES CXX
)

find_package(Qt6 6.8 REQUIRED COMPONENTS
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

# --- Installation rules ---
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(FILES packaging/yangl.desktop DESTINATION share/applications)
install(FILES packaging/yangl.png DESTINATION share/icons/hicolor/256x256/apps)

# --- Packaging (CPack) ---
# Generator is selected at pack time via: cpack -G DEB|RPM

set(CPACK_PACKAGE_NAME "yangl")
set(CPACK_PACKAGE_VERSION "${YANGL_VERSION_FULL}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "Yet Another NordVPN GUI for Linux")
set(CPACK_PACKAGE_DESCRIPTION
    "Lightweight, unofficial system tray application for managing NordVPN connections on Linux. Provides quick connect/disconnect, server selection via geo chart, customizable tray icons, and configurable actions.")
set(CPACK_PACKAGE_MAINTAINER "Denis <denis@yangl>")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/sendevent/yangl/")
set(CPACK_PACKAGE_CONTACT "https://github.com/sendevent/yangl/")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/lgpl-3.0.txt")

# --- DEB-specific ---
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}.deb")

# --- RPM-specific ---
set(CPACK_RPM_PACKAGE_LICENSE "LGPLv3")
set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")
set(CPACK_RPM_PACKAGE_URL "${CPACK_PACKAGE_HOMEPAGE_URL}")
set(CPACK_RPM_PACKAGE_DESCRIPTION "${CPACK_PACKAGE_DESCRIPTION}")
set(CPACK_RPM_PACKAGE_AUTOREQ ON)
set(CPACK_RPM_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}.${CMAKE_SYSTEM_PROCESSOR}.rpm")

include(CPack)
