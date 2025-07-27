---
sidebar_position: 6
---

# Surface Renderer

The surface renderer provides software-based 2D rendering directly to SDL surfaces. It implements the same drawing API as the hardware renderer but operates entirely in CPU memory, making it ideal for pixel-perfect rendering, image processing, and situations where hardware acceleration is not available.

## Overview

Key features:
- **Software rendering** - No GPU required
- **Pixel-perfect** - Exact control over every pixel
- **Same API** - Compatible with hardware renderer
- **DDA algorithms** - High-quality drawing primitives
- **Direct pixel access** - Read/write individual pixels
- **Blending modes** - Full alpha blending support

## Creating a Surface Renderer

```cpp
#include <sdlpp/video/surface_renderer.hh>

// Create from existing surface
sdlpp::surface surface(800, 600, SDL_PIXELFORMAT_ARGB8888);
sdlpp::surface_renderer renderer(surface);

// Create with new surface
sdlpp::surface_renderer renderer2(1024, 768, SDL_PIXELFORMAT_RGBA8888);

// Access the underlying surface
SDL_Surface* raw_surface = renderer.get_surface();
```

## Basic Drawing

The surface renderer supports all the same drawing operations as the hardware renderer:

```cpp
// Set drawing color
renderer.set_draw_color({255, 0, 0, 255}); // Red

// Clear surface
renderer.clear();

// Draw primitives
renderer.draw_point({100, 100});
renderer.draw_line({0, 0}, {100, 100});
renderer.draw_rect({{50, 50}, {200, 150}});
renderer.fill_rect({{300, 50}, {100, 100}});

// The surface is immediately updated (no present() needed)
```

## DDA Algorithms

All DDA algorithms from the hardware renderer are available:

```cpp
// Antialiased lines
renderer.draw_line_aa({0.5f, 0.5f}, {99.5f, 99.5f});

// Thick lines
renderer.draw_line_thick({10, 10}, {90, 90}, 5.0f);

// Circles and ellipses
renderer.draw_circle({200, 200}, 50);
renderer.fill_ellipse({300, 300}, 80, 60);

// Bezier curves
renderer.draw_bezier_cubic({0, 0}, {30, 100}, {70, 100}, {100, 0});

// Polygons
std::vector<sdlpp::point<int>> vertices = {
    {100, 100}, {200, 100}, {250, 200}, {150, 250}, {50, 200}
};
renderer.fill_polygon(vertices);
```

## Direct Pixel Access

Unlike hardware renderer, surface renderer allows direct pixel manipulation:

```cpp
// Set individual pixels (fast path)
renderer.put_pixel(100, 100, 0xFF0000FF); // RGBA format

// Get pixel value
uint32_t pixel = renderer.get_pixel(100, 100);

// Batch pixel operations with locking
{
    sdlpp::surface_renderer::surface_lock lock(renderer.get_surface());
    if (lock.is_locked()) {
        // Direct memory access while locked
        for (int y = 0; y < 100; ++y) {
            for (int x = 0; x < 100; ++x) {
                renderer.put_pixel(x, y, 0xFF0000FF);
            }
        }
    }
}
```

## Blending

Surface renderer supports all blend modes:

```cpp
// Set blend mode for drawing operations
renderer.set_draw_blend_mode(sdlpp::blend_mode::blend);

// Draw with alpha blending
renderer.set_draw_color({255, 0, 0, 128}); // 50% transparent red
renderer.fill_rect({{100, 100}, {200, 200}});

// Available blend modes:
// - none: Overwrite pixels
// - blend: Alpha blending
// - add: Additive blending
// - mod: Color modulation
// - mul: Color multiplication
```

## Surface Blending

Blend one surface onto another:

```cpp
// Create source surface with content
sdlpp::surface src_surface(100, 100, SDL_PIXELFORMAT_ARGB8888);
sdlpp::surface_renderer src_renderer(src_surface);
src_renderer.set_draw_color({0, 255, 0, 255});
src_renderer.fill_circle({50, 50}, 40);

// Blend onto destination
sdlpp::surface_renderer dst_renderer(dest_surface);
dst_renderer.blend_surface(src_renderer, 
                          std::nullopt,      // Full source
                          {200, 200},        // Destination position
                          sdlpp::blend_mode::blend);
```

## Gradient Fills

Surface renderer provides gradient fill capabilities:

```cpp
// Linear gradient fill
renderer.fill_rect_gradient(
    {{100, 100}, {200, 100}},
    {255, 0, 0, 255},    // Top-left: red
    {0, 255, 0, 255},    // Top-right: green
    {0, 0, 255, 255},    // Bottom-right: blue
    {255, 255, 0, 255}   // Bottom-left: yellow
);
```

## Clipping

