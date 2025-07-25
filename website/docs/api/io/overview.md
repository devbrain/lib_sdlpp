---
title: I/O Module Overview
sidebar_label: Overview
sidebar_position: 1
---

# I/O Module Overview

The SDL++ I/O module provides comprehensive file and data I/O capabilities, including synchronous and asynchronous operations, and persistent storage.

## Components

### [IOStream](./iostream)
Unified interface for file and memory I/O operations with SDL's IOStream wrapper.

**Key Features:**
- File reading and writing
- Memory streams
- Binary and text I/O
- Endian-aware operations
- RAII resource management

**Use Cases:**
- Loading game assets
- Saving/loading binary data
- Memory-based I/O operations
- Cross-platform file access

### [Async I/O](./async-io)
High-performance asynchronous file operations for non-blocking I/O.

**Key Features:**
- Non-blocking file operations
- Queue-based task management
- Callback and polling support
- Scatter-gather I/O
- Progress tracking

**Use Cases:**
- Background asset loading
- Large file streaming
- Parallel file processing
- Responsive UI during I/O

### [Storage](./storage)
Cross-platform persistent storage for application preferences and user data.

**Key Features:**
- Platform-specific storage locations
- Key-value storage
- Hierarchical data organization
- Atomic transactions
- Binary data support

**Use Cases:**
- User preferences
- Game saves
- Application settings
- Cache management

## Choosing the Right Component

### When to Use IOStream
- General file I/O operations
- Synchronous reading/writing
- Memory-based streams
- Simple file access patterns

### When to Use Async I/O
- Large file operations
- Background loading
- Maintaining responsive UI
- High-performance I/O

### When to Use Storage
- User preferences
- Application settings
- Small persistent data
- Cross-platform data storage

## Common Patterns

### Asset Loading System

```cpp
class asset_system {
    // Use IOStream for small, immediate loads
    std::shared_ptr<texture> load_texture_sync(const std::string& path) {
        auto stream = sdlpp::iostream::open(path, sdlpp::file_mode::read);
        if (!stream) return nullptr;
        
        return parse_texture(*stream);
    }
    
    // Use Async I/O for large or background loads
    std::future<std::shared_ptr<model>> load_model_async(const std::string& path) {
        return std::async(std::launch::async, [path] {
            // Async loading implementation
        });
    }
    
    // Use Storage for asset metadata cache
    bool is_asset_cached(const std::string& path) {
        return asset_cache.has_key("cache/" + path);
    }
};
```

### Configuration System

```cpp
class config_system {
    sdlpp::storage user_settings;    // User preferences
    sdlpp::iostream config_file;     // Read-only config
    
    void load_configuration() {
        // Load defaults from file
        auto stream = sdlpp::iostream::open("config/defaults.ini", 
                                           sdlpp::file_mode::read);
        
        // Override with user settings
        auto volume = user_settings.get_float("audio/volume", 1.0f);
    }
};
```

## File Mode Reference

```cpp
enum class file_mode {
    read = SDL_IO_RDONLY,      // Read only
    write = SDL_IO_WRONLY,     // Write only (truncate)
    read_write = SDL_IO_RDWR,  // Read and write
    append = SDL_IO_APPEND,    // Append mode
    create = SDL_IO_CREATE     // Create if doesn't exist
};

// Common combinations
auto read_only = sdlpp::file_mode::read;
auto write_new = sdlpp::file_mode::write | sdlpp::file_mode::create;
auto append_log = sdlpp::file_mode::append | sdlpp::file_mode::create;
```

## Error Handling

All I/O operations return `tl::expected` for consistent error handling:

```cpp
// IOStream
auto stream = sdlpp::iostream::open("file.dat", sdlpp::file_mode::read);
if (!stream) {
    std::cerr << "Error: " << stream.error() << std::endl;
}

// Async I/O
auto op = file.read_async(buffer, size, offset);
if (!op.succeeded()) {
    std::cerr << "Async error: " << op.error() << std::endl;
}

// Storage
auto storage = sdlpp::storage::open_user_storage("app", "data");
if (!storage) {
    std::cerr << "Storage error: " << storage.error() << std::endl;
}
```

## Best Practices

1. **Choose the Right Tool**: Use IOStream for simple I/O, Async for performance
2. **Handle Errors**: Always check return values
3. **Use RAII**: Let destructors handle cleanup
4. **Buffer Appropriately**: Don't read/write byte by byte
5. **Consider Platforms**: Test on all target platforms

## Platform Considerations

### Windows
- Storage: `%APPDATA%`
- Path separators: Use `std::filesystem::path`
- Text files: Handle CRLF line endings

### macOS
- Storage: `~/Library/Application Support`
- Permissions: May need entitlements
- Sandboxing: Affects file access

### Linux
- Storage: `~/.local/share` (XDG)
- Permissions: Respect file modes
- Case sensitivity: File names are case-sensitive

### Mobile
- Storage: App-specific directories
- Permissions: May need runtime permissions
- Size limits: Consider storage constraints

## Example: Complete I/O System

```cpp
class game_io_system {
    sdlpp::storage settings;
    sdlpp::async_io_queue async_queue;
    
public:
    game_io_system() 
        : settings(sdlpp::storage::open_user_storage("MyGame", "settings").value()),
          async_queue(sdlpp::async_io_queue::create(16).value()) {}
    
    // Synchronous config loading
    game_config load_config() {
        auto stream = sdlpp::iostream::open("data/config.dat", 
                                           sdlpp::file_mode::read);
        if (!stream) {
            return game_config::defaults();
        }
        
        return parse_config(*stream);
    }
    
    // Asynchronous level loading
    void load_level_async(const std::string& level_id,
                         std::function<void(level_data)> callback) {
        auto path = "levels/" + level_id + ".lvl";
        auto file = sdlpp::async_io::open(path, sdlpp::file_mode::read);
        
        if (!file) {
            callback(level_data{});
            return;
        }
        
        // Load asynchronously
        // ... async implementation
    }
    
    // User preferences
    void save_preferences(const user_preferences& prefs) {
        settings.begin_transaction();
        
        settings.set_float("audio/master", prefs.master_volume);
        settings.set_float("audio/music", prefs.music_volume);
        settings.set_float("audio/sfx", prefs.sfx_volume);
        settings.set_bool("video/fullscreen", prefs.fullscreen);
        settings.set_string("video/resolution", prefs.resolution);
        
        settings.commit();
    }
    
    user_preferences load_preferences() {
        user_preferences prefs;
        
        prefs.master_volume = settings.get_float("audio/master", 1.0f);
        prefs.music_volume = settings.get_float("audio/music", 0.7f);
        prefs.sfx_volume = settings.get_float("audio/sfx", 0.8f);
        prefs.fullscreen = settings.get_bool("video/fullscreen", false);
        prefs.resolution = settings.get_string("video/resolution", "1920x1080");
        
        return prefs;
    }
};
```