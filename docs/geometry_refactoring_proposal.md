# Geometry System Refactoring Proposal

## Overview

This document outlines a proposed refactoring of SDL++'s geometry system to allow the library to be used without depending on SDL++'s built-in geometry types. This enables users to integrate SDL++ with their preferred math libraries (GLM, Eigen, custom implementations) without any overhead or forced dependencies.

## Goals

1. **Zero Dependency**: Allow SDL++ to compile without including any concrete geometry types
2. **Type Safety**: Maintain compile-time type checking through C++20 concepts
3. **Flexibility**: Support any geometry library that satisfies the concepts
4. **Convenience**: Provide built-in types for users who want them
5. **Zero Overhead**: No runtime cost for abstraction

## Proposed Solution

Combine three approaches:
1. **Header Separation**: Split geometry into concepts and types
2. **Optional Types**: Make built-in geometry types conditionally included
3. **Template Everything**: Use concepts instead of concrete types in APIs

## File Structure

### Current Structure (Assumed)
```
utility/
├── geometry.hh           # Contains both concepts and types
├── geometry_concepts.hh  # May or may not exist separately
```

### Proposed Structure
```
utility/
├── geometry_concepts.hh  # Pure concepts, no concrete types
├── geometry_types.hh     # Concrete implementations
├── geometry.hh          # Convenience header (includes both conditionally)
└── geometry_fwd.hh      # Forward declarations (optional)
```

## Implementation Details

### 1. geometry_concepts.hh
```cpp
//
// SDL++ Geometry Concepts
// This header has ZERO dependencies and defines only concepts
//

#pragma once

#include <concepts>
#include <type_traits>

namespace sdlpp {
    
    /**
     * @brief Concept for point-like types
     * 
     * Requires:
     * - value_type typedef
     * - x and y members convertible to value_type
     */
    template<typename T>
    concept point_like = requires(T t) {
        typename T::value_type;
        { t.x } -> std::convertible_to<typename T::value_type>;
        { t.y } -> std::convertible_to<typename T::value_type>;
    };
    
    /**
     * @brief Concept for size-like types
     * 
     * Requires:
     * - value_type typedef
     * - width and height members convertible to value_type
     */
    template<typename T>
    concept size_like = requires(T t) {
        typename T::value_type;
        { t.width } -> std::convertible_to<typename T::value_type>;
        { t.height } -> std::convertible_to<typename T::value_type>;
    };
    
    /**
     * @brief Concept for rectangle-like types
     * 
     * Requires:
     * - value_type typedef
     * - x, y, w, h members convertible to value_type
     */
    template<typename T>
    concept rect_like = requires(T t) {
        typename T::value_type;
        { t.x } -> std::convertible_to<typename T::value_type>;
        { t.y } -> std::convertible_to<typename T::value_type>;
        { t.w } -> std::convertible_to<typename T::value_type>;
        { t.h } -> std::convertible_to<typename T::value_type>;
    };
    
    /**
     * @brief Concept for line-like types
     */
    template<typename T>
    concept line_like = requires(T t) {
        typename T::value_type;
        { t.x1 } -> std::convertible_to<typename T::value_type>;
        { t.y1 } -> std::convertible_to<typename T::value_type>;
        { t.x2 } -> std::convertible_to<typename T::value_type>;
        { t.y2 } -> std::convertible_to<typename T::value_type>;
    };
    
    // Utility functions that work with any geometry type
    template<point_like P>
    constexpr auto get_x(const P& p) noexcept { return p.x; }
    
    template<point_like P>
    constexpr auto get_y(const P& p) noexcept { return p.y; }
    
    template<size_like S>
    constexpr auto get_width(const S& s) noexcept { return s.width; }
    
    template<size_like S>
    constexpr auto get_height(const S& s) noexcept { return s.height; }
    
    template<size_like S>
    constexpr auto get_area(const S& s) noexcept {
        return s.width * s.height;
    }
    
    template<rect_like R>
    constexpr auto get_area(const R& r) noexcept {
        return r.w * r.h;
    }
    
    // Conversion helpers for SDL C API
    template<point_like P>
    constexpr auto to_sdl_point(const P& p) noexcept {
        return SDL_Point{
            static_cast<int>(p.x),
            static_cast<int>(p.y)
        };
    }
    
    template<rect_like R>
    constexpr auto to_sdl_rect(const R& r) noexcept {
        return SDL_Rect{
            static_cast<int>(r.x),
            static_cast<int>(r.y),
            static_cast<int>(r.w),
            static_cast<int>(r.h)
        };
    }
}
```

