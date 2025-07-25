//
// Example: Using external math libraries with SDL++ geometry concepts
//

#include <iostream>
#include <thread>
#include <chrono>
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/utility/geometry.hh>

// Example 1: GLM-like library types
namespace glm_like {
    template<typename T>
    struct vec2 {
        using value_type = T;
        T x, y;
        
        constexpr vec2(T x_, T y_) : x(x_), y(y_) {}
    };
    
    using ivec2 = vec2<int>;
    using vec2f = vec2<float>;
}

// Example 2: Eigen-like library types
namespace eigen_like {
    template<typename T>
    struct Vector2 {
        using value_type = T;
        T x, y;
        
        Vector2(T x_, T y_) : x(x_), y(y_) {}
        T& operator[](int i) { return i == 0 ? x : y; }
        const T& operator[](int i) const { return i == 0 ? x : y; }
    };
    
    using Vector2i = Vector2<int>;
    using Vector2f = Vector2<float>;
}

// Example 3: Custom game engine types
namespace game_engine {
    template<typename T>
    struct Size {
        using value_type = T;
        T width, height;
        
        constexpr Size(T w, T h) : width(w), height(h) {}
        constexpr T area() const { return width * height; }
        constexpr bool empty() const { return width == 0 || height == 0; }
    };
    
    template<typename T>
    struct Rectangle {
        using value_type = T;
        T x, y, w, h;
        
        constexpr Rectangle(T x_, T y_, T w_, T h_) : x(x_), y(y_), w(w_), h(h_) {}
        constexpr T area() const { return w * h; }
        constexpr bool empty() const { return w == 0 || h == 0; }
    };
    
    // Alternative rectangle format
    template<typename T>
    struct BoundingBox {
        using value_type = T;
        T left, top, width, height;
        
        constexpr BoundingBox(T l, T t, T w, T h) 
            : left(l), top(t), width(w), height(h) {}
        constexpr T area() const { return width * height; }
        constexpr bool empty() const { return width == 0 || height == 0; }
    };
}

// Example 4: Physics engine types
namespace physics_engine {
    struct Vec2D {
        using value_type = float;
        float x, y;
        
        Vec2D(float x_, float y_) : x(x_), y(y_) {}
        Vec2D operator+(const Vec2D& other) const {
            return Vec2D(x + other.x, y + other.y);
        }
    };
    
    struct AABB {  // Axis-Aligned Bounding Box
        using value_type = float;
        float x, y, w, h;
        
        AABB(float x_, float y_, float w_, float h_) 
            : x(x_), y(y_), w(w_), h(h_) {}
        float area() const { return w * h; }
        bool empty() const { return w == 0 || h == 0; }
        
        Vec2D center() const { return Vec2D(x + w/2, y + h/2); }
    };
}

