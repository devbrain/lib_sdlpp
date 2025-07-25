//
// Example: Window with Renderer Access
//

#include <iostream>
#include <chrono>
#include <thread>
#include "sdlpp/core/core.hh"
#include "sdlpp/video/window.hh"
#include "sdlpp/video/renderer.hh"
#include "sdlpp/video/color.hh"
#include "sdlpp/utility/geometry.hh"
#include "sdlpp/core/error.hh"
#include "sdlpp/events/events.hh"

int main() {
    // Initialize SDL
    try {
        sdlpp::init init(sdlpp::init_flags::video);
        
        if (!init.is_initialized()) {
            std::cerr << "Failed to initialize SDL" << std::endl;
            return 1;
        }
    
    // Create window
    auto window_result = sdlpp::window::create("Renderer Access Example", 800, 600,
        sdlpp::window_flags::resizable);
    
    if (!window_result) {
        std::cerr << "Failed to create window: " << window_result.error() << std::endl;
        return 1;
    }
    
    auto& window = *window_result;
    
    // Method 1: Create renderer through window
    auto renderer_result = window.create_renderer();
    if (!renderer_result) {
        std::cerr << "Failed to create renderer: " << renderer_result.error() << std::endl;
        return 1;
    }
    
    auto& renderer = *renderer_result;
    
    // Verify we can access the renderer through the window
    if (window.has_renderer()) {
        std::cout << "Window has a renderer!" << std::endl;
        
        // Get raw pointer for advanced operations
        auto raw_ptr = window.get_renderer_ptr();
        std::cout << "Raw renderer pointer: " << raw_ptr << std::endl;
        std::cout << "Matches our renderer: " << (raw_ptr == renderer.get() ? "Yes" : "No") << std::endl;
    }
    
    // Simple render loop
    bool running = true;
    auto& event_queue = sdlpp::get_event_queue();
    
    while (running) {
        // Handle events
        while (auto event_opt = event_queue.poll()) {
            auto& event = *event_opt;
            if (event.type() == sdlpp::event_type::quit) {
                running = false;
            }
        }
        
        // Clear screen with blue
        [[maybe_unused]] auto _ = renderer.set_draw_color(sdlpp::color{0, 100, 200, 255});
        [[maybe_unused]] auto _2 = renderer.clear();
        
        // Draw a red rectangle
        [[maybe_unused]] auto _3 = renderer.set_draw_color(sdlpp::color{255, 0, 0, 255});
        [[maybe_unused]] auto _4 = renderer.fill_rect(sdlpp::rect_i{100, 100, 200, 150});
        
        // Present
        [[maybe_unused]] auto _5 = renderer.present();
        
        // ~60 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    // Renderer is automatically destroyed when it goes out of scope
    // After this, window.has_renderer() would return false
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}