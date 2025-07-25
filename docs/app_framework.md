# SDL++ Application Framework

The SDL++ application framework provides a modern C++ wrapper around SDL3's new application model (`SDL_AppInit`, `SDL_AppIterate`, `SDL_AppEvent`, `SDL_AppQuit`). It offers multiple paradigms for structuring applications while maintaining type safety and RAII principles.

## Overview

SDL3 introduces a new application model where instead of writing your own `main()` function, you implement callback functions that SDL calls at appropriate times. SDL++ wraps this model to provide a more C++-friendly interface.

## Quick Start

### Basic Class-Based Application

```cpp
#include <sdlpp/app/app.hh>
#include <sdlpp/app/app_impl.hh>

class my_app : public sdlpp::application {
protected:
    bool on_init() override {
        std::cout << "App initialized!\n";
        return true;
    }
    
    void on_frame() override {
        auto& renderer = get_renderer();
        renderer.clear();
        // Draw your content
    }
    
    bool on_event(const sdlpp::event& e) override {
        if (e.is<sdlpp::quit_event>()) {
            return false; // Stop the app
        }
        return true;
    }
};

SDLPP_MAIN(my_app)
```

### Function-Based Application

```cpp
#include <sdlpp/app/app_builder.hh>

int main(int argc, char* argv[]) {
    return sdlpp::app_builder()
        .with_window("My App", 800, 600)
        .with_renderer()
        .on_frame([]() {
            // Render frame
            return true;
        })
        .on_event([](const sdlpp::event& e) {
            return e.type() != sdlpp::event_type::quit;
        })
        .run(argc, argv);
}
```

## Core Concepts

### Application Lifecycle

1. **Initialization** (`SDL_AppInit` → `init()`)
   - SDL is initialized
   - Window and renderer are created (if configured)
   - Your `on_init()` is called
   - Return `false` to abort startup

2. **Main Loop** (`SDL_AppIterate` → `iterate()`)
   - Called repeatedly while app is running
   - Processes events from the queue
   - Calls your `on_frame()`
   - Updates timing information
   - Return `false` to quit

3. **Event Handling** (`SDL_AppEvent` → `event()`)
   - Called for each SDL event
   - Wrapped in type-safe `sdlpp::event`
   - Return `false` to quit

4. **Cleanup** (`SDL_AppQuit` → `quit()`)
   - Your `on_quit()` is called
   - Resources are cleaned up in reverse order

### Return Values

The SDL3 callbacks use `SDL_AppResult`:
- `SDL_APP_CONTINUE` - Continue running
- `SDL_APP_SUCCESS` - Quit gracefully (exit code 0)
- `SDL_APP_FAILURE` - Quit with error (exit code 1)

SDL++ translates boolean returns to these values:
- `true` → `SDL_APP_CONTINUE`
- `false` in init → `SDL_APP_FAILURE`
- `false` in iterate/event → `SDL_APP_SUCCESS`

## Application Types

### 1. Basic Application (`application`)

The base class providing common functionality:

```cpp
class application : public app_interface {
    // Configuration
    struct config {
        init_flags sdl_flags = init_flags::video | init_flags::events;
        std::string window_title = "SDL++ Application";
        int window_width = 1280;
        int window_height = 720;
        window_flags window_flags = window_flags::resizable;
        renderer_flags renderer_flags = renderer_flags::accelerated;
        bool auto_create_window = true;
        bool auto_create_renderer = true;
        bool handle_quit_event = true;
    };
    
    // Hooks for derived classes
    virtual bool on_init() { return true; }
    virtual void on_frame() {}
    virtual bool on_event(const event& e) { return true; }
    virtual void on_quit() {}
    
    // Utilities
    window& get_window();
    renderer& get_renderer();
    float get_delta_time() const;
    float get_fps() const;
    void request_quit();
};
```

### 2. Stateful Application (`stateful_application<T>`)

Manages application state with type safety:

```cpp
struct game_state {
    int score = 0;
    float player_x = 100.0f;
    bool paused = false;
};

class my_game : public sdlpp::stateful_application<game_state> {
protected:
    void on_frame() override {
        if (!state().paused) {
            state().player_x += 100.0f * get_delta_time();
        }
    }
};
```

### 3. Scene-Based Application (`scene_application`)

Manages a stack of scenes (screens/states):

```cpp
class menu_scene : public sdlpp::scene {
    void on_enter() override { /* Initialize menu */ }
    void render(sdlpp::renderer& r) override { /* Draw menu */ }
    bool handle_event(const sdlpp::event& e) override {
        if (/* start game */) {
            app().push_scene<game_scene>();
        }
        return true;
    }
};

class my_app : public sdlpp::scene_application {
    bool on_init() override {
        push_scene<menu_scene>();
        return true;
    }
};
```

### 4. Game Application (`game_application`)

Fixed timestep game loop for consistent physics:

