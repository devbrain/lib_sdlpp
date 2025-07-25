---
title: Version Information
sidebar_label: Version
---

# Version Information

SDL++ provides utilities to check both compile-time and runtime SDL version information, useful for feature detection and compatibility checks.

## Version Structure

```cpp
namespace sdlpp {
    struct version {
        Uint8 major;
        Uint8 minor;
        Uint8 patch;
        
        // Comparison operators
        auto operator<=>(const version&) const = default;
        
        // Convert to string
        std::string to_string() const;
    };
}
```

## Compile-Time Version

Get the version of SDL headers used during compilation:

```cpp
#include <sdlpp/core/version.hh>

// Get compile-time version
auto compile_version = sdlpp::version_info::compile_time;

std::cout << "Compiled against SDL " 
          << compile_version.to_string() << std::endl;

// Check minimum version at compile time
static_assert(sdlpp::version_info::compile_time >= sdlpp::version{3, 0, 0},
              "SDL 3.0.0 or higher required");
```

## Runtime Version

Get the version of SDL library at runtime:

```cpp
// Get runtime version
auto runtime_version = sdlpp::version_info::runtime();

std::cout << "Running with SDL " 
          << runtime_version.to_string() << std::endl;

// Check for version mismatch
if (runtime_version < sdlpp::version_info::compile_time) {
    std::cerr << "Warning: Runtime SDL version is older than compile-time version" 
              << std::endl;
}
```

## Feature Detection

Use version checking for feature availability:

```cpp
// Check if a feature is available
bool has_vulkan_support() {
    // Vulkan support added in SDL 2.0.6
    return sdlpp::version_info::runtime() >= sdlpp::version{2, 0, 6};
}

// Conditional feature usage
void create_window_with_optional_features() {
    auto flags = sdlpp::window::flags::shown;
    
    // Metal support added in SDL 2.0.12
    if (sdlpp::version_info::runtime() >= sdlpp::version{2, 0, 12}) {
        flags |= sdlpp::window::flags::metal;
    }
    
    auto window = sdlpp::window::create("App", 800, 600, flags);
}
```

## Version Utilities

### String Conversion

```cpp
auto version = sdlpp::version{3, 1, 0};
std::string ver_str = version.to_string();  // "3.1.0"

// Format for display
std::cout << "SDL++ using SDL " << sdlpp::version_info::runtime().to_string() 
          << " (compiled with " << sdlpp::version_info::compile_time.to_string() 
          << ")" << std::endl;
```

### Version Comparison

```cpp
auto v1 = sdlpp::version{3, 0, 0};
auto v2 = sdlpp::version{3, 0, 1};

if (v1 < v2) {
    std::cout << v1.to_string() << " is older than " << v2.to_string() << std::endl;
}

// Check exact version
if (sdlpp::version_info::runtime() == sdlpp::version{3, 0, 0}) {
    std::cout << "Running exactly SDL 3.0.0" << std::endl;
}

// Check minimum version
if (sdlpp::version_info::runtime() >= sdlpp::version{3, 1, 0}) {
    std::cout << "Running SDL 3.1.0 or newer" << std::endl;
}
```

## Platform-Specific Version Info

```cpp
void print_detailed_version_info() {
    std::cout << "SDL++ Version Information:" << std::endl;
    std::cout << "-------------------------" << std::endl;
    
    auto compile_ver = sdlpp::version_info::compile_time;
    auto runtime_ver = sdlpp::version_info::runtime();
    
    std::cout << "Compile-time SDL: " << compile_ver.to_string() << std::endl;
    std::cout << "Runtime SDL: " << runtime_ver.to_string() << std::endl;
    
    // SDL revision (git commit hash)
    std::cout << "SDL Revision: " << sdlpp::get_revision() << std::endl;
    
    // Platform
    std::cout << "Platform: " << sdlpp::get_platform() << std::endl;
    
    // Check ABI compatibility
    if (compile_ver.major != runtime_ver.major) {
        std::cerr << "WARNING: Major version mismatch - ABI incompatible!" 
                  << std::endl;
    }
}
```

## Version Requirements Pattern

```cpp
// Define required features with versions
struct feature_requirements {
    sdlpp::version min_version;
    std::string feature_name;
    bool required;
};

bool check_requirements() {
    std::vector<feature_requirements> requirements = {
        {{3, 0, 0}, "Base SDL3 support", true},
        {{3, 1, 0}, "Enhanced GPU API", false},
        {{3, 2, 0}, "Properties system", false}
    };
    
    auto current = sdlpp::version_info::runtime();
    bool all_required_met = true;
    
    for (const auto& req : requirements) {
        bool available = current >= req.min_version;
        
        std::cout << req.feature_name << ": " 
                  << (available ? "Available" : "Not available")
                  << " (requires " << req.min_version.to_string() << ")"
                  << std::endl;
        
        if (req.required && !available) {
            all_required_met = false;
        }
    }
    
    return all_required_met;
}
```

## Best Practices

1. **Always Check Runtime Version**: Don't assume runtime matches compile-time
2. **Graceful Degradation**: Provide fallbacks for newer features
3. **Document Requirements**: Clearly state minimum SDL version needed
4. **Test Multiple Versions**: Test with different SDL versions if supporting range
5. **ABI Compatibility**: Major version changes break ABI compatibility

## Example: Version-Aware Application

```cpp
class version_aware_app {
public:
    bool initialize() {
        // Check minimum version
        const auto min_required = sdlpp::version{3, 0, 0};
        if (sdlpp::version_info::runtime() < min_required) {
            std::cerr << "This application requires SDL " 
                      << min_required.to_string() << " or higher. "
                      << "You have " << sdlpp::version_info::runtime().to_string() 
                      << std::endl;
            return false;
        }
        
        // Log version info
        std::cout << "Initializing with SDL " 
                  << sdlpp::version_info::runtime().to_string() << std::endl;
        
        // Enable version-specific features
        enable_optional_features();
        
        return true;
    }
    
private:
    void enable_optional_features() {
        // GPU API in SDL 3.1.0+
        if (sdlpp::version_info::runtime() >= sdlpp::version{3, 1, 0}) {
            std::cout << "Enabling GPU API support" << std::endl;
            use_gpu_api = true;
        }
        
        // Properties system in SDL 3.2.0+
        if (sdlpp::version_info::runtime() >= sdlpp::version{3, 2, 0}) {
            std::cout << "Enabling properties system" << std::endl;
            use_properties = true;
        }
    }
    
    bool use_gpu_api = false;
    bool use_properties = false;
};
```