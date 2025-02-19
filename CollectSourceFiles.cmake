# Define a macro to collect *.h, *.cpp, *.ui files, append them to the app's source
# and add the current subfolder to the INCLUDE dirs
macro(collect_source_files)
    file(GLOB SUBDIR_SOURCES CONFIGURE_DEPENDS "*.h" "*.cpp" "*.ui")
    set(APP_SOURCES "${APP_SOURCES}" "${SUBDIR_SOURCES}" PARENT_SCOPE)
    # set(YANGL_INCLUDE_DIRS "${YANGL_INCLUDE_DIRS}" "${CMAKE_CURRENT_SOURCE_DIR}/" PARENT_SCOPE)
endmacro()


