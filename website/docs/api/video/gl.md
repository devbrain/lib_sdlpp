---
sidebar_position: 4
---

# OpenGL Support

The GL module provides comprehensive OpenGL and OpenGL ES context management. It handles context creation, configuration, and platform-specific details while maintaining a clean, type-safe interface.

## Basic Usage

### Creating an OpenGL Context

```cpp
#include <sdlpp/video/gl.hh>

// Create window with OpenGL support
auto window = sdlpp::window::create("OpenGL App", 800, 600,
    sdlpp::window_flags::opengl | sdlpp::window_flags::resizable);

// Create OpenGL context
auto context = sdlpp::gl_context::create(*window);
if (!context) {
    std::cerr << "Failed to create GL context: " << context.error() << "\n";
    return -1;
}

// Make context current
context->make_current(*window);

// Your OpenGL code here
glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT);

// Swap buffers
window->gl_swap();
```

### Context Configuration

```cpp
// Set attributes before creating context
sdlpp::gl::set_attribute(sdlpp::gl_attr::context_major_version, 4);
sdlpp::gl::set_attribute(sdlpp::gl_attr::context_minor_version, 5);
sdlpp::gl::set_attribute(sdlpp::gl_attr::context_profile_mask, 
    sdlpp::gl_profile::core);

// Configure framebuffer
sdlpp::gl::set_attribute(sdlpp::gl_attr::red_size, 8);
sdlpp::gl::set_attribute(sdlpp::gl_attr::green_size, 8);
sdlpp::gl::set_attribute(sdlpp::gl_attr::blue_size, 8);
sdlpp::gl::set_attribute(sdlpp::gl_attr::alpha_size, 8);
sdlpp::gl::set_attribute(sdlpp::gl_attr::depth_size, 24);
sdlpp::gl::set_attribute(sdlpp::gl_attr::stencil_size, 8);

// Enable double buffering (default)
sdlpp::gl::set_attribute(sdlpp::gl_attr::doublebuffer, 1);

// Query actual values after context creation
auto major = sdlpp::gl::get_attribute(sdlpp::gl_attr::context_major_version);
auto minor = sdlpp::gl::get_attribute(sdlpp::gl_attr::context_minor_version);
std::cout << "OpenGL " << major << "." << minor << " context created\n";
```

## Advanced Context Options

### Context Profiles

```cpp
// Core Profile (modern OpenGL, no deprecated features)
sdlpp::gl::set_attribute(sdlpp::gl_attr::context_profile_mask,
    sdlpp::gl_profile::core);

// Compatibility Profile (includes deprecated features)
sdlpp::gl::set_attribute(sdlpp::gl_attr::context_profile_mask,
    sdlpp::gl_profile::compatibility);

// OpenGL ES Profile
sdlpp::gl::set_attribute(sdlpp::gl_attr::context_profile_mask,
    sdlpp::gl_profile::es);
```

### Debug Context

```cpp
// Enable debug context for development
sdlpp::gl::set_attribute(sdlpp::gl_attr::context_flags,
    sdlpp::gl_context_flag::debug);

// After context creation, set up debug callback
void APIENTRY gl_debug_callback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const GLchar* message, 
    const void* userParam) {
    std::cerr << "GL Debug: " << message << "\n";
}

glEnable(GL_DEBUG_OUTPUT);
glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
glDebugMessageCallback(gl_debug_callback, nullptr);
```

### Context Sharing

```cpp
// Create main context
auto main_context = sdlpp::gl_context::create(*main_window);

// Create shared context for background loading
auto shared_window = sdlpp::window::create("Hidden", 1, 1,
    sdlpp::window_flags::opengl | sdlpp::window_flags::hidden);
    
sdlpp::gl::set_share_with_current_context(1);
auto shared_context = sdlpp::gl_context::create(*shared_window);

// Use shared context in loading thread
std::thread loader([&]() {
    shared_context->make_current(*shared_window);
    // Load textures, compile shaders, etc.
    // Resources are available in main context
});
```

## Multisampling (MSAA)

