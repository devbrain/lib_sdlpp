---
title: Platform Detection
sidebar_label: Platform
---

# Platform Detection and System Information

SDL++ provides comprehensive platform detection and system information APIs to help write cross-platform code.

## Platform Detection

### Compile-Time Platform Detection

```cpp
#include <sdlpp/system/platform.hh>

// Platform macros
#ifdef SDLPP_PLATFORM_WINDOWS
    // Windows-specific code
#endif

#ifdef SDLPP_PLATFORM_MACOS
    // macOS-specific code
#endif

#ifdef SDLPP_PLATFORM_LINUX
    // Linux-specific code
#endif

#ifdef SDLPP_PLATFORM_IOS
    // iOS-specific code
#endif

#ifdef SDLPP_PLATFORM_ANDROID
    // Android-specific code
#endif

#ifdef SDLPP_PLATFORM_EMSCRIPTEN
    // Web/Emscripten-specific code
#endif
```

### Runtime Platform Detection

```cpp
// Get platform name
std::string platform = sdlpp::get_platform();
// Returns: "Windows", "Mac OS X", "Linux", "iOS", "Android", etc.

// Platform-specific checks
if (sdlpp::platform::is_windows()) {
    // Windows-specific behavior
}

if (sdlpp::platform::is_mobile()) {
    // Mobile platform (iOS, Android)
    enable_touch_controls();
}

if (sdlpp::platform::is_desktop()) {
    // Desktop platform (Windows, macOS, Linux)
    enable_keyboard_shortcuts();
}

if (sdlpp::platform::is_web()) {
    // Running in web browser
    limit_memory_usage();
}
```

## Device Type Detection

```cpp
// Check device type
if (sdlpp::platform::is_tablet()) {
    // Tablet-specific UI
    increase_button_sizes();
}

if (sdlpp::platform::is_tv()) {
    // TV/Console interface
    enable_controller_navigation();
}

// Get device orientation (mobile)
auto orientation = sdlpp::platform::get_device_orientation();
switch (orientation) {
    case sdlpp::device_orientation::landscape:
        // Landscape layout
        break;
    case sdlpp::device_orientation::portrait:
        // Portrait layout
        break;
}
```

## System Directories

```cpp
// Get standard system directories
auto home_dir = sdlpp::get_base_path();        // Application directory
auto pref_dir = sdlpp::get_pref_path("MyCompany", "MyApp"); // User data

// Platform-specific directories
#ifdef SDLPP_PLATFORM_WINDOWS
auto appdata = sdlpp::platform::get_windows_appdata_path();
auto documents = sdlpp::platform::get_windows_documents_path();
#endif

#ifdef SDLPP_PLATFORM_MACOS
auto app_support = sdlpp::platform::get_macos_application_support_path();
#endif

#ifdef SDLPP_PLATFORM_LINUX
auto config_home = sdlpp::platform::get_xdg_config_home();
auto data_home = sdlpp::platform::get_xdg_data_home();
#endif
```

## System Information

### OS Version

```cpp
// Get OS version info
auto os_info = sdlpp::platform::get_os_info();
std::cout << "OS: " << os_info.name << std::endl;
std::cout << "Version: " << os_info.version << std::endl;
std::cout << "Build: " << os_info.build << std::endl;

// Version comparison
if (sdlpp::platform::is_windows()) {
    auto version = sdlpp::platform::get_windows_version();
    if (version >= sdlpp::windows_version::windows_10) {
        // Use Windows 10+ features
    }
}

#ifdef SDLPP_PLATFORM_ANDROID
auto api_level = sdlpp::platform::get_android_api_level();
if (api_level >= 29) {  // Android 10+
    // Use newer Android features
}
#endif
```

### Hardware Information

```cpp
// System RAM
auto ram_mb = sdlpp::get_system_ram();
std::cout << "System RAM: " << ram_mb << " MB" << std::endl;

// CPU information
auto cpu_count = sdlpp::get_cpu_count();
auto cpu_cache_line = sdlpp::get_cpu_cache_line_size();

std::cout << "CPU cores: " << cpu_count << std::endl;
std::cout << "Cache line size: " << cpu_cache_line << " bytes" << std::endl;

// Check CPU features
if (sdlpp::has_sse()) {
    // Use SSE optimizations
}

if (sdlpp::has_avx()) {
    // Use AVX optimizations
}

if (sdlpp::has_neon()) {
    // Use ARM NEON optimizations
}
```

## Power Information

