---
title: Texture Management
sidebar_label: Texture
---

# Texture Management

SDL++ provides RAII-based texture management for efficient GPU-based rendering.

## Creating Textures

### From Surface

```cpp
#include <sdlpp/video/texture.hh>
#include <sdlpp/video/surface.hh>

// Load surface first
auto surface_result = load_image("sprite.png");
if (!surface_result) {
    std::cerr << "Failed to load image: " << surface_result.error() << std::endl;
    return;
}

// Create texture from surface
auto texture_result = renderer.create_texture(surface_result.value());
if (!texture_result) {
    std::cerr << "Failed to create texture: " << texture_result.error() << std::endl;
    return;
}

auto& texture = texture_result.value();
```

### Direct Creation

```cpp
// Create empty texture for rendering target
auto target_texture = renderer.create_texture(
    sdlpp::pixel_format_enum::RGBA8888,
    sdlpp::texture_access::target,
    1920, 1080
);

// Create streaming texture for dynamic updates
auto streaming_texture = renderer.create_texture(
    sdlpp::pixel_format_enum::RGB24,
    sdlpp::texture_access::streaming,
    640, 480
);

// Create static texture (default)
auto static_texture = renderer.create_texture(
    sdlpp::pixel_format_enum::RGBA8888,
    sdlpp::texture_access::static_,
    256, 256
);
```

## Texture Properties

### Query Information

```cpp
// Get texture info
auto info = texture.query();
if (info) {
    std::cout << "Format: " << static_cast<int>(info->format) << std::endl;
    std::cout << "Access: " << static_cast<int>(info->access) << std::endl;
    std::cout << "Size: " << info->width << "x" << info->height << std::endl;
}

// Get just size
auto [width, height] = texture.get_size();
```

### Blend Mode

```cpp
// Set blend mode for transparency
auto result = texture.set_blend_mode(sdlpp::blend_mode::blend);

// Available blend modes:
// - none: No blending (copy pixels)
// - blend: Alpha blending (default)
// - add: Additive blending
// - mod: Color modulation
// - mul: Multiply

// Get current blend mode
auto mode = texture.get_blend_mode();
```

### Color Modulation

```cpp
// Tint texture red
texture.set_color_mod(255, 128, 128);

// Get current color mod
auto [r, g, b] = texture.get_color_mod();

// Reset to white (no tint)
texture.set_color_mod(255, 255, 255);
```

### Alpha Modulation

```cpp
// Set transparency (0-255)
texture.set_alpha_mod(128);  // 50% transparent

// Get current alpha
Uint8 alpha = texture.get_alpha_mod();

// Fade in/out effect
void fade_texture(sdlpp::texture& tex, float alpha) {
    tex.set_alpha_mod(static_cast<Uint8>(alpha * 255));
}
```

## Rendering Textures

### Basic Rendering

```cpp
// Render entire texture at position
renderer.copy(texture, sdlpp::point_i{100, 100});

// Render to specific destination rectangle
renderer.copy(texture, sdlpp::rect_i{100, 100, 200, 150});

// Render with source rectangle (sprite sheet)
sdlpp::rect_i src{0, 0, 32, 32};  // First sprite
sdlpp::rect_i dst{100, 100, 64, 64};  // Scale 2x
renderer.copy(texture, src, dst);
```

### Advanced Rendering

```cpp
// Render with rotation and flip
renderer.copy_ex(
    texture,
    std::nullopt,  // Source rect (entire texture)
    sdlpp::rect_i{400, 300, 100, 100},  // Destination
    45.0,  // Rotation angle in degrees
    sdlpp::point_i{50, 50},  // Rotation center
    sdlpp::renderer_flip::none  // No flip
);

// Flip options:
// - none: No flipping
// - horizontal: Flip horizontally
// - vertical: Flip vertically
// - horizontal | vertical: Flip both
```

## Streaming Textures

For dynamic content like video frames:

```cpp
class video_player {
    sdlpp::texture texture;
    
public:
    bool init(sdlpp::renderer& r, int width, int height) {
        auto tex_result = r.create_texture(
            sdlpp::pixel_format_enum::YV12,  // YUV format
            sdlpp::texture_access::streaming,
            width, height
        );
        
        if (!tex_result) return false;
        texture = std::move(tex_result.value());
        return true;
    }
    
    void update_frame(const uint8_t* y_plane, const uint8_t* u_plane, 
                     const uint8_t* v_plane, int y_pitch, int uv_pitch) {
        // Lock texture for writing
        auto lock_result = texture.lock();
        if (!lock_result) return;
        
        auto& [pixels, pitch] = lock_result.value();
        
        // Copy YUV data
        uint8_t* dst = static_cast<uint8_t*>(pixels);
        // Copy Y plane, U plane, V plane...
        
        // Unlock automatically when lock_result goes out of scope
    }
    
    void render(sdlpp::renderer& r, const sdlpp::rect_i& dst) {
        r.copy(texture, dst);
    }
};
```

