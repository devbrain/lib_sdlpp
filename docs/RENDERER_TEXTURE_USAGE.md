# Renderer and Texture Usage Guide

## Overview

The renderer and texture modules provide modern C++ wrappers around SDL3's 2D hardware-accelerated rendering system. Renderers manage the rendering context and drawing operations, while textures represent GPU-resident images for fast rendering.

## Key Classes

### `renderer`
Hardware-accelerated 2D rendering context.

### `texture`
GPU-resident image for fast rendering.

### Supporting Types
- `blend_mode` - Blending modes for rendering
- `scale_mode` - Scaling algorithms (nearest/linear)
- `flip_mode` - Texture flipping options
- `texture_access` - Texture access patterns

## Basic Usage

### Initialize SDL and Create Window

```cpp
#include <sdlpp/sdl.hh>
#include "sdlpp/renderer.hh"
#include "sdlpp/texture.hh"

// Initialize SDL
if (!SDL_Init(SDL_INIT_VIDEO)) {
    std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
    return -1;
}

// Create window
SDL_Window* window = SDL_CreateWindow(
    "Renderer Example",
    640, 480,
    SDL_WINDOW_SHOWN
);

// Create renderer
auto renderer = sdlpp::renderer::create(window);
if (!renderer) {
    std::cerr << "Failed to create renderer: " << renderer.error() << std::endl;
    return -1;
}
```

### Basic Drawing Operations

```cpp
// Clear screen with color
renderer->set_draw_color(sdlpp::colors::black);
renderer->clear();

// Draw primitives
renderer->set_draw_color(sdlpp::colors::white);
renderer->draw_point(100, 100);
renderer->draw_line(0, 0, 640, 480);

// Draw rectangles
sdlpp::rect outline{10, 10, 100, 100};
renderer->draw_rect(outline);

sdlpp::rect filled{120, 10, 100, 100};
renderer->fill_rect(filled);

// Present to screen
renderer->present();
```

### Flexible Copy Methods

The renderer's `copy` and `copy_ex` methods now accept both integer (`rect`) and floating-point (`frect`) rectangles:

```cpp
// Integer rectangles for pixel-perfect alignment
renderer->copy(*texture, 
    sdlpp::rect{0, 0, 64, 64},      // Source: top-left quarter
    sdlpp::rect{100, 100, 128, 128} // Destination: scaled 2x
);

// Floating-point rectangles for smooth animation and sub-pixel positioning
renderer->copy(*texture,
    sdlpp::frect{0.0f, 0.0f, 64.0f, 64.0f},     // Source
    sdlpp::frect{100.5f, 100.5f, 128.0f, 128.0f} // Destination with sub-pixel position
);

// Mix integer source with floating-point destination
renderer->copy_ex(*texture,
    sdlpp::rect{0, 0, 64, 64},              // Integer source rect
    sdlpp::frect{150.25f, 150.75f, 64.0f, 64.0f}, // Float destination
    45.0,                                    // Rotation
    sdlpp::fpoint{32.0f, 32.0f},           // Center point
    sdlpp::flip_mode::none
);
```

### Working with Textures

```cpp
// Create texture from surface
auto surface = sdlpp::surface::create_rgb(256, 256, sdlpp::pixel_format_enum::RGBA8888);
surface->fill(sdlpp::colors::blue);

auto texture = sdlpp::texture::create(*renderer, *surface);
if (!texture) {
    std::cerr << "Failed to create texture: " << texture.error() << std::endl;
    return;
}

// Render texture
renderer->copy(*texture);  // Full texture to full target
renderer->copy(*texture, std::nullopt, sdlpp::rect{100, 100, 256, 256});  // Positioned with integer rect
renderer->copy(*texture, std::nullopt, sdlpp::frect{100.5f, 100.5f, 256.0f, 256.0f});  // Sub-pixel positioning

// Render with rotation and flip
renderer->copy_ex(*texture, 
    std::nullopt,                    // Source rect (entire texture)
    sdlpp::rect{320, 240, 128, 128}, // Destination
    45.0,                            // Rotation angle
    std::nullopt,                    // Rotation center (default: center)
    sdlpp::flip_mode::horizontal    // Flip mode
);
```

## Advanced Features

### Blend Modes

