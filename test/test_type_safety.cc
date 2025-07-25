#include <doctest/doctest.h>
#include <sdlpp/detail/type_utils.hh>
#include <limits>
#include <cstdint>

TEST_SUITE("type safety") {
    TEST_CASE("safe_numeric_cast") {
        using namespace sdlpp::detail;
        
        SUBCASE("positive int to size_t") {
            auto result = safe_numeric_cast<size_t, int>(42);
            REQUIRE(result.has_value());
            CHECK(*result == 42);
        }
        
        SUBCASE("negative int to size_t fails") {
            auto result = safe_numeric_cast<size_t, int>(-1);
            REQUIRE(!result.has_value());
            CHECK(result.error() == "Cannot convert negative value to unsigned type");
        }
        
        SUBCASE("large size_t to int fails") {
            size_t large_value = static_cast<size_t>(std::numeric_limits<int>::max()) + 1;
            auto result = safe_numeric_cast<int, size_t>(large_value);
            REQUIRE(!result.has_value());
            CHECK(result.error().find("too large") != std::string::npos);
        }
        
        SUBCASE("size_t to int within bounds") {
            size_t value = 12345;
            auto result = safe_numeric_cast<int, size_t>(value);
            REQUIRE(result.has_value());
            CHECK(*result == 12345);
        }
    }
    
    TEST_CASE("size_to_int") {
        using namespace sdlpp::detail;
        
        SUBCASE("valid conversion") {
            auto result = size_to_int(100);
            REQUIRE(result.has_value());
            CHECK(*result == 100);
        }
        
        SUBCASE("overflow") {
            size_t large = static_cast<size_t>(std::numeric_limits<int>::max()) + 100;
            auto result = size_to_int(large);
            REQUIRE(!result.has_value());
        }
    }
    
    TEST_CASE("int_to_size") {
        using namespace sdlpp::detail;
        
        SUBCASE("positive int") {
            auto result = int_to_size(42);
            REQUIRE(result.has_value());
            CHECK(*result == 42);
        }
        
        SUBCASE("negative int fails") {
            auto result = int_to_size(-42);
            REQUIRE(!result.has_value());
            CHECK(result.error().find("negative") != std::string::npos);
        }
        
        SUBCASE("zero") {
            auto result = int_to_size(0);
            REQUIRE(result.has_value());
            CHECK(*result == 0);
        }
    }
    
    TEST_CASE("clamp_size_to_int") {
        using namespace sdlpp::detail;
        
        SUBCASE("small size") {
            size_t small = 100;
            int result = clamp_size_to_int(small);
            CHECK(result == 100);
        }
        
        SUBCASE("large size clamped") {
            size_t large = static_cast<size_t>(std::numeric_limits<int>::max()) + 1000;
            int result = clamp_size_to_int(large);
            CHECK(result == std::numeric_limits<int>::max());
        }
        
        SUBCASE("exact max") {
            size_t exact = static_cast<size_t>(std::numeric_limits<int>::max());
            int result = clamp_size_to_int(exact);
            CHECK(result == std::numeric_limits<int>::max());
        }
    }
}