## Render Targets

Use textures as render targets:

```cpp
class render_to_texture {
    sdlpp::texture target;
    
public:
    bool create_target(sdlpp::renderer& r, int width, int height) {
        auto tex_result = r.create_texture(
            sdlpp::pixel_format_enum::RGBA8888,
            sdlpp::texture_access::target,
            width, height
        );
        
        if (!tex_result) return false;
        target = std::move(tex_result.value());
        return true;
    }
    
    void render_scene(sdlpp::renderer& r) {
        // Set texture as render target
        auto old_target = r.get_render_target();
        r.set_render_target(target);
        
        // Clear and render to texture
        r.set_draw_color({0, 0, 0, 0});  // Transparent
        r.clear();
        
        // Draw scene
        draw_game_world(r);
        
        // Restore original target
        r.set_render_target(old_target);
    }
    
    void apply_post_processing(sdlpp::renderer& r) {
        // Render texture with effects
        target.set_blend_mode(sdlpp::blend_mode::add);
        r.copy(target, std::nullopt);  // Full screen
    }
};
```

## Texture Atlas/Sprite Sheet

```cpp
class sprite_sheet {
    sdlpp::texture texture;
    int sprite_width;
    int sprite_height;
    int columns;
    
public:
    sprite_sheet(sdlpp::texture tex, int sw, int sh)
        : texture(std::move(tex)), sprite_width(sw), sprite_height(sh) {
        auto [w, h] = texture.get_size();
        columns = w / sprite_width;
    }
    
    sdlpp::rect_i get_sprite_rect(int index) const {
        int row = index / columns;
        int col = index % columns;
        
        return {
            col * sprite_width,
            row * sprite_height,
            sprite_width,
            sprite_height
        };
    }
    
    void draw_sprite(sdlpp::renderer& r, int index, 
                    const sdlpp::rect_i& dst) {
        auto src = get_sprite_rect(index);
        r.copy(texture, src, dst);
    }
};
```

## Texture Cache

```cpp
class texture_cache {
    std::unordered_map<std::string, sdlpp::texture> textures;
    sdlpp::renderer* renderer;
    
public:
    explicit texture_cache(sdlpp::renderer& r) : renderer(&r) {}
    
    sdlpp::texture* load(const std::string& path) {
        // Check cache first
        if (auto it = textures.find(path); it != textures.end()) {
            return &it->second;
        }
        
        // Load new texture
        auto surface_result = load_image(path);
        if (!surface_result) {
            std::cerr << "Failed to load: " << path << std::endl;
            return nullptr;
        }
        
        auto texture_result = renderer->create_texture(surface_result.value());
        if (!texture_result) {
            std::cerr << "Failed to create texture: " << path << std::endl;
            return nullptr;
        }
        
        // Cache and return
        auto [it, inserted] = textures.emplace(path, std::move(texture_result.value()));
        return &it->second;
    }
    
    void clear() {
        textures.clear();
    }
    
    void remove(const std::string& path) {
        textures.erase(path);
    }
};
```

## Best Practices

1. **Batch Similar Textures**: Group sprites into texture atlases
2. **Use Appropriate Access**: Static for images, streaming for video, target for RTT
3. **Minimize State Changes**: Batch renders with same texture together
4. **Power of Two**: Some GPUs prefer power-of-two dimensions
5. **Texture Limits**: Check maximum texture size with renderer

## Example: Sprite Animation

```cpp
class animated_sprite {
    sdlpp::texture texture;
    std::vector<sdlpp::rect_i> frames;
    size_t current_frame = 0;
    double frame_time = 0.1;  // 10 FPS
    double accumulator = 0.0;
    
public:
    animated_sprite(sdlpp::texture tex, int frame_width, int frame_height)
        : texture(std::move(tex)) {
        // Generate frame rectangles
        auto [w, h] = texture.get_size();
        
        for (int y = 0; y < h; y += frame_height) {
            for (int x = 0; x < w; x += frame_width) {
                frames.push_back({x, y, frame_width, frame_height});
            }
        }
    }
    
    void update(double dt) {
        accumulator += dt;
        
        while (accumulator >= frame_time) {
            accumulator -= frame_time;
            current_frame = (current_frame + 1) % frames.size();
        }
    }
    
    void draw(sdlpp::renderer& r, const sdlpp::point_i& position) {
        if (frames.empty()) return;
        
        auto src = frames[current_frame];
        sdlpp::rect_i dst{position.x, position.y, src.w, src.h};
        
        r.copy(texture, src, dst);
    }
    
    void set_animation_speed(double fps) {
        frame_time = 1.0 / fps;
    }
    
    void reset() {
        current_frame = 0;
        accumulator = 0.0;
    }
};
```