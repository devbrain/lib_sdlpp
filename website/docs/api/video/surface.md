---
sidebar_position: 3
---

# Surface

The surface module provides software rendering capabilities through pixel manipulation. Surfaces are in-memory representations of images that can be modified directly and used for software-based graphics operations.

## Basic Usage

### Creating Surfaces

```cpp
#include <sdlpp/video/surface.hh>

// Create a blank surface
auto surface = sdlpp::surface::create(800, 600, sdlpp::pixel_format::rgba8888);
if (!surface) {
    std::cerr << "Failed to create surface: " << surface.error() << "\n";
    return -1;
}

// Load from file
auto image = sdlpp::surface::load("image.bmp");
if (!image) {
    std::cerr << "Failed to load image: " << image.error() << "\n";
    return -1;
}

// Create from existing pixel data
std::vector<uint32_t> pixels(640 * 480, 0xFF0000FF); // Red pixels
auto from_pixels = sdlpp::surface::create_from(
    pixels.data(),
    640, 480,
    640 * sizeof(uint32_t),  // pitch
    sdlpp::pixel_format::rgba8888
);
```

### Surface Properties

```cpp
// Get surface dimensions
int width = surface->get_width();
int height = surface->get_height();
auto [w, h] = surface->get_size();

// Get pixel format info
auto format = surface->get_format();
int bpp = format->bits_per_pixel;
int pitch = surface->get_pitch();

// Get pixel data pointer
void* pixels = surface->pixels();
const void* const_pixels = surface->pixels() const;
```

## Pixel Manipulation

### Direct Pixel Access

```cpp
// Lock surface for pixel access (required for some surfaces)
{
    auto lock = surface->lock();
    
    // Access pixels as array
    auto* pixels = static_cast<uint32_t*>(surface->pixels());
    int pitch = surface->get_pitch() / sizeof(uint32_t);
    
    // Set pixel at (x, y) to red
    int x = 100, y = 50;
    pixels[y * pitch + x] = 0xFF0000FF; // RGBA
}
// Surface automatically unlocked when lock goes out of scope

// Alternative: Use put_pixel/get_pixel (slower but safer)
surface->put_pixel(100, 50, {255, 0, 0, 255});
auto color = surface->get_pixel(100, 50);
```

### Fill Operations

```cpp
// Fill entire surface
surface->fill({255, 128, 0, 255}); // Orange

// Fill rectangle
sdlpp::rect area{{50, 50}, {200, 150}};
surface->fill_rect(area, {0, 255, 0, 255}); // Green rectangle

// Fill multiple rectangles
std::vector<sdlpp::rect> rects = {
    {{10, 10}, {50, 50}},
    {{70, 10}, {50, 50}},
    {{130, 10}, {50, 50}}
};
surface->fill_rects(rects, {0, 0, 255, 255}); // Blue rectangles
```

## Blitting (Surface Copying)

### Basic Blitting

```cpp
// Copy entire source to destination at (0, 0)
auto result = sdlpp::surface::blit(source, destination);

// Copy to specific position
auto result = sdlpp::surface::blit(source, destination, {100, 50});

// Copy specific region
sdlpp::rect src_rect{{0, 0}, {64, 64}};
auto result = sdlpp::surface::blit(source, src_rect, destination, {200, 100});
```

### Scaled Blitting

```cpp
// Scale entire surface to fit destination
auto result = sdlpp::surface::blit_scaled(source, destination);

// Scale to specific rectangle
sdlpp::rect dst_rect{{50, 50}, {200, 200}};
auto result = sdlpp::surface::blit_scaled(source, destination, dst_rect);

// Scale specific region to specific rectangle
sdlpp::rect src_rect{{0, 0}, {32, 32}};
sdlpp::rect dst_rect{{100, 100}, {128, 128}};
auto result = sdlpp::surface::blit_scaled(source, src_rect, destination, dst_rect);
```

## Format Conversion

### Converting Pixel Formats