### 2. geometry_types.hh
```cpp
//
// SDL++ Built-in Geometry Types
// This header provides concrete implementations
//

#pragma once

#include <sdlpp/utility/geometry_concepts.hh>
#include <algorithm>
#include <cmath>

namespace sdlpp {
    
    /**
     * @brief Built-in point type
     */
    template<typename T>
    struct point {
        using value_type = T;
        T x{}, y{};
        
        constexpr point() = default;
        constexpr point(T x_, T y_) : x(x_), y(y_) {}
        
        // Operators
        constexpr point operator+(const point& other) const {
            return {x + other.x, y + other.y};
        }
        
        constexpr point operator-(const point& other) const {
            return {x - other.x, y - other.y};
        }
        
        constexpr point& operator+=(const point& other) {
            x += other.x;
            y += other.y;
            return *this;
        }
        
        constexpr point& operator-=(const point& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }
        
        constexpr bool operator==(const point&) const = default;
    };
    
    /**
     * @brief Built-in size type
     */
    template<typename T>
    struct size {
        using value_type = T;
        T width{}, height{};
        
        constexpr size() = default;
        constexpr size(T w, T h) : width(w), height(h) {}
        
        constexpr T area() const { return width * height; }
        constexpr bool empty() const { return width <= 0 || height <= 0; }
        
        constexpr bool operator==(const size&) const = default;
    };
    
    /**
     * @brief Built-in rectangle type
     */
    template<typename T>
    struct rect {
        using value_type = T;
        T x{}, y{}, w{}, h{};
        
        constexpr rect() = default;
        constexpr rect(T x_, T y_, T w_, T h_) : x(x_), y(y_), w(w_), h(h_) {}
        constexpr rect(const point<T>& pos, const size<T>& sz)
            : x(pos.x), y(pos.y), w(sz.width), h(sz.height) {}
        
        constexpr T area() const { return w * h; }
        constexpr bool empty() const { return w <= 0 || h <= 0; }
        
        constexpr point<T> position() const { return {x, y}; }
        constexpr size<T> size() const { return {w, h}; }
        
        constexpr bool contains(const point<T>& p) const {
            return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
        }
        
        constexpr bool operator==(const rect&) const = default;
    };
    
    /**
     * @brief Built-in line type
     */
    template<typename T>
    struct line {
        using value_type = T;
        T x1{}, y1{}, x2{}, y2{};
        
        constexpr line() = default;
        constexpr line(T x1_, T y1_, T x2_, T y2_) 
            : x1(x1_), y1(y1_), x2(x2_), y2(y2_) {}
        constexpr line(const point<T>& p1, const point<T>& p2)
            : x1(p1.x), y1(p1.y), x2(p2.x), y2(p2.y) {}
        
        constexpr point<T> start() const { return {x1, y1}; }
        constexpr point<T> end() const { return {x2, y2}; }
        
        constexpr bool operator==(const line&) const = default;
    };
    
    // Verify our types satisfy our concepts
    static_assert(point_like<point<int>>);
    static_assert(size_like<size<int>>);
    static_assert(rect_like<rect<int>>);
    static_assert(line_like<line<int>>);
    
    // Common type aliases
    using point_i = point<int>;
    using point_f = point<float>;
    using point_d = point<double>;
    
    using size_i = size<int>;
    using size_f = size<float>;
    using size_d = size<double>;
    
    using rect_i = rect<int>;
    using rect_f = rect<float>;
    using rect_d = rect<double>;
    
    using line_i = line<int>;
    using line_f = line<float>;
    using line_d = line<double>;
}
```

