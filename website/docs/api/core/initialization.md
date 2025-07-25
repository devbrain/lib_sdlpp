---
title: Initialization
sidebar_label: Initialization
---

# SDL++ Initialization

SDL++ provides RAII-based initialization for SDL subsystems, ensuring proper cleanup when the application exits.

## Basic Usage

```cpp
#include <sdlpp/core/sdl.hh>

int main() {
    // Initialize SDL with video and audio subsystems
    sdlpp::init sdl_init(sdlpp::init_flags::video | sdlpp::init_flags::audio);
    
    // SDL is now initialized
    // ... your application code ...
    
    // SDL is automatically cleaned up when sdl_init goes out of scope
    return 0;
}
```

## Init Flags

SDL++ provides type-safe initialization flags:

```cpp
namespace sdlpp {
    enum class init_flags : Uint32 {
        none           = 0,
        timer          = SDL_INIT_TIMER,
        audio          = SDL_INIT_AUDIO,
        video          = SDL_INIT_VIDEO,
        joystick       = SDL_INIT_JOYSTICK,
        haptic         = SDL_INIT_HAPTIC,
        gamepad        = SDL_INIT_GAMEPAD,
        events         = SDL_INIT_EVENTS,
        sensor         = SDL_INIT_SENSOR,
        camera         = SDL_INIT_CAMERA,
        everything     = timer | audio | video | joystick | 
                        haptic | gamepad | events | sensor | camera
    };
}
```

## Multiple Subsystem Initialization

You can initialize subsystems separately:

```cpp
// Initialize core systems first
sdlpp::init core_init(sdlpp::init_flags::video | sdlpp::init_flags::events);

// Later, initialize audio when needed
sdlpp::init audio_init(sdlpp::init_flags::audio);

// Each init object manages its own subsystems
```

## Checking Initialization Status

```cpp
// Check if specific subsystems are initialized
if (sdlpp::was_init(sdlpp::init_flags::video)) {
    // Video is initialized
}

// Get all initialized subsystems
auto initialized = sdlpp::was_init(sdlpp::init_flags::everything);
```

## Error Handling

The init constructor doesn't throw exceptions. If initialization fails, the destructor will still be called safely:

```cpp
try {
    sdlpp::init sdl_init(sdlpp::init_flags::video);
    
    // Check if initialization succeeded by trying to create a window
    auto window_result = sdlpp::window::create("Test", 800, 600);
    if (!window_result) {
        std::cerr << "SDL initialization may have failed: " 
                  << window_result.error() << std::endl;
        return 1;
    }
} catch (const std::exception& e) {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
}
```

## Best Practices

1. **Initialize Early**: Create the init object at the beginning of main()
2. **Use Specific Flags**: Only initialize subsystems you need
3. **Single Init Object**: Usually one init object with all needed flags is sufficient
4. **Stack Allocation**: Always create init objects on the stack for RAII

## Example: Conditional Initialization

```cpp
#include <sdlpp/core/sdl.hh>

int main(int argc, char* argv[]) {
    // Determine what to initialize based on command line
    auto flags = sdlpp::init_flags::video | sdlpp::init_flags::events;
    
    bool enable_audio = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--audio") {
            enable_audio = true;
            flags = flags | sdlpp::init_flags::audio;
        }
    }
    
    // Initialize SDL with determined flags
    sdlpp::init sdl_init(flags);
    
    if (enable_audio) {
        std::cout << "Audio subsystem initialized" << std::endl;
    }
    
    return 0;
}
```