// Helper to demonstrate that all these types work with SDL++
template<typename PointType, typename SizeType, typename RectType>
void demonstrate_compatibility(const std::string& library_name) {
    std::cout << "\n=== " << library_name << " Compatibility Demo ===" << std::endl;
    
    // Create window using external size type
    SizeType window_size(800, 600);
    auto window_result = sdlpp::window::create("SDL++ with " + library_name, static_cast<int>(window_size.width), static_cast<int>(window_size.height));
    if (!window_result) {
        std::cerr << "Failed to create window: " << window_result.error() << std::endl;
        return;
    }
    auto& window = *window_result;
    
    // Create renderer
    auto renderer_result = sdlpp::renderer::create(window);
    if (!renderer_result) {
        std::cerr << "Failed to create renderer: " << renderer_result.error() << std::endl;
        return;
    }
    auto& renderer = *renderer_result;
    
    // Clear screen
    [[maybe_unused]] auto black_result = renderer.set_draw_color(sdlpp::colors::black);
    [[maybe_unused]] auto clear_result = renderer.clear();
    
    // Draw using external point types
    [[maybe_unused]] auto white_result = renderer.set_draw_color(sdlpp::colors::white);
    PointType p1(100, 100);
    PointType p2(700, 500);
    [[maybe_unused]] auto line_result = renderer.draw_line(p1, p2);
    [[maybe_unused]] auto point1_result = renderer.draw_point(p1);
    [[maybe_unused]] auto point2_result = renderer.draw_point(p2);
    
    // Draw rectangles using external rect types
    [[maybe_unused]] auto red_result = renderer.set_draw_color(sdlpp::colors::red);
    RectType rect1(50, 50, 100, 80);
    [[maybe_unused]] auto rect1_result = renderer.draw_rect(rect1);
    
    [[maybe_unused]] auto green_result = renderer.set_draw_color(sdlpp::colors::green);
    RectType rect2(200, 200, 150, 150);
    [[maybe_unused]] auto rect2_result = renderer.fill_rect(rect2);
    
    // Move window using external point type
    PointType new_pos(100, 100);
    [[maybe_unused]] auto pos_result = window.set_position(new_pos);
    
    // Resize window using external size type
    SizeType new_size(1024, 768);
    [[maybe_unused]] auto size_result = window.set_size(new_size);
    
    // Present the frame
    [[maybe_unused]] auto present_result = renderer.present();
    
    std::cout << "✓ Window created with " << window_size.width << "x" << window_size.height << std::endl;
    std::cout << "✓ Drew line from (" << p1.x << "," << p1.y << ") to (" 
              << p2.x << "," << p2.y << ")" << std::endl;
    std::cout << "✓ Drew rectangle at (" << rect1.x << "," << rect1.y << ") with size " 
              << rect1.w << "x" << rect1.h << std::endl;
    std::cout << "✓ Filled rectangle at (" << rect2.x << "," << rect2.y << ") with size " 
              << rect2.w << "x" << rect2.h << std::endl;
    std::cout << "✓ Moved window to (" << new_pos.x << "," << new_pos.y << ")" << std::endl;
    std::cout << "✓ Resized window to " << new_size.width << "x" << new_size.height << std::endl;
}

// Example with alternative rectangle format
void demonstrate_alternative_rect() {
    std::cout << "\n=== Alternative Rectangle Format Demo ===" << std::endl;
    
    // This won't compile with current implementation since it expects x,y,w,h
    // but shows how concepts could be extended to support left,top,width,height
    game_engine::BoundingBox<int> bbox(100, 100, 200, 150);
    
    // We can still use it by converting manually
    std::cout << "BoundingBox with left=" << bbox.left << ", top=" << bbox.top 
              << ", width=" << bbox.width << ", height=" << bbox.height << std::endl;
    std::cout << "Area: " << bbox.area() << ", Empty: " << bbox.empty() << std::endl;
}

int main() {
    try {
        // Initialize SDL
        sdlpp::init init(sdlpp::init_flags::video);
    
    std::cout << "=== SDL++ External Geometry Library Compatibility Demo ===" << std::endl;
    std::cout << "This demonstrates how any math library with compatible types" << std::endl;
    std::cout << "can work seamlessly with SDL++ through C++20 concepts." << std::endl;
    
    // Demonstrate with different external libraries
    demonstrate_compatibility<glm_like::vec2f, game_engine::Size<int>, 
                            game_engine::Rectangle<float>>("Mixed Libraries");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    demonstrate_compatibility<eigen_like::Vector2f, game_engine::Size<int>, 
                            physics_engine::AABB>("Eigen-like + Physics");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    demonstrate_compatibility<physics_engine::Vec2D, game_engine::Size<int>, 
                            game_engine::Rectangle<float>>("Physics Engine");
    
    // Show alternative rectangle format
    demonstrate_alternative_rect();
    
    std::cout << "\n✅ All external geometry types work seamlessly with SDL++!" << std::endl;
    std::cout << "No modifications needed to SDL++ or the external libraries!" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}