```cpp
class physics_game : public sdlpp::game_application {
protected:
    void fixed_update(float dt) override {
        // Called at exactly 60 Hz (configurable)
        // dt is always 1/60 second
        physics_step(dt);
    }
    
    void render(float alpha) override {
        // Called once per frame
        // alpha is interpolation factor [0,1]
        draw_interpolated(alpha);
    }
};
```

### 5. App Builder (Functional Style)

For simpler applications or prototypes:

```cpp
sdlpp::app_builder()
    .with_window("Particle Demo", 1024, 768)
    .with_renderer()
    .on_init([&](int, char*[]) {
        particles.reserve(1000);
        return true;
    })
    .on_frame([&]() {
        update_particles();
        render_particles();
        return !should_quit;
    })
    .on_event([&](const auto& e) {
        handle_input(e);
        return true;
    })
    .run(argc, argv);
```

## Configuration Options

### Window Configuration

```cpp
SDLPP_MAIN_WITH_CONFIG(my_app,
    .window_title = "My Game",
    .window_width = 1920,
    .window_height = 1080,
    .window_flags = window_flags::fullscreen | window_flags::high_pixel_density,
    .auto_create_renderer = true
)
```

### Game Configuration

```cpp
game_application::game_config config;
config.fixed_update_rate = 120.0f;  // 120 Hz physics
config.max_updates_per_frame = 10;  // Prevent spiral of death
config.enable_interpolation = true;
config.enable_frame_smoothing = true;
```

## Advanced Usage

### Custom Event Processing

```cpp
bool on_event(const event& e) override {
    // Pattern matching on event types
    if (auto* kb = e.as<keyboard_event>()) {
        if (kb->type == event_type::key_down) {
            handle_key(kb->key);
        }
    }
    else if (auto* mouse = e.as<mouse_button_event>()) {
        handle_click(mouse->x, mouse->y);
    }
    
    // Or using visitor pattern
    e.visit([](const auto& evt) {
        using T = std::decay_t<decltype(evt)>;
        if constexpr (std::is_same_v<T, window_event>) {
            handle_window_event(evt);
        }
    });
    
    return true;
}
```

### State Machines

```cpp
class game_app : public sdlpp::state_machine_application {
    bool on_init() override {
        register_state<menu_state>();
        register_state<play_state>();
        register_state<pause_state>();
        
        transition_to<menu_state>();
        return true;
    }
};
```

### Performance Monitoring

```cpp
class my_game : public sdlpp::monitored_game_application<> {
    void render(float alpha) override {
        // ... render game ...
        
        auto& monitor = get_monitor();
        draw_text("FPS: " + std::to_string(monitor.get_average_fps()));
        draw_text("Frame time: " + std::to_string(monitor.get_average_frame_time()));
    }
};
```

### Scene Transitions

```cpp
class game_scene : public sdlpp::scene {
    bool handle_event(const event& e) override {
        if (game_over) {
            // Fade to game over screen
            app().push_scene<fade_transition>(
                std::make_unique<game_scene>(*this),
                std::make_unique<game_over_scene>(),
                1.0f  // 1 second fade
            );
        }
        return true;
    }
};
```

## Migration from SDL2

If you're migrating from SDL2's traditional main loop:

**SDL2 Style:**
```cpp
int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(...);
    SDL_Renderer* renderer = SDL_CreateRenderer(...);
    
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }
        
        SDL_RenderClear(renderer);
        // ... render ...
        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
```

**SDL++ Style:**
```cpp
class my_app : public sdlpp::application {
protected:
    void on_frame() override {
        auto& renderer = get_renderer();
        renderer.clear();
        // ... render ...
        // present() called automatically
    }
};

SDLPP_MAIN(my_app)
```

## Best Practices

1. **Resource Management**
   - Let the framework manage SDL init/quit
   - Use RAII wrappers for all resources
   - Don't manually call SDL_Quit()

2. **Event Handling**
   - Return false to quit gracefully
   - Use type-safe event casting
   - Handle quit events unless you disable auto-handling

3. **Timing**
   - Use get_delta_time() for frame-independent movement
   - Use fixed timestep for physics simulation
   - Enable frame smoothing for consistent performance

4. **Error Handling**
   - Override on_error() for custom error handling
   - Check return values of resource creation
   - Use expected<T, error> for fallible operations

## Troubleshooting

### App doesn't start
- Check on_init() returns true
- Verify SDL initialization flags
- Check window/renderer creation errors

### Events not received
- Ensure event queue is being polled
- Check event type casting
- Verify event handling returns true to continue

### Performance issues
- Use release builds
- Enable vsync in renderer flags
- Profile with performance_monitor
- Check fixed update rate isn't too high

## Examples

See the `examples/app/` directory for complete examples:
- `example_basic_app.cc` - Simple moving rectangle
- `example_app_builder.cc` - Particle system using builder
- `example_scene_app.cc` - Menu system with scenes
- `example_game_app.cc` - Physics simulation with fixed timestep
- `example_callbacks.cc` - Minimal function-based app