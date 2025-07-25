#include <doctest/doctest.h>
#include <sdlpp/ui/message_box.hh>
#include <sdlpp/core/core.hh>

TEST_SUITE("message_box") {
    // Note: Message boxes require user interaction, so we can only test
    // the API construction and basic validation
    
    TEST_CASE("message_box_button construction") {
        sdlpp::message_box_button button;
        button.id = 42;
        button.text = "Click Me";
        button.flags = sdlpp::message_box_button_flags::return_key_default;
        
        auto sdl_button = button.to_sdl();
        CHECK(sdl_button.buttonID == 42);
        CHECK(sdl_button.text != nullptr);
        CHECK(sdl_button.flags == SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT);
    }
    
    TEST_CASE("message_box_color construction") {
        sdlpp::message_box_color color{255, 128, 64};
        
        auto sdl_color = color.to_sdl();
        CHECK(sdl_color.r == 255);
        CHECK(sdl_color.g == 128);
        CHECK(sdl_color.b == 64);
    }
    
    TEST_CASE("message_box_color_scheme") {
        sdlpp::message_box_color_scheme scheme;
        
        // Set colors
        scheme.set_color(sdlpp::message_box_color_type::background, {32, 32, 32})
              .set_color(sdlpp::message_box_color_type::text, {255, 255, 255});
        
        // Get colors
        auto bg = scheme.get_color(sdlpp::message_box_color_type::background);
        CHECK(bg.r == 32);
        CHECK(bg.g == 32);
        CHECK(bg.b == 32);
        
        auto text = scheme.get_color(sdlpp::message_box_color_type::text);
        CHECK(text.r == 255);
        CHECK(text.g == 255);
        CHECK(text.b == 255);
        
        // Convert to SDL
        auto sdl_scheme = scheme.to_sdl();
        CHECK(sdl_scheme.colors[SDL_MESSAGEBOX_COLOR_BACKGROUND].r == 32);
        CHECK(sdl_scheme.colors[SDL_MESSAGEBOX_COLOR_TEXT].r == 255);
    }
    
    TEST_CASE("message_box_builder") {
        // We can't actually show the message box in tests, but we can
        // verify the builder pattern works
        sdlpp::message_box_builder builder;
        
        builder.set_title("Test Title")
               .set_message("Test Message")
               .set_type(sdlpp::message_box_flags::warning)
               .add_button(1, "OK", true)
               .add_button(0, "Cancel", false, true);
        
        // Can't test show() without user interaction
        
        // Test with custom button
        sdlpp::message_box_button custom_button;
        custom_button.id = 2;
        custom_button.text = "Custom";
        builder.add_button(custom_button);
        
        // Test with color scheme
        sdlpp::message_box_color_scheme dark_theme;
        dark_theme.set_color(sdlpp::message_box_color_type::background, {0, 0, 0});
        builder.set_color_scheme(dark_theme);
    }
}