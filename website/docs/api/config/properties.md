---
title: Properties System
sidebar_label: Properties
---

# SDL Properties System

SDL++ provides a type-safe wrapper around SDL's properties system (requires SDL 3.2.0+), enabling flexible key-value storage with automatic memory management.

## Overview

The properties system allows attaching arbitrary data to SDL objects and creating custom property containers. It supports various data types including integers, floats, strings, pointers, and boolean values.

## Basic Usage

```cpp
#include <sdlpp/config/properties.hh>

// Create a properties container
auto props_result = sdlpp::properties::create();
if (!props_result) {
    std::cerr << "Failed to create properties: " << props_result.error() << std::endl;
    return;
}

auto& props = props_result.value();

// Set properties
props.set("width", 1920);
props.set("height", 1080);
props.set("title", std::string("My Window"));
props.set("fullscreen", true);
props.set("vsync", 1.0f);

// Get properties
auto width = props.get<int>("width").value_or(800);
auto title = props.get<std::string>("title").value_or("Untitled");
auto fullscreen = props.get<bool>("fullscreen").value_or(false);
```

## Property Types

SDL++ properties support multiple types through a variant system:

```cpp
// Supported types
props.set("integer", 42);                    // int64_t
props.set("float", 3.14f);                   // float  
props.set("string", std::string("hello"));   // std::string
props.set("boolean", true);                  // bool
props.set("pointer", some_ptr, cleanup_fn);  // void* with optional cleanup

// Type-safe retrieval
auto int_val = props.get<int>("integer");
auto float_val = props.get<float>("float");
auto str_val = props.get<std::string>("string");
auto bool_val = props.get<bool>("boolean");
auto ptr_val = props.get<void*>("pointer");
```

## Property IDs

For performance-critical code, use property IDs instead of strings:

```cpp
// Get or create property ID
auto width_id = sdlpp::get_property_id("window.width");
auto height_id = sdlpp::get_property_id("window.height");

// Use IDs for faster access
props.set(width_id, 1920);
props.set(height_id, 1080);

auto w = props.get<int>(width_id);
auto h = props.get<int>(height_id);
```

## Pointer Properties with Cleanup

Store pointers with automatic cleanup:

```cpp
// Custom data with cleanup
struct user_data {
    std::string name;
    int score;
};

// Set pointer with cleanup function
props.set_pointer("user_data", 
    new user_data{"Player", 100},
    [](void* ptr) {
        delete static_cast<user_data*>(ptr);
    }
);

// Retrieve and use
if (auto ptr = props.get<void*>("user_data")) {
    auto* data = static_cast<user_data*>(*ptr);
    std::cout << data->name << ": " << data->score << std::endl;
}

// Cleanup happens automatically when properties are destroyed
// or when the property is overwritten
```

## Property Groups

Organize properties into logical groups:

```cpp
class window_properties {
    sdlpp::properties props;
    
    // Property IDs for performance
    inline static sdlpp::property_id id_width = sdlpp::get_property_id("window.width");
    inline static sdlpp::property_id id_height = sdlpp::get_property_id("window.height");
    inline static sdlpp::property_id id_title = sdlpp::get_property_id("window.title");
    inline static sdlpp::property_id id_fullscreen = sdlpp::get_property_id("window.fullscreen");
    
public:
    window_properties() : props(sdlpp::properties::create().value()) {}
    
    // Typed accessors
    void set_size(int width, int height) {
        props.set(id_width, width);
        props.set(id_height, height);
    }
    
    std::pair<int, int> get_size() const {
        return {
            props.get<int>(id_width).value_or(800),
            props.get<int>(id_height).value_or(600)
        };
    }
    
    void set_title(const std::string& title) {
        props.set(id_title, title);
    }
    
    std::string get_title() const {
        return props.get<std::string>(id_title).value_or("Untitled");
    }
};
```

## Copying Properties

Copy properties between containers:

```cpp
// Create source properties
auto source = sdlpp::properties::create().value();
source.set("name", std::string("Player"));
source.set("level", 10);
source.set("health", 100.0f);

// Create destination and copy
auto dest = sdlpp::properties::create().value();
auto result = sdlpp::copy_properties(source, dest);

if (!result) {
    std::cerr << "Failed to copy properties: " << result.error() << std::endl;
}
```

## Property Enumeration

Iterate over all properties:

```cpp
// Enumerate all properties
props.enumerate([](const std::string& name, sdlpp::property_type type) {
    std::cout << "Property: " << name << " (type: ";
    
    switch (type) {
        case sdlpp::property_type::integer:
            std::cout << "integer";
            break;
        case sdlpp::property_type::float_:
            std::cout << "float";
            break;
        case sdlpp::property_type::string:
            std::cout << "string";
            break;
        case sdlpp::property_type::boolean:
            std::cout << "boolean";
            break;
        case sdlpp::property_type::pointer:
            std::cout << "pointer";
            break;
    }
    
    std::cout << ")" << std::endl;
});
```

## Thread Safety

Properties can be accessed from multiple threads with proper locking:

