---
sidebar_position: 1
---

# Examples

This section contains practical examples demonstrating various SDL++ features. Each example is self-contained and can be compiled independently.

## Getting Started Examples

### [Hello Window](hello-window)
The simplest SDL++ application - creates a window and handles the quit event.

### [Drawing Shapes](drawing-shapes)
Basic 2D rendering with rectangles, lines, and points.

### [Handling Input](handling-input)
Keyboard and mouse input handling with real-time feedback.

### [Loading Images](loading-images)
Loading and displaying images using surfaces and textures.

## Graphics Examples

### [Animation](animation)
Smooth sprite animation using textures and timing.

### [Particle System](particle-system)
A simple particle system with physics simulation.

### [Render Targets](render-targets)
Off-screen rendering and post-processing effects.

### [OpenGL Integration](opengl-integration)
Using SDL++ with modern OpenGL for 3D graphics.

## Audio Examples

### [Playing Sounds](playing-sounds)
Loading and playing WAV files with volume control.

### [Audio Streaming](audio-streaming)
Real-time audio generation and streaming.

### [Music Playback](music-playback)
Background music with fade in/out effects.

## Game Examples

### [Pong Clone](pong-clone)
Classic Pong game demonstrating game loop and collision detection.

### [Snake Game](snake-game)
Snake game with score tracking and increasing difficulty.

### [Platformer Physics](platformer-physics)
Basic platformer with gravity, jumping, and collision detection.

## Advanced Examples

### [Multithreading](multithreading)
Resource loading in background threads.

### [Networking](networking)
Simple client-server communication.

### [Custom Events](custom-events)
Creating and handling application-specific events.

### [Dear ImGui Integration](imgui-integration)
Using Dear ImGui for debug UI and tools.

## Running the Examples

### Prerequisites

1. Build SDL++ with examples enabled:
```bash
cmake -B build -DSDLPP_BUILD_EXAMPLES=ON
cmake --build build
```

2. Run examples from the build directory:
```bash
./build/bin/example_hello_window
./build/bin/example_drawing_shapes
# etc...
```

### Building Individual Examples

Each example can also be built standalone:

```cpp
// example.cpp
#include <sdlpp/sdlpp.hh>

int main() {
    // Example code here
}
```

Compile with:
```bash
g++ -std=c++20 example.cpp -lsdlpp -lSDL3 -o example
```

## Example Structure

Each example follows this structure:

1. **Initialization** - Set up SDL and create resources
2. **Main Loop** - Handle events, update state, render
3. **Cleanup** - RAII handles cleanup automatically

```cpp
int main() {
    // Initialize SDL
    auto init = sdlpp::init::create(sdlpp::init_flags::video);
    if (!init) return -1;
    
    // Create window and renderer
    auto window = sdlpp::window::create("Example", 800, 600);
    if (!window) return -1;
    
    auto renderer = sdlpp::renderer::create(*window);
    if (!renderer) return -1;
    
    // Main loop
    bool running = true;
    while (running) {
        // Handle events
        while (auto event = sdlpp::event_queue::poll()) {
            if (event->type() == sdlpp::event_type::quit) {
                running = false;
            }
        }
        
        // Update
        update_game_state();
        
        // Render
        renderer->clear();
        render_scene(*renderer);
        renderer->present();
    }
    
    return 0;
}
```

## Contributing Examples

We welcome example contributions! When submitting examples:

1. Keep them focused on demonstrating specific features
2. Include comments explaining key concepts
3. Follow the project's coding style
4. Test on multiple platforms if possible

Submit examples via [GitHub pull requests](https://github.com/sdlpp/sdlpp/pulls).

## Resources

- [SDL++ API Reference](../api/overview)
- [SDL3 Documentation](https://wiki.libsdl.org/SDL3)
- [Game Programming Patterns](https://gameprogrammingpatterns.com/)
- [Learn OpenGL](https://learnopengl.com/)