```cpp
// Convert to specific format
auto converted = surface->convert_format(sdlpp::pixel_format::rgb888);
if (!converted) {
    std::cerr << "Conversion failed: " << converted.error() << "\n";
}

// Convert using another surface's format
auto target_format = destination->get_format();
auto converted = surface->convert(target_format);

// Optimize for display (deprecated in SDL3, but concept remains)
auto optimized = surface->convert_format(
    sdlpp::get_window_pixel_format(*window)
);
```

### Premultiplied Alpha

```cpp
// Convert to premultiplied alpha (better for blending)
auto premultiplied = surface->premultiply_alpha();

// Check if surface has premultiplied alpha
bool is_premultiplied = surface->has_premultiplied_alpha();
```

## Color Key and Blending

### Color Key (Transparency)

```cpp
// Set color key (make specific color transparent)
surface->set_color_key(true, {255, 0, 255}); // Magenta = transparent

// Get current color key
auto [enabled, color] = surface->get_color_key();

// Disable color key
surface->set_color_key(false);
```

### Blend Modes

```cpp
// Set blend mode
surface->set_blend_mode(sdlpp::blend_mode::blend);

// Available blend modes:
// - none: Copy pixels directly
// - blend: Alpha blending (default)
// - add: Additive blending
// - mod: Color modulation
// - mul: Color multiplication

// Get current blend mode
auto mode = surface->get_blend_mode();
```

### Surface Modulation

```cpp
// Color modulation (tinting)
surface->set_color_mod({255, 128, 128}); // Reddish tint
auto [r, g, b] = surface->get_color_mod();

// Alpha modulation (transparency)
surface->set_alpha_mod(128); // 50% transparent
auto alpha = surface->get_alpha_mod();
```

## Clipping

```cpp
// Set clipping rectangle
sdlpp::rect clip{{50, 50}, {200, 200}};
surface->set_clip_rect(clip);

// Get current clip rectangle
auto current_clip = surface->get_clip_rect();

// Disable clipping
surface->set_clip_rect(std::nullopt);

// Check if point is in clip rect
bool in_clip = surface->clip_rect_contains({100, 100});
```

## Surface Transformations

### Flipping

```cpp
// Flip horizontally
auto flipped_h = surface->flip(sdlpp::flip_mode::horizontal);

// Flip vertically  
auto flipped_v = surface->flip(sdlpp::flip_mode::vertical);

// Flip both
auto flipped_both = surface->flip(
    sdlpp::flip_mode::horizontal | sdlpp::flip_mode::vertical
);
```

### Rotation and Scaling

```cpp
// Rotate 90 degrees clockwise
auto rotated = surface->rotate_90(1);

// Rotate 180 degrees
auto rotated_180 = surface->rotate_90(2);

// Rotate 270 degrees (90 counter-clockwise)
auto rotated_270 = surface->rotate_90(3);

// Scale by factor
auto scaled = surface->scale(2.0f); // Double size

// Scale to specific size
auto resized = surface->scale_to(1920, 1080);
```

## Palette Management

### Working with Palettized Surfaces

```cpp
// Check if surface has palette
if (surface->get_format()->palette) {
    auto palette = surface->get_format()->palette;
    
    // Get number of colors
    int num_colors = palette->ncolors;
    
    // Access colors
    for (int i = 0; i < num_colors; ++i) {
        auto& color = palette->colors[i];
        // Use color.r, color.g, color.b, color.a
    }
    
    // Set palette colors
    std::vector<sdlpp::color> new_colors = {
        {255, 0, 0, 255},    // Red
        {0, 255, 0, 255},    // Green
        {0, 0, 255, 255},    // Blue
        // ... up to 256 colors
    };
    surface->set_palette_colors(new_colors, 0); // Starting at index 0
}
```

## Advanced Operations

### Custom Pixel Operations

```cpp
// Apply custom operation to each pixel
surface->map_pixels([](uint32_t pixel) -> uint32_t {
    // Example: Invert colors
    return pixel ^ 0x00FFFFFF; // Keep alpha, invert RGB
});

// Process with position info
surface->map_pixels_xy([](int x, int y, uint32_t pixel) -> uint32_t {
    // Example: Gradient based on position
    uint8_t intensity = static_cast<uint8_t>((x + y) / 2);
    return (pixel & 0xFF000000) | (intensity << 16) | (intensity << 8) | intensity;
});
```