```cpp
class thread_safe_properties {
    sdlpp::properties props;
    mutable std::shared_mutex mutex;
    
public:
    template<typename T>
    void set(const std::string& name, T&& value) {
        std::unique_lock lock(mutex);
        props.set(name, std::forward<T>(value));
    }
    
    template<typename T>
    std::optional<T> get(const std::string& name) const {
        std::shared_lock lock(mutex);
        return props.get<T>(name);
    }
    
    void lock_and_modify(std::function<void(sdlpp::properties&)> fn) {
        std::unique_lock lock(mutex);
        fn(props);
    }
};
```

## Integration with SDL Objects

Many SDL objects support properties:

```cpp
// Get window properties (if supported)
auto window_props = window.get_properties();
if (window_props) {
    // Access window-specific properties
    auto hwnd = window_props->get<void*>("SDL.window.win32.hwnd");
}

// Get renderer properties
auto renderer_props = renderer.get_properties();
if (renderer_props) {
    auto device = renderer_props->get<void*>("SDL.renderer.d3d12.device");
}
```

## Common Patterns

### Configuration Storage

```cpp
class app_config {
    sdlpp::properties props;
    
public:
    bool load_from_file(const std::string& path) {
        // Load JSON/XML/INI and populate properties
        auto config = parse_config_file(path);
        
        for (const auto& [key, value] : config) {
            std::visit([this, &key](auto&& val) {
                props.set(key, val);
            }, value);
        }
        
        return true;
    }
    
    void save_to_file(const std::string& path) {
        std::ofstream file(path);
        
        props.enumerate([&file, this](const std::string& name, auto type) {
            file << name << " = ";
            
            switch (type) {
                case sdlpp::property_type::integer:
                    file << props.get<int>(name).value();
                    break;
                case sdlpp::property_type::float_:
                    file << props.get<float>(name).value();
                    break;
                case sdlpp::property_type::string:
                    file << props.get<std::string>(name).value();
                    break;
                case sdlpp::property_type::boolean:
                    file << props.get<bool>(name).value();
                    break;
            }
            
            file << "\n";
        });
    }
};
```

### Component System

```cpp
class entity {
    sdlpp::properties components;
    
public:
    template<typename T>
    void add_component(std::unique_ptr<T> component) {
        auto name = std::string(typeid(T).name());
        components.set_pointer(name, component.release(),
            [](void* ptr) { delete static_cast<T*>(ptr); });
    }
    
    template<typename T>
    T* get_component() {
        auto name = std::string(typeid(T).name());
        auto ptr = components.get<void*>(name);
        return ptr ? static_cast<T*>(*ptr) : nullptr;
    }
    
    template<typename T>
    void remove_component() {
        auto name = std::string(typeid(T).name());
        components.set_pointer(name, nullptr, nullptr);
    }
};
```

## Best Practices

1. **Check Version**: Properties require SDL 3.2.0+, always check at runtime
2. **Use IDs**: For frequently accessed properties, use property IDs
3. **Type Safety**: Use the templated get/set methods for type safety
4. **Cleanup Functions**: Always provide cleanup for pointer properties
5. **Error Handling**: Check return values, especially for create()

## Example: Game Settings System

```cpp
class game_settings {
    sdlpp::properties props;
    
    // Property IDs
    struct ids {
        static inline auto graphics_quality = sdlpp::get_property_id("graphics.quality");
        static inline auto graphics_resolution = sdlpp::get_property_id("graphics.resolution");
        static inline auto graphics_fullscreen = sdlpp::get_property_id("graphics.fullscreen");
        static inline auto audio_master_volume = sdlpp::get_property_id("audio.master_volume");
        static inline auto audio_sfx_volume = sdlpp::get_property_id("audio.sfx_volume");
        static inline auto audio_music_volume = sdlpp::get_property_id("audio.music_volume");
        static inline auto controls_sensitivity = sdlpp::get_property_id("controls.sensitivity");
    };
    
public:
    game_settings() {
        auto props_result = sdlpp::properties::create();
        if (!props_result) {
            throw std::runtime_error("Failed to create properties");
        }
        props = std::move(props_result.value());
        
        // Set defaults
        set_defaults();
    }
    
    void set_defaults() {
        props.set(ids::graphics_quality, 2);  // 0=Low, 1=Medium, 2=High
        props.set(ids::graphics_resolution, std::string("1920x1080"));
        props.set(ids::graphics_fullscreen, false);
        props.set(ids::audio_master_volume, 1.0f);
        props.set(ids::audio_sfx_volume, 0.8f);
        props.set(ids::audio_music_volume, 0.6f);
        props.set(ids::controls_sensitivity, 1.0f);
    }
    
    // Graphics settings
    int get_quality() const {
        return props.get<int>(ids::graphics_quality).value_or(2);
    }
    
    void set_quality(int quality) {
        props.set(ids::graphics_quality, std::clamp(quality, 0, 2));
    }
    
    // Audio settings
    float get_master_volume() const {
        return props.get<float>(ids::audio_master_volume).value_or(1.0f);
    }
    
    void set_master_volume(float volume) {
        props.set(ids::audio_master_volume, std::clamp(volume, 0.0f, 1.0f));
    }
    
    // Persistence
    bool save(const std::string& filename) {
        // Implement serialization
        return true;
    }
    
    bool load(const std::string& filename) {
        // Implement deserialization
        return true;
    }
};
```