---
sidebar_position: 2
---

# Renderer

The renderer module provides hardware-accelerated 2D rendering capabilities. Renderers are created for a specific window and handle all drawing operations using the GPU when available.

## Basic Usage

### Creating a Renderer

```cpp
#include <sdlpp/video/renderer.hh>

// Create renderer with default settings
auto renderer = sdlpp::renderer::create(*window);
if (!renderer) {
    std::cerr << "Failed to create renderer: " << renderer.error() << "\n";
    return -1;
}

// Create renderer with specific flags
auto renderer2 = sdlpp::renderer::create(*window, 
    sdlpp::renderer_flags::accelerated | sdlpp::renderer_flags::presentvsync);

// Create renderer with specific driver
auto renderer3 = sdlpp::renderer::create(*window, "opengl");
```

### Basic Drawing Operations

```cpp
// Clear the screen
renderer->set_draw_color({64, 128, 255, 255});  // Light blue
renderer->clear();

// Draw primitives
renderer->set_draw_color({255, 0, 0, 255});     // Red
renderer->draw_point({100, 100});
renderer->draw_line({50, 50}, {150, 150});
renderer->draw_rect({{200, 200}, {100, 50}});
renderer->fill_rect({{320, 200}, {100, 50}});

// Present the frame
renderer->present();
```

### Viewport and Clipping

```cpp
// Set viewport (render to portion of window)
renderer->set_viewport({{0, 0}, {400, 300}});

// Set clipping rectangle
renderer->set_clip_rect({{50, 50}, {200, 200}});

// Reset viewport/clipping
renderer->set_viewport(std::nullopt);  // Full window
renderer->set_clip_rect(std::nullopt);  // No clipping

// Get current settings
auto viewport = renderer->get_viewport();
auto clip = renderer->get_clip_rect();
```

## Texture Management

### Creating Textures

```cpp
// From surface
auto surface = sdlpp::surface::load("image.png");
auto texture = renderer->create_texture(*surface);

// Blank texture for render target
auto target = renderer->create_texture(
    sdlpp::pixel_format::rgba8888,
    sdlpp::texture_access::target,
    800, 600
);

// Query texture info
auto info = texture->query();
std::cout << "Size: " << info.width << "x" << info.height << "\n";
std::cout << "Format: " << info.format << "\n";
```

### Rendering Textures

```cpp
// Simple copy (entire texture to entire renderer)
renderer->copy(*texture);

// Copy with source and destination rectangles
sdlpp::rect src{{0, 0}, {64, 64}};    // Source portion
sdlpp::rect dst{{100, 100}, {128, 128}};  // Destination (scaled)
renderer->copy(*texture, src, dst);

// Extended copy with rotation and flipping
renderer->copy_ex(
    *texture,
    src,                              // Source rect
    dst,                              // Destination rect
    45.0,                             // Rotation angle (degrees)
    sdlpp::point{64, 64},            // Rotation center
    sdlpp::flip_mode::horizontal     // Flip mode
);
```

### Texture Properties

```cpp
// Blend mode
texture->set_blend_mode(sdlpp::blend_mode::blend);
auto blend = texture->get_blend_mode();

// Color modulation
texture->set_color_mod({255, 128, 128});  // Reddish tint
auto [r, g, b] = texture->get_color_mod();

// Alpha modulation
texture->set_alpha_mod(128);  // 50% transparent
auto alpha = texture->get_alpha_mod();

// Scale mode (filtering)
texture->set_scale_mode(sdlpp::scale_mode::linear);
```

## Render Targets

### Off-screen Rendering

```cpp
// Create render target texture
auto target = renderer->create_texture(
    sdlpp::pixel_format::rgba8888,
    sdlpp::texture_access::target,
    512, 512
);

// Set as render target
renderer->set_target(*target);

// Draw to texture
renderer->set_draw_color({0, 0, 0, 0});
renderer->clear();
renderer->set_draw_color({255, 255, 0, 255});
renderer->fill_circle({256, 256}, 100);

// Reset to default target (window)
renderer->set_target(std::nullopt);

// Now use the texture normally
renderer->copy(*target, std::nullopt, {{50, 50}, {256, 256}});
```

