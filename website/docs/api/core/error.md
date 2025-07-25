---
title: Error Handling
sidebar_label: Error Handling
---

# Error Handling in SDL++

SDL++ uses `tl::expected` for error handling, providing a type-safe alternative to exceptions for SDL operations.

## Expected Type

Most SDL++ functions return `tl::expected<T, std::string>`:

```cpp
// Function returns expected<window, std::string>
auto window_result = sdlpp::window::create("My Window", 800, 600);

if (window_result) {
    // Success - access the window
    auto& window = window_result.value();
    // Use window...
} else {
    // Error - get error message
    std::cerr << "Failed to create window: " << window_result.error() << std::endl;
}
```

## Error Checking Patterns

### Direct Check
```cpp
auto result = sdlpp::renderer::create(window);
if (!result) {
    std::cerr << "Error: " << result.error() << std::endl;
    return;
}
auto& renderer = result.value();
```

### Monadic Operations
```cpp
auto final_result = sdlpp::window::create("Test", 800, 600)
    .and_then([](auto& window) {
        return sdlpp::renderer::create(window);
    })
    .and_then([](auto& renderer) {
        return renderer.set_draw_color({255, 0, 0, 255});
    });

if (!final_result) {
    std::cerr << "Operation failed: " << final_result.error() << std::endl;
}
```

### Value Or Default
```cpp
auto window_result = sdlpp::window::create("Test", 800, 600);
// Will create a minimal window if the first attempt fails
auto& window = window_result.value_or(
    sdlpp::window::create("Fallback", 400, 300).value()
);
```

## SDL Error Functions

### Getting SDL Errors
```cpp
// Get the last SDL error
std::string error = sdlpp::get_error();

// Clear the error string
sdlpp::clear_error();

// Set a custom error
sdlpp::set_error("Custom error message");
```

### Error Context Helper
```cpp
// RAII error context - automatically adds context to errors
{
    sdlpp::error_context ctx("Creating game window");
    
    auto window_result = sdlpp::window::create("Game", 1920, 1080);
    if (!window_result) {
        // Error will include context: "Creating game window: <SDL error>"
        return window_result.error();
    }
}
```

## Common Error Handling Patterns

### Early Return Pattern
```cpp
tl::expected<void, std::string> initialize_graphics() {
    auto window_result = sdlpp::window::create("App", 800, 600);
    if (!window_result) {
        return tl::unexpected(window_result.error());
    }
    
    auto renderer_result = sdlpp::renderer::create(window_result.value());
    if (!renderer_result) {
        return tl::unexpected(renderer_result.error());
    }
    
    // Store results in member variables
    m_window = std::move(window_result.value());
    m_renderer = std::move(renderer_result.value());
    
    return {};  // Success
}
```

### Try-Style Macro (Optional)
```cpp
// Define a TRY macro for cleaner code
#define TRY(expr) \
    do { \
        auto result = (expr); \
        if (!result) { \
            return tl::unexpected(result.error()); \
        } \
    } while(0)

tl::expected<void, std::string> load_texture(const std::string& path) {
    TRY(sdlpp::img::init(sdlpp::img::init_flags::png));
    
    auto surface_result = sdlpp::img::load(path);
    TRY(surface_result);
    
    auto texture_result = m_renderer.create_texture(surface_result.value());
    TRY(texture_result);
    
    m_texture = std::move(texture_result.value());
    return {};
}
```

### Aggregate Error Collection
```cpp
std::vector<std::string> load_resources() {
    std::vector<std::string> errors;
    
    auto tex1 = load_texture("player.png");
    if (!tex1) errors.push_back("Player texture: " + tex1.error());
    
    auto tex2 = load_texture("enemy.png");
    if (!tex2) errors.push_back("Enemy texture: " + tex2.error());
    
    auto sound1 = load_sound("shoot.wav");
    if (!sound1) errors.push_back("Shoot sound: " + sound1.error());
    
    return errors;
}
```

## Best Practices

1. **Always Check Results**: Never ignore `expected` return values
2. **Provide Context**: Add meaningful context to error messages
3. **Handle Gracefully**: Provide fallbacks where appropriate
4. **Log Errors**: Log errors for debugging but don't expose internal details to users
5. **Clean Up**: Ensure resources are cleaned up even when errors occur (RAII helps)

## Example: Robust Initialization

```cpp
class Application {
    sdlpp::window m_window;
    sdlpp::renderer m_renderer;
    
public:
    tl::expected<void, std::string> initialize() {
        // Create window with fallback sizes
        auto window_result = sdlpp::window::create("My App", 1920, 1080);
        if (!window_result) {
            // Try smaller size
            window_result = sdlpp::window::create("My App", 1280, 720);
            if (!window_result) {
                return tl::unexpected("Failed to create window: " + 
                                    window_result.error());
            }
        }
        m_window = std::move(window_result.value());
        
        // Create renderer with fallback options
        auto renderer_result = sdlpp::renderer::create(m_window, 
            sdlpp::renderer::flags::accelerated);
        if (!renderer_result) {
            // Try software renderer
            renderer_result = sdlpp::renderer::create(m_window, 
                sdlpp::renderer::flags::software);
            if (!renderer_result) {
                return tl::unexpected("Failed to create renderer: " + 
                                    renderer_result.error());
            }
        }
        m_renderer = std::move(renderer_result.value());
        
        return {};  // Success
    }
};
```