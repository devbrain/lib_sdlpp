# SDL++ - Modern C++20 Wrapper for SDL3

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B20)
[![SDL3](https://img.shields.io/badge/SDL-3.0-green.svg)](https://www.libsdl.org/)

SDL++ is a modern, type-safe C++20 wrapper library for SDL3 (Simple DirectMedia Layer version 3). It provides RAII-managed interfaces, comprehensive error handling, and seamless integration with modern C++ features while maintaining the power and flexibility of SDL3.

## Features

- 🎯 **Type-Safe API**: Leverages C++20 concepts and strong typing
- 🔒 **RAII Resource Management**: Automatic cleanup with smart pointers
- ⚡ **Zero-Cost Abstractions**: No runtime overhead compared to raw SDL3
- 🛡️ **Error Handling**: Uses `std::expected` for explicit error handling
- 🎮 **Complete SDL3 Coverage**: Window, rendering, audio, input, and more
- 📐 **Geometry Concepts**: Integrate with your own math libraries
- ⏱️ **Modern Time API**: Full `std::chrono` integration
- 🏗️ **Modular Design**: Use only what you need

## Quick Start

```cpp
#include <sdlpp/sdlpp.hpp>

int main() {
    // Initialize SDL
    auto sdl = sdlpp::sdl::init(sdlpp::init_flags::video);
    if (!sdl) {
        std::cerr << "Failed to initialize SDL: " << sdl.error() << '\n';
        return 1;
    }

    // Create a window
    auto window = sdlpp::window::create("SDL++ Demo", 800, 600);
    if (!window) {
        std::cerr << "Failed to create window: " << window.error() << '\n';
        return 1;
    }

    // Create a renderer
    auto renderer = window->create_renderer();
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << renderer.error() << '\n';
        return 1;
    }

    // Main loop
    bool running = true;
    while (running) {
        // Handle events
        while (auto event = sdlpp::event_queue::poll()) {
            if (event->type == sdlpp::event_type::quit) {
                running = false;
            }
        }

        // Clear screen
        renderer->set_draw_color(0, 0, 0, 255);
        renderer->clear();
        
        // Draw something
        renderer->set_draw_color(255, 0, 0, 255);
        renderer->fill_rect({{100, 100}, {200, 150}});
        
        // Present
        renderer->present();
    }

    return 0;
}
```

## Requirements

- C++20 compatible compiler (GCC 10+, Clang 11+, MSVC 2019+)
- SDL3 (3.0.0 or later)
- CMake 3.20 or later

## Building

### Basic Build

```bash
git clone https://github.com/yourusername/sdlpp.git
cd sdlpp
cmake -B build
cmake --build build
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `SDLPP_BUILD_SHARED` | ON | Build as shared library |
| `SDLPP_BUILD_WITH_TESTS` | ON | Build unit tests |
| `SDLPP_BUILD_WITH_EXAMPLES` | ON | Build example programs |
| `SDLPP_INSTALL` | ON | Generate install targets |
| `SDLPP_ENABLE_SANITIZERS` | OFF | Enable AddressSanitizer and UBSan |
| `SDLPP_ENABLE_LTO` | OFF | Enable link-time optimization |

### Installation

```bash
cmake --build build
sudo cmake --install build
```

## Using SDL++ in Your Project

### CMake Integration

```cmake
find_package(sdlpp REQUIRED)
target_link_libraries(your_app PRIVATE sdlpp::sdlpp)
```

### pkg-config

```bash
g++ main.cpp `pkg-config --cflags --libs sdlpp` -std=c++20
```

## Module Overview

### Core (`sdlpp/core`)
- SDL initialization and version management
- Error handling utilities
- Timer and timing utilities
- Logging facilities

### Video (`sdlpp/video`)
- Window management
- 2D rendering (hardware accelerated)
- Surface operations (software rendering)
- OpenGL/Vulkan context management
- Display and video mode queries

### Audio (`sdlpp/audio`)
- Audio device management
- Audio streams and callbacks
- WAV file loading
- Audio format conversions

### Events (`sdlpp/events`)
- Event polling and handling
- Custom event support
- Event filtering
- Keyboard, mouse, and controller events

### Input (`sdlpp/input`)
- Keyboard input and scancodes
- Mouse input and cursors
- Game controller support
- Joystick handling
- Touch and gesture support
- Haptic feedback

### Application Framework (`sdlpp/app`)
- SDL3 application model
- Fixed timestep game loops
- Scene management
- FPS limiting

## Examples

The library includes numerous examples demonstrating various features:

- **Basic Window**: Creating windows and handling events
- **2D Rendering**: Drawing shapes, textures, and sprites
- **OpenGL Integration**: Using SDL++ with OpenGL
- **Audio Playback**: Playing sounds and music
- **Game Controller**: Handling gamepad input
- **Application Framework**: Using the built-in app classes

Browse the [examples directory](examples/) for more.

## Documentation

Full API documentation is available at [https://sdlpp.readthedocs.io](https://sdlpp.readthedocs.io) (placeholder).

### Key Concepts

#### RAII and Resource Management
All SDL resources are wrapped in RAII classes that automatically handle cleanup:

```cpp
{
    auto window = sdlpp::window::create("My Window", 800, 600);
    // Window is automatically destroyed when it goes out of scope
}
```

#### Error Handling
SDL++ uses `std::expected` for error handling:

```cpp
auto result = sdlpp::some_operation();
if (!result) {
    std::cerr << "Error: " << result.error() << '\n';
} else {
    // Use result.value()
}
```

#### Geometry Concepts
SDL++ uses C++20 concepts to work with any math library:

```cpp
// Works with any type that satisfies point_like concept
MyVector2 pos{100, 200};
window.set_position(pos);

// Works with glm, Eigen, or your custom types
glm::ivec2 size{800, 600};
window.set_size(size);
```

### Development

```bash
# Clone with submodules
git clone --recursive https://github.com/yourusername/sdlpp.git

# Build with tests and examples
cmake -B build -DSDLPP_BUILD_WITH_TESTS=ON -DSDLPP_BUILD_WITH_EXAMPLES=ON
cmake --build build

# Run tests
./build/bin/unittest
```

## License

SDL++ is licensed under the MIT License. See [LICENSE](LICENSE) file for details.

## Acknowledgments

- [SDL Development Team](https://www.libsdl.org/) for creating SDL3
- [doctest](https://github.com/onqtam/doctest) for the testing framework
- [tl::expected](https://github.com/TartanLlama/expected) for error handling
- [failsafe](https://github.com/devbrain/failsafe) for enhanced error reporting

---

Made with ❤️ for the game development community