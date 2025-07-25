//
// Simple hints test to verify basic functionality
//

#include <doctest/doctest.h>
#include "sdlpp/config/hints.hh"

using namespace sdlpp;

TEST_SUITE("hints_simple") {
    TEST_CASE("basic hint operations work") {
        // Test with a real SDL hint
        std::string hint_name(sdlpp::hints::app_name);
        
        // Set and get
        CHECK(hint_manager::set(hint_name, "Test App"));
        auto value = hint_manager::get(hint_name);
        CHECK(value.has_value());
        CHECK(value.value() == "Test App");
        
        // Reset
        CHECK(hint_manager::reset(hint_name));
        CHECK(!hint_manager::is_set(hint_name));
    }
    
    TEST_CASE("scoped hints work") {
        std::string hint_name(sdlpp::hints::render_vsync);
        
        // Set initial value
        [[maybe_unused]] auto result = hint_manager::set(hint_name, "0");
        
        {
            auto scoped = hint_manager::set_scoped(hint_name, "1");
            CHECK(hint_manager::get(hint_name).value() == "1");
        }
        
        // Should be restored
        CHECK(hint_manager::get(hint_name).value() == "0");
    }
    

}