```cpp
// Set blend mode for primitives
renderer->set_draw_blend_mode(sdlpp::blend_mode::blend);
renderer->set_draw_color(sdlpp::color{255, 0, 0, 128}); // Semi-transparent red
renderer->fill_rect(sdlpp::rect{50, 50, 200, 200});

// Set blend mode for textures
texture->set_blend_mode(sdlpp::blend_mode::add);
renderer->copy(*texture);
```

### Color and Alpha Modulation

```cpp
// Modulate texture color (tint)
texture->set_color_mod(sdlpp::color{255, 128, 128}); // Reddish tint

// Modulate alpha (transparency)
texture->set_alpha_mod(128); // 50% transparent

// Combined effect
renderer->copy(*texture);
```

### Streaming Textures

```cpp
// Create streaming texture for dynamic content
auto streaming = sdlpp::texture::create(
    *renderer,
    sdlpp::pixel_format_enum::RGBA8888,
    sdlpp::texture_access::streaming,
    320, 240
);

// Update texture pixels
{
    sdlpp::texture::lock_guard lock(*streaming);
    if (lock.is_locked()) {
        // Write to lock.pixels with pitch lock.pitch
        uint32_t* pixels = static_cast<uint32_t*>(lock.pixels);
        // ... update pixels ...
    }
} // Automatically unlocked

// Alternative: update specific region
std::vector<uint32_t> pixel_data(320 * 240);
// ... fill pixel_data ...
streaming->update(std::nullopt, pixel_data.data(), 320 * sizeof(uint32_t));
```

### Render Targets

```cpp
// Create render target texture
auto target = sdlpp::texture::create(
    *renderer,
    sdlpp::pixel_format_enum::RGBA8888,
    sdlpp::texture_access::target,
    512, 512
);

// Set as render target
renderer->set_target(*target);

// Draw to texture
renderer->set_draw_color(sdlpp::colors::black);
renderer->clear();
renderer->set_draw_color(sdlpp::colors::green);
renderer->fill_rect(sdlpp::rect{100, 100, 312, 312});

// Reset to default target (window)
renderer->set_target(sdlpp::texture());

// Now use the texture with rendered content
renderer->copy(*target);
```

### Viewport and Clipping

```cpp
// Set viewport (render to subsection of target)
renderer->set_viewport(sdlpp::rect{0, 0, 320, 240});
renderer->clear(); // Only clears viewport area

// Reset viewport
renderer->set_viewport(std::nullopt);

// Set clipping rectangle
renderer->set_clip_rect(sdlpp::rect{100, 100, 440, 280});
renderer->fill_rect(sdlpp::rect{0, 0, 640, 480}); // Only visible in clip area

// Check if clipping enabled
if (renderer->is_clip_enabled()) {
    auto clip = renderer->get_clip_rect();
    // ...
}
```

### Render Scale

```cpp
// Set logical size with automatic scaling
renderer->set_scale(2.0f, 2.0f); // Everything drawn at 2x size

// Get current scale
auto scale = renderer->get_scale();
if (scale) {
    std::cout << "Scale: " << scale->x << "x" << scale->y << std::endl;
}
```

## Software Rendering

```cpp
// Create surface for software rendering
auto surface = sdlpp::surface::create_rgb(640, 480, sdlpp::pixel_format_enum::RGBA8888);

// Create software renderer
auto sw_renderer = sdlpp::renderer::create_software(surface->get());
if (!sw_renderer) {
    std::cerr << "Failed to create software renderer" << std::endl;
    return;
}

// Use same drawing operations
sw_renderer->set_draw_color(sdlpp::colors::red);
sw_renderer->clear();
sw_renderer->draw_line(0, 0, 640, 480);
sw_renderer->present();

// Surface now contains the rendered image
```

## Performance Tips

### Texture Creation
```cpp
// Static textures (uploaded once, drawn many times)
auto static_tex = sdlpp::texture::create(
    *renderer,
    sdlpp::pixel_format_enum::RGBA8888,
    sdlpp::texture_access::static_access,
    width, height
);

// Streaming textures (updated frequently)
auto stream_tex = sdlpp::texture::create(
    *renderer,
    sdlpp::pixel_format_enum::RGBA8888,
    sdlpp::texture_access::streaming,
    width, height
);

// Target textures (rendered to)
auto target_tex = sdlpp::texture::create(
    *renderer,
    sdlpp::pixel_format_enum::RGBA8888,
    sdlpp::texture_access::target,
    width, height
);
```

