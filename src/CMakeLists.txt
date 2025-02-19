
message("inapp-1 YANGL_INCLUDE_DIRS: " ${YANGL_INCLUDE_DIRS})

add_subdirectory(app)
add_subdirectory(actions)
add_subdirectory(cli)
add_subdirectory(geo)
add_subdirectory(settings)
add_subdirectory(version)

message("BEFORE: " ${APP_SOURCES})
collect_source_files()
message("After: " ${APP_SOURCES})

set(APP_RCC_SOURCES rsc.qrc)
qt_add_resources(APP_RCC_SOURCES ${APP_RCC})

list(APPEND APP_SOURCES ${APP_RCC_SOURCES} "main.cpp")

qt_add_executable(${PROJECT_NAME} ${APP_SOURCES})

set(YANGL_INCLUDE_DIRS "${YANGL_INCLUDE_DIRS}" "${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories(${PROJECT_NAME} PUBLIC "${YANGL_INCLUDE_DIRS}")

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Qml
    Qt6::Quick
    Qt6::Concurrent
    Qt6::QuickWidgets
    Qt6::Location
    Qt6::Positioning
)