```cpp
// Enable multisampling
sdlpp::gl::set_attribute(sdlpp::gl_attr::multisamplebuffers, 1);
sdlpp::gl::set_attribute(sdlpp::gl_attr::multisamplesamples, 4); // 4x MSAA

// Create context and window as usual
auto window = sdlpp::window::create("MSAA App", 800, 600,
    sdlpp::window_flags::opengl);
auto context = sdlpp::gl_context::create(*window);

// Enable multisampling in OpenGL
glEnable(GL_MULTISAMPLE);
```

## VSync Control

```cpp
// Enable VSync (default)
sdlpp::gl::set_swap_interval(1);

// Disable VSync
sdlpp::gl::set_swap_interval(0);

// Adaptive VSync (if supported)
sdlpp::gl::set_swap_interval(-1);

// Query current setting
int interval = sdlpp::gl::get_swap_interval();
```

## Extension Management

```cpp
// Check if extension is supported
bool has_anisotropic = sdlpp::gl::extension_supported(
    "GL_EXT_texture_filter_anisotropic");

// Platform-specific extension checking
#ifdef _WIN32
bool has_swap_control = sdlpp::gl::extension_supported(
    "WGL_EXT_swap_control");
#elif defined(__linux__)
bool has_swap_control = sdlpp::gl::extension_supported(
    "GLX_EXT_swap_control");
#endif
```

## Function Loading

```cpp
// Get OpenGL function pointers
auto glCreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>(
    sdlpp::gl::get_proc_address("glCreateShader"));

// Using a loader library (recommended)
#include <glad/glad.h>

// After context creation
if (!gladLoadGLLoader((GLADloadproc)sdlpp::gl::get_proc_address)) {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
}
```

## Framebuffer Configuration

### Stencil and Depth Buffers

```cpp
// Configure depth buffer
sdlpp::gl::set_attribute(sdlpp::gl_attr::depth_size, 24);

// Configure stencil buffer
sdlpp::gl::set_attribute(sdlpp::gl_attr::stencil_size, 8);

// Packed depth-stencil (more efficient)
sdlpp::gl::set_attribute(sdlpp::gl_attr::depth_size, 24);
sdlpp::gl::set_attribute(sdlpp::gl_attr::stencil_size, 8);
```

### Color Buffer

```cpp
// High color precision (10 bits per channel)
sdlpp::gl::set_attribute(sdlpp::gl_attr::red_size, 10);
sdlpp::gl::set_attribute(sdlpp::gl_attr::green_size, 10);
sdlpp::gl::set_attribute(sdlpp::gl_attr::blue_size, 10);
sdlpp::gl::set_attribute(sdlpp::gl_attr::alpha_size, 2);

// HDR rendering (floating point buffer)
sdlpp::gl::set_attribute(sdlpp::gl_attr::floatbuffers, 1);
```

### Accumulation Buffer

```cpp
// Configure accumulation buffer (legacy, rarely used)
sdlpp::gl::set_attribute(sdlpp::gl_attr::accum_red_size, 16);
sdlpp::gl::set_attribute(sdlpp::gl_attr::accum_green_size, 16);
sdlpp::gl::set_attribute(sdlpp::gl_attr::accum_blue_size, 16);
sdlpp::gl::set_attribute(sdlpp::gl_attr::accum_alpha_size, 16);
```

## Context Management

### Multiple Windows

```cpp
// Window and context pairs
struct gl_window {
    sdlpp::window window;
    sdlpp::gl_context context;
};

std::vector<gl_window> windows;

// Create multiple OpenGL windows
for (int i = 0; i < 3; ++i) {
    auto window = sdlpp::window::create(
        "GL Window " + std::to_string(i),
        640, 480,
        sdlpp::window_flags::opengl
    );
    
    auto context = sdlpp::gl_context::create(*window);
    
    windows.push_back({std::move(*window), std::move(*context)});
}

// Render to each window
for (auto& w : windows) {
    w.context.make_current(w.window);
    
    // Render scene
    glClear(GL_COLOR_BUFFER_BIT);
    render_scene();
    
    w.window.gl_swap();
}
```

### Context Reset Notification

