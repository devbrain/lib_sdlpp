# Window Usage Guide

## Overview

The window module provides a modern C++ wrapper around SDL3's window system. Windows represent operating system windows that can be used for rendering and receiving input events.

## Key Features

- RAII-managed window lifetime
- Type-safe window flags with bitwise operations
- Comprehensive property access and manipulation
- Error handling with `sdlpp::expected`
- Support for both hardware and software rendering

## Basic Usage

### Creating a Window

```cpp
#include "sdlpp/window.hh"

// Basic window creation
auto window = sdlpp::window::create("My Application", 800, 600);
if (!window) {
    std::cerr << "Failed to create window: " << window.error() << std::endl;
    return -1;
}

// Window with flags
auto flags = sdlpp::window_flags::resizable | sdlpp::window_flags::opengl;
auto window2 = sdlpp::window::create("OpenGL Window", 1024, 768, flags);

// Centered window
auto window3 = sdlpp::window::create_centered("Centered App", 640, 480);

// Window at specific position
auto window4 = sdlpp::window::create_at("Positioned", 100, 100, 800, 600);
```

### Window Properties

```cpp
// Get window properties
std::string title = window->get_title();
uint32_t id = window->get_id();
auto size = window->get_size();
auto position = window->get_position();

// Set window properties
window->set_title("New Title");
window->set_size(1280, 720);
window->set_position(200, 150);

// Using geometry types
window->set_size(sdlpp::size{1920, 1080});
window->set_position(sdlpp::point{0, 0});
```

### Window State Management

```cpp
// Visibility
window->show();
window->hide();
window->raise();  // Bring to front

// Window state
window->maximize();
window->minimize();
window->restore();

// Fullscreen
window->set_fullscreen(sdlpp::fullscreen_mode::fullscreen);
window->set_fullscreen(sdlpp::fullscreen_mode::windowed);

// Check state
bool is_fullscreen = window->is_fullscreen();
auto flags = window->get_flags();
if ((flags & sdlpp::window_flags::minimized) != sdlpp::window_flags::none) {
    // Window is minimized
}
```

## Advanced Features

### Window Constraints

```cpp
// Set size limits
window->set_minimum_size(640, 480);
window->set_maximum_size(1920, 1080);

// Get current limits
auto min_size = window->get_minimum_size();
auto max_size = window->get_maximum_size();
```

### Window Appearance

```cpp
// Opacity (platform-dependent)
window->set_opacity(0.8f);  // 80% opaque
auto opacity = window->get_opacity();

// Always on top
window->set_always_on_top(true);

// Resizable
window->set_resizable(true);

// Window flash (get user attention)
window->flash(SDL_FLASH_BRIEFLY);
```

### Window Icon

```cpp
// Set window icon from surface
auto icon_surface = sdlpp::surface::load("icon.png");
if (icon_surface) {
    window->set_icon(icon_surface->get());
}
```

## Rendering Setup

### Hardware-Accelerated Rendering

```cpp
// Create window for hardware rendering
auto window = sdlpp::window::create("Game", 1280, 720, 
    sdlpp::window_flags::opengl | sdlpp::window_flags::resizable);

// Create renderer for the window - Method 1: Using window method
auto renderer = window->create_renderer();
if (!renderer) {
    std::cerr << "Failed to create renderer: " << renderer.error() << std::endl;
    return -1;
}

// Method 2: Using renderer::create (equivalent)
auto renderer2 = sdlpp::renderer::create(*window);
if (!renderer2) {
    std::cerr << "Failed to create renderer: " << renderer2.error() << std::endl;
    return -1;
}
```

### Accessing Existing Renderer

```cpp
// Check if window has a renderer
if (window->has_renderer()) {
    // Get raw SDL_Renderer pointer (not owned)
    SDL_Renderer* raw_renderer = window->get_renderer_ptr();
    
    // Use raw pointer with SDL functions if needed
    SDL_SetRenderDrawColor(raw_renderer, 255, 0, 0, 255);
}
```

### Software Rendering

```cpp
// Get window surface for software rendering
auto surface = window->get_surface();
if (!surface) {
    std::cerr << "Failed to get window surface: " << surface.error() << std::endl;
    return -1;
}

// Draw to surface (owned by window, don't free)
// ... perform software rendering ...

// Update window
window->update_surface();

// Or update specific regions
std::vector<SDL_Rect> dirty_rects = {{100, 100, 50, 50}, {200, 200, 100, 100}};
window->update_surface_rects(dirty_rects);
```

## Window Flags Reference

