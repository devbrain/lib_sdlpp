---
title: Hints System
sidebar_label: Hints
---

# SDL Hints System

SDL++ provides a type-safe interface to SDL's hint system, which allows runtime configuration of various SDL behaviors.

## Setting Hints

Hints should typically be set before initializing SDL subsystems:

```cpp
#include <sdlpp/config/hints.hh>

// Set hints before SDL initialization
sdlpp::set_hint(sdlpp::hint::render_scale_quality, "2");  // Linear filtering
sdlpp::set_hint(sdlpp::hint::render_vsync, "1");         // Enable vsync
sdlpp::set_hint(sdlpp::hint::video_allow_screensaver, "0"); // Disable screensaver

// Initialize SDL after setting hints
sdlpp::init sdl_init(sdlpp::init_flags::video);
```

## Common Hints

### Rendering Hints

```cpp
// Texture filtering quality (0=nearest, 1=linear, 2=best)
sdlpp::set_hint(sdlpp::hint::render_scale_quality, "1");

// VSync behavior
sdlpp::set_hint(sdlpp::hint::render_vsync, "1");  // Enable
sdlpp::set_hint(sdlpp::hint::render_vsync, "adaptive"); // Adaptive vsync

// Render driver selection
sdlpp::set_hint(sdlpp::hint::render_driver, "opengl");  // Force OpenGL
sdlpp::set_hint(sdlpp::hint::render_driver, "metal");   // Force Metal (macOS)
sdlpp::set_hint(sdlpp::hint::render_driver, "vulkan");  // Force Vulkan

// Batching behavior  
sdlpp::set_hint(sdlpp::hint::render_batching, "1");  // Enable render batching
```

### Video Hints

```cpp
// Allow high-DPI mode
sdlpp::set_hint(sdlpp::hint::video_highdpi_disabled, "0");

// Minimize on focus loss
sdlpp::set_hint(sdlpp::hint::video_minimize_on_focus_loss, "1");

// Window frame behavior
sdlpp::set_hint(sdlpp::hint::video_window_share_pixel_format, "1");

// Screensaver control
sdlpp::set_hint(sdlpp::hint::video_allow_screensaver, "0");  // Disable
```

### Input Hints

```cpp
// Mouse behavior
sdlpp::set_hint(sdlpp::hint::mouse_relative_mode_warp, "1");
sdlpp::set_hint(sdlpp::hint::mouse_focus_clickthrough, "1");
sdlpp::set_hint(sdlpp::hint::mouse_touch_events, "0");  // Disable touch->mouse

// Touch behavior
sdlpp::set_hint(sdlpp::hint::touch_mouse_events, "1");  // Enable touch->mouse

// Gamepad/Controller
sdlpp::set_hint(sdlpp::hint::gamecontroller_use_button_labels, "1");
sdlpp::set_hint(sdlpp::hint::joystick_allow_background_events, "1");
```

### Platform-Specific Hints

```cpp
// Android
#ifdef __ANDROID__
sdlpp::set_hint(sdlpp::hint::android_block_on_pause, "0");
sdlpp::set_hint(sdlpp::hint::android_trap_back_button, "1");
#endif

// iOS
#ifdef __IPHONEOS__
sdlpp::set_hint(sdlpp::hint::ios_hide_home_indicator, "1");
sdlpp::set_hint(sdlpp::hint::accelerometer_as_joystick, "0");
#endif

// Windows
#ifdef _WIN32
sdlpp::set_hint(sdlpp::hint::windows_disable_thread_naming, "0");
#endif
```

## Getting Hint Values

```cpp
// Get current hint value
std::string quality = sdlpp::get_hint(sdlpp::hint::render_scale_quality);
if (quality.empty()) {
    // Hint not set, using default
}

// Get with default value
std::string vsync = sdlpp::get_hint_with_default(
    sdlpp::hint::render_vsync, "1"
);
```

## Hint Callbacks

Monitor hint changes with callbacks:

```cpp
class hint_watcher {
    sdlpp::hint_callback_id callback_id;
    
public:
    void start_watching() {
        callback_id = sdlpp::add_hint_callback(
            sdlpp::hint::render_scale_quality,
            [](const std::string& name, 
               const std::string& old_value,
               const std::string& new_value) {
                std::cout << "Render quality changed from " 
                         << old_value << " to " << new_value << std::endl;
                         
                // React to change
                if (new_value == "2") {
                    reload_textures_with_better_filtering();
                }
            }
        );
    }
    
    ~hint_watcher() {
        // Callback automatically removed when id goes out of scope
    }
};
```

## Hint Priority Levels

Set hints with specific priority:

```cpp
enum class hint_priority {
    default_priority = SDL_HINT_DEFAULT,
    normal = SDL_HINT_NORMAL,
    override = SDL_HINT_OVERRIDE
};

// Normal priority (can be overridden)
sdlpp::set_hint_with_priority(
    sdlpp::hint::render_vsync, "1", 
    sdlpp::hint_priority::normal
);

// Override priority (cannot be changed by normal priority sets)
sdlpp::set_hint_with_priority(
    sdlpp::hint::render_driver, "opengl",
    sdlpp::hint_priority::override
);
```

## RAII Hint Scope

Temporarily change hints with automatic restoration:

