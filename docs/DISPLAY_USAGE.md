# Display Module Usage Guide

## Overview

The display module provides a modern C++ wrapper around SDL3's display querying functionality. It allows you to query information about connected displays, available display modes, and display properties.

## Key Classes

### `display`
Represents a single display/monitor with methods to query its properties.

### `display_manager`
Static utility class for querying system displays.

### `display_mode`
Structure containing display mode information (resolution, refresh rate, etc.).

## Basic Usage

### Initialize SDL Video

Before using the display module, you must initialize SDL's video subsystem:

```cpp
#include <sdlpp/sdl.hh>
#include "sdlpp/display.hh"

// Initialize SDL video subsystem
if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    return -1;
}

// Now you can use display functionality
auto displays = sdlpp::display_manager::get_displays();
```

### Query All Displays

```cpp
auto result = sdlpp::display_manager::get_displays();
if (result) {
    const auto& displays = *result;
    std::cout << "Found " << displays.size() << " displays" << std::endl;
    
    for (const auto& display : displays) {
        auto name = display.get_name();
        if (name) {
            std::cout << "Display: " << *name << std::endl;
        }
    }
}
```

### Get Primary Display

```cpp
auto primary = sdlpp::display_manager::get_primary_display();
if (primary) {
    auto name = primary->get_name();
    auto bounds = primary->get_bounds();
    auto scale = primary->get_content_scale();
    
    if (name && bounds && scale) {
        std::cout << "Primary display: " << *name << std::endl;
        std::cout << "Bounds: " << bounds->x << "," << bounds->y 
                  << " " << bounds->w << "x" << bounds->h << std::endl;
        std::cout << "Content scale: " << *scale << std::endl;
    }
}
```

### Query Display Modes

```cpp
auto primary = sdlpp::display_manager::get_primary_display();
if (primary) {
    // Get current mode
    auto current = primary->get_current_mode();
    if (current) {
        std::cout << "Current: " << current->width << "x" << current->height 
                  << " @ " << current->refresh_rate << "Hz" << std::endl;
    }
    
    // Get all fullscreen modes
    auto modes = primary->get_fullscreen_modes();
    if (modes) {
        for (const auto& mode : *modes) {
            std::cout << mode.width << "x" << mode.height 
                      << " @ " << mode.get_precise_refresh_rate() << "Hz";
            if (mode.is_high_dpi()) {
                std::cout << " (HiDPI)";
            }
            std::cout << std::endl;
        }
    }
    
    // Find best mode for specific resolution
    auto best = primary->get_closest_fullscreen_mode(1920, 1080, 60.0f);
    if (best) {
        std::cout << "Best match: " << best->width << "x" << best->height 
                  << " @ " << best->refresh_rate << "Hz" << std::endl;
    }
}
```

### Display for Window Positioning

```cpp
// Find display for a specific point
sdlpp::point cursor_pos{1920, 540};
auto display_for_cursor = sdlpp::display_manager::get_display_for_point(cursor_pos);

// Find best display for a window
sdlpp::rect window_rect{100, 100, 800, 600};
auto display_for_window = sdlpp::display_manager::get_display_for_rect(window_rect);

if (display_for_window) {
    // Get usable bounds (excluding taskbars, etc.)
    auto usable = display_for_window->get_usable_bounds();
    if (usable) {
        // Position window in center of usable area
        int window_x = usable->x + (usable->w - window_rect.w) / 2;
        int window_y = usable->y + (usable->h - window_rect.h) / 2;
    }
}
```

### Display Orientation (Mobile/Tablet)

```cpp
auto display = sdlpp::display_manager::get_primary_display();
if (display) {
    auto current_orient = display->get_current_orientation();
    auto natural_orient = display->get_natural_orientation();
    
    if (current_orient && natural_orient) {
        using sdlpp::display_orientation;
        
        bool is_rotated = (*current_orient == display_orientation::landscape && 
                          *natural_orient == display_orientation::portrait) ||
                         (*current_orient == display_orientation::portrait && 
                          *natural_orient == display_orientation::landscape);
                          
        if (is_rotated) {
            std::cout << "Display is rotated from natural orientation" << std::endl;
        }
    }
}
```

### System Theme Detection

```cpp
auto theme = sdlpp::display_manager::get_system_theme();
if (theme == sdlpp::system_theme::dark) {
    // Use dark theme colors
} else if (theme == sdlpp::system_theme::light) {
    // Use light theme colors
} else {
    // Theme unknown, use default
}
```

## Advanced Usage

### High-DPI Support