```cpp
// Get power/battery status
auto power_info = sdlpp::get_power_info();

switch (power_info.state) {
    case sdlpp::power_state::on_battery:
        std::cout << "Running on battery" << std::endl;
        if (power_info.percent) {
            std::cout << "Battery: " << *power_info.percent << "%" << std::endl;
        }
        if (power_info.seconds) {
            std::cout << "Time remaining: " << *power_info.seconds / 60 << " minutes" << std::endl;
        }
        break;
        
    case sdlpp::power_state::charging:
        std::cout << "Charging" << std::endl;
        break;
        
    case sdlpp::power_state::charged:
        std::cout << "Fully charged" << std::endl;
        break;
        
    case sdlpp::power_state::no_battery:
        std::cout << "No battery (desktop)" << std::endl;
        break;
}

// Adjust behavior based on power
if (power_info.state == sdlpp::power_state::on_battery && 
    power_info.percent && *power_info.percent < 20) {
    // Low battery mode
    reduce_frame_rate();
    dim_screen();
}
```

## Display Information

```cpp
// Get display information
auto displays = sdlpp::get_displays();
std::cout << "Number of displays: " << displays.size() << std::endl;

for (const auto& display : displays) {
    auto bounds = sdlpp::get_display_bounds(display);
    auto usable = sdlpp::get_display_usable_bounds(display);
    auto name = sdlpp::get_display_name(display);
    
    std::cout << "Display: " << name << std::endl;
    std::cout << "  Bounds: " << bounds.w << "x" << bounds.h 
              << " at " << bounds.x << "," << bounds.y << std::endl;
    std::cout << "  Usable: " << usable.w << "x" << usable.h << std::endl;
    
    // Get display modes
    auto modes = sdlpp::get_display_modes(display);
    for (const auto& mode : modes) {
        std::cout << "  Mode: " << mode.w << "x" << mode.h 
                  << " @ " << mode.refresh_rate << "Hz" << std::endl;
    }
    
    // Get DPI
    auto [hdpi, vdpi, ddpi] = sdlpp::get_display_dpi(display);
    std::cout << "  DPI: " << hdpi << " (h), " << vdpi << " (v), " 
              << ddpi << " (diagonal)" << std::endl;
}
```

## Platform-Specific Features

### Windows

```cpp
#ifdef SDLPP_PLATFORM_WINDOWS
// Get window handle
HWND hwnd = sdlpp::platform::get_window_hwnd(window);

// Dark mode detection
if (sdlpp::platform::is_windows_dark_mode()) {
    apply_dark_theme();
}

// High contrast mode
if (sdlpp::platform::is_windows_high_contrast()) {
    apply_high_contrast_theme();
}
#endif
```

### macOS

```cpp
#ifdef SDLPP_PLATFORM_MACOS
// Get NSWindow
NSWindow* ns_window = sdlpp::platform::get_nswindow(window);

// Check for Retina display
if (sdlpp::platform::is_retina_display()) {
    use_high_res_assets();
}
#endif
```

### Linux

```cpp
#ifdef SDLPP_PLATFORM_LINUX
// Check display server
if (sdlpp::platform::is_wayland()) {
    std::cout << "Running on Wayland" << std::endl;
} else if (sdlpp::platform::is_x11()) {
    std::cout << "Running on X11" << std::endl;
    
    // Get X11 display
    Display* x_display = sdlpp::platform::get_x11_display();
}

// Desktop environment detection
auto desktop = sdlpp::platform::get_desktop_environment();
// Returns: "GNOME", "KDE", "XFCE", etc.
#endif
```

### Mobile Platforms

```cpp
#ifdef SDLPP_PLATFORM_IOS
// Check device type
if (sdlpp::platform::is_ipad()) {
    // iPad-specific features
}

// Safe area insets (for notched devices)
auto insets = sdlpp::platform::get_safe_area_insets();
// Adjust UI for safe areas
#endif

#ifdef SDLPP_PLATFORM_ANDROID
// Get JNI environment
JNIEnv* env = sdlpp::platform::get_jni_env();

// Check permissions
if (!sdlpp::platform::has_android_permission("android.permission.CAMERA")) {
    sdlpp::platform::request_android_permission("android.permission.CAMERA");
}

// Get external storage paths
auto external_storage = sdlpp::platform::get_android_external_storage_path();
#endif
```

## Common Platform Patterns

### Cross-Platform File Paths