### 3. geometry.hh
```cpp
//
// SDL++ Geometry - Convenience Header
// Includes concepts and optionally includes types
//

#pragma once

// Always include concepts
#include <sdlpp/utility/geometry_concepts.hh>

// Conditionally include concrete types
#ifndef SDLPP_NO_BUILTIN_GEOMETRY
    #include <sdlpp/utility/geometry_types.hh>
#endif

// Additional utilities that work with any geometry
namespace sdlpp {
    
    /**
     * @brief Calculate distance between two points
     */
    template<point_like P1, point_like P2>
    auto distance(const P1& p1, const P2& p2) {
        auto dx = p2.x - p1.x;
        auto dy = p2.y - p1.y;
        return std::sqrt(dx * dx + dy * dy);
    }
    
    /**
     * @brief Check if two rectangles intersect
     */
    template<rect_like R1, rect_like R2>
    bool intersects(const R1& r1, const R2& r2) {
        return !(r1.x + r1.w <= r2.x || 
                 r2.x + r2.w <= r1.x ||
                 r1.y + r1.h <= r2.y ||
                 r2.y + r2.h <= r1.y);
    }
}
```

### 4. Update SDL++ Headers

All SDL++ headers should be updated to:
1. Include only `geometry_concepts.hh`
2. Use template functions with concept constraints
3. Never depend on concrete geometry types

Example for `window.hh`:
```cpp
#pragma once

#include <sdlpp/utility/geometry_concepts.hh>  // NOT geometry.hh!
#include <sdlpp/detail/expected.hh>
#include <SDL3/SDL.h>
#include <string>
#include <tuple>

namespace sdlpp {
    
    class window {
    public:
        /**
         * @brief Create window with size
         * @tparam S Size-like type
         */
        template<size_like S>
        static expected<window, std::string> create(
            const std::string& title, 
            const S& size) {
            return create(title, size.width, size.height);
        }
        
        /**
         * @brief Set window position
         * @tparam P Point-like type
         */
        template<point_like P>
        expected<void, std::string> set_position(const P& pos) {
            SDL_SetWindowPosition(ptr_.get(), 
                static_cast<int>(pos.x), 
                static_cast<int>(pos.y));
            return {};
        }
        
        /**
         * @brief Get window position
         * @tparam P Point type to return (optional)
         * @return Point of type P, or tuple<int, int> if P not specified
         */
        template<typename P = void>
        [[nodiscard]] auto get_position() const {
            int x, y;
            SDL_GetWindowPosition(ptr_.get(), &x, &y);
            
            if constexpr (std::is_void_v<P>) {
                // Return as tuple
                return std::tuple{x, y};
            } else {
                static_assert(point_like<P>, "P must satisfy point_like concept");
                return P{x, y};
            }
        }
        
        /**
         * @brief Set window size
         * @tparam S Size-like type
         */
        template<size_like S>
        expected<void, std::string> set_size(const S& size) {
            SDL_SetWindowSize(ptr_.get(), 
                static_cast<int>(size.width), 
                static_cast<int>(size.height));
            return {};
        }
        
        /**
         * @brief Get window size
         * @tparam S Size type to return (optional)
         * @return Size of type S, or tuple<int, int> if S not specified
         */
        template<typename S = void>
        [[nodiscard]] auto get_size() const {
            int w, h;
            SDL_GetWindowSize(ptr_.get(), &w, &h);
            
            if constexpr (std::is_void_v<S>) {
                return std::tuple{w, h};
            } else {
                static_assert(size_like<S>, "S must satisfy size_like concept");
                return S{w, h};
            }
        }
        
        // ... rest of window implementation
    };
}
```

## Usage Examples

### Example 1: Using SDL++ with built-in geometry
```cpp
#include <sdlpp/window.hh>
#include <sdlpp/utility/geometry.hh>  // Gets concepts + types

int main() {
    // Using SDL++ built-in types
    auto window = sdlpp::window::create("My App", sdlpp::size{800, 600});
    window->set_position(sdlpp::point{100, 100});
    
    // Get position as SDL++ type
    auto pos = window->get_position<sdlpp::point<int>>();
    std::cout << "Window at: " << pos.x << ", " << pos.y << "\n";
    
    // Or get as tuple
    auto [x, y] = window->get_position();
    std::cout << "Window at: " << x << ", " << y << "\n";
}
```

