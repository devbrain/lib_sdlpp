# First try to find failsafe locally
find_package(failsafe QUIET)

if(NOT failsafe_FOUND)
    message(STATUS "failsafe not found locally, fetching from GitHub...")
    
    fetchcontent_declare(
                FAILSAFE
                GIT_REPOSITORY "https://github.com/devbrain/failsafe.git"
                GIT_TAG master
                GIT_PROGRESS TRUE
                UPDATE_COMMAND ""
                OVERRIDE_FIND_PACKAGE
        )
    fetchcontent_makeavailable(FAILSAFE)
    add_library(thirdparty::failsafe ALIAS failsafe)
else()
    message(STATUS "Found failsafe locally")
    if(NOT TARGET thirdparty::failsafe)
        add_library(thirdparty::failsafe ALIAS failsafe::failsafe)
    endif()
endif()
