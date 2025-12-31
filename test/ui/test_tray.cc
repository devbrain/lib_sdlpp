#include <doctest/doctest.h>
#include <sdlpp/ui/tray.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/video/display.hh>
#include <sdlpp/core/core.hh>
#include <string_view>

TEST_SUITE("tray") {
    // Note: System tray functionality requires a running desktop environment
    // and user interaction, so tests are limited to API validation
    
    TEST_CASE("tray_entry_flags") {
        auto none = sdlpp::tray_entry_flags::none;
        auto checked = sdlpp::tray_entry_flags::checked;
        auto disabled = sdlpp::tray_entry_flags::disabled;
        
        CHECK(static_cast<Uint32>(none) == 0);
        CHECK(static_cast<Uint32>(checked) == SDL_TRAYENTRY_CHECKED);
        CHECK(static_cast<Uint32>(disabled) == SDL_TRAYENTRY_DISABLED);
    }
    
    TEST_CASE("tray_entry wrapper") {
        // Can't create a real entry without a tray/menu
        // Just test that the type exists and methods are accessible
        sdlpp::tray_entry entry;
        CHECK_FALSE(entry.is_valid());
        CHECK(entry.get() == nullptr);
        
        // Test methods would return nullopt/errors for invalid entry
        CHECK_FALSE(entry.get_label().has_value());
        CHECK_FALSE(entry.is_checked());
        CHECK_FALSE(entry.is_enabled());
        
        auto label_result = entry.set_label("Test");
        CHECK_FALSE(label_result.has_value());
        
        auto check_result = entry.set_checked(true);
        CHECK_FALSE(check_result.has_value());
        
        auto enable_result = entry.set_enabled(false);
        CHECK_FALSE(enable_result.has_value());
    }
    
    TEST_CASE("tray_menu") {
        SUBCASE("default construction") {
            sdlpp::tray_menu menu;
            CHECK_FALSE(menu.is_valid());
            CHECK(menu.get() == nullptr);
            CHECK(menu.get_entries().empty());
        }
        
        SUBCASE("move semantics") {
            sdlpp::tray_menu menu1;
            sdlpp::tray_menu menu2(std::move(menu1));
            
            CHECK_FALSE(menu1.is_valid());
            CHECK_FALSE(menu2.is_valid());  // Both invalid since menu1 was already invalid
            
            menu2 = std::move(menu1);
            CHECK_FALSE(menu2.is_valid());
        }
    }
    
    TEST_CASE("tray") {
        SUBCASE("default construction") {
            sdlpp::tray tray;
            CHECK_FALSE(tray.is_valid());
            CHECK_FALSE(static_cast<bool>(tray));
            CHECK(tray.get() == nullptr);
        }
        
        SUBCASE("move semantics") {
            sdlpp::tray tray1;
            sdlpp::tray tray2(std::move(tray1));
            
            CHECK_FALSE(tray1.is_valid());
            CHECK_FALSE(tray2.is_valid());  // Both invalid since tray1 was already invalid
            
            tray2 = std::move(tray1);
            CHECK_FALSE(tray2.is_valid());
        }
        
        SUBCASE("creation would require surface") {
            sdlpp::init* init_guard = nullptr;
            try {
                static sdlpp::init init(sdlpp::init_flags::video);
                init_guard = &init;
            } catch (const std::exception& ex) {
                MESSAGE("Skipping tray test: SDL video init failed: " << ex.what());
                return;
            }

            const char* driver = SDL_GetCurrentVideoDriver();
            if (!driver || std::string_view(driver) == "dummy") {
                MESSAGE("Skipping tray test with dummy video driver");
                return;
            }

            if (sdlpp::display_manager::get_display_count() == 0) {
                MESSAGE("Skipping tray test with no video displays");
                return;
            }
            (void)init_guard;

            // Create a small test surface
            auto surface_result = sdlpp::surface::create_rgb(16, 16, sdlpp::pixel_format_enum::RGBA8888);
            if (surface_result) {
                auto& surface = surface_result.value();

                // Attempt to create tray (may fail on headless systems)
                auto tray_result = sdlpp::tray::create(surface, "Test Tray");

                // We can't assert success since it depends on the environment
                // Just verify the API is callable
                if (tray_result) {
                    auto& tray = tray_result.value();
                    CHECK(tray.is_valid());

                    // Test setting icon/tooltip
                    auto icon_result = tray.set_icon(surface);
                    auto tooltip_result = tray.set_tooltip("New Tooltip");

                    // Get menu
                    [[maybe_unused]] auto& menu = tray.get_menu();
                    // Menu operations would go here
                }
            }
        }
    }
    
    TEST_CASE("update_trays") {
        // Just verify the function is callable
        sdlpp::update_trays();
    }
}
