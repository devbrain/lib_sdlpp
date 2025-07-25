//
// Example: SDL++ Hints System Usage
//

#include <iostream>
#include <sdlpp/core/core.hh>
#include "sdlpp/config/hints.hh"
#include "sdlpp/video/window.hh"
#include "sdlpp/video/renderer.hh"

using namespace sdlpp;

// Example 1: Basic hint usage
void example_basic_hints() {
    std::cout << "\n=== Basic Hints Example ===" << std::endl;
    
    // Set video driver hint before initializing SDL
    (void)hint_manager::set(hints::video_driver, "dummy");
    std::cout << "Video driver hint set to: " 
              << hint_manager::get(hints::video_driver).value_or("(not set)") << std::endl;
    
    // Set vsync preference
    hint_utils::set_vsync(true);
    std::cout << "VSync enabled: " << hint_manager::get_boolean(hints::render_vsync) << std::endl;
    
    // Check if a hint is set
    if (hint_manager::is_set(std::string(hints::render_vsync))) {
        std::cout << "Render vsync hint is configured" << std::endl;
    }
    
    // Get hint with default value
    auto audio_driver = hint_manager::get_or(hints::audio_driver, "default_audio");
    std::cout << "Audio driver: " << audio_driver << std::endl;
}

// Example 2: Scoped hints for temporary configuration
void example_scoped_hints() {
    std::cout << "\n=== Scoped Hints Example ===" << std::endl;
    
    // Original app name
    hint_utils::set_app_name("My App");
    std::cout << "Original app name: " 
              << hint_manager::get(hints::app_name).value() << std::endl;
    
    {
        // Temporarily change app name for a specific operation
        auto scoped = hint_manager::set_scoped(hints::app_name, "My App - Special Mode");
        std::cout << "Temporary app name: " 
                  << hint_manager::get(hints::app_name).value() << std::endl;
        
        // Do some work with different app name...
    }
    
    // Automatically restored
    std::cout << "Restored app name: " 
              << hint_manager::get(hints::app_name).value() << std::endl;
}

// Example 3: Hint callbacks for monitoring changes
void example_hint_callbacks() {
    std::cout << "\n=== Hint Callbacks Example ===" << std::endl;
    
    // Monitor vsync changes
    auto vsync_monitor = hint_manager::add_callback(hints::render_vsync,
        [](const std::string&,
           const std::optional<std::string>& old_value,
           const std::optional<std::string>& new_value) {
            std::cout << "VSync hint changed:" << std::endl;
            std::cout << "  Old value: " << old_value.value_or("(not set)") << std::endl;
            std::cout << "  New value: " << new_value.value_or("(not set)") << std::endl;
        });
    
    // Trigger callbacks
    hint_utils::set_vsync(true);
    hint_utils::set_vsync(false);
    [[maybe_unused]] auto reset_result = hint_manager::reset(hints::render_vsync);
}

// Example 4: Platform-specific hints
void example_platform_hints() {
    std::cout << "\n=== Platform-Specific Hints Example ===" << std::endl;
    
#ifdef __APPLE__
    // macOS-specific hints
    hint_manager::set_boolean(hints::mac_ctrl_click_emulate_right_click, true);
    std::cout << "macOS Ctrl+Click right-click emulation enabled" << std::endl;
#else
    std::cout << "No platform-specific hints set for this platform" << std::endl;
#endif
}

// Example 5: Configure SDL for specific use cases
void example_game_configuration() {
    std::cout << "\n=== Game Configuration Example ===" << std::endl;
    
    // Configuration for a game
    std::unordered_map<std::string, std::string> game_hints = {
        {std::string(hints::render_vsync), "1"},                          // Enable vsync
        {std::string(hints::joystick_allow_background_events), "1"},      // Allow controller input in background
        {std::string(hints::timer_resolution), "1"},                      // 1ms timer precision
        {std::string(hints::app_name), "My Game"},                        // Set app name
        {std::string(hints::mouse_focus_clickthrough), "0"}              // Require click to focus
    };
    
    size_t set_count = hint_manager::set_multiple(game_hints, hint_priority::normal);
    std::cout << "Set " << set_count << " game configuration hints" << std::endl;
}

// Example 6: Debug/development hints
void example_debug_hints() {
    std::cout << "\n=== Debug Hints Example ===" << std::endl;
    
    // Enable debug features during development
    bool debug_mode = true;
    
    if (debug_mode) {
        // Use scoped hints so they're automatically disabled when done debugging
        auto debug_scope1 = hint_manager::set_scoped("SDL_HINT_DEBUG_EVENTS", "1");
        auto debug_scope2 = hint_manager::set_scoped("SDL_HINT_RENDER_DEBUG", "1");
        
        std::cout << "Debug hints enabled for this scope" << std::endl;
        
        // Debug work happens here...
    }
    
    std::cout << "Debug hints automatically disabled" << std::endl;
}

// Example 7: Hints with SDL initialization
void example_init_with_hints() {
    std::cout << "\n=== SDL Initialization with Hints ===" << std::endl;
    
    // Configure SDL before initialization
    [[maybe_unused]] auto video_result = hint_manager::set(hints::video_driver, "dummy");
    [[maybe_unused]] auto audio_result = hint_manager::set(hints::audio_driver, "dummy");
    hint_utils::set_vsync(false);
    
    // Initialize SDL
    try {
        sdlpp::init init(sdlpp::init_flags::video | sdlpp::init_flags::audio);
        std::cout << "SDL initialized with custom hints" << std::endl;
        
        // Create window and renderer to test hints
        auto window = sdlpp::window::create("Hints Test", 800, 600);
        if (window) {
            auto renderer = sdlpp::renderer::create(*window);
            if (renderer) {
                // Check if vsync is actually disabled
                auto vsync_enabled = hint_manager::get_boolean(hints::render_vsync);
                std::cout << "VSync is " << (vsync_enabled ? "enabled" : "disabled") << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize SDL: " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "=== SDL++ Hints System Examples ===" << std::endl;
    
    // Run examples
    example_basic_hints();
    example_scoped_hints();
    example_hint_callbacks();
    example_platform_hints();
    example_game_configuration();
    example_debug_hints();
    example_init_with_hints();
    
    std::cout << "\n✅ All hint examples completed!" << std::endl;
    
    return 0;
}