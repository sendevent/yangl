add_subdirectory(app)
add_subdirectory(actions)
add_subdirectory(cli)
add_subdirectory(geo)
add_subdirectory(settings)
add_subdirectory(version)

collect_source_files()

qt_add_resources(APP_RCC_SOURCES rsc.qrc)

list(APPEND APP_SOURCES ${APP_RCC_SOURCES} "main.cpp")

qt_add_executable(${PROJECT_NAME} ${APP_SOURCES})