Control which areas can be modified:

```cpp
// Set clipping rectangle
renderer.set_clip_rect({{100, 100}, {400, 300}});

// All drawing operations are now clipped
renderer.draw_line({0, 0}, {800, 600}); // Only visible within clip

// Check if clipping is enabled
if (renderer.is_clip_enabled()) {
    auto clip = renderer.get_clip_rect();
}

// Disable clipping
renderer.set_clip_rect(std::nullopt);
```

## Performance Optimization

### Pixel Format

Choose appropriate pixel formats for performance:

```cpp
// 32-bit formats are fastest (aligned access)
SDL_PIXELFORMAT_ARGB8888  // Best for most cases
SDL_PIXELFORMAT_RGBA8888  // Alternative 32-bit

// 16-bit formats use less memory
SDL_PIXELFORMAT_RGB565    // No alpha
SDL_PIXELFORMAT_ARGB1555  // 1-bit alpha
```

### Batching Operations

```cpp
// Lock surface once for multiple operations
{
    sdlpp::surface_renderer::surface_lock lock(renderer.get_surface());
    
    // All operations here avoid lock/unlock overhead
    for (const auto& point : points) {
        renderer.put_pixel(point.x, point.y, color);
    }
}
```

### Direct Memory Access

For maximum performance, access pixels directly:

```cpp
auto* surface = renderer.get_surface();
auto* pixels = static_cast<uint32_t*>(surface->pixels);
int pitch = surface->pitch / sizeof(uint32_t);

// Direct write (ensure surface is locked!)
pixels[y * pitch + x] = 0xFF0000FF;
```

## Converting to Texture

Use surface renderer for offline rendering, then convert to texture:

```cpp
// Render to surface
sdlpp::surface surface(512, 512, SDL_PIXELFORMAT_ARGB8888);
sdlpp::surface_renderer sw_renderer(surface);

// Draw complex scene
render_complex_scene(sw_renderer);

// Convert to texture for hardware rendering
auto texture = sdlpp::texture::create(hw_renderer, surface);
```

## Use Cases

Surface renderer is ideal for:

1. **Image processing** - Filters, effects, transformations
2. **Procedural generation** - Textures, patterns, noise
3. **Pixel art tools** - Precise pixel control
4. **Software fallback** - When hardware acceleration unavailable
5. **Headless rendering** - Server-side image generation
6. **Testing** - Deterministic rendering for unit tests

## Comparison with Hardware Renderer

| Feature | Hardware Renderer | Surface Renderer |
|---------|------------------|------------------|
| Performance | Fast (GPU) | Slower (CPU) |
| Pixel Access | No | Yes |
| Texture Support | Yes | No |
| Present Required | Yes | No |
| Memory Location | GPU | System RAM |
| Deterministic | No | Yes |
| Headless Support | No | Yes |

## Example: Image Filter

```cpp
void apply_blur_filter(sdlpp::surface& image) {
    sdlpp::surface_renderer renderer(image);
    sdlpp::surface temp(image.get_width(), image.get_height(), 
                       image.get_format());
    
    // Simple box blur
    for (int y = 1; y < image.get_height() - 1; ++y) {
        for (int x = 1; x < image.get_width() - 1; ++x) {
            uint32_t sum_r = 0, sum_g = 0, sum_b = 0;
            
            // Sample 3x3 area
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    auto pixel = renderer.get_pixel(x + dx, y + dy);
                    // Extract and accumulate color components
                    // ... (format-specific code)
                }
            }
            
            // Write averaged pixel to temp surface
            // ...
        }
    }
}
```

## Error Handling

All operations return `expected` for error handling:

```cpp
auto result = renderer.set_draw_color({255, 0, 0, 255});
if (!result) {
    std::cerr << "Failed: " << result.error() << "\n";
}

// Chain operations
auto draw_result = renderer.clear()
    .and_then([&]() { return renderer.draw_circle({100, 100}, 50); })
    .and_then([&]() { return renderer.fill_rect({{200, 200}, {100, 100}}); });
```

## Best Practices

1. **Lock surfaces** - Use RAII lock guard for batch operations
2. **Choose format wisely** - 32-bit formats are fastest
3. **Minimize format conversions** - Keep surfaces in same format
4. **Use appropriate renderer** - Hardware for display, software for processing
5. **Profile performance** - CPU rendering can be slow for large surfaces
6. **Cache results** - Pre-render to surfaces when possible

## See Also

- [Hardware Renderer](renderer.md) - GPU-accelerated rendering
- [Surface](surface.md) - Surface management
- [Renderer Concepts](../../concepts/renderer-concepts.md) - Generic renderer programming
- [DDA Algorithms](../../guides/dda-rendering.md) - Understanding DDA