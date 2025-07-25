---
sidebar_position: 2
---

# Quick Start

Let's create your first SDL++ application - a simple window that responds to user input.

## Basic Window Application

Create a new file `main.cpp`:

```cpp
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/events/events.hh>
#include <iostream>

int main() {
    // Initialize SDL with video support
    auto init = sdlpp::init::create(sdlpp::init_flags::video);
    if (!init) {
        std::cerr << "Failed to initialize SDL: " << init.error() << "\n";
        return 1;
    }
    
    // Create a window
    auto window = sdlpp::window::create("My First SDL++ App", 800, 600);
    if (!window) {
        std::cerr << "Failed to create window: " << window.error() << "\n";
        return 1;
    }
    
    // Create a renderer for the window
    auto renderer = sdlpp::renderer::create(*window);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << renderer.error() << "\n";
        return 1;
    }
    
    // Main loop
    bool running = true;
    while (running) {
        // Handle events
        while (auto event = sdlpp::event_queue::poll()) {
            if (event->type() == sdlpp::event_type::quit) {
                running = false;
            }
        }
        
        // Clear screen with blue color
        renderer->set_draw_color({0, 128, 255, 255});
        renderer->clear();
        
        // Present the frame
        renderer->present();
    }
    
    return 0;
}
```

## Building Your Application

Create a `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.31)
project(MySDLppApp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Fetch SDL++
include(FetchContent)
FetchContent_Declare(
    sdlpp
    GIT_REPOSITORY https://github.com/sdlpp/sdlpp.git
    GIT_TAG main
)
FetchContent_MakeAvailable(sdlpp)

# Create executable
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE sdlpp)
```

Build and run:

```bash
mkdir build && cd build
cmake ..
cmake --build .
./myapp
```

## Understanding the Code

Let's break down what's happening:

### 1. Initialization

```cpp
auto init = sdlpp::init::create(sdlpp::init_flags::video);
```

This initializes SDL's video subsystem. The `init` object uses RAII - SDL will be automatically cleaned up when it goes out of scope.

### 2. Window Creation

```cpp
auto window = sdlpp::window::create("My First SDL++ App", 800, 600);
```

Creates an 800x600 window with the given title. The window is automatically destroyed when the `window` object goes out of scope.

### 3. Renderer Creation

```cpp
auto renderer = sdlpp::renderer::create(*window);
```

Creates a hardware-accelerated renderer attached to our window. This is used for all drawing operations.

### 4. Event Handling

```cpp
while (auto event = sdlpp::event_queue::poll()) {
    if (event->type() == sdlpp::event_type::quit) {
        running = false;
    }
}
```

SDL++ provides a type-safe event system. `poll()` returns an optional event - if there's no event, it returns `std::nullopt`.

### 5. Rendering

```cpp
renderer->set_draw_color({0, 128, 255, 255});  // RGBA
renderer->clear();
renderer->present();
```

Each frame:
1. Set the draw color (blue in this case)
2. Clear the entire screen with that color
3. Present the rendered frame to the screen

## Adding Interactivity

Let's enhance our application to respond to keyboard input:

```cpp
// In the event loop:
while (auto event = sdlpp::event_queue::poll()) {
    event->visit([&](auto&& e) {
        using T = std::decay_t<decltype(e)>;
        
        if constexpr (std::is_same_v<T, sdlpp::quit_event>) {
            running = false;
        } else if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
            if (e.is_pressed() && e.scan == sdlpp::scancode::escape) {
                running = false;
            }
        }
    });
}
```

This uses SDL++'s variant-based event system to handle different event types safely.

## Drawing Shapes

Add some shapes to your window:

