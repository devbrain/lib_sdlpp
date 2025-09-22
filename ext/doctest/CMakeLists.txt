# First check if doctest target already exists (from parent project)
if(TARGET doctest OR TARGET doctest::doctest)
    message(STATUS "Found doctest target from parent project")
else()
    # Try to find doctest locally
    find_package(doctest QUIET)

    if(NOT doctest_FOUND)
        message(STATUS "doctest not found locally, fetching from GitHub...")

        set(DOCTEST_WITH_TESTS OFF CACHE INTERNAL "")
        set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE INTERNAL "")

        fetchcontent_declare(
                DOCTEST
                GIT_REPOSITORY "https://github.com/onqtam/doctest.git"
                GIT_TAG v2.4.12
                GIT_PROGRESS TRUE
                UPDATE_COMMAND ""
                OVERRIDE_FIND_PACKAGE
        )
        fetchcontent_makeavailable(DOCTEST)

    else()
        message(STATUS "Found doctest locally")
    endif()
endif()
