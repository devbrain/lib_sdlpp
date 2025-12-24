#include <doctest/doctest.h>
#include <sdlpp/system/shared_object.hh>
#include <cmath>

// For testing, we'll try to load the math library which should be
// available on most systems
#ifdef _WIN32
    static const char* MATH_LIB = "msvcrt.dll";
    static const char* MATH_FUNC = "sqrt";
#elif defined(__APPLE__)
    static const char* MATH_LIB = "libm.dylib";
    static const char* MATH_FUNC = "sqrt";
#else
    static const char* MATH_LIB = "libm.so.6";
    static const char* MATH_FUNC = "sqrt";
#endif

TEST_SUITE("shared_object") {
    TEST_CASE("construction and destruction") {
        SUBCASE("default construction") {
            sdlpp::shared_object obj;
            CHECK_FALSE(obj.is_valid());
            CHECK_FALSE(static_cast<bool>(obj));
            CHECK(obj.get() == nullptr);
        }
        
        SUBCASE("move construction") {
            auto result = sdlpp::shared_object::load(MATH_LIB);
            if (result) {
                sdlpp::shared_object obj1 = std::move(result.value());
                CHECK(obj1.is_valid());
                
                sdlpp::shared_object obj2(std::move(obj1));
                CHECK(obj2.is_valid());
                CHECK_FALSE(obj1.is_valid());
            }
        }
        
        SUBCASE("move assignment") {
            auto result1 = sdlpp::shared_object::load(MATH_LIB);
            auto result2 = sdlpp::shared_object::load(MATH_LIB);
            
            if (result1 && result2) {
                sdlpp::shared_object obj1 = std::move(result1.value());
                sdlpp::shared_object obj2 = std::move(result2.value());
                
                CHECK(obj1.is_valid());
                CHECK(obj2.is_valid());
                
                obj2 = std::move(obj1);
                CHECK_FALSE(obj1.is_valid());
                CHECK(obj2.is_valid());
            }
        }
    }
    
    TEST_CASE("loading shared objects") {
        SUBCASE("load valid library") {
            auto result = sdlpp::shared_object::load(MATH_LIB);
            
            // Math library should be available on all platforms
            REQUIRE(result.has_value());
            CHECK(result.value().is_valid());
        }
        
        SUBCASE("load invalid library") {
            auto result = sdlpp::shared_object::load("this_library_does_not_exist.so");
            CHECK_FALSE(result.has_value());
            CHECK(!result.error().empty());
        }
        
        SUBCASE("convenience function") {
            auto result = sdlpp::load_shared_object(MATH_LIB);
            if (result) {
                CHECK(result.value().is_valid());
            }
        }
    }
    
    TEST_CASE("symbol resolution") {
        auto lib_result = sdlpp::shared_object::load(MATH_LIB);
        
        // Skip these tests if we can't load the math library
        if (!lib_result) {
            return;
        }
        
        auto& lib = lib_result.value();
        
        SUBCASE("get function pointer") {
            // Try to get sqrt function
            using sqrt_func = double(double);
            auto func_result = lib.get_function<sqrt_func>(MATH_FUNC);
            
            if (func_result) {
                auto sqrt_ptr = func_result.value();
                CHECK(sqrt_ptr != nullptr);
                
                // Test the function
                double result = sqrt_ptr(4.0);
                CHECK(result == doctest::Approx(2.0));
                
                result = sqrt_ptr(9.0);
                CHECK(result == doctest::Approx(3.0));
            }
        }
        
        SUBCASE("get non-existent symbol") {
            using fake_func = void();
            auto func_result = lib.get_function<fake_func>("this_function_does_not_exist");
            CHECK_FALSE(func_result.has_value());
            CHECK(!func_result.error().empty());
        }
        
        SUBCASE("has_symbol") {
            // sqrt should exist in math library
            bool has_sqrt = lib.has_symbol(MATH_FUNC);
            CHECK(has_sqrt);
            
            // This should not exist
            bool has_fake = lib.has_symbol("this_function_does_not_exist");
            CHECK_FALSE(has_fake);
        }
        
        SUBCASE("get raw symbol") {
            auto symbol_result = lib.get_symbol(MATH_FUNC);
            if (symbol_result) {
                CHECK(symbol_result.value() != nullptr);
            }
        }
    }
    
    TEST_CASE("error handling") {
        SUBCASE("operations on invalid object") {
            sdlpp::shared_object obj;
            
            using func_type = void();
            auto func_result = obj.get_function<func_type>("any_name");
            CHECK_FALSE(func_result.has_value());
            CHECK(func_result.error() == "Shared object not loaded");
            
            auto data_result = obj.get_data<int>("any_name");
            CHECK_FALSE(data_result.has_value());
            CHECK(data_result.error() == "Shared object not loaded");
            
            auto symbol_result = obj.get_symbol("any_name");
            CHECK_FALSE(symbol_result.has_value());
            CHECK(symbol_result.error() == "Shared object not loaded");
        }
    }
    
    TEST_CASE("reset and release") {
        SUBCASE("reset") {
            auto result = sdlpp::shared_object::load(MATH_LIB);
            if (result) {
                auto& obj = result.value();
                CHECK(obj.is_valid());
                
                obj.reset();
                CHECK_FALSE(obj.is_valid());
                
                // Reset on already invalid object should be safe
                obj.reset();
                CHECK_FALSE(obj.is_valid());
            }
        }
        
        SUBCASE("release") {
            auto result = sdlpp::shared_object::load(MATH_LIB);
            if (result) {
                auto& obj = result.value();
                CHECK(obj.is_valid());
                
                SDL_SharedObject* handle = obj.release();
                CHECK(handle != nullptr);
                CHECK_FALSE(obj.is_valid());
                CHECK(obj.get() == nullptr);
                
                // Manual cleanup
                SDL_UnloadObject(handle);
            }
        }
    }
    
    TEST_CASE("symbol resolver") {
        // Define a test API structure
        struct TestAPI : sdlpp::symbol_resolver<TestAPI> {
            using sqrt_func = double(double);
            using cos_func = double(double);
            
            sqrt_func* sqrt_fn = nullptr;
            cos_func* cos_fn = nullptr;
            
            static constexpr auto symbols() {
                return std::make_tuple(
                    bind("sqrt", &TestAPI::sqrt_fn),
                    bind("cos", &TestAPI::cos_fn)
                );
            }
        };
        
        SUBCASE("load multiple symbols") {
            auto lib_result = sdlpp::shared_object::load(MATH_LIB);
            if (!lib_result) {
                return;
            }
            
            TestAPI api;
            auto load_result = api.load_from(lib_result.value());
            
            if (load_result) {
                CHECK(api.sqrt_fn != nullptr);
                CHECK(api.cos_fn != nullptr);
                
                // Test the functions
                if (api.sqrt_fn && api.cos_fn) {
                    CHECK(api.sqrt_fn(16.0) == doctest::Approx(4.0));
                    CHECK(api.cos_fn(0.0) == doctest::Approx(1.0));
                }
            }
        }
        
        SUBCASE("fail on missing symbol") {
            struct BadAPI : sdlpp::symbol_resolver<BadAPI> {
                using fake_func = void();
                fake_func* fake_fn = nullptr;
                
                static constexpr auto symbols() {
                    return std::make_tuple(
                        bind("this_does_not_exist", &BadAPI::fake_fn)
                    );
                }
            };
            
            auto lib_result = sdlpp::shared_object::load(MATH_LIB);
            if (!lib_result) {
                return;
            }
            
            BadAPI api;
            auto load_result = api.load_from(lib_result.value());
            CHECK_FALSE(load_result.has_value());
            CHECK(load_result.error().find("this_does_not_exist") != std::string::npos);
        }
    }
}