```cpp
// Draw a red rectangle
renderer->set_draw_color({255, 0, 0, 255});
renderer->fill_rect({{100, 100}, {200, 150}});

// Draw a green line
renderer->set_draw_color({0, 255, 0, 255});
renderer->draw_line({50, 50}, {750, 550});

// Draw blue points
renderer->set_draw_color({0, 0, 255, 255});
for (int i = 0; i < 10; ++i) {
    renderer->draw_point({100 + i * 50, 300});
}
```

## Error Handling Best Practices

SDL++ uses `std::expected` for error handling. Here's a more robust example:

```cpp
auto create_app_window() -> sdlpp::expected<sdlpp::window, std::string> {
    auto window = sdlpp::window::create("App", 800, 600);
    if (!window) {
        return sdlpp::unexpected("Failed to create window: " + window.error());
    }
    
    // Configure window
    window->set_minimum_size(640, 480);
    window->set_resizable(true);
    
    return window;
}

// Usage:
auto window_result = create_app_window();
if (!window_result) {
    std::cerr << window_result.error() << "\n";
    return 1;
}
auto window = std::move(*window_result);
```

## What's Next?

Congratulations! You've created your first SDL++ application. Here are some next steps:

- Learn about [Project Setup](project-setup) for larger applications
- Explore [Window Management](../guides/window-management) for advanced window features
- Dive into [Rendering](../guides/rendering) for textures and advanced graphics
- Master [Event Handling](../guides/event-handling) for complex input scenarios

## Complete Example

Here's a complete example with error handling and basic interactivity:

<details>
<summary>Click to expand full example</summary>

```cpp
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/timer/timer.hh>
#include <iostream>
#include <cmath>

int main() {
    // Initialize SDL
    auto init = sdlpp::init::create(sdlpp::init_flags::video);
    if (!init) {
        std::cerr << "Failed to initialize SDL: " << init.error() << "\n";
        return 1;
    }
    
    // Create window and renderer
    auto window = sdlpp::window::create("SDL++ Demo", 800, 600, 
        sdlpp::window_flags::resizable);
    if (!window) {
        std::cerr << "Failed to create window: " << window.error() << "\n";
        return 1;
    }
    
    auto renderer = sdlpp::renderer::create(*window);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << renderer.error() << "\n";
        return 1;
    }
    
    // Game state
    bool running = true;
    float angle = 0.0f;
    sdlpp::point center{400, 300};
    
    // Frame rate limiter
    sdlpp::frame_limiter limiter(60.0);
    
    while (running) {
        // Handle events
        while (auto event = sdlpp::event_queue::poll()) {
            event->visit([&](auto&& e) {
                using T = std::decay_t<decltype(e)>;
                
                if constexpr (std::is_same_v<T, sdlpp::quit_event>) {
                    running = false;
                } else if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
                    if (e.is_pressed()) {
                        switch (e.scan) {
                            case sdlpp::scancode::escape:
                                running = false;
                                break;
                            case sdlpp::scancode::space:
                                angle = 0.0f;
                                break;
                            default:
                                break;
                        }
                    }
                } else if constexpr (std::is_same_v<T, sdlpp::mouse_motion_event>) {
                    center = {e.x, e.y};
                }
            });
        }
        
        // Update
        angle += 0.02f;
        
        // Render
        renderer->set_draw_color({20, 20, 20, 255});
        renderer->clear();
        
        // Draw rotating squares
        for (int i = 0; i < 8; ++i) {
            float a = angle + (i * M_PI / 4);
            int x = center.x + static_cast<int>(100 * std::cos(a));
            int y = center.y + static_cast<int>(100 * std::sin(a));
            
            renderer->set_draw_color({
                static_cast<uint8_t>(255 * (i + 1) / 8),
                static_cast<uint8_t>(128),
                static_cast<uint8_t>(255 - 255 * (i + 1) / 8),
                255
            });
            
            renderer->fill_rect({{x - 20, y - 20}, {40, 40}});
        }
        
        renderer->present();
        
        // Limit frame rate
        limiter.wait_for_next_frame();
    }
    
    return 0;
}
```

</details>