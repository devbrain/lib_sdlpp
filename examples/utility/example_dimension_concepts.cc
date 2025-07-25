//
// Example: Dimension Concepts in Action
//

#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>
#include <sdlpp/core/core.hh>
#include <sdlpp/utility/dimension.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/core/error.hh>

// Custom user-defined dimension type
template<typename T>
class safe_dimension {
    T val_;
public:
    constexpr explicit safe_dimension(T v) : val_(std::max(T{0}, v)) {}
    constexpr T value() const noexcept { return val_; }
    constexpr bool is_zero() const noexcept { return val_ == 0; }
    constexpr bool is_positive() const noexcept { return val_ > 0; }
};

// Custom dimensions container
template<typename T>
struct safe_dimensions {
    safe_dimension<T> width;
    safe_dimension<T> height;
    
    constexpr safe_dimensions(T w, T h) : width(w), height(h) {}
    constexpr auto area() const { return width.value() * height.value(); }
    constexpr bool is_empty() const { return width.is_zero() || height.is_zero(); }
    constexpr bool is_valid() const { return width.is_positive() && height.is_positive(); }
};

// Generic function that works with any dimensions-like type
template<sdlpp::dimensions_like D>
void print_dimensions_info(const D& dims, const std::string& name) {
    std::cout << name << " dimensions:" << std::endl;
    std::cout << "  Width: " << dims.width.value() << std::endl;
    std::cout << "  Height: " << dims.height.value() << std::endl;
    std::cout << "  Area: " << dims.area() << std::endl;
    std::cout << "  Is empty? " << (dims.is_empty() ? "Yes" : "No") << std::endl;
    std::cout << "  Is valid? " << (dims.is_valid() ? "Yes" : "No") << std::endl;
    std::cout << std::endl;
}

// Function that only accepts valid dimensions
template<sdlpp::dimensions_like D>
auto calculate_aspect_ratio(const D& dims) {
    if (!dims.is_valid()) {
        return 0.0;  // Return 0 for invalid dimensions
    }
    return static_cast<double>(dims.width.value()) / dims.height.value();
}

// Generic window factory that works with any dimensions type
template<sdlpp::dimensions_like D>
auto create_window_generic(const std::string& title, const D& dims) {
    // Convert to SDL-compatible dimensions
    auto [w, h] = sdlpp::to_sdl_dimensions(sdlpp::dimensions<int>(
        static_cast<int>(dims.width.value()),
        static_cast<int>(dims.height.value())
    ));
    
    std::cout << "Creating window \"" << title << "\" with size " 
              << w << "x" << h << std::endl;
    
    return sdlpp::window::create(title, w, h);
}

// Function that works with multiple dimension types
template<sdlpp::dimensions_like... Ds>
auto find_largest_area(const Ds&... dimensions) {
    std::vector<size_t> areas;
    ((areas.push_back(static_cast<size_t>(sdlpp::get_area(dimensions)))), ...);
    return *std::max_element(areas.begin(), areas.end());
}

int main() {
    try {
        // Initialize SDL
        sdlpp::init init(sdlpp::init_flags::video);
    
    std::cout << "=== Dimension Concepts Demonstration ===\n" << std::endl;
    
    // Example 1: Using built-in dimension types
    {
        std::cout << "1. Built-in dimension types:" << std::endl;
        
        sdlpp::dimensions<int> screen(1920, 1080);
        print_dimensions_info(screen, "Screen");
        
        if (screen.is_valid()) {
            auto ratio = calculate_aspect_ratio(screen);
            std::cout << "Aspect ratio: " << ratio << " (" 
                      << (ratio > 1.7 ? "16:9" : "Other") << ")" << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Example 2: Using custom dimension types
    {
        std::cout << "2. Custom dimension types:" << std::endl;
        
        safe_dimensions<int> custom(800, 600);
        print_dimensions_info(custom, "Custom");
        
        // The same generic functions work!
        if (custom.is_valid()) {
            auto ratio = calculate_aspect_ratio(custom);
            std::cout << "Aspect ratio: " << ratio << " (4:3)" << std::endl;
        }
        std::cout << std::endl;
    }
    
    // Example 3: Negative dimension handling
    {
        std::cout << "3. Negative dimension handling:" << std::endl;
        
        sdlpp::dimensions<int> neg_dims(-100, -200);
        print_dimensions_info(neg_dims, "Negative input");
        
        safe_dimensions<int> safe_neg(-50, -100);
        print_dimensions_info(safe_neg, "Safe negative");
        std::cout << std::endl;
    }
    
    // Example 4: Finding largest area among different types
    {
        std::cout << "4. Finding largest area (mixed types):" << std::endl;
        
        sdlpp::dimensions<int> small(640, 480);
        safe_dimensions<int> medium(1024, 768);
        sdlpp::dimensions<float> large(1920.0f, 1080.0f);
        
        auto largest = find_largest_area(small, medium, large);
        std::cout << "Largest area: " << largest << std::endl;
        std::cout << std::endl;
    }
    
    // Example 5: Creating windows with different dimension types
    {
        std::cout << "5. Generic window creation:" << std::endl;
        
        // Using standard dimensions
        sdlpp::window_dimensions std_dims(400, 300);
        [[maybe_unused]] auto window1 = create_window_generic("Standard Window", std_dims);
        
        // Using custom dimensions
        safe_dimensions<int> custom_dims(600, 400);
        [[maybe_unused]] auto window2 = create_window_generic("Custom Window", custom_dims);
        
        std::cout << std::endl;
    }
    
    // Example 6: Compile-time dimension validation
    {
        std::cout << "6. Compile-time validation:" << std::endl;
        
        // This function only compiles for types satisfying dimensions_like
        auto validate_dimensions = []<sdlpp::dimensions_like D>(const D& dims) {
            return dims.is_valid();
        };
        
        sdlpp::dimensions<int> dims(100, 200);
        std::cout << "Built-in dimensions valid? " << validate_dimensions(dims) << std::endl;
        
        safe_dimensions<float> safe(50.0f, 75.0f);
        std::cout << "Custom dimensions valid? " << validate_dimensions(safe) << std::endl;
        
        // This would not compile:
        // int bad = 42;
        // validate_dimensions(bad);  // Error: int doesn't satisfy dimensions_like
    }
    
    std::cout << "\nAll concept-based operations completed successfully!" << std::endl;
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}