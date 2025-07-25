cmake_minimum_required(VERSION 3.20)
project(sdlpp
    VERSION 0.1.0
    DESCRIPTION "Modern C++20 wrapper for SDL3"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build options
option(SDLPP_BUILD_WITH_TESTS "Build SDL++ tests" ON)
option(SDLPP_BUILD_WITH_EXAMPLES "Build SDL++ examples" ON)
option(SDLPP_BUILD_SHARED "Build SDL++ as a shared library" ON)
option(SDLPP_INSTALL "Generate install targets" ON)

# Include standard install directories
include(GNUInstallDirs)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Build configuration options
option(SDLPP_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(SDLPP_ENABLE_LTO "Enable link-time optimization" OFF)

# Set visibility settings for shared libraries
if(SDLPP_BUILD_SHARED)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_C_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)
    include(GenerateExportHeader)
endif()

# Define warning flags for our project
set(SDLPP_WARNING_FLAGS)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(SDLPP_WARNING_FLAGS
        -Wall           # All standard warnings
        -Wextra         # Extra warnings
        -Wpedantic      # Strict ISO C++ compliance
        -Wcast-align    # Warn about potential performance problem casts
        -Wcast-qual     # Warn about casts that remove qualifiers
        -Wconversion    # Warn about type conversions that may lose data
        -Wdouble-promotion  # Warn about float to double promotion
        -Wformat=2      # Additional format string warnings
        -Wnon-virtual-dtor  # Warn about non-virtual destructors
        -Wold-style-cast    # Warn about C-style casts
        -Woverloaded-virtual # Warn about overloaded virtual functions
        -Wshadow        # Warn about variable shadowing
        -Wsign-conversion   # Warn about sign conversions
        -Wundef         # Warn about undefined identifiers in #if
        -Wunused        # Warn about unused entities
        -Wzero-as-null-pointer-constant # Warn about using 0 as nullptr
    )
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND SDLPP_WARNING_FLAGS
            -Wlogical-op    # Warn about logical operations being used where bitwise were probably wanted
            -Wuseless-cast  # Warn about useless casts
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(SDLPP_WARNING_FLAGS
        /W4             # Highest warning level
        /permissive-    # Strict standard conformance
        /Zc:__cplusplus # Report correct __cplusplus value
    )
endif()

# Treat warnings as errors in CI/Release builds (optional)
# list(APPEND SDLPP_WARNING_FLAGS -Werror)

# Sanitizer flags
if(SDLPP_ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        set(SDLPP_SANITIZER_FLAGS
            -fsanitize=address
            -fsanitize=undefined
            -fno-omit-frame-pointer
        )
        list(APPEND SDLPP_WARNING_FLAGS ${SDLPP_SANITIZER_FLAGS})
        
        # Link flags for sanitizers
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address -fsanitize=undefined")
    else()
        message(WARNING "Sanitizers are only supported with GCC and Clang")
    endif()
endif()

# Link-time optimization
if(SDLPP_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "Link-time optimization enabled")
    else()
        message(WARNING "Link-time optimization not supported: ${ipo_output}")
    endif()
endif()

# Output binary to predictable location.
set(BINARY_OUT_DIR ${CMAKE_BINARY_DIR}/bin)
set(LIB_OUT_DIR ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${LIB_OUT_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_OUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BINARY_OUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${LIB_OUT_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_OUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BINARY_OUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${LIB_OUT_DIR})
# ===========================================================================
set (PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(ext)
add_subdirectory(src/sdlpp)

if(SDLPP_BUILD_WITH_TESTS)
    add_subdirectory(test)
endif()

if(SDLPP_BUILD_WITH_EXAMPLES)
    add_subdirectory(examples)
endif()

# Feature summary
include(FeatureSummary)

set_package_properties(SDL3 PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "Simple DirectMedia Layer 3"
    URL "https://www.libsdl.org/"
)

set_package_properties(doctest PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "C++ testing framework"
    URL "https://github.com/onqtam/doctest"
)

set_package_properties(failsafe PROPERTIES
    TYPE REQUIRED
    DESCRIPTION "C++ error handling library"
    URL "https://github.com/devbrain/failsafe"
)

add_feature_info("Shared Library" SDLPP_BUILD_SHARED "Build SDL++ as a shared library")
add_feature_info("Tests" SDLPP_BUILD_WITH_TESTS "Build unit tests")
add_feature_info("Examples" SDLPP_BUILD_WITH_EXAMPLES "Build example applications")
add_feature_info("Install" SDLPP_INSTALL "Generate installation targets")
add_feature_info("Sanitizers" SDLPP_ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan")
add_feature_info("LTO" SDLPP_ENABLE_LTO "Enable link-time optimization")

message(STATUS "")
message(STATUS "SDL++ ${PROJECT_VERSION} Configuration Summary:")
message(STATUS "================================")
message(STATUS "Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix:    ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Library type:      ${SDLPP_LIBRARY_TYPE}")
message(STATUS "")

feature_summary(WHAT ALL)