### Surface Composition

```cpp
// Composite multiple surfaces
auto background = sdlpp::surface::create(800, 600, sdlpp::pixel_format::rgba8888);
background->fill({64, 64, 64, 255});

auto sprite1 = sdlpp::surface::load("sprite1.png");
auto sprite2 = sdlpp::surface::load("sprite2.png");

// Layer sprites on background
sprite1->set_blend_mode(sdlpp::blend_mode::blend);
sprite2->set_blend_mode(sdlpp::blend_mode::blend);

sdlpp::surface::blit(*sprite1, *background, {100, 100});
sdlpp::surface::blit(*sprite2, *background, {200, 150});
```

## Performance Considerations

### Format Matching

```cpp
// Convert surfaces to matching format for faster blitting
auto src_format = source->get_format();
auto dst_format = destination->get_format();

if (src_format != dst_format) {
    source = source->convert(dst_format);
}
```

### Batch Operations

```cpp
// Lock once for multiple operations
{
    auto lock = surface->lock();
    
    // Perform multiple pixel operations
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Direct pixel manipulation
        }
    }
} // Unlock once
```

### Pre-multiplication

```cpp
// Pre-multiply alpha for better blending performance
if (!surface->has_premultiplied_alpha()) {
    surface = surface->premultiply_alpha();
}
```

## Common Patterns

### Loading and Caching Images

```cpp
class image_cache {
    std::unordered_map<std::string, sdlpp::surface> cache;
    
public:
    sdlpp::surface* load(const std::string& path) {
        auto it = cache.find(path);
        if (it != cache.end()) {
            return &it->second;
        }
        
        auto surface = sdlpp::surface::load(path);
        if (surface) {
            auto [inserted, success] = cache.emplace(path, std::move(*surface));
            return &inserted->second;
        }
        return nullptr;
    }
};
```

### Software Rendering Pipeline

```cpp
// Create off-screen surface for rendering
auto framebuffer = sdlpp::surface::create(800, 600, sdlpp::pixel_format::rgba8888);

// Render scene
framebuffer->fill({0, 0, 0, 255}); // Clear to black
render_background(*framebuffer);
render_sprites(*framebuffer);
render_ui(*framebuffer);

// Copy to window
auto window_surface = window->get_surface();
sdlpp::surface::blit(*framebuffer, *window_surface);
window->update_surface();
```

## Error Handling

```cpp
// All operations return expected<T, string>
auto surface = sdlpp::surface::create(800, 600, sdlpp::pixel_format::rgba8888);
if (!surface) {
    std::cerr << "Failed to create surface: " << surface.error() << "\n";
    return;
}

// Chain operations
auto result = surface->convert_format(sdlpp::pixel_format::rgb888)
    .and_then([](auto&& s) { return s.flip(sdlpp::flip_mode::horizontal); })
    .and_then([](auto&& s) { return s.scale(2.0f); });

if (!result) {
    std::cerr << "Operation failed: " << result.error() << "\n";
}
```

## Best Practices

1. **Match Formats**: Convert surfaces to the same format before blitting
2. **Lock Appropriately**: Lock surfaces when doing direct pixel access
3. **Use Color Key**: For simple transparency without alpha channel
4. **Batch Operations**: Group pixel operations to minimize locking
5. **Cache Surfaces**: Load images once and reuse
6. **Consider Hardware**: Use renderer/textures for better performance
7. **Profile First**: Software rendering can be fast enough for many uses

## API Reference

### Classes

- `surface` - Main surface class
- `pixel_format` - Pixel format information
- `palette` - Color palette for indexed formats

### Enums

- `blend_mode` - Blending modes
- `flip_mode` - Flipping directions
- `pixel_format_enum` - Pixel format constants

### Free Functions

- `surface::create()` - Create new surface
- `surface::load()` - Load from file
- `surface::blit()` - Copy surfaces
- `surface::blit_scaled()` - Copy with scaling