## Advanced Drawing

### Drawing Multiple Points/Lines

```cpp
// Draw multiple points
std::vector<sdlpp::point> points = {
    {10, 10}, {20, 20}, {30, 30}, {40, 40}
};
renderer->draw_points(points);

// Draw connected lines
std::vector<sdlpp::point> line_points = {
    {100, 100}, {200, 150}, {300, 100}, {200, 50}
};
renderer->draw_lines(line_points);

// Draw multiple rectangles
std::vector<sdlpp::rect> rects = {
    {{10, 10}, {50, 50}},
    {{70, 10}, {50, 50}},
    {{130, 10}, {50, 50}}
};
renderer->draw_rects(rects);
renderer->fill_rects(rects);
```

### Geometry Rendering

```cpp
// Render geometric shapes with textures
std::vector<sdlpp::vertex> vertices = {
    {{100, 100}, {255, 0, 0, 255}, {0, 0}},
    {{200, 100}, {0, 255, 0, 255}, {1, 0}},
    {{150, 200}, {0, 0, 255, 255}, {0.5, 1}}
};

std::vector<int> indices = {0, 1, 2};

renderer->render_geometry(vertices, indices, texture.get());
```

## Renderer Properties

### Query Renderer Info

```cpp
// Get renderer info
auto info = renderer->get_info();
std::cout << "Renderer: " << info.name << "\n";
std::cout << "Flags: " << info.flags << "\n";
std::cout << "Max texture size: " << info.max_texture_width 
          << "x" << info.max_texture_height << "\n";

// Supported texture formats
for (auto format : info.texture_formats) {
    std::cout << "Supports: " << format << "\n";
}
```

### VSync Control

```cpp
// Enable VSync
renderer->set_vsync(1);  // Standard VSync

// Adaptive VSync (if supported)
renderer->set_vsync(-1);

// Disable VSync
renderer->set_vsync(0);

// Query current setting
auto vsync = renderer->get_vsync();
```

### Logical Size (Resolution Independence)

```cpp
// Set logical size for resolution-independent rendering
renderer->set_logical_size(1920, 1080);

// Set logical presentation mode
renderer->set_logical_presentation(
    sdlpp::logical_presentation::letterbox,  // Maintain aspect ratio
    sdlpp::scale_mode::linear               // Smooth scaling
);

// Get current logical size
auto [width, height] = renderer->get_logical_size();
```

## Blend Modes

```cpp
// Set global blend mode for drawing operations
renderer->set_draw_blend_mode(sdlpp::blend_mode::blend);

// Available blend modes:
// - none: No blending
// - blend: Alpha blending (default)
// - add: Additive blending
// - mod: Color modulation
// - mul: Color multiplication

// Custom blend mode
auto custom_blend = sdlpp::compose_custom_blend_mode(
    sdlpp::blend_factor::src_alpha,
    sdlpp::blend_factor::one_minus_src_alpha,
    sdlpp::blend_operation::add,
    sdlpp::blend_factor::one,
    sdlpp::blend_factor::one_minus_src_alpha,
    sdlpp::blend_operation::add
);
renderer->set_draw_blend_mode(custom_blend);
```

## Performance Tips

### Batching Draw Calls

```cpp
// Bad: Many individual draw calls
for (const auto& enemy : enemies) {
    renderer->copy(*enemy_texture, std::nullopt, enemy.rect);
}

// Better: Use render_geometry for many sprites
std::vector<sdlpp::vertex> vertices;
for (const auto& enemy : enemies) {
    // Add 4 vertices for each enemy quad
    add_sprite_vertices(vertices, enemy);
}
renderer->render_geometry(vertices, indices, enemy_texture.get());
```