```cpp
// Enable reset notification (robustness extension)
sdlpp::gl::set_attribute(sdlpp::gl_attr::context_flags,
    sdlpp::gl_context_flag::robust_access);

sdlpp::gl::set_attribute(sdlpp::gl_attr::context_reset_notification,
    sdlpp::gl_context_reset::lose_context);

// In render loop, check for context loss
GLenum status = glGetGraphicsResetStatus();
if (status != GL_NO_ERROR) {
    // Context lost, recreate resources
    recreate_opengl_resources();
}
```

## Platform-Specific Features

### High DPI Support

```cpp
// Get drawable size (for high DPI displays)
auto [window_w, window_h] = window->get_size();
auto [drawable_w, drawable_h] = window->gl_get_drawable_size();

// Set viewport to match drawable size
glViewport(0, 0, drawable_w, drawable_h);

// Calculate DPI scale
float scale_x = static_cast<float>(drawable_w) / window_w;
float scale_y = static_cast<float>(drawable_h) / window_h;
```

### sRGB Framebuffer

```cpp
// Enable sRGB framebuffer
sdlpp::gl::set_attribute(sdlpp::gl_attr::framebuffer_srgb_capable, 1);

// After context creation
glEnable(GL_FRAMEBUFFER_SRGB);
```

## Common Patterns

### Basic Render Loop

```cpp
void render_loop(sdlpp::window& window, sdlpp::gl_context& context) {
    context.make_current(window);
    
    // Setup OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    bool running = true;
    while (running) {
        // Handle events
        while (auto event = sdlpp::event_queue::poll()) {
            if (event->type() == sdlpp::event_type::quit) {
                running = false;
            }
        }
        
        // Clear
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render
        render_scene();
        
        // Swap buffers
        window.gl_swap();
    }
}
```

### Resource Loading Thread

```cpp
class gl_resource_loader {
    sdlpp::window hidden_window;
    sdlpp::gl_context load_context;
    std::thread loader_thread;
    
public:
    gl_resource_loader() 
        : hidden_window(*sdlpp::window::create("Loader", 1, 1,
            sdlpp::window_flags::opengl | sdlpp::window_flags::hidden))
    {
        // Share with current context
        sdlpp::gl::set_share_with_current_context(1);
        load_context = *sdlpp::gl_context::create(hidden_window);
    }
    
    template<typename Func>
    void load_async(Func&& func) {
        loader_thread = std::thread([this, func = std::forward<Func>(func)]() {
            load_context.make_current(hidden_window);
            func();
        });
    }
};
```

## Error Handling

```cpp
// All GL operations return expected<T, string>
auto context = sdlpp::gl_context::create(*window);
if (!context) {
    std::cerr << "GL context creation failed: " << context.error() << "\n";
    
    // Try fallback options
    sdlpp::gl::set_attribute(sdlpp::gl_attr::context_major_version, 3);
    sdlpp::gl::set_attribute(sdlpp::gl_attr::context_minor_version, 3);
    
    context = sdlpp::gl_context::create(*window);
    if (!context) {
        std::cerr << "Fallback also failed: " << context.error() << "\n";
        return -1;
    }
}
```

## Best Practices

1. **Set Attributes First**: Configure all GL attributes before creating context
2. **Check Capabilities**: Query actual values after context creation
3. **Handle Context Loss**: Implement recovery for robust applications
4. **Use Debug Context**: Enable during development for better error messages
5. **Share Contexts**: Use shared contexts for background resource loading
6. **Match Buffer Formats**: Ensure color depth matches display capabilities
7. **Profile Appropriately**: Use Core profile for modern OpenGL

## API Reference

### Classes

- `gl_context` - OpenGL context management
- `gl` - Static GL configuration functions

### Enums

- `gl_attr` - OpenGL context attributes
- `gl_profile` - Context profile options
- `gl_context_flag` - Context creation flags
- `gl_context_release_behavior` - Context release options
- `gl_context_reset` - Reset notification options

### Functions

- `gl::set_attribute()` - Set context attributes
- `gl::get_attribute()` - Query context attributes
- `gl::set_swap_interval()` - Control VSync
- `gl::get_proc_address()` - Get GL function pointers
- `gl::extension_supported()` - Check extension support
- `window::gl_swap()` - Swap front/back buffers
- `window::gl_get_drawable_size()` - Get framebuffer size