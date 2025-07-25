---
title: Application Framework
sidebar_label: Application Framework
---

# SDL++ Application Framework

SDL++ provides a modern application framework that integrates with SDL3's callback-based architecture, offering multiple paradigms for structuring your application.

## Basic Application

The simplest way to create an SDL++ application:

```cpp
#include <sdlpp/app/app.hh>

class my_app : public sdlpp::application {
protected:
    bool on_init() override {
        std::cout << "Initializing..." << std::endl;
        return true;  // Return false to abort
    }
    
    void on_frame() override {
        auto& renderer = get_renderer();
        renderer.clear();
        // Draw your content here
    }
    
    void on_event(const sdlpp::event& e) override {
        if (e.type() == sdlpp::event_type::quit) {
            quit();
        }
    }
    
    void on_cleanup() override {
        std::cout << "Cleaning up..." << std::endl;
    }
};

// Macro creates main() and handles SDL initialization
SDLPP_MAIN(my_app)
```

## Application Configuration

Configure your application using the builder pattern:

```cpp
#include <sdlpp/app/app_builder.hh>

int main(int argc, char* argv[]) {
    return sdlpp::app_builder()
        .with_title("My Game")
        .with_size(1280, 720)
        .with_window_flags(sdlpp::window::flags::resizable)
        .with_renderer_flags(sdlpp::renderer::flags::accelerated | 
                           sdlpp::renderer::flags::vsync)
        .with_target_fps(60.0)
        .with_background_color({64, 64, 64, 255})
        .run<my_app>(argc, argv);
}
```

## Application Lifecycle

The application framework manages the complete lifecycle:

```cpp
class lifecycle_app : public sdlpp::application {
protected:
    bool on_init() override {
        // Called once at startup
        // Initialize resources, load configuration
        // Return false to abort startup
        return load_resources();
    }
    
    void on_frame() override {
        // Called every frame
        // Update and render your application
        update(get_frame_time());
        render();
    }
    
    void on_event(const sdlpp::event& e) override {
        // Handle SDL events
        // Called for each pending event
    }
    
    void on_app_terminating() override {
        // App is being terminated by the OS
        save_state();
    }
    
    void on_app_low_memory() override {
        // System is low on memory
        free_unused_resources();
    }
    
    void on_app_will_enter_background() override {
        // App is about to go to background (mobile)
        pause_game();
    }
    
    void on_app_did_enter_background() override {
        // App is now in background
    }
    
    void on_app_will_enter_foreground() override {
        // App is about to return to foreground
    }
    
    void on_app_did_enter_foreground() override {
        // App is now in foreground
        resume_game();
    }
    
    void on_cleanup() override {
        // Called once at shutdown
        // Clean up resources
    }
};
```

## Built-in Features

The application base class provides many utilities:

```cpp
class feature_demo : public sdlpp::application {
protected:
    void on_frame() override {
        // Access window and renderer
        auto& window = get_window();
        auto& renderer = get_renderer();
        
        // Get timing information
        double fps = get_fps();
        double frame_time = get_frame_time();  // in seconds
        auto frame_count = get_frame_count();
        
        // Clear with background color
        clear_background();
        
        // Draw FPS counter (debug builds)
        if (is_debug_build()) {
            draw_fps();  // Built-in FPS display
        }
        
        // Check if should quit
        if (should_quit()) {
            // Cleanup in progress
        }
    }
};
```

## Functional Application Style

For simpler applications, use the functional style:

```cpp
#include <sdlpp/app/app.hh>

int main(int argc, char* argv[]) {
    sdlpp::state game_state;
    
    return sdlpp::run_application(argc, argv, {
        .init = [&]() {
            // Initialize
            return true;
        },
        .frame = [&](sdlpp::window& window, sdlpp::renderer& renderer) {
            renderer.clear();
            // Draw content
        },
        .event = [&](const sdlpp::event& e) {
            if (e.type() == sdlpp::event_type::quit) {
                return false;  // Stop app
            }
            return true;  // Continue
        },
        .cleanup = [&]() {
            // Cleanup
        }
    });
}
```

## Game Application

For games, use the specialized game application base:

```cpp
#include <sdlpp/app/game_app.hh>

class my_game : public sdlpp::game_application {
protected:
    bool on_init() override {
        // Initialize game
        return true;
    }
    
    void on_update(double dt) override {
        // Update game logic with delta time
        player.update(dt);
        enemies.update(dt);
        check_collisions();
    }
    
    void on_render() override {
        // Render game
        draw_background();
        player.draw(get_renderer());
        enemies.draw(get_renderer());
        draw_ui();
    }
    
    void on_fixed_update() override {
        // Fixed timestep update (60Hz by default)
        // Good for physics
        physics_world.step();
    }
};

SDLPP_MAIN(my_game)
```