### Texture Atlas

```cpp
// Use texture atlases to reduce texture switches
struct sprite {
    sdlpp::rect source;  // Position in atlas
    sdlpp::rect dest;    // Screen position
};

// Render many sprites from one texture
for (const auto& s : sprites) {
    renderer->copy(*atlas_texture, s.source, s.dest);
}
```

### Render Targets for Caching

```cpp
// Pre-render complex UI to texture
auto ui_cache = renderer->create_texture(
    sdlpp::pixel_format::rgba8888,
    sdlpp::texture_access::target,
    ui_width, ui_height
);

// Render UI once
renderer->set_target(*ui_cache);
render_complex_ui();
renderer->set_target(std::nullopt);

// Use cached version in main loop
renderer->copy(*ui_cache);
```

## DDA Rendering

SDL++ includes Digital Differential Analyzer (DDA) algorithms for high-quality 2D graphics:

### Antialiased Lines

```cpp
// Standard lines
renderer->draw_line({0, 0}, {100, 100});

// Antialiased lines for smoother appearance
renderer->draw_line_aa({0.5f, 0.5f}, {99.5f, 99.5f});

// Thick lines with variable width
renderer->draw_line_thick({10, 10}, {90, 90}, 5.0f);
```

### Circles and Ellipses

```cpp
// Circle outline and filled
renderer->draw_circle({100, 100}, 50);
renderer->fill_circle({200, 200}, 30);

// Ellipse outline and filled
renderer->draw_ellipse({300, 300}, 60, 40);
renderer->fill_ellipse({400, 400}, 80, 50);

// Elliptical arcs (angles in radians)
renderer->draw_ellipse_arc({500, 500}, 100, 75, 0.0f, M_PI);

// Using Euler angles for type safety
#include <euler/angles/angle.hh>
renderer->draw_ellipse_arc({600, 600}, 100, 75,
                          euler::radian<float>(0.0f),
                          euler::radian<float>(M_PI_2));
```

### Bezier Curves

```cpp
// Quadratic Bezier curve (3 control points)
renderer->draw_bezier_quad({0, 100}, {50, 0}, {100, 100});

// Cubic Bezier curve (4 control points)
renderer->draw_bezier_cubic({0, 0}, {30, 100}, {70, 100}, {100, 0});

// Works with any point-like type
euler::point2<float> p0{0, 0}, p1{50, 100}, p2{100, 0};
renderer->draw_bezier_quad(p0, p1, p2);
```

### Splines

```cpp
// B-spline with control points
std::vector<euler::point2<float>> controls = {
    {0, 0}, {50, 100}, {100, 50}, {150, 100}, {200, 0}
};
renderer->draw_bspline(controls, 3); // degree 3 (cubic)

// Catmull-Rom spline (passes through all points)
std::vector<sdlpp::point<float>> points = {
    {0, 50}, {50, 0}, {100, 100}, {150, 50}
};
renderer->draw_catmull_rom(points, 0.5f); // tension parameter
```

### Parametric Curves

```cpp
// Draw any parametric curve
auto spiral = [](float t) -> sdlpp::point<float> {
    float r = t * 10;
    return {400 + r * std::cos(t), 300 + r * std::sin(t)};
};
renderer->draw_curve(spiral, 0.0f, 4.0f * M_PI, 200); // 200 steps

// Heart shape
auto heart = [](float t) -> sdlpp::point<float> {
    float x = 16 * std::pow(std::sin(t), 3);
    float y = 13 * std::cos(t) - 5 * std::cos(2*t) - 
              2 * std::cos(3*t) - std::cos(4*t);
    return {x * 5 + 400, -y * 5 + 300};
};
renderer->draw_curve(heart, 0.0f, 2.0f * M_PI, 100);
```

### Polygons