```cpp
class platform_paths {
public:
    static std::filesystem::path get_save_directory(const std::string& app_name) {
        #ifdef SDLPP_PLATFORM_WINDOWS
        return std::filesystem::path(std::getenv("APPDATA")) / app_name;
        
        #elif defined(SDLPP_PLATFORM_MACOS)
        return std::filesystem::path(std::getenv("HOME")) / 
               "Library" / "Application Support" / app_name;
               
        #elif defined(SDLPP_PLATFORM_LINUX)
        auto xdg_data = std::getenv("XDG_DATA_HOME");
        if (xdg_data) {
            return std::filesystem::path(xdg_data) / app_name;
        }
        return std::filesystem::path(std::getenv("HOME")) / ".local" / "share" / app_name;
        
        #else
        return sdlpp::get_pref_path("", app_name);
        #endif
    }
};
```

### Platform-Specific Features

```cpp
class platform_features {
    struct features {
        bool dark_mode = false;
        bool reduced_motion = false;
        bool high_contrast = false;
        float ui_scale = 1.0f;
    };
    
public:
    static features detect() {
        features f;
        
        #ifdef SDLPP_PLATFORM_WINDOWS
        f.dark_mode = sdlpp::platform::is_windows_dark_mode();
        f.high_contrast = sdlpp::platform::is_windows_high_contrast();
        f.ui_scale = sdlpp::platform::get_windows_dpi_scale();
        #endif
        
        #ifdef SDLPP_PLATFORM_MACOS
        f.dark_mode = sdlpp::platform::is_macos_dark_mode();
        f.reduced_motion = sdlpp::platform::prefers_reduced_motion();
        #endif
        
        // Mobile platforms
        if (sdlpp::platform::is_mobile()) {
            f.reduced_motion = sdlpp::platform::prefers_reduced_motion();
        }
        
        return f;
    }
};
```

### Platform Capabilities

```cpp
class platform_caps {
public:
    static void log_capabilities() {
        std::cout << "Platform: " << sdlpp::get_platform() << std::endl;
        std::cout << "CPU cores: " << sdlpp::get_cpu_count() << std::endl;
        std::cout << "RAM: " << sdlpp::get_system_ram() << " MB" << std::endl;
        
        // Graphics capabilities
        if (sdlpp::platform::supports_metal()) {
            std::cout << "Metal supported" << std::endl;
        }
        if (sdlpp::platform::supports_vulkan()) {
            std::cout << "Vulkan supported" << std::endl;
        }
        
        // Input capabilities
        if (sdlpp::platform::has_keyboard()) {
            std::cout << "Keyboard available" << std::endl;
        }
        if (sdlpp::platform::has_mouse()) {
            std::cout << "Mouse available" << std::endl;
        }
        if (sdlpp::platform::has_touch()) {
            std::cout << "Touch input available" << std::endl;
        }
    }
};
```

## Best Practices

1. **Check at Runtime**: Always verify platform features at runtime
2. **Graceful Fallbacks**: Provide alternatives for platform-specific features
3. **Test on Target**: Test on actual devices, not just simulators
4. **Respect System Settings**: Honor dark mode, reduced motion, etc.
5. **Handle Permissions**: Check and request permissions on mobile

## Example: Platform-Aware Application

```cpp
class platform_aware_app {
    platform_features features;
    
public:
    void initialize() {
        // Detect platform features
        features = platform_features::detect();
        
        // Apply platform-specific settings
        apply_platform_theme();
        configure_input();
        setup_directories();
    }
    
private:
    void apply_platform_theme() {
        if (features.dark_mode) {
            set_theme(themes::dark);
        }
        
        if (features.high_contrast) {
            enable_high_contrast();
        }
        
        set_ui_scale(features.ui_scale);
    }
    
    void configure_input() {
        if (sdlpp::platform::has_touch()) {
            enable_touch_controls();
            increase_hit_targets();
        }
        
        if (sdlpp::platform::is_tv()) {
            enable_gamepad_navigation();
            show_button_prompts();
        }
        
        if (sdlpp::platform::has_keyboard()) {
            enable_keyboard_shortcuts();
        }
    }
    
    void setup_directories() {
        auto save_dir = platform_paths::get_save_directory("MyApp");
        std::filesystem::create_directories(save_dir);
        
        // Set up platform-specific paths
        #ifdef SDLPP_PLATFORM_ANDROID
        // Use external storage for large files
        auto external = sdlpp::platform::get_android_external_storage_path();
        if (!external.empty()) {
            set_download_directory(external);
        }
        #endif
    }
    
    void adapt_to_power_state() {
        auto power = sdlpp::get_power_info();
        
        if (power.state == sdlpp::power_state::on_battery) {
            // Battery mode
            set_frame_rate(30);  // Reduce from 60
            disable_background_animations();
            
            if (power.percent && *power.percent < 20) {
                // Critical battery
                enable_power_saving_mode();
            }
        } else {
            // Plugged in
            set_frame_rate(60);
            enable_all_features();
        }
    }
};
```