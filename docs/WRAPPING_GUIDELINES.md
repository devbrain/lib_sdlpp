# SDL3 C++ Wrapping Guidelines

This document provides guidelines and best practices for creating modern C++ wrappers around SDL3 functionality. Following these patterns ensures consistency, type safety, and ease of use across the entire wrapper library.

## Table of Contents

1. [Core Principles](#core-principles)
2. [Design Patterns](#design-patterns)
3. [Type System](#type-system)
4. [Error Handling](#error-handling)
5. [Memory Management](#memory-management)
6. [API Design](#api-design)
7. [Testing](#testing)
8. [Documentation](#documentation)
9. [Examples](#examples)

## Core Principles

### 1. Zero-Cost Abstractions
- Wrappers should have minimal or no runtime overhead
- Use templates and compile-time features where possible
- Inline functions for simple conversions

### 2. Type Safety
- Use strong types instead of raw pointers or integers
- Leverage C++20 concepts for compile-time validation
- Prevent common errors through the type system

### 3. RAII (Resource Acquisition Is Initialization)
- All SDL resources must be wrapped in RAII classes
- Automatic cleanup on scope exit
- No manual memory management required by users

### 4. Modern C++ Features
- Target C++20 standard
- Use concepts, std::span, designated initializers
- Leverage standard library types where appropriate

### 5. SDL3 Compatibility
- Maintain easy interoperability with raw SDL3 APIs
- Provide `to_sdl()` and `from_sdl()` conversion methods
- Allow access to underlying SDL types when needed

## Design Patterns

### Resource Wrapper Pattern

```cpp
// Use the custom pointer template for SDL resources
using window_ptr = pointer<SDL_Window, SDL_DestroyWindow>;

class window {
private:
    window_ptr ptr;
    
public:
    explicit window(SDL_Window* w) : ptr(w) {}
    
    // Provide access to raw pointer for SDL API calls
    SDL_Window* get() const { return ptr.get(); }
    
    // Convenient conversion
    explicit operator bool() const { return ptr != nullptr; }
};
```

### Template-Based Type Pattern

For types that have multiple representations (int/float variants):

```cpp
template<arithmetic T>
struct basic_point {
    using value_type = T;
    T x = 0, y = 0;
    
    // Conversion to SDL types
    [[nodiscard]] constexpr auto to_sdl() const {
        if constexpr (std::is_same_v<T, int>) {
            return SDL_Point{x, y};
        } else if constexpr (std::is_same_v<T, float>) {
            return SDL_FPoint{x, y};
        }
    }
    
    // Conversion from SDL types
    static constexpr basic_point from_sdl(const auto& p) {
        return {p.x, p.y};
    }
};

// Type aliases for common cases
using point = basic_point<int>;
using fpoint = basic_point<float>;
```

### Factory Function Pattern

For complex object creation:

```cpp
// Return sdlpp::expected for error handling
inline sdlpp::expected<window, std::string> create_window(
    const std::string& title,
    int width, int height,
    window_flags flags = window_flags::none) {
    
    SDL_Window* win = SDL_CreateWindow(
        title.c_str(), width, height, 
        static_cast<SDL_WindowFlags>(flags)
    );
    
    if (!win) {
        return tl::unexpected(SDL_GetError());
    }
    
    return window(win);
}
```

## Type System

### Enums and Flags

Wrap SDL enums as enum classes:

```cpp
enum class pixel_format_enum : uint32_t {
    unknown = SDL_PIXELFORMAT_UNKNOWN,
    rgba8888 = SDL_PIXELFORMAT_RGBA8888,
    // ... more formats
};

// For flag combinations, use a separate flags type
enum class window_flags : uint32_t {
    none = 0,
    fullscreen = SDL_WINDOW_FULLSCREEN,
    opengl = SDL_WINDOW_OPENGL,
    // ... more flags
};

// Enable bitwise operations for flags
constexpr window_flags operator|(window_flags a, window_flags b) {
    return static_cast<window_flags>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}
```

### Concepts

Define concepts for type constraints:

```cpp
// Concept for types that can be color components
template<typename T>
concept color_component = (std::is_same_v<T, uint8_t> || 
                          std::is_same_v<T, float>);

// Concept for point-like types
template<typename T>
concept point_like = requires(T t) {
    { t.x } -> std::convertible_to<typename T::value_type>;
    { t.y } -> std::convertible_to<typename T::value_type>;
    typename T::value_type;
    requires arithmetic<typename T::value_type>;
};
```

## Error Handling

### Use sdlpp::expected

All functions that can fail should return `sdlpp::expected`:

```cpp
template<typename T>
using result = sdlpp::expected<T, std::string>;

// For void returns
using result_void = sdlpp::expected<void, std::string>;

// Example usage
result<texture> load_texture(const std::string& path) {
    SDL_Texture* tex = IMG_LoadTexture(renderer, path.c_str());
    if (!tex) {
        return tl::unexpected(SDL_GetError());
    }
    return texture(tex);
}
```

### Error Context

Provide meaningful error messages:

```cpp
result<int> get_attribute(window_attribute attr) {
    int value = 0;
    if (SDL_GL_GetAttribute(static_cast<SDL_GLattr>(attr), &value) != 0) {
        return tl::unexpected(
            std::format("Failed to get GL attribute {}: {}", 
                       static_cast<int>(attr), SDL_GetError())
        );
    }
    return value;
}
```

## Memory Management

### Smart Pointer Usage

Use the custom `pointer` template for SDL resources:

```cpp
template<typename T, auto Deleter>
class pointer {
    // Implementation handles cleanup automatically
};

// Usage
using surface_ptr = pointer<SDL_Surface, SDL_DestroySurface>;
using texture_ptr = pointer<SDL_Texture, SDL_DestroyTexture>;
```

### Ownership Semantics

Be clear about ownership:

```cpp
class texture {
private:
    texture_ptr ptr;  // Owns the SDL_Texture
    
public:
    // Takes ownership
    explicit texture(SDL_Texture* t) : ptr(t) {}
    
    // Move-only semantics
    texture(texture&&) = default;
    texture& operator=(texture&&) = default;
    
    // No copying
    texture(const texture&) = delete;
    texture& operator=(const texture&) = delete;
};
```

### Non-Owning Wrappers

For types that don't own resources:

```cpp
class renderer_ref {
private:
    SDL_Renderer* renderer;  // Non-owning pointer
    
public:
    explicit renderer_ref(SDL_Renderer* r) : renderer(r) {}
    
    // Can be copied
    renderer_ref(const renderer_ref&) = default;
    renderer_ref& operator=(const renderer_ref&) = default;
};
```

## API Design

### Method Naming

Follow C++ standard library conventions:

```cpp
class window {
    // Use snake_case for methods
    result_void set_title(const std::string& title);
    result<std::string> get_title() const;
    
    // Properties without get_ prefix
    result<point> position() const;
    result<size> size() const;
    
    // Actions as verbs
    result_void show();
    result_void hide();
    result_void maximize();
};
```

### Parameter Types

Use appropriate C++ types:

```cpp
// Use std::string_view for string parameters
result_void set_title(std::string_view title);

// Use std::span for array parameters
result_void update_texture(std::span<const uint8_t> pixels);

// Use std::optional for optional parameters
result<texture> create_texture(
    size dimensions,
    pixel_format fmt,
    std::optional<texture_access> access = std::nullopt
);
```

### Chaining Support

Enable method chaining where it makes sense:

```cpp
class render_command {
    render_command& set_color(const color& c) {
        // ... implementation
        return *this;
    }
    
    render_command& draw_line(const point& from, const point& to) {
        // ... implementation
        return *this;
    }
};
```

## Testing

### Test Structure

```cpp
TEST_SUITE("component_name") {
    TEST_CASE("construction") {
        SUBCASE("default construction") {
            // Test default constructor
        }
        
        SUBCASE("value construction") {
            // Test parameterized constructor
        }
    }
    
    TEST_CASE("operations") {
        // Test methods and operations
    }
    
    TEST_CASE("error handling") {
        // Test error cases
    }
}
```

### Test Coverage

Ensure tests cover:
- All constructors and factory functions
- Conversion to/from SDL types
- Error cases and edge conditions
- RAII cleanup (using mock SDL functions if needed)
- Concept constraints

## Documentation

### File Header

```cpp
//
// Created by [author] on [date].
//

#pragma once

/**
 * @file [filename].hh
 * @brief [Brief description of the file's purpose]
 * 
 * [Longer description of what this file provides, main classes,
 *  and how it relates to SDL3 functionality]
 */
```

### Class Documentation

```cpp
/**
 * @brief RAII wrapper for SDL_Window
 * 
 * This class provides a safe, RAII-managed interface to SDL's window
 * functionality. Windows are automatically destroyed when the object
 * goes out of scope.
 * 
 * Example usage:
 * @code
 * auto window = create_window("My App", 800, 600);
 * if (window) {
 *     // Use the window
 * }
 * @endcode
 */
class window {
    // ...
};
```

### Method Documentation

```cpp
/**
 * @brief Set the window title
 * @param title New title for the window
 * @return Expected<void> - empty on success, error message on failure
 * @note This is a wrapper around SDL_SetWindowTitle
 */
result_void set_title(std::string_view title);
```

## Examples

### Complete Wrapper Example

Here's a complete example wrapping SDL_Timer functionality:

```cpp
// timer.hh
#pragma once

/**
 * @file timer.hh
 * @brief Modern C++ wrapper for SDL3 timer functionality
 */

#include <sdlpp/sdl.hh>
#include <tl/expected.hpp>
#include <chrono>
#include <functional>

namespace sdlpp {
    /**
     * @brief Type-safe wrapper for timer IDs
     */
    class timer_id {
    private:
        SDL_TimerID id;
        
    public:
        explicit timer_id(SDL_TimerID id) : id(id) {}
        SDL_TimerID get() const { return id; }
        explicit operator bool() const { return id != 0; }
    };
    
    /**
     * @brief RAII wrapper for SDL timers
     */
    class timer {
    private:
        timer_id id;
        std::function<uint32_t(uint32_t)> callback;
        
        static uint32_t sdl_callback(void* userdata, SDL_TimerID, uint32_t interval) {
            auto* self = static_cast<timer*>(userdata);
            return self->callback(interval);
        }
        
    public:
        timer() : id(0) {}
        
        timer(timer&& other) noexcept 
            : id(other.id), callback(std::move(other.callback)) {
            other.id = timer_id(0);
        }
        
        timer& operator=(timer&& other) noexcept {
            if (this != &other) {
                stop();
                id = other.id;
                callback = std::move(other.callback);
                other.id = timer_id(0);
            }
            return *this;
        }
        
        ~timer() {
            stop();
        }
        
        // Delete copy operations
        timer(const timer&) = delete;
        timer& operator=(const timer&) = delete;
        
        /**
         * @brief Start the timer
         * @param interval Interval in milliseconds
         * @param cb Callback function
         * @return true if started successfully
         */
        bool start(std::chrono::milliseconds interval, 
                  std::function<uint32_t(uint32_t)> cb) {
            stop();
            callback = std::move(cb);
            SDL_TimerID new_id = SDL_AddTimer(
                static_cast<uint32_t>(interval.count()),
                sdl_callback, this
            );
            id = timer_id(new_id);
            return id;
        }
        
        /**
         * @brief Stop the timer
         */
        void stop() {
            if (id) {
                SDL_RemoveTimer(id.get());
                id = timer_id(0);
            }
        }
        
        /**
         * @brief Check if timer is running
         */
        [[nodiscard]] bool is_running() const {
            return static_cast<bool>(id);
        }
    };
    
    /**
     * @brief Get the current timer ticks
     * @return Current ticks as chrono duration
     */
    [[nodiscard]] inline std::chrono::milliseconds get_ticks() {
        return std::chrono::milliseconds(SDL_GetTicks());
    }
    
    /**
     * @brief Delay for specified duration
     * @param duration Time to delay
     */
    inline void delay(std::chrono::milliseconds duration) {
        SDL_Delay(static_cast<uint32_t>(duration.count()));
    }
}
```

This example demonstrates:
- RAII resource management
- Type-safe ID wrapper
- Modern C++ features (chrono, move semantics)
- Clear documentation
- SDL3 API integration
- Callback handling

## Checklist for New Wrappers

When creating a new SDL3 wrapper, ensure you:

- [ ] Use RAII for all SDL resources
- [ ] Provide `to_sdl()`/`from_sdl()` conversions
- [ ] Return `sdlpp::expected` for fallible operations
- [ ] Add comprehensive documentation
- [ ] Create unit tests with good coverage
- [ ] Use appropriate C++20 features
- [ ] Follow naming conventions
- [ ] Handle move semantics properly
- [ ] Prevent copying for resource-owning types
- [ ] Add concepts where type constraints are needed
- [ ] Provide convenient factory functions
- [ ] Include usage examples in documentation