## Scene-Based Application

For applications with multiple screens/scenes:

```cpp
#include <sdlpp/app/scene_app.hh>

// Define scenes
class menu_scene : public sdlpp::scene {
public:
    void on_enter() override {
        std::cout << "Entering menu" << std::endl;
    }
    
    void on_update(double dt) override {
        // Update menu
    }
    
    void on_render(sdlpp::renderer& r) override {
        r.clear({0, 0, 0, 255});
        // Draw menu
    }
    
    void on_event(const sdlpp::event& e) override {
        if (/* start button pressed */) {
            get_app().change_scene<game_scene>();
        }
    }
};

class game_scene : public sdlpp::scene {
    // ... game implementation
};

// Application using scenes
class my_app : public sdlpp::scene_application {
protected:
    bool on_init() override {
        // Register scenes
        register_scene<menu_scene>("menu");
        register_scene<game_scene>("game");
        
        // Start with menu
        change_scene("menu");
        return true;
    }
};
```

## Advanced Application Features

### Custom Frame Timing

```cpp
class custom_timing_app : public sdlpp::application {
protected:
    bool on_init() override {
        // Set custom frame rate
        set_target_fps(120.0);
        
        // Or use fixed timestep
        set_fixed_timestep(std::chrono::milliseconds(16));  // 60Hz
        
        // Enable frame time smoothing
        set_frame_time_smoothing(true, 0.1);  // 10% new, 90% old
        
        return true;
    }
};
```

### Resource Management

```cpp
class resource_app : public sdlpp::application {
    sdlpp::texture player_texture;
    
protected:
    bool on_init() override {
        // Application provides resource loading helpers
        auto tex_result = load_texture("assets/player.png");
        if (!tex_result) {
            log_error("Failed to load player texture: ", tex_result.error());
            return false;
        }
        player_texture = std::move(tex_result.value());
        
        return true;
    }
};
```

## Best Practices

1. **Choose the Right Base**: Use `application` for general apps, `game_application` for games
2. **Handle Lifecycle Events**: Especially important for mobile platforms
3. **Use Built-in Features**: FPS counter, frame timing, etc. are already provided
4. **Clean Separation**: Keep logic in update, rendering in render
5. **Error Handling**: Return false from `on_init()` if initialization fails

## Example: Complete Game Application

```cpp
#include <sdlpp/app/game_app.hh>

class space_shooter : public sdlpp::game_application {
    // Game objects
    player_ship player;
    std::vector<enemy> enemies;
    std::vector<bullet> bullets;
    
    // Resources
    sdlpp::texture ship_texture;
    sdlpp::texture enemy_texture;
    
protected:
    bool on_init() override {
        set_title("Space Shooter");
        set_window_size(1024, 768);
        set_target_fps(60.0);
        
        // Load resources
        if (!load_textures()) {
            return false;
        }
        
        // Initialize game
        player.set_position(512, 700);
        spawn_enemies();
        
        return true;
    }
    
    void on_update(double dt) override {
        // Update game objects
        player.update(dt);
        
        for (auto& enemy : enemies) {
            enemy.update(dt);
        }
        
        for (auto& bullet : bullets) {
            bullet.update(dt);
        }
        
        // Check collisions
        check_bullet_enemy_collisions();
        check_player_enemy_collisions();
        
        // Remove dead objects
        remove_destroyed_objects();
        
        // Spawn new enemies if needed
        maybe_spawn_enemies();
    }
    
    void on_render() override {
        // Clear screen
        clear_background({0, 0, 32, 255});
        
        // Draw game objects
        player.draw(get_renderer());
        
        for (const auto& enemy : enemies) {
            enemy.draw(get_renderer());
        }
        
        for (const auto& bullet : bullets) {
            bullet.draw(get_renderer());
        }
        
        // Draw UI
        draw_score();
        draw_lives();
    }
    
    void on_event(const sdlpp::event& e) override {
        player.handle_event(e);
        
        if (e.type() == sdlpp::event_type::key_down) {
            if (e.key().scancode == sdlpp::scancode::escape) {
                quit();
            }
        }
    }
};

SDLPP_MAIN(space_shooter)
```