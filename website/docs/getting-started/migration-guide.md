---
sidebar_position: 3
---

# Migration Guide

This guide helps developers migrate from raw SDL3 C API to SDL++ C++20 wrapper. Whether you're coming from SDL2, SDL3, or another multimedia library, this guide will help you understand SDL++'s approach and patterns.

## Core Concepts

### 1. RAII Resource Management

SDL++ uses RAII (Resource Acquisition Is Initialization) for all SDL resources. Resources are automatically cleaned up when objects go out of scope.

<div className="code-comparison">
<div>

**SDL3 C API**
```c
SDL_Window* window = SDL_CreateWindow(...);
// ... use window ...
SDL_DestroyWindow(window);
```

</div>
<div>

**SDL++ C++ API**
```cpp
auto window = sdlpp::window::create(...);
// Automatically destroyed
```

</div>
</div>

### 2. Error Handling with expected

Instead of null pointer checks and global error state, SDL++ uses `tl::expected` for explicit error handling.

<div className="code-comparison">
<div>

**SDL3 C API**
```c
SDL_Renderer* renderer = SDL_CreateRenderer(window);
if (!renderer) {
    printf("Error: %s\n", SDL_GetError());
    return -1;
}
```

</div>
<div>

**SDL++ C++ API**
```cpp
auto renderer = sdlpp::renderer::create(window);
if (!renderer) {
    std::cerr << "Error: " << renderer.error() << "\n";
    return -1;
}
```

</div>
</div>

### 3. Type-Safe Enums

Macros and bit flags are replaced with type-safe enum classes that support bitwise operations.

<div className="code-comparison">
<div>

**SDL3 C API**
```c
Uint32 flags = SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL;
```

</div>
<div>

**SDL++ C++ API**
```cpp
auto flags = sdlpp::window_flags::fullscreen | 
             sdlpp::window_flags::opengl;
```

</div>
</div>

## Common Migration Patterns

### Initialization

<div className="code-comparison">
<div>

**SDL3 C API**
```c
if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    printf("SDL init failed: %s\n", SDL_GetError());
    return -1;
}
// ... use SDL ...
SDL_Quit();
```

</div>
<div>

**SDL++ C++ API**
```cpp
auto init = sdlpp::init::create(
    sdlpp::init_flags::video | 
    sdlpp::init_flags::audio
);
if (!init) {
    std::cerr << "SDL init failed: " << init.error() << "\n";
    return -1;
}
// Automatically quits
```

</div>
</div>

### Window Management

<div className="code-comparison">
<div>

**SDL3 C API**
```c
SDL_Window* window = SDL_CreateWindow(
    "Title", 800, 600, SDL_WINDOW_SHOWN);
if (!window) {
    printf("Window creation failed: %s\n", 
           SDL_GetError());
    return -1;
}

// Set window properties
SDL_SetWindowTitle(window, "New Title");
SDL_SetWindowSize(window, 1024, 768);

// Cleanup
SDL_DestroyWindow(window);
```

</div>
<div>

**SDL++ C++ API**
```cpp
auto window = sdlpp::window::create("Title", 800, 600);
if (!window) {
    std::cerr << "Window creation failed: " 
              << window.error() << "\n";
    return -1;
}

// Set window properties
window->set_title("New Title");
window->set_size(1024, 768);

// Automatic cleanup
```

</div>
</div>

### Event Handling

<div className="code-comparison">
<div>

**SDL3 C API**
```c
SDL_Event event;
while (SDL_PollEvent(&event)) {
    switch (event.type) {
        case SDL_EVENT_QUIT:
            running = false;
            break;
        case SDL_EVENT_KEY_DOWN:
            if (event.key.key == SDLK_ESCAPE) {
                running = false;
            }
            break;
        case SDL_EVENT_MOUSE_MOTION:
            x = event.motion.x;
            y = event.motion.y;
            break;
    }
}
```

</div>
<div>

**SDL++ C++ API**
```cpp
while (auto event = sdlpp::event_queue::poll()) {
    event->visit([&](auto&& e) {
        using T = std::decay_t<decltype(e)>;
        
        if constexpr (std::is_same_v<T, sdlpp::quit_event>) {
            running = false;
        } else if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
            if (e.is_pressed() && e.scan == sdlpp::scancode::escape) {
                running = false;
            }
        } else if constexpr (std::is_same_v<T, sdlpp::mouse_motion_event>) {
            x = e.x;
            y = e.y;
        }
    });
}
```

