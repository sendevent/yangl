# "v 1.2.3" — to be actualized manually:

set(YANGL_V_MAJOR 1)
set(YANGL_V_MINOR 0)
set(YANGL_V_PATCH 0)

# the rest is the automatic acquirement of the necessary fields:

include(${CMAKE_CURRENT_SOURCE_DIR}/GetGitRevisionDescription.cmake)

string(TIMESTAMP YANGL_V_BUILD_TIME_HUMAN "%b %d %Y, %H:%M:%SZ" UTC)
string(TIMESTAMP YANGL_V_BUILD_TIME_SECONDS "%s" UTC)

get_git_head_revision(YANGL_V_BRANCH_FULL YANGL_V_COMMIT_FULL)

string(REPLACE "refs/heads/" "" YANGL_V_BRANCH "${YANGL_V_BRANCH_FULL}")

string(SUBSTRING ${YANGL_V_COMMIT_FULL} 0 7 YANGL_V_COMMIT_SHORT)

git_local_changes(__HAS_UNCOMMITED_SOURCES)
string(COMPARE NOTEQUAL ${__HAS_UNCOMMITED_SOURCES} "CLEAN" YANGL_V_HAS_CHANGES)

# generate the header:
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/appversiondefs.h.in ${CMAKE_CURRENT_SOURCE_DIR}/appversiondefs.h)

collect_source_files()