```cpp
auto display = sdlpp::display_manager::get_primary_display();
if (display) {
    auto scale = display->get_content_scale();
    if (scale && *scale > 1.0f) {
        std::cout << "High-DPI display with scale: " << *scale << std::endl;
        
        // When creating windows, consider the scale factor
        int logical_width = 800;
        int logical_height = 600;
        int pixel_width = static_cast<int>(logical_width * (*scale));
        int pixel_height = static_cast<int>(logical_height * (*scale));
    }
}
```

### Multi-Monitor Setup

```cpp
auto displays = sdlpp::display_manager::get_displays();
if (displays && displays->size() > 1) {
    std::cout << "Multi-monitor setup detected" << std::endl;
    
    // Find total desktop area
    int min_x = INT_MAX, min_y = INT_MAX;
    int max_x = INT_MIN, max_y = INT_MIN;
    
    for (const auto& display : *displays) {
        auto bounds = display.get_bounds();
        if (bounds) {
            min_x = std::min(min_x, bounds->x);
            min_y = std::min(min_y, bounds->y);
            max_x = std::max(max_x, bounds->x + bounds->w);
            max_y = std::max(max_y, bounds->y + bounds->h);
        }
    }
    
    std::cout << "Total desktop area: " << (max_x - min_x) 
              << "x" << (max_y - min_y) << std::endl;
}
```

### Display Properties

```cpp
auto display = sdlpp::display_manager::get_primary_display();
if (display) {
    auto props = display->get_properties();
    if (props) {
        // Properties can be queried using SDL3 property API
        // Example: HDR capabilities, color space, etc.
    }
}
```

## Error Handling

All methods return `sdlpp::expected` with error messages:

```cpp
auto result = sdlpp::display_manager::get_primary_display();
if (!result) {
    std::cerr << "Error: " << result.error() << std::endl;
    return;
}

auto& display = *result;
// Use display...
```

## Best Practices

1. **Always check SDL initialization**: The display module requires SDL video to be initialized.

2. **Handle missing displays gracefully**: Headless systems may have no displays.

3. **Cache display information**: Display configuration rarely changes during runtime.

4. **Test multi-monitor setups**: Consider displays with negative coordinates.

5. **Respect user preferences**: Use system theme and display scaling.

6. **Validate display modes**: Not all modes may be available for fullscreen.

## Screen Saver Control

The display module includes screen saver management functionality:

### Basic Control

```cpp
// Disable screen saver (e.g., during video playback)
if (sdlpp::screen_saver::disable()) {
    // Screen saver disabled successfully
    play_video();
    
    // Re-enable when done
    sdlpp::screen_saver::enable();
}

// Check current state
bool is_enabled = sdlpp::screen_saver::is_enabled();
```

### RAII Guard

For automatic screen saver management:

```cpp
void play_fullscreen_video() {
    // Screen saver automatically disabled
    sdlpp::screen_saver::guard no_screensaver;
    
    if (no_screensaver.is_active()) {
        // Screen saver was successfully disabled
        render_video_frames();
    }
    
    // Screen saver automatically re-enabled when guard goes out of scope
}
```

### Important Notes

1. **SDL Initialization**: Screen saver functions require SDL video subsystem to be initialized
2. **Default State**: SDL disables the screen saver by default when initialized
3. **Auto-restore**: Screen saver is automatically re-enabled when SDL quits
4. **Thread Safety**: Must be called from the main thread
5. **Use Cases**: Disable for games, video players, presentations; keep enabled otherwise

## Common Patterns

### Window Centering
```cpp
auto center_window_on_display(const sdlpp::display& display, 
                              int window_width, int window_height) 
    -> sdlpp::expected<sdlpp::point, std::string> {
    
    auto bounds = display.get_usable_bounds();
    if (!bounds) {
        return tl::unexpected(bounds.error());
    }
    
    return sdlpp::point{
        bounds->x + (bounds->w - window_width) / 2,
        bounds->y + (bounds->h - window_height) / 2
    };
}
```

### Mode Selection
```cpp
auto find_best_fullscreen_mode(const sdlpp::display& display, float aspect_ratio)
    -> sdlpp::expected<sdlpp::display_mode, std::string> {
    
    auto modes = display.get_fullscreen_modes();
    if (!modes) {
        return tl::unexpected(modes.error());
    }
    
    const float epsilon = 0.01f;
    
    // Find modes matching aspect ratio
    for (const auto& mode : *modes) {
        float mode_aspect = static_cast<float>(mode.width) / mode.height;
        if (std::abs(mode_aspect - aspect_ratio) < epsilon) {
            return mode;
        }
    }
    
    return tl::unexpected("No matching aspect ratio found");
}
```

## Platform Notes

- **Windows**: Display indices may change when monitors are connected/disconnected
- **macOS**: Retina displays report logical coordinates with high pixel density
- **Linux**: Wayland and X11 may report different display configurations
- **Mobile**: Orientation changes trigger display mode changes