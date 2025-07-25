---
sidebar_position: 2
---

# Hello Window

The simplest SDL++ application that creates a window and handles basic events.

## Complete Code

```cpp
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/events/events.hh>
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    // Initialize SDL with video subsystem
    auto init = sdlpp::init::create(sdlpp::init_flags::video);
    if (!init) {
        std::cerr << "Failed to initialize SDL: " << init.error() << "\n";
        return 1;
    }
    
    // Create a window
    auto window = sdlpp::window::create(
        "Hello SDL++!",  // Title
        800,             // Width
        600              // Height
    );
    
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
        // Process all pending events
        while (auto event = sdlpp::event_queue::poll()) {
            // Check if user wants to quit
            if (event->type() == sdlpp::event_type::quit) {
                running = false;
            }
            
            // Handle keyboard events
            event->visit([&](auto&& e) {
                using T = std::decay_t<decltype(e)>;
                
                if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
                    if (e.is_pressed() && e.scan == sdlpp::scancode::escape) {
                        std::cout << "Escape pressed, exiting...\n";
                        running = false;
                    }
                }
            });
        }
        
        // Clear the screen with a nice blue color
        renderer->set_draw_color({64, 128, 255, 255});
        renderer->clear();
        
        // Present the rendered frame
        renderer->present();
        
        // Small delay to prevent CPU spinning
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    std::cout << "Goodbye!\n";
    return 0;
}
```

## Code Breakdown

### 1. Headers

```cpp
#include <sdlpp/core/core.hh>      // SDL initialization
#include <sdlpp/video/window.hh>   // Window management
#include <sdlpp/video/renderer.hh> // Rendering
#include <sdlpp/events/events.hh>  // Event handling
```

These are the essential headers for any SDL++ application.

### 2. SDL Initialization

```cpp
auto init = sdlpp::init::create(sdlpp::init_flags::video);
if (!init) {
    std::cerr << "Failed to initialize SDL: " << init.error() << "\n";
    return 1;
}
```

- `sdlpp::init::create()` initializes SDL subsystems
- We only need `video` for this example
- The `init` object uses RAII - SDL is automatically cleaned up when it goes out of scope
- Always check for errors using the `expected` pattern

### 3. Window Creation

```cpp
auto window = sdlpp::window::create("Hello SDL++!", 800, 600);
```

- Creates an 800x600 window with the given title
- The window is automatically destroyed when `window` goes out of scope
- No need for manual `SDL_DestroyWindow()` calls

### 4. Renderer Creation

```cpp
auto renderer = sdlpp::renderer::create(*window);
```

- Creates a hardware-accelerated renderer for the window
- Used for all drawing operations
- Automatically cleaned up with RAII

### 5. Event Loop

```cpp
while (auto event = sdlpp::event_queue::poll()) {
    if (event->type() == sdlpp::event_type::quit) {
        running = false;
    }
}
```

- `poll()` returns the next event or `std::nullopt` if queue is empty
- Check event type to handle different events
- The quit event is sent when user closes the window

### 6. Event Visitor Pattern

```cpp
event->visit([&](auto&& e) {
    using T = std::decay_t<decltype(e)>;
    
    if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
        // Handle keyboard event
    }
});
```

- The visitor pattern provides type-safe event handling
- `if constexpr` ensures only relevant code is compiled
- Access to strongly-typed event data

### 7. Rendering

```cpp
renderer->set_draw_color({64, 128, 255, 255}); // RGBA
renderer->clear();
renderer->present();
```

- Set color for clearing (light blue)
- Clear the entire screen
- Present swaps the back buffer to display

## Variations

### Centered Window

```cpp
auto window = sdlpp::window::create_centered("Centered App", 800, 600);
```

### Resizable Window

```cpp
auto window = sdlpp::window::create(
    "Resizable Window",
    800, 600,
    sdlpp::window_flags::resizable
);
```

### Fullscreen Window

```cpp
auto window = sdlpp::window::create(
    "Fullscreen App",
    1920, 1080,
    sdlpp::window_flags::fullscreen
);
```

### Window with OpenGL Support

```cpp
auto window = sdlpp::window::create(
    "OpenGL Window",
    800, 600,
    sdlpp::window_flags::opengl
);

// Create OpenGL context
auto gl_context = sdlpp::gl_context::create(*window);
```

## Error Handling

### Using Result Types

```cpp
auto create_app() -> sdlpp::expected<void, std::string> {
    auto init = TRY(sdlpp::init::create(sdlpp::init_flags::video));
    auto window = TRY(sdlpp::window::create("App", 800, 600));
    auto renderer = TRY(sdlpp::renderer::create(*window));
    
    // Run main loop...
    
    return {};
}

int main() {
    auto result = create_app();
    if (!result) {
        std::cerr << "Error: " << result.error() << "\n";
        return 1;
    }
    return 0;
}
```

## Building and Running

### Using CMake

```cmake
cmake_minimum_required(VERSION 3.31)
project(HelloWindow)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(sdlpp REQUIRED)

add_executable(hello_window hello_window.cpp)
target_link_libraries(hello_window PRIVATE sdlpp::sdlpp)
```

### Command Line

```bash
# Compile
g++ -std=c++20 hello_window.cpp -lsdlpp -lSDL3 -o hello_window

# Run
./hello_window
```

## Common Issues

### Window Not Appearing

Make sure you have a render/present loop:
```cpp
renderer->clear();
renderer->present();
```

### High CPU Usage

Add a small delay in your main loop:
```cpp
std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
```

Or use VSync:
```cpp
renderer->set_vsync(1);
```

### Window Immediately Closes

Ensure your event loop is checking the correct event type:
```cpp
if (event->type() == sdlpp::event_type::quit) // Not 'close' or 'exit'
```

## Next Steps

- Add [drawing shapes](drawing-shapes) to your window
- Handle [keyboard and mouse input](handling-input)
- Load and display [images](loading-images)
- Create [animations](animation)

## Full Project Structure

```
hello_window/
├── CMakeLists.txt
├── hello_window.cpp
└── build/
    └── hello_window (executable)
```

This example demonstrates the fundamental structure of every SDL++ application. From here, you can add graphics, sound, input handling, and game logic to create more complex applications.