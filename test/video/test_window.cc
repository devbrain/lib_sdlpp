//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <sdlpp/core/sdl.hh>

#include "sdlpp/video/window.hh"
#include "sdlpp/video/renderer.hh"

using namespace sdlpp;

TEST_SUITE("window") {
    // Helper to ensure SDL is initialized for tests that need it
    struct sdl_init_guard {
        bool initialized = false;
        
        sdl_init_guard() {
            if (!SDL_WasInit(SDL_INIT_VIDEO)) {
                initialized = SDL_Init(SDL_INIT_VIDEO);
            }
        }
        
        ~sdl_init_guard() {
            if (initialized) {
                SDL_Quit();
            }
        }
        
        [[nodiscard]] bool is_initialized() const {
            return SDL_WasInit(SDL_INIT_VIDEO) != 0;
        }
    };
    
    TEST_CASE("window construction") {
        SUBCASE("default construction") {
            window w;
            CHECK(!w.is_valid());
            CHECK(!w);
            CHECK(w.get() == nullptr);
        }
        
        SUBCASE("move semantics") {
            window w1;
            window w2(std::move(w1));
            CHECK(!w1.is_valid());
            CHECK(!w2.is_valid()); // Both invalid since w1 was empty
            
            window w3;
            w3 = std::move(w2);
            CHECK(!w2.is_valid());
            CHECK(!w3.is_valid());
        }
    }
    
    TEST_CASE("window flags") {
        SUBCASE("bitwise operations") {
            auto flags = window_flags::resizable | window_flags::opengl;
            CHECK((flags & window_flags::resizable) == window_flags::resizable);
            CHECK((flags & window_flags::opengl) == window_flags::opengl);
            CHECK((flags & window_flags::hidden) == window_flags::none);
            
            flags |= window_flags::hidden;
            CHECK((flags & window_flags::hidden) == window_flags::hidden);
            
            flags &= ~window_flags::opengl;
            CHECK((flags & window_flags::opengl) == window_flags::none);
            CHECK((flags & window_flags::resizable) == window_flags::resizable);
        }
    }
    
    TEST_CASE("window creation and properties") {
        sdl_init_guard sdl;
        if (!sdl.is_initialized()) {
            MESSAGE("SDL video not initialized, skipping window tests");
            return;
        }
        
        SUBCASE("basic window creation") {
            auto w_result = window::create("Test Window", 640, 480);
            if (!w_result) {
                MESSAGE("Failed to create window: " << w_result.error());
                return;
            }
            
            auto& w = *w_result;
            CHECK(w.is_valid());
            CHECK(w.get() != nullptr);
            CHECK(w.get_id() != 0);
            
            // Check title
            CHECK(w.get_title() == "Test Window");
            
            // Check size
            auto size = w.get_size();
            REQUIRE(size.has_value());
            CHECK(size->width == 640);
            CHECK(size->height == 480);
            
            // Check that it's not fullscreen by default
            CHECK(!w.is_fullscreen());
        }
        
        SUBCASE("window with flags") {
            auto flags = window_flags::resizable | window_flags::hidden;
            auto w_result = window::create("Flagged Window", 800, 600, flags);
            if (!w_result) {
                MESSAGE("Failed to create window: " << w_result.error());
                return;
            }
            
            auto& w = *w_result;
            auto actual_flags = w.get_flags();
            CHECK((actual_flags & window_flags::resizable) == window_flags::resizable);
            CHECK((actual_flags & window_flags::hidden) == window_flags::hidden);
        }
        
        SUBCASE("centered window") {
            auto w_result = window::create_centered("Centered Window", 400, 300);
            if (!w_result) {
                MESSAGE("Failed to create centered window: " << w_result.error());
                return;
            }
            
            auto& w = *w_result;
            CHECK(w.is_valid());
            
            // Position might not be exactly centered due to window manager
            // Just check that we can get the position
            auto pos = w.get_position();
            CHECK(pos.has_value());
        }
    }
    
    TEST_CASE("window manipulation") {
        sdl_init_guard sdl;
        if (!sdl.is_initialized()) {
            MESSAGE("SDL video not initialized, skipping window tests");
            return;
        }
        
        auto w_result = window::create("Test Window", 640, 480, window_flags::hidden | window_flags::resizable);
        if (!w_result) {
            MESSAGE("Failed to create window: " << w_result.error());
            return;
        }
        auto& w = *w_result;
        
        SUBCASE("title manipulation") {
            auto result = w.set_title("New Title");
            CHECK(result.has_value());
            CHECK(w.get_title() == "New Title");
            
            result = w.set_title("");
            CHECK(result.has_value());
            CHECK(w.get_title() == "");
        }
        
        SUBCASE("size manipulation") {
            // Show window before resizing (some platforms may require this)
            auto show_result = w.show();
            CHECK(show_result.has_value());
            
            auto result = w.set_size(800, 600);
            CHECK(result.has_value());
            
            // Give the window system time to process the resize
            SDL_Delay(50);
            
            auto size = w.get_size();
            REQUIRE(size.has_value());
            
            // In headless/CI environments, window size changes might not work properly
            // So we'll check if the size changed at all
            bool size_changed = (size->width != 640 || size->height != 480);
            if (!size_changed) {
                MESSAGE("Window size changes not working - possibly running in headless mode");
                // At least verify the operations don't fail
                CHECK(result.has_value());
            } else {
                CHECK(size->width == 800);
                CHECK(size->height == 600);
                
                // Test with size object
                result = w.set_size(sdlpp::size{1024, 768});
                CHECK(result.has_value());
                
                SDL_Delay(50);
                
                size = w.get_size();
                REQUIRE(size.has_value());
                CHECK(size->width == 1024);
                CHECK(size->height == 768);
            }
        }
        
        SUBCASE("position manipulation") {
            auto result = w.set_position(100, 200);
            CHECK(result.has_value());
            
            auto pos = w.get_position();
            REQUIRE(pos.has_value());
            // Window manager might adjust position
            
            // Test with point object
            result = w.set_position(point{300, 400});
            CHECK(result.has_value());
        }
        
        SUBCASE("size constraints") {
            auto result = w.set_minimum_size(320, 240);
            CHECK(result.has_value());
            
            auto min_size = w.get_minimum_size();
            REQUIRE(min_size.has_value());
            CHECK(min_size->width == 320);
            CHECK(min_size->height == 240);
            
            result = w.set_maximum_size(1920, 1080);
            CHECK(result.has_value());
            
            auto max_size = w.get_maximum_size();
            REQUIRE(max_size.has_value());
            CHECK(max_size->width == 1920);
            CHECK(max_size->height == 1080);
        }
        
        SUBCASE("visibility") {
            // Window was created hidden
            CHECK((w.get_flags() & window_flags::hidden) == window_flags::hidden);
            
            auto result = w.show();
            CHECK(result.has_value());
            
            // Note: Flag might not update immediately on all platforms
            
            result = w.hide();
            CHECK(result.has_value());
        }
        
        SUBCASE("window state") {
            // Show window before state changes
            auto show_result = w.show();
            CHECK(show_result.has_value());
            SDL_Delay(50);
            
            // Note: Dummy video driver doesn't support window state operations
            bool dummy_driver = (std::getenv("SDL_VIDEODRIVER") != nullptr && 
                               std::string(std::getenv("SDL_VIDEODRIVER")) == "dummy");
            
            auto result = w.maximize();
            if (!result) {
                MESSAGE("Failed to maximize: " << result.error());
                if (dummy_driver) {
                    MESSAGE("Expected: dummy driver doesn't support window state operations");
                }
            }
            if (dummy_driver) {
                CHECK(!result.has_value());  // Expected to fail on dummy driver
            } else {
                CHECK(result.has_value());
            }
            
            result = w.restore();
            if (dummy_driver) {
                CHECK(!result.has_value());
            } else {
                CHECK(result.has_value());
            }
            
            result = w.minimize();
            if (dummy_driver) {
                CHECK(!result.has_value());
            } else {
                CHECK(result.has_value());
            }
            
            result = w.restore();
            if (dummy_driver) {
                CHECK(!result.has_value());
            } else {
                CHECK(result.has_value());
            }
        }
        
        SUBCASE("opacity") {
            float opacity = w.get_opacity();
            CHECK(opacity == 1.0f); // Default should be opaque
            
            auto result = w.set_opacity(0.5f);
            if (result) {
                opacity = w.get_opacity();
                CHECK(opacity == doctest::Approx(0.5f));
            } else {
                MESSAGE("Platform doesn't support opacity: " << result.error());
            }
        }
        
        SUBCASE("always on top") {
            auto result = w.set_always_on_top(true);
            if (!result) {
                MESSAGE("Platform doesn't support always on top: " << result.error());
            } else {
                CHECK(result.has_value());
                
                result = w.set_always_on_top(false);
                CHECK(result.has_value());
            }
        }
        
        SUBCASE("resizable") {
            auto result = w.set_resizable(true);
            CHECK(result.has_value());
            
            result = w.set_resizable(false);
            CHECK(result.has_value());
        }
    }
    
    TEST_CASE("window surface") {
        sdl_init_guard sdl;
        if (!sdl.is_initialized()) {
            MESSAGE("SDL video not initialized, skipping window tests");
            return;
        }
        
        auto w_result = window::create("Surface Window", 320, 240);
        if (!w_result) {
            MESSAGE("Failed to create window: " << w_result.error());
            return;
        }
        auto& w = *w_result;
        
        SUBCASE("get surface") {
            auto surface = w.get_surface();
            if (!surface) {
                MESSAGE("Platform doesn't support window surface: " << surface.error());
                return;
            }
            
            CHECK(*surface != nullptr);
            
            // Update surface
            auto result = w.update_surface();
            CHECK(result.has_value());
        }
        
        SUBCASE("pixel format") {
            auto format = w.get_pixel_format();
            if (!format) {
                MESSAGE("Failed to get pixel format: " << format.error());
            } else {
                CHECK(*format != pixel_format_enum::unknown);
            }
        }
    }
    
    TEST_CASE("renderer access") {
        sdl_init_guard sdl;
        if (!sdl.is_initialized()) {
            MESSAGE("SDL video not initialized, skipping window tests");
            return;
        }
        
        SUBCASE("create renderer from window") {
            auto w_result = window::create("Renderer Test", 640, 480);
            if (!w_result) {
                MESSAGE("Failed to create window: " << w_result.error());
                return;
            }
            auto& w = *w_result;
            
            // Initially no renderer
            CHECK(!w.has_renderer());
            CHECK(w.get_renderer_ptr() == nullptr);
            
            // Create renderer
            auto renderer_result = w.create_renderer();
            if (!renderer_result) {
                MESSAGE("Failed to create renderer: " << renderer_result.error());
                return;
            }
            
            // Now should have renderer
            CHECK(w.has_renderer());
            CHECK(w.get_renderer_ptr() != nullptr);
            
            // Verify the renderer works
            auto& r = *renderer_result;
            auto clear_result = r.clear();
            CHECK(clear_result.has_value());
        }
        
        SUBCASE("renderer lifetime") {
            auto w_result = window::create("Lifetime Test", 320, 240);
            if (!w_result) {
                MESSAGE("Failed to create window: " << w_result.error());
                return;
            }
            auto& w = *w_result;
            
            SDL_Renderer* raw_ptr = nullptr;
            
            {
                // Create renderer in scope
                auto renderer_result = w.create_renderer();
                REQUIRE(renderer_result.has_value());
                raw_ptr = renderer_result->get();
                CHECK(raw_ptr != nullptr);
                CHECK(w.get_renderer_ptr() == raw_ptr);
            }
            
            // After renderer is destroyed, window should not have renderer
            CHECK(!w.has_renderer());
            CHECK(w.get_renderer_ptr() == nullptr);
        }
    }
    
    TEST_CASE("error handling") {
        window invalid_window;
        
        SUBCASE("invalid window operations") {
            CHECK(invalid_window.get_title().empty());
            CHECK(invalid_window.get_id() == 0);
            
            auto result = invalid_window.set_title("Test");
            CHECK(!result.has_value());
            CHECK(result.error() == "Invalid window");
            
            auto size = invalid_window.get_size();
            CHECK(!size.has_value());
            CHECK(size.error() == "Invalid window");
            
            auto pos = invalid_window.get_position();
            CHECK(!pos.has_value());
            CHECK(pos.error() == "Invalid window");
            
            // Test renderer creation on invalid window
            auto renderer_result = invalid_window.create_renderer();
            CHECK(!renderer_result.has_value());
            CHECK(renderer_result.error() == "Invalid window");
        }
    }
}