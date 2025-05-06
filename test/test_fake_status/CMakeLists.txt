cmake_minimum_required(VERSION 3.16)
project(test_fake_status LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(test_fake_status main.cpp)
target_link_libraries(test_fake_status PRIVATE Qt6::Core)

target_compile_definitions(test_fake_status PRIVATE QT_DEPRECATED_WARNINGS)

target_include_directories(test_fake_status PRIVATE ${Qt6Core_INCLUDE_DIRS})

set_target_properties(test_fake_status PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/../tests"
)

# install(TARGETS test_fake_status DESTINATION /opt/test_fake_status/bin)