```cpp
// Define vertices
std::vector<sdlpp::point<int>> vertices = {
    {100, 100}, {200, 100}, {250, 200}, {150, 250}, {50, 200}
};

// Draw polygon outline
renderer->draw_polygon(vertices);

// Fill polygon
renderer->fill_polygon(vertices);

// Antialiased polygon outline
renderer->draw_polygon_aa(vertices);

// Star polygon
std::vector<sdlpp::point<float>> star;
for (int i = 0; i < 10; ++i) {
    float angle = i * M_PI / 5;
    float r = (i % 2) ? 50 : 100;
    star.push_back({400 + r * std::cos(angle), 300 + r * std::sin(angle)});
}
renderer->fill_polygon(star);
```

### DDA Performance

The DDA algorithms use optimized batched pixel writing for performance:

```cpp
// All DDA operations internally batch pixels
// This is much faster than individual pixel writes
renderer->draw_circle({100, 100}, 50);  // Batches ~314 pixels

// For software rendering, use surface_renderer
sdlpp::surface surf(800, 600, SDL_PIXELFORMAT_ARGB8888);
sdlpp::surface_renderer sw_renderer(surf);

// Same API, optimized for software rendering
sw_renderer.draw_circle({100, 100}, 50);
```

## Common Patterns

### Double Buffering

```cpp
// SDL++ automatically uses double buffering
while (running) {
    // Clear back buffer
    renderer->clear();
    
    // Draw to back buffer
    render_scene();
    
    // Swap buffers
    renderer->present();
}
```

### Fade In/Out Effect

```cpp
void fade_in(sdlpp::renderer& renderer, sdlpp::texture& texture, 
             std::chrono::milliseconds duration) {
    auto start = sdlpp::timer::get_ticks();
    
    while (true) {
        auto elapsed = sdlpp::timer::get_ticks() - start;
        float alpha = std::min(1.0f, elapsed / duration.count());
        
        renderer.clear();
        texture.set_alpha_mod(static_cast<Uint8>(255 * alpha));
        renderer.copy(texture);
        renderer.present();
        
        if (alpha >= 1.0f) break;
    }
}
```

## Error Handling

All renderer operations return `sdlpp::expected` for error handling:

```cpp
auto result = renderer->set_target(*texture);
if (!result) {
    std::cerr << "Failed to set render target: " << result.error() << "\n";
    // Handle error
}

// Chain operations
auto setup_result = renderer->set_logical_size(1920, 1080)
    .and_then([&]() { return renderer->set_vsync(1); })
    .and_then([&]() { return renderer->clear(); });

if (!setup_result) {
    std::cerr << "Renderer setup failed: " << setup_result.error() << "\n";
}
```

## Platform Considerations

- **Metal (macOS/iOS)**: Requires `metal` renderer flag
- **DirectX (Windows)**: Default on Windows, best performance
- **OpenGL**: Cross-platform, requires `opengl` flag
- **Vulkan**: Requires `vulkan` flag and Vulkan drivers
- **Software**: Always available but slow, use as fallback

## Best Practices

1. **Create Early**: Create renderer immediately after window
2. **Batch Operations**: Group similar draw calls together
3. **Minimize State Changes**: Changing blend modes, colors, etc. has overhead
4. **Use Render Targets**: Cache complex rendering
5. **Profile Performance**: Use SDL's built-in renderer info
6. **Handle Errors**: Always check return values
7. **Respect VSync**: Use frame limiters with VSync disabled

## API Reference

### Classes

- `renderer` - Main renderer class
- `texture` - Texture resource class
- `renderer_info` - Renderer capabilities

### Enums

- `renderer_flags` - Renderer creation flags
- `texture_access` - Texture access patterns
- `blend_mode` - Blending modes
- `flip_mode` - Texture flipping options
- `scale_mode` - Texture filtering modes

### Free Functions

- `renderer::create()` - Create a renderer
- `renderer::get_drivers()` - List available drivers