```cpp
// Window creation flags
window_flags::fullscreen        // Fullscreen window
window_flags::opengl           // Window usable with OpenGL context
window_flags::vulkan           // Window usable with Vulkan
window_flags::metal            // Window usable with Metal
window_flags::hidden           // Window is not visible
window_flags::borderless       // No window decoration
window_flags::resizable        // Window can be resized
window_flags::minimized        // Window is minimized
window_flags::maximized        // Window is maximized
window_flags::mouse_grabbed    // Window has grabbed mouse input
window_flags::always_on_top    // Window should always be above others
window_flags::utility          // Window should be treated as a utility window
window_flags::tooltip          // Window should be treated as a tooltip
window_flags::popup_menu       // Window should be treated as a popup menu
window_flags::high_pixel_density // Window should support high-DPI
window_flags::transparent      // Window background is transparent
window_flags::not_focusable    // Window should not be focusable
```

## Display Information

```cpp
// Get display containing the window
auto display_id = window->get_display();
if (display_id) {
    // Use with display module to get display info
}

// Get window pixel format
auto format = window->get_pixel_format();
```

## Common Patterns

### Game Window Setup

```cpp
auto setup_game_window() -> sdlpp::expected<std::pair<sdlpp::window, sdlpp::renderer>, std::string> {
    // Create fullscreen window on primary display
    auto window = TRY(sdlpp::window::create_centered(
        "My Game",
        1920, 1080,
        sdlpp::window_flags::fullscreen | sdlpp::window_flags::opengl
    ));
    
    // Create hardware-accelerated renderer
    auto renderer = TRY(sdlpp::renderer::create(window));
    
    // Set VSync
    TRY(renderer.set_vsync(1));
    
    return std::make_pair(std::move(window), std::move(renderer));
}
```

### Resizable Window with Constraints

```cpp
auto create_editor_window() -> sdlpp::expected<sdlpp::window, std::string> {
    auto window = TRY(sdlpp::window::create(
        "Editor",
        1024, 768,
        sdlpp::window_flags::resizable | sdlpp::window_flags::maximized
    ));
    
    // Set reasonable constraints
    TRY(window.set_minimum_size(800, 600));
    TRY(window.set_maximum_size(3840, 2160));
    
    return window;
}
```

### Multi-Window Application

```cpp
class multi_window_app {
    sdlpp::window main_window;
    std::vector<sdlpp::window> tool_windows;
    
public:
    sdlpp::expected<void, std::string> init() {
        // Create main window
        main_window = TRY(sdlpp::window::create_centered(
            "Main Window", 1280, 720, sdlpp::window_flags::resizable
        ));
        
        // Create tool windows
        auto tools = TRY(sdlpp::window::create(
            "Tools", 300, 600, 
            sdlpp::window_flags::utility | sdlpp::window_flags::always_on_top
        ));
        TRY(tools.set_position(0, 100));
        tool_windows.push_back(std::move(tools));
        
        return {};
    }
    
    void handle_window_event(SDL_WindowEvent& event) {
        // Find which window the event is for
        uint32_t window_id = event.windowID;
        
        if (window_id == main_window.get_id()) {
            // Handle main window events
        } else {
            // Check tool windows
            for (auto& tool : tool_windows) {
                if (window_id == tool.get_id()) {
                    // Handle tool window event
                    break;
                }
            }
        }
    }
};
```

## Error Handling

All window operations return `sdlpp::expected` for consistent error handling:

```cpp
auto result = window->set_opacity(0.5f);
if (!result) {
    // Platform might not support transparency
    std::cerr << "Warning: " << result.error() << std::endl;
}

// Chain operations with error propagation
auto setup_window() -> sdlpp::expected<void, std::string> {
    auto window = TRY(sdlpp::window::create("App", 800, 600));
    TRY(window.set_minimum_size(640, 480));
    TRY(window.show());
    TRY(window.raise());
    return {};
}
```

## Platform Considerations

- **Opacity**: Not supported on all platforms
- **Always on Top**: Behavior varies by window manager
- **Fullscreen**: Different behavior between exclusive and desktop fullscreen
- **High DPI**: Requires special handling on some platforms
- **Window Position**: Window managers may override requested positions
- **Flash**: Implementation varies by platform

## Best Practices

1. **Check Capabilities**: Some features are platform-dependent
2. **Handle Resize Events**: Update rendering when window size changes
3. **Save Window State**: Remember size/position between sessions
4. **Respect System Settings**: Don't force fullscreen or always-on-top
5. **Test Multi-Monitor**: Ensure proper behavior across displays
6. **Clean Shutdown**: Windows are automatically destroyed with RAII