</div>
</div>

### Rendering

<div className="code-comparison">
<div>

**SDL3 C API**
```c
SDL_Renderer* renderer = SDL_CreateRenderer(window);
SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
SDL_RenderClear(renderer);

SDL_FRect rect = {10, 10, 100, 100};
SDL_RenderFillRect(renderer, &rect);

SDL_RenderPresent(renderer);
SDL_DestroyRenderer(renderer);
```

</div>
<div>

**SDL++ C++ API**
```cpp
auto renderer = sdlpp::renderer::create(*window);
renderer->set_draw_color({255, 0, 0, 255});
renderer->clear();

renderer->fill_rect({{10, 10}, {100, 100}});

renderer->present();
// Automatic cleanup
```

</div>
</div>

### File I/O

<div className="code-comparison">
<div>

**SDL3 C API**
```c
SDL_IOStream* io = SDL_IOFromFile("file.txt", "r");
if (!io) {
    printf("Failed to open: %s\n", SDL_GetError());
    return -1;
}

Sint64 size = SDL_GetIOSize(io);
char* buffer = malloc(size + 1);
SDL_ReadIO(io, buffer, size);
buffer[size] = '\0';

SDL_CloseIO(io);
free(buffer);
```

</div>
<div>

**SDL++ C++ API**
```cpp
auto io = sdlpp::iostream::open("file.txt", 
                                sdlpp::file_mode::read);
if (!io) {
    std::cerr << "Failed to open: " << io.error() << "\n";
    return -1;
}

auto content = io->read_string();
// No manual cleanup needed
```

</div>
</div>

### Texture Management

<div className="code-comparison">
<div>

**SDL3 C API**
```c
SDL_Surface* surface = SDL_LoadBMP("image.bmp");
if (!surface) {
    printf("Failed to load: %s\n", SDL_GetError());
    return -1;
}

SDL_Texture* texture = SDL_CreateTextureFromSurface(
    renderer, surface);
SDL_DestroySurface(surface);

if (!texture) {
    printf("Failed to create texture: %s\n", 
           SDL_GetError());
    return -1;
}

SDL_RenderTexture(renderer, texture, NULL, NULL);
SDL_DestroyTexture(texture);
```

</div>
<div>

**SDL++ C++ API**
```cpp
auto surface = sdlpp::surface::load("image.bmp");
if (!surface) {
    std::cerr << "Failed to load: " << surface.error() << "\n";
    return -1;
}

auto texture = renderer->create_texture(*surface);
if (!texture) {
    std::cerr << "Failed to create texture: " 
              << texture.error() << "\n";
    return -1;
}

renderer->copy(*texture);
// Automatic cleanup
```

</div>
</div>

## Advanced Patterns

### Timer Usage

<div className="code-comparison">
<div>

**SDL3 C API**
```c
Uint64 start = SDL_GetPerformanceCounter();
// ... do work ...
Uint64 end = SDL_GetPerformanceCounter();
double elapsed_ms = (double)(end - start) * 1000.0 / 
                    SDL_GetPerformanceFrequency();
```

</div>
<div>

**SDL++ C++ API**
```cpp
auto start = sdlpp::timer::get_performance_counter();
// ... do work ...
auto elapsed = sdlpp::timer::get_performance_counter() - start;
auto elapsed_ms = sdlpp::timer::performance_to_ms(elapsed);

// Or using std::chrono
auto duration = sdlpp::timer::performance_to_duration(elapsed);
```

</div>
</div>

### Platform Detection

<div className="code-comparison">
<div>

**SDL3 C API**
```c
const char* platform = SDL_GetPlatform();
if (strcmp(platform, "Windows") == 0) {
    // Windows-specific code
} else if (strcmp(platform, "Linux") == 0) {
    // Linux-specific code
}
```

</div>
<div>

**SDL++ C++ API**
```cpp
// Compile-time platform detection
if constexpr (sdlpp::platform::is_windows()) {
    // Windows-specific code
} else if constexpr (sdlpp::platform::is_linux()) {
    // Linux-specific code
}

// Runtime platform detection
if (sdlpp::platform::get_platform() == "Windows") {
    // Windows-specific code
}
```

