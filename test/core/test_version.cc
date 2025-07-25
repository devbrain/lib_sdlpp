//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <sstream>

#include "sdlpp/core/version.hh"

using namespace sdlpp;
using namespace sdlpp::literals;

TEST_SUITE("version") {
    
    TEST_CASE("version construction") {
        SUBCASE("default constructor") {
            version v;
            CHECK(v.major() == 0);
            CHECK(v.minor() == 0);
            CHECK(v.micro() == 0);
            CHECK(v.patch() == 0); // Alias for micro
        }
        
        SUBCASE("component constructor") {
            version v(3, 2, 1);
            CHECK(v.major() == 3);
            CHECK(v.minor() == 2);
            CHECK(v.micro() == 1);
        }
        
        SUBCASE("numeric constructor") {
            // SDL uses format MMMNNNCCC (3 digits each)
            version v1(3002001); // 3.2.1
            CHECK(v1.major() == 3);
            CHECK(v1.minor() == 2);
            CHECK(v1.micro() == 1);
            
            version v2(10005023); // 10.5.23
            CHECK(v2.major() == 10);
            CHECK(v2.minor() == 5);
            CHECK(v2.micro() == 23);
        }
        
        SUBCASE("user-defined literal") {
            auto v = 321_v; // 3.2.1
            CHECK(v.major() == 3);
            CHECK(v.minor() == 2);
            CHECK(v.micro() == 1);
        }
    }
    
    TEST_CASE("version conversion") {
        SUBCASE("to_number") {
            version v(3, 2, 1);
            CHECK(v.to_number() == 3002001);
            
            version v2(10, 99, 456);
            CHECK(v2.to_number() == 10099456);
        }
        
        SUBCASE("to_string") {
            version v1(3, 2, 1);
            CHECK(v1.to_string() == "3.2.1");
            
            version v2(10, 0, 0);
            CHECK(v2.to_string() == "10.0.0");
        }
        
        SUBCASE("tuple conversion") {
            version v(3, 2, 1);
            auto [major, minor, micro] = v;
            CHECK(major == 3);
            CHECK(minor == 2);
            CHECK(micro == 1);
        }
        
        SUBCASE("stream output") {
            version v(3, 2, 1);
            std::ostringstream oss;
            oss << v;
            CHECK(oss.str() == "3.2.1");
        }
    }
    
    TEST_CASE("version comparison") {
        version v1(3, 2, 1);
        version v2(3, 2, 1);
        version v3(3, 2, 0);
        version v4(3, 3, 0);
        version v5(4, 0, 0);
        
        SUBCASE("equality") {
            CHECK(v1 == v2);
            CHECK(v1 != v3);
            CHECK(v1 != v4);
        }
        
        SUBCASE("ordering") {
            CHECK(v3 < v1);
            CHECK(v1 > v3);
            CHECK(v1 <= v2);
            CHECK(v1 >= v2);
            CHECK(v4 > v1);
            CHECK(v5 > v4);
        }
        
        SUBCASE("at_least") {
            CHECK(v1.at_least(3, 2, 1));
            CHECK(v1.at_least(3, 2, 0));
            CHECK(v1.at_least(3, 0, 0));
            CHECK(!v1.at_least(3, 2, 2));
            CHECK(!v1.at_least(3, 3, 0));
            
            CHECK(v1.at_least(v3));
            CHECK(!v1.at_least(v4));
        }
    }
    
    TEST_CASE("version_info") {
        SUBCASE("compile-time version") {
            auto ct = version_info::compile_time;
            CHECK(ct.major() == SDL_MAJOR_VERSION);
            CHECK(ct.minor() == SDL_MINOR_VERSION);
            CHECK(ct.micro() == SDL_MICRO_VERSION);
            CHECK(ct.to_number() == SDL_VERSION);
        }
        
        SUBCASE("runtime version") {
            auto rt = version_info::runtime();
            // Runtime version should be valid
            CHECK(rt.major() >= 3);
            CHECK(rt.minor() >= 0);
            CHECK(rt.micro() >= 0);
            
            // Check consistency with SDL
            CHECK(rt.to_number() == SDL_GetVersion());
        }
        
        SUBCASE("revision") {
            auto rev = version_info::revision();
            // Revision might be empty, but should not crash
            bool valid = rev.empty() || rev.length() > 0;
            CHECK(valid);
        }
        
        SUBCASE("version matching") {
            // These might or might not match depending on build
            bool match = version_info::versions_match();
            bool at_least = version_info::runtime_at_least_compiled();
            
            // If versions match, runtime should be at least compiled
            if (match) {
                CHECK(at_least);
            }
            
            // Just verify the functions execute without crashing
            // The actual values depend on the build configuration
        }
    }
    
    TEST_CASE("compatibility") {
        SUBCASE("compile-time checks") {
            // These should compile without error
            constexpr bool has_3_0 = version_compat::compile_time_at_least<3, 0, 0>();
            CHECK(has_3_0);
            
            constexpr bool has_2_0 = version_compat::compile_time_at_least<2, 0, 0>();
            CHECK(has_2_0);
            
            // This would not compile if SDL < 3.0.0
            version_compat::require_compile_time<3, 0, 0>();
        }
        
        SUBCASE("runtime checks") {
            // Should have at least SDL 3.0.0
            CHECK(version_compat::runtime_at_least(3, 0, 0));
            
            // Should not have future version
            CHECK(!version_compat::runtime_at_least(99, 0, 0));
        }
        
        SUBCASE("compatibility report") {
            auto report = version_compat::report();
            CHECK(!report.empty());
            CHECK(report.find("Compiled against:") != std::string::npos);
            CHECK(report.find("Runtime version:") != std::string::npos);
            CHECK(report.find("Status:") != std::string::npos);
        }
    }
    
    TEST_CASE("feature detection") {
        SUBCASE("compile-time features") {
            // Properties and GPU were added in SDL 3.2.0
            if (version_info::compile_time.at_least(3, 2, 0)) {
                CHECK(version_info::features::has_properties);
                CHECK(version_info::features::has_gpu);
            }
        }
        
        SUBCASE("runtime features") {
            // Check if properties are available at runtime
            bool props_available = version_info::features::available_at_runtime(3, 2, 0);
            
            // If compile-time has it, runtime should too (unless older runtime)
            if (version_info::features::has_properties && 
                version_info::runtime_at_least_compiled()) {
                CHECK(props_available);
            }
        }
    }
    
    TEST_CASE("edge cases") {
        SUBCASE("large version numbers") {
            version v(999, 999, 999);
            CHECK(v.to_number() == 999999999);
            CHECK(v.to_string() == "999.999.999");
        }
        
        SUBCASE("zero versions") {
            version v(0, 0, 0);
            CHECK(v.to_number() == 0);
            CHECK(v.to_string() == "0.0.0");
            CHECK(!v.at_least(0, 0, 1));
        }
        
        SUBCASE("version arithmetic boundaries") {
            // Test version number overflow behavior
            version v1(1, 999, 999);
            version v2(2, 0, 0);
            CHECK(v2 > v1);
            CHECK(v2.to_number() > v1.to_number());
        }
    }
}