### Batch Operations
```cpp
// Batch similar operations together
renderer->set_draw_color(sdlpp::colors::white);

std::vector<sdlpp::point> points;
for (int i = 0; i < 1000; ++i) {
    points.push_back({rand() % 640, rand() % 480});
}
renderer->draw_points(points); // Much faster than individual draw_point calls

// Batch rectangle fills
std::vector<sdlpp::rect> rects;
// ... fill rects ...
renderer->fill_rects(rects);
```

### Texture Formats
```cpp
// Use format matching the renderer for best performance
auto props = renderer->get_properties();
// Query renderer's preferred format from properties
// Create textures with matching format
```

## Error Handling

All operations return `sdlpp::expected`:

```cpp
auto result = renderer->set_draw_color(sdlpp::colors::red);
if (!result) {
    std::cerr << "Error: " << result.error() << std::endl;
}

// Chain operations with early return
auto draw_scene() -> sdlpp::expected<void, std::string> {
    TRY(renderer->clear());
    TRY(renderer->set_draw_color(sdlpp::colors::white));
    TRY(renderer->draw_rect(sdlpp::rect{10, 10, 100, 100}));
    TRY(renderer->present());
    return {};
}
```

## VSync Control

```cpp
// Enable VSync (1) 
renderer->set_vsync(1);

// Disable VSync (0)
renderer->set_vsync(0);

// Adaptive VSync (-1) - tears if frame rate would drop
renderer->set_vsync(-1);

// Check current setting
auto vsync = renderer->get_vsync();
```

## Common Patterns

### Game Loop
```cpp
bool running = true;
while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
    }
    
    // Clear
    renderer->set_draw_color(sdlpp::colors::black);
    renderer->clear();
    
    // Draw game objects
    for (const auto& sprite : sprites) {
        renderer->copy(sprite.texture, std::nullopt, sprite.position);
    }
    
    // Present
    renderer->present();
}
```

### Sprite Sheet
```cpp
class sprite_sheet {
    sdlpp::texture texture;
    int sprite_width, sprite_height;
    
public:
    void draw_sprite(sdlpp::renderer& rend, int index, const sdlpp::point& pos) {
        int sprites_per_row = texture.get_size()->width / sprite_width;
        int row = index / sprites_per_row;
        int col = index % sprites_per_row;
        
        sdlpp::rect src{
            col * sprite_width,
            row * sprite_height,
            sprite_width,
            sprite_height
        };
        
        sdlpp::rect dst{pos.x, pos.y, sprite_width, sprite_height};
        rend.copy(texture, src, dst);
    }
};
```

### Render-to-Texture Effects
```cpp
// Create effect pipeline
auto create_blur_effect(sdlpp::renderer& rend, const sdlpp::texture& input) 
    -> sdlpp::expected<sdlpp::texture, std::string> {
    
    auto size = TRY(input.get_size());
    
    // Create intermediate texture
    auto temp = TRY(sdlpp::texture::create(
        rend,
        sdlpp::pixel_format_enum::RGBA8888,
        sdlpp::texture_access::target,
        size.width, size.height
    ));
    
    // Horizontal blur pass
    TRY(rend.set_target(temp));
    TRY(rend.clear());
    
    // Multiple offset copies with alpha
    TRY(input.set_alpha_mod(64));
    for (int offset = -2; offset <= 2; ++offset) {
        sdlpp::rect dst{offset, 0, size.width, size.height};
        TRY(rend.copy(input, std::nullopt, dst));
    }
    
    // Reset
    TRY(rend.set_target(sdlpp::texture()));
    TRY(input.set_alpha_mod(255));
    
    return temp;
}
```

## Platform Notes

- **Windows**: Direct3D 11/12 renderers available
- **macOS**: Metal renderer recommended
- **Linux**: OpenGL/Vulkan renderers
- **Mobile**: OpenGL ES renderers

Use `renderer_driver::*` constants to request specific backends:

```cpp
auto renderer = sdlpp::renderer::create(window, sdlpp::renderer_driver::metal);
```

## Best Practices

1. **One Renderer Per Window**: Create exactly one renderer per window
2. **Texture Formats**: Match texture format to renderer for performance
3. **Batch Operations**: Group similar draw calls together
4. **Clear Before Drawing**: Always clear before starting a new frame
5. **Lock Streaming Textures**: Use lock_guard for safe pixel access
6. **Check Capabilities**: Not all blend modes/features supported on all backends
7. **Handle Lost Context**: Recreate textures if renderer is reset