### Example 2: Using SDL++ without built-in geometry
```cpp
#define SDLPP_NO_BUILTIN_GEOMETRY  // Disable built-in types
#include <sdlpp/window.hh>
#include <sdlpp/utility/geometry.hh>  // Only gets concepts!

// Your own geometry types
struct vec2 {
    using value_type = int;
    int x, y;
};

struct dimensions {
    using value_type = int;
    int width, height;
};

int main() {
    // Using custom types
    auto window = sdlpp::window::create("My App", dimensions{800, 600});
    window->set_position(vec2{100, 100});
    
    // Get position as custom type
    auto pos = window->get_position<vec2>();
    
    // Or as tuple
    auto [x, y] = window->get_position();
}
```

### Example 3: Using with GLM
```cpp
#include <sdlpp/window.hh>
#include <sdlpp/renderer.hh>
#include <glm/glm.hpp>

// GLM types automatically work because they satisfy the concepts
int main() {
    auto window = sdlpp::window::create("GLM Example", glm::ivec2{1024, 768});
    auto renderer = sdlpp::renderer::create(*window);
    
    // Draw line using GLM vectors
    renderer->draw_line(glm::vec2{0, 0}, glm::vec2{100, 100});
    
    // Get window size as GLM vector
    auto size = window->get_size<glm::ivec2>();
}
```

### Example 4: Using with Eigen
```cpp
#include <sdlpp/window.hh>
#include <Eigen/Core>

// Adapter to make Eigen vectors work with SDL++ concepts
template<typename T>
struct eigen_point_adapter {
    using value_type = T;
    Eigen::Vector2<T>& vec;
    
    T& x() { return vec.x(); }
    T& y() { return vec.y(); }
    const T& x() const { return vec.x(); }
    const T& y() const { return vec.y(); }
};

int main() {
    Eigen::Vector2i size(800, 600);
    // Need adapter since Eigen uses x(), y() methods not members
    auto window = sdlpp::window::create("Eigen Example", 
        eigen_point_adapter<int>{size});
}
```

## Benefits

1. **Zero Overhead**: Templates are resolved at compile time
2. **No Forced Dependencies**: Can use SDL++ without any geometry types
3. **Type Safety**: Concepts ensure compatibility at compile time
4. **Flexibility**: Works with any geometry library
5. **Convenience**: Built-in types available when needed
6. **Clear Separation**: Interface (concepts) vs implementation (types)

## Migration Guide

### For SDL++ Library Code

1. Replace `#include <sdlpp/utility/geometry.hh>` with `#include <sdlpp/utility/geometry_concepts.hh>`
2. Change function signatures from concrete types to concepts:
   ```cpp
   // Before
   void set_position(const point<int>& p);
   
   // After
   template<point_like P>
   void set_position(const P& p);
   ```
3. Update return types to support both concrete types and tuples
4. Use `to_sdl_point()`, `to_sdl_rect()` helpers when calling SDL C API

### For SDL++ Users

1. If using built-in geometry: No changes needed
2. If using custom geometry: Define `SDLPP_NO_BUILTIN_GEOMETRY` before including
3. Ensure your types have `value_type` typedef and appropriate members

## Potential Issues and Solutions

### Issue 1: Compile Time
**Problem**: Heavy template usage may increase compile times.
**Solution**: Use explicit instantiation for common types in implementation files.

### Issue 2: Error Messages
**Problem**: Concept constraint failures can produce verbose error messages.
**Solution**: Provide clear static_assert messages and good documentation.

### Issue 3: API Complexity
**Problem**: Template syntax in public API may intimidate users.
**Solution**: Provide good examples and keep the simple case simple.

### Issue 4: ABI Stability
**Problem**: Templates in public API make ABI stability difficult.
**Solution**: This is already the case with header-only design; document clearly.

## Testing Strategy

1. **Concept Tests**: Verify concepts accept appropriate types
2. **Integration Tests**: Test with multiple geometry libraries (GLM, Eigen, etc.)
3. **Compilation Tests**: Ensure library compiles with `SDLPP_NO_BUILTIN_GEOMETRY`
4. **Performance Tests**: Verify zero overhead claim

## Conclusion

This refactoring provides maximum flexibility while maintaining type safety and zero overhead. Users can choose to use SDL++ with their preferred geometry library or use the convenient built-in types. The clear separation between concepts and implementations makes the library more modular and easier to integrate into existing codebases.