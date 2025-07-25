//
// Example: Type-Safe Dimensions
//

#include <iostream>
#include <sdlpp/core/core.hh>
#include "sdlpp/utility/dimension.hh"
#include "sdlpp/video/window.hh"
#include "sdlpp/video/surface.hh"
#include "sdlpp/core/error.hh"

int main() {
    try {
        // Initialize SDL
        sdlpp::init init(sdlpp::init_flags::video);
        
        if (!init.is_initialized()) {
            std::cerr << "Failed to initialize SDL" << std::endl;
            return 1;
        }
    
    // Example 1: Type-safe window creation
    {
        std::cout << "=== Type-Safe Window Creation ===" << std::endl;
        
        // Old way - could pass negative dimensions by accident
        // auto window = sdlpp::window::create("Test", -100, -200);  // Would fail at runtime
        
        // New way - type safe
        sdlpp::window_dimensions dims(800, 600);
        auto window = sdlpp::window::create("Type-Safe Window", dims);
        
        if (window) {
            std::cout << "Created window with dimensions: " 
                      << dims.width.value() << "x" << dims.height.value() << std::endl;
        }
        
        // Negative dimensions are automatically handled
        sdlpp::window_dimensions bad_dims(-100, -200);
        std::cout << "Negative dimensions (-100, -200) become: "
                  << bad_dims.width.value() << "x" << bad_dims.height.value() << std::endl;
    }
    
    // Example 2: Semantic difference between position and size
    {
        std::cout << "\n=== Position vs Size Semantics ===" << std::endl;
        
        // Positions can be negative (off-screen)
        sdlpp::window_position pos(-50, -100);
        std::cout << "Window position (can be negative): " 
                  << pos.x.value << ", " << pos.y.value << std::endl;
        
        // Sizes cannot be negative
        sdlpp::window_dimensions size(-50, -100);
        std::cout << "Window size (clamped to non-negative): "
                  << size.width.value() << "x" << size.height.value() << std::endl;
    }
    
    // Example 3: Overflow protection
    {
        std::cout << "\n=== Overflow Protection ===" << std::endl;
        
        // Large dimensions that would overflow
        sdlpp::dimensions<int> large(100000, 100000);
        auto area = large.area();
        
        std::cout << "Area of 100000x100000: " << area << std::endl;
        std::cout << "Type of area: " << typeid(area).name() << " (larger than int)" << std::endl;
        
        // Addition overflow protection
        sdlpp::dimension<int> d1(std::numeric_limits<int>::max() - 10);
        sdlpp::dimension<int> d2(20);
        auto sum = d1 + d2;
        
        std::cout << "MAX-10 + 20 = " << sum.value() 
                  << " (clamped to MAX: " << std::numeric_limits<int>::max() << ")" << std::endl;
    }
    
    // Example 4: Surface creation with type safety
    {
        std::cout << "\n=== Type-Safe Surface Creation ===" << std::endl;
        
        // Create surface with explicit dimensions
        sdlpp::dimensions<int> surf_dims(320, 240);
        auto surface = sdlpp::surface::create_rgb(surf_dims, sdlpp::pixel_format_enum::RGBA8888);
        
        if (surface) {
            std::cout << "Created surface: " << surf_dims.width.value() 
                      << "x" << surf_dims.height.value() << std::endl;
            std::cout << "Is empty? " << (surf_dims.is_empty() ? "Yes" : "No") << std::endl;
            std::cout << "Is valid? " << (surf_dims.is_valid() ? "Yes" : "No") << std::endl;
        }
        
        // Zero dimensions
        sdlpp::dimensions<int> zero_dims(0, 100);
        std::cout << "\nZero width surface (0x100):" << std::endl;
        std::cout << "Is empty? " << (zero_dims.is_empty() ? "Yes" : "No") << std::endl;
        std::cout << "Is valid? " << (zero_dims.is_valid() ? "Yes" : "No") << std::endl;
    }
    
    // Example 5: Arithmetic operations maintain invariants
    {
        std::cout << "\n=== Arithmetic Operations ===" << std::endl;
        
        sdlpp::dimension<int> width(200);
        sdlpp::dimension<int> height(100);
        
        // Subtraction that would go negative
        auto result = height - width;
        std::cout << "100 - 200 = " << result.value() << " (clamped to 0)" << std::endl;
        
        // Multiplication by negative
        auto scaled = width * -2;
        std::cout << "200 * -2 = " << scaled.value() << " (negative result becomes 0)" << std::endl;
        
        // Division
        auto half = width / 2;
        std::cout << "200 / 2 = " << half.value() << std::endl;
    }
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}