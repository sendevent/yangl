# "v 1.2.3" — to be actualized manually:

set(YANGL_V_MAJOR 2)
set(YANGL_V_MINOR 0)
set(YANGL_V_PATCH 4)

# the rest is the automatic acquirement of the necessary fields:

include(${CMAKE_CURRENT_SOURCE_DIR}/GetGitRevisionDescription.cmake)

string(TIMESTAMP YANGL_V_BUILD_TIME_HUMAN "%b %d %Y, %H:%M:%SZ" UTC)
string(TIMESTAMP YANGL_V_BUILD_TIME_SECONDS "%s" UTC)

get_git_head_revision(YANGL_V_BRANCH_FULL YANGL_V_COMMIT_FULL)

string(REPLACE "refs/heads/" "" YANGL_V_BRANCH "${YANGL_V_BRANCH_FULL}")

string(SUBSTRING ${YANGL_V_COMMIT_FULL} 0 7 YANGL_V_COMMIT_SHORT)

git_local_changes(__HAS_UNCOMMITED_SOURCES)
string(COMPARE NOTEQUAL ${__HAS_UNCOMMITED_SOURCES} "CLEAN" YANGL_V_HAS_CHANGES)

# monotonic build number from total commit count:
if(NOT GIT_FOUND)
    find_package(Git QUIET)
endif()
if(GIT_FOUND)
    execute_process(
        COMMAND "${GIT_EXECUTABLE}" rev-list --count HEAD
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE _rev_list_res
        OUTPUT_VARIABLE YANGL_V_BUILD_NUMBER
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT _rev_list_res EQUAL 0)
        set(YANGL_V_BUILD_NUMBER 0)
    endif()
else()
    set(YANGL_V_BUILD_NUMBER 0)
endif()

# generate the header:
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/version/appversiondefs.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/version/appversiondefs.h)

set(YANGL_VERSION "${YANGL_V_MAJOR}.${YANGL_V_MINOR}.${YANGL_V_PATCH}")
set(YANGL_VERSION_FULL "${YANGL_VERSION}.${YANGL_V_BUILD_NUMBER}")


