add_subdirectory(app)
add_subdirectory(actions)
add_subdirectory(cli)
add_subdirectory(geo)
add_subdirectory(settings)
add_subdirectory(version)

collect_source_files()

qt_add_resources(APP_RCC_SOURCES rsc.qrc)

# Static library from all source files except main.cpp (shared by app and tests)
# Resources are NOT included here — they must be compiled into each binary separately
# to ensure proper Qt resource initialization.
file(GLOB_RECURSE _YANGL_LIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.ui"
)
list(FILTER _YANGL_LIB_SOURCES EXCLUDE REGEX "main\\.cpp$")
add_library(yangl_lib STATIC ${_YANGL_LIB_SOURCES})

qt_add_executable(${PROJECT_NAME} main.cpp ${APP_RCC_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE yangl_lib)
