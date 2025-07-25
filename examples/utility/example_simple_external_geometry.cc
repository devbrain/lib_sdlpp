//
// Simple example: Drop-in replacement of geometry types
//

#include <iostream>
#include <thread>
#include <chrono>
#include <sdlpp/core/core.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>

// Simulate using GLM library
namespace glm {
    struct vec2 {
        using value_type = float;
        float x, y;
    };
    
    struct ivec2 {
        using value_type = int;  
        int x, y;
    };
}

// Custom game types
struct GameSize {
    using value_type = int;
    int width, height;
    
    int area() const { return width * height; }
    bool empty() const { return width <= 0 || height <= 0; }
};

struct GameRect {
    using value_type = float;
    float x, y, w, h;
    
    float area() const { return w * h; }
    bool empty() const { return w <= 0 || h <= 0; }
};

int main() {
    try {
        sdlpp::init init(sdlpp::init_flags::video);
    
    // BEFORE: Using SDL++ built-in types
    // auto window = sdlpp::window::create("Test", sdlpp::size{800, 600});
    // window->set_position(sdlpp::point{100, 100});
    
    // AFTER: Using external library types - works without any code changes!
    GameSize window_size{800, 600};
    auto window = sdlpp::window::create("Test", static_cast<int>(window_size.width), static_cast<int>(window_size.height));
    
    if (window) {
        glm::ivec2 pos{100, 100};
        [[maybe_unused]] auto pos_result = window->set_position(pos);
        
        auto renderer = sdlpp::renderer::create(*window);
        if (renderer) {
            // Mix and match different library types freely
            glm::vec2 p1{10.5f, 20.5f};
            glm::vec2 p2{100.5f, 200.5f};
            [[maybe_unused]] auto line_result = renderer->draw_line(p1, p2);
            
            GameRect rect{50, 50, 100, 100};
            [[maybe_unused]] auto rect_result = renderer->fill_rect(rect);
        }
    }
    
    std::cout << "✅ External geometry types work seamlessly!" << std::endl;
    std::cout << "No changes needed to existing code when switching libraries!" << std::endl;
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}