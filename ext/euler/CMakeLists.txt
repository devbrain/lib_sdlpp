# First try to find euler locally
find_package(euler QUIET)

if(NOT euler_FOUND)
    message(STATUS "euler not found locally, fetching from GitHub...")
    
    # Ensure euler build options are set appropriately
    set(EULER_BUILD_TESTS OFF CACHE INTERNAL "")
    set(EULER_BUILD_EXAMPLES OFF CACHE INTERNAL "")
    set(EULER_BUILD_BENCHMARKS OFF CACHE INTERNAL "")
    
    fetchcontent_declare(
                euler
                GIT_REPOSITORY "https://github.com/devbrain/euler.git"
                GIT_TAG master
                GIT_PROGRESS TRUE
                UPDATE_COMMAND ""
                OVERRIDE_FIND_PACKAGE
        )
    fetchcontent_makeavailable(euler)
    
    # Fix euler's include directories to use proper paths
    if(TARGET euler)
        get_target_property(euler_includes euler INTERFACE_INCLUDE_DIRECTORIES)
        list(REMOVE_ITEM euler_includes "${CMAKE_SOURCE_DIR}/include")
        set_target_properties(euler PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "")
        target_include_directories(euler INTERFACE 
            $<BUILD_INTERFACE:${euler_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        )
    endif()
    add_library(thirdparty::euler ALIAS euler)
else()
    message(STATUS "Found euler locally")
    if(NOT TARGET thirdparty::euler)
        add_library(thirdparty::euler ALIAS euler::euler)
    endif()
endif()
