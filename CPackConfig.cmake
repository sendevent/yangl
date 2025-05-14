cmake_minimum_required(VERSION 3.31)

# Set the project name (should match your CMakeLists.txt)
set(CPACK_PACKAGE_NAME "yangl")
set(CPACK_PROJECT_NAME ${CPACK_PACKAGE_NAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "/opt/${CPACK_PACKAGE_NAME}")
set(CPACK_PACKAGE_ARCHITECTURE "x86_64")
# Set the project version (should match your CMakeLists.txt)
set(CPACK_PACKAGE_VERSION ${YANGL_VERSION})
set(CPACK_PROJECT_VERSION ${CPACK_PACKAGE_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# Set the package description
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Yet Another NordVPN GUI for Linux")
set(CPACK_PACKAGE_DESCRIPTION "Yet Another NordVPN GUI for Linux: A lightweight, unofficial system tray app for NordVPN client on Linux")

# Set the package vendor
set(CPACK_PACKAGE_VENDOR "sendevent")

# Set the package maintainer (replace with your email)
set(CPACK_PACKAGE_CONTACT "your_email@example.com")

# Set the package license
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_ARCHITECTURE})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/lgpl-3.0.txt")

# Set the generators (DEB and RPM)
#set(CPACK_GENERATORS "DEB;RPM")
set(CPACK_GENERATORS "DEB") # Generate only DEB for now

# Configure DEB specific settings
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_CONTACT})

# Configure RPM specific settings
set(CPACK_RPM_PACKAGE_LICENSE "GPL")
set(CPACK_RPM_PACKAGE_VENDOR ${CPACK_PACKAGE_VENDOR})
set(CPACK_RPM_PACKAGE_GROUP "Applications/Internet")

# Include default CPack configuration file
include(CPack)
