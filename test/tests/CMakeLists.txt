set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Test)

include(${CMAKE_CURRENT_SOURCE_DIR}/TestMacros.cmake)

add_subdirectory(app)
add_subdirectory(actions)
add_subdirectory(cli)
add_subdirectory(geo)