```cpp
{
    // Temporarily disable screensaver
    sdlpp::hint_scope no_screensaver(
        sdlpp::hint::video_allow_screensaver, "0"
    );
    
    // Play video or run benchmark
    play_fullscreen_video();
    
    // Screensaver setting automatically restored
}

// Multiple hints
{
    sdlpp::hint_scope_multiple hints({
        {sdlpp::hint::render_vsync, "0"},           // Disable vsync
        {sdlpp::hint::render_scale_quality, "0"},   // Fastest filtering
        {sdlpp::hint::render_batching, "0"}         // Disable batching
    });
    
    run_performance_test();
    
    // All hints restored
}
```

## Common Hint Patterns

### Performance Profile Manager

```cpp
class performance_profile {
public:
    enum class profile {
        quality,
        balanced,
        performance
    };
    
    static void apply_profile(profile p) {
        switch (p) {
        case profile::quality:
            sdlpp::set_hint(sdlpp::hint::render_scale_quality, "2");
            sdlpp::set_hint(sdlpp::hint::render_vsync, "1");
            sdlpp::set_hint(sdlpp::hint::render_batching, "1");
            break;
            
        case profile::balanced:
            sdlpp::set_hint(sdlpp::hint::render_scale_quality, "1");
            sdlpp::set_hint(sdlpp::hint::render_vsync, "adaptive");
            sdlpp::set_hint(sdlpp::hint::render_batching, "1");
            break;
            
        case profile::performance:
            sdlpp::set_hint(sdlpp::hint::render_scale_quality, "0");
            sdlpp::set_hint(sdlpp::hint::render_vsync, "0");
            sdlpp::set_hint(sdlpp::hint::render_batching, "0");
            break;
        }
    }
};
```

### Configuration File Integration

```cpp
class hint_config {
public:
    void load_from_file(const std::string& path) {
        auto config = parse_config_file(path);
        
        for (const auto& [key, value] : config) {
            if (is_valid_hint(key)) {
                sdlpp::set_hint(key, value);
            }
        }
    }
    
    void save_to_file(const std::string& path) {
        std::ofstream file(path);
        
        for (const auto& hint : get_all_hints()) {
            auto value = sdlpp::get_hint(hint);
            if (!value.empty()) {
                file << hint << "=" << value << "\n";
            }
        }
    }
};
```

### Platform Adaptive Hints

```cpp
class platform_hints {
public:
    static void apply_platform_defaults() {
        #ifdef __ANDROID__
        // Android optimizations
        sdlpp::set_hint(sdlpp::hint::android_block_on_pause, "0");
        sdlpp::set_hint(sdlpp::hint::accelerometer_as_joystick, "0");
        sdlpp::set_hint(sdlpp::hint::render_batching, "1");
        #endif
        
        #ifdef __IPHONEOS__
        // iOS optimizations
        sdlpp::set_hint(sdlpp::hint::ios_hide_home_indicator, "2");
        sdlpp::set_hint(sdlpp::hint::render_driver, "metal");
        #endif
        
        #ifdef _WIN32
        // Windows optimizations
        sdlpp::set_hint(sdlpp::hint::windows_no_close_on_alt_f4, "0");
        #endif
        
        #ifdef __linux__
        // Linux optimizations
        sdlpp::set_hint(sdlpp::hint::video_x11_net_wm_bypass_compositor, "0");
        #endif
    }
};
```

## Best Practices

1. **Set Early**: Set hints before initializing related subsystems
2. **Document Defaults**: Document what hints your application expects
3. **Allow User Override**: Let users configure hints via settings
4. **Test Combinations**: Test different hint combinations on target platforms
5. **Use Scoped Changes**: Use RAII for temporary hint modifications

## Example: Complete Hint Configuration

```cpp
class app_hints {
    struct hint_def {
        std::string name;
        std::string default_value;
        std::string description;
    };
    
    std::vector<hint_def> app_hints = {
        {sdlpp::hint::render_scale_quality, "1", "Texture filtering quality"},
        {sdlpp::hint::render_vsync, "1", "Vertical sync"},
        {sdlpp::hint::render_batching, "1", "Render batching"},
        {sdlpp::hint::video_allow_screensaver, "0", "Allow screensaver"},
        {sdlpp::hint::mouse_focus_clickthrough, "1", "Click through unfocused window"}
    };
    
public:
    void apply_defaults() {
        for (const auto& hint : app_hints) {
            if (sdlpp::get_hint(hint.name).empty()) {
                sdlpp::set_hint(hint.name, hint.default_value);
            }
        }
    }
    
    void apply_user_preferences(const user_settings& settings) {
        // Apply performance profile
        performance_profile::apply_profile(settings.performance_mode);
        
        // Apply specific overrides
        if (settings.force_vsync) {
            sdlpp::set_hint_with_priority(
                sdlpp::hint::render_vsync, "1",
                sdlpp::hint_priority::override
            );
        }
        
        // Platform-specific
        platform_hints::apply_platform_defaults();
    }
    
    void log_active_hints() {
        std::cout << "Active SDL Hints:\n";
        for (const auto& hint : app_hints) {
            auto value = sdlpp::get_hint(hint.name);
            std::cout << "  " << hint.name << " = " 
                     << (value.empty() ? "<default>" : value)
                     << " (" << hint.description << ")\n";
        }
    }
};

// Usage
int main() {
    app_hints hints;
    hints.apply_defaults();
    hints.apply_user_preferences(load_user_settings());
    hints.log_active_hints();
    
    sdlpp::init sdl_init(sdlpp::init_flags::everything);
    // ... rest of application
}
```