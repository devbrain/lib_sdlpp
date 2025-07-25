# Check if std::expected is available
include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")
check_cxx_source_compiles("
    #include <expected>
    int main() {
        std::expected<int, char> e{42};
        return e.value();
    }
" HAVE_STD_EXPECTED)

if(HAVE_STD_EXPECTED)
    message(STATUS "Using std::expected from C++23 standard library")
else()
    message(STATUS "std::expected not available, using tl::expected fallback")
endif()

if(NOT HAVE_STD_EXPECTED)
    # Fetch tl::expected only if std::expected is not available
    set(EXPECTED_BUILD_PACKAGE OFF CACHE INTERNAL "")
    set(EXPECTED_BUILD_TESTS OFF CACHE INTERNAL "")
    fetchcontent_declare(
            EXPECTED
            GIT_REPOSITORY "https://github.com/TartanLlama/expected.git"
            GIT_TAG v1.2.0
            GIT_PROGRESS TRUE
            UPDATE_COMMAND ""
    )
    fetchcontent_makeavailable(EXPECTED)
    add_library(thirdparty::expected ALIAS expected)
else()
    # Create an interface library that uses std::expected
    add_library(thirdparty_expected_std INTERFACE)
    add_library(thirdparty::expected ALIAS thirdparty_expected_std)
endif()