</div>
</div>

## Error Handling Best Practices

### Basic Error Handling

```cpp
auto result = sdlpp::window::create("My App", 800, 600);
if (!result) {
    std::cerr << "Error: " << result.error() << "\n";
    return -1;
}
auto window = std::move(*result);
```

### Using the TRY Macro

```cpp
auto setup_graphics() -> sdlpp::expected<void, std::string> {
    auto window = TRY(sdlpp::window::create("App", 800, 600));
    auto renderer = TRY(sdlpp::renderer::create(window));
    
    TRY(renderer->set_vsync(1));
    TRY(window->show());
    
    return {};
}
```

### Chaining Operations

```cpp
auto result = sdlpp::window::create("App", 800, 600)
    .and_then([](auto&& window) {
        return sdlpp::renderer::create(window)
            .transform([&window](auto&& renderer) {
                return std::make_pair(
                    std::move(window), 
                    std::move(renderer)
                );
            });
    });

if (!result) {
    std::cerr << "Setup failed: " << result.error() << "\n";
    return -1;
}

auto [window, renderer] = std::move(*result);
```

## Memory Management Tips

### Stack vs Heap Allocation

```cpp
// Prefer stack allocation (automatic storage)
auto window = sdlpp::window::create("App", 800, 600);

// Heap allocation when needed
auto window_ptr = std::make_unique<sdlpp::window>(
    sdlpp::window::create("App", 800, 600).value()
);
```

### Move Semantics

```cpp
// SDL++ objects are move-only
auto window1 = sdlpp::window::create("App", 800, 600);
auto window2 = std::move(window1);  // OK
// auto window3 = window2;  // Error: no copy constructor
```

## Integration with Existing Code

### Accessing Raw SDL Handles

```cpp
auto window = sdlpp::window::create("App", 800, 600);
SDL_Window* raw_window = window->get();  // Get raw pointer

// Use with C SDL functions if needed
SDL_ShowWindow(raw_window);
```

### Wrapping Existing SDL Resources

```cpp
// If you have a raw SDL_Window* from elsewhere
SDL_Window* raw = /* from C code */;
sdlpp::window_ptr wrapped(raw);  // Takes ownership
```

## Common Pitfalls and Solutions

### 1. Forgetting to Check Results

```cpp
// Bad: Ignoring errors
auto window = sdlpp::window::create("App", 800, 600);
window->set_title("New Title");  // Crash if creation failed!

// Good: Always check
auto window = sdlpp::window::create("App", 800, 600);
if (!window) {
    // Handle error
    return;
}
window->set_title("New Title");
```

### 2. Mixing Raw SDL and SDL++

```cpp
// Avoid mixing ownership models
auto window = sdlpp::window::create("App", 800, 600);
SDL_DestroyWindow(window->get());  // Bad! Double-free
```

### 3. Incorrect Event Handling

```cpp
// Bad: Not using visitor pattern
sdlpp::event event;
if (event.type() == sdlpp::event_type::key_down) {
    // How to access keyboard data?
}

// Good: Use visitor or as<T>
event.visit([](auto&& e) {
    using T = std::decay_t<decltype(e)>;
    if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
        // Access keyboard data safely
    }
});
```

## Next Steps

- Review the [API Reference](../api/overview) for detailed documentation
- Check out [Examples](../../examples) for complete working code
- Join our [Discord](https://discord.gg/sdlpp) for migration help

## Quick Reference Table

| SDL3 Function | SDL++ Equivalent |
|--------------|------------------|
| `SDL_Init()` | `sdlpp::init::create()` |
| `SDL_CreateWindow()` | `sdlpp::window::create()` |
| `SDL_CreateRenderer()` | `sdlpp::renderer::create()` |
| `SDL_PollEvent()` | `sdlpp::event_queue::poll()` |
| `SDL_LoadBMP()` | `sdlpp::surface::load()` |
| `SDL_CreateTexture()` | `renderer->create_texture()` |
| `SDL_GetError()` | `result.error()` |
| `SDL_Delay()` | `sdlpp::timer::delay()` |
| `SDL_GetTicks()` | `sdlpp::timer::get_ticks()` |
| `SDL_IOFromFile()` | `sdlpp::iostream::open()` |