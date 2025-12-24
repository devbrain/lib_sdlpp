# SDL++ Surface Renderer Implementation Plan

## Overview
This document outlines the implementation plan for a surface-based renderer that provides the same DDA drawing capabilities as the hardware renderer but operates directly on SDL surfaces for software rendering.

## Key Design Principles

### Performance Optimization
- **Function Pointer Dispatch**: Use function pointers for pixel operations based on pixel format
- **Direct Memory Access**: Bypass SDL's per-pixel functions for better performance
- **Format-Specific Optimizations**: Specialized implementations for common formats (RGB888, RGBA8888, etc.)
- **Batch Operations**: Process multiple pixels when possible to improve cache efficiency

### Interface Consistency
- **Same API as Hardware Renderer**: All drawing methods match the hardware renderer
- **Concept-Based Design**: Use C++ concepts instead of concrete types
- **RAII Principles**: Automatic surface locking/unlocking
- **Expected Return Types**: Consistent error handling

## Architecture

### Core Components

```cpp
class surface_renderer {
private:
    // Surface management
    SDL_Surface* surface_;
    bool owns_surface_;
    
    // Pixel operation function pointers
    using put_pixel_func = void(*)(void* pixels, int pitch, int x, int y, uint32_t color);
    using get_pixel_func = uint32_t(*)(const void* pixels, int pitch, int x, int y);
    using blend_pixel_func = void(*)(void* pixels, int pitch, int x, int y, uint32_t color, uint8_t alpha);
    
    put_pixel_func put_pixel_;
    get_pixel_func get_pixel_;
    blend_pixel_func blend_pixel_;
    
    // Current drawing state
    color draw_color_;
    blend_mode blend_mode_;
    rect<int> clip_rect_;
    
    // Format-specific data
    SDL_PixelFormat* format_;
    uint32_t mapped_color_;
    
    // Lock management
    class surface_lock {
        SDL_Surface* surface;
        bool locked;
    public:
        surface_lock(SDL_Surface* s);
        ~surface_lock();
        void* pixels() const;
        int pitch() const;
    };
};
```

## Implementation Phases

### Phase 1: Core Infrastructure and Basic Drawing
**Priority**: Critical  
**Timeline**: Week 1

#### Tasks:
1. **Surface Renderer Class Structure**
   ```cpp
   class surface_renderer {
   public:
       // Constructors
       explicit surface_renderer(SDL_Surface* surface);
       surface_renderer(int width, int height, pixel_format format);
       
       // Basic operations
       expected<void, std::string> clear();
       expected<void, std::string> set_draw_color(const color& c);
       expected<color, std::string> get_draw_color() const;
       
       // Basic drawing
       template<point_like P>
       expected<void, std::string> draw_point(const P& p);
       
       template<point_like P1, point_like P2>
       expected<void, std::string> draw_line(const P1& start, const P2& end);
   };
   ```

2. **Pixel Format Dispatch System**
   ```cpp
   namespace detail {
       // RGB888 (24-bit)
       void put_pixel_rgb888(void* pixels, int pitch, int x, int y, uint32_t color);
       
       // RGBA8888 (32-bit)
       void put_pixel_rgba8888(void* pixels, int pitch, int x, int y, uint32_t color);
       
       // RGB565 (16-bit)
       void put_pixel_rgb565(void* pixels, int pitch, int x, int y, uint32_t color);
       
       // Generic fallback
       void put_pixel_generic(void* pixels, int pitch, int x, int y, uint32_t color, SDL_PixelFormat* fmt);
   }
   ```

3. **Function Pointer Setup**
   ```cpp
   void surface_renderer::setup_pixel_functions() {
       switch (format_->format) {
           case SDL_PIXELFORMAT_RGB888:
               put_pixel_ = &detail::put_pixel_rgb888;
               break;
           case SDL_PIXELFORMAT_RGBA8888:
           case SDL_PIXELFORMAT_ARGB8888:
               put_pixel_ = &detail::put_pixel_rgba8888;
               break;
           case SDL_PIXELFORMAT_RGB565:
               put_pixel_ = &detail::put_pixel_rgb565;
               break;
           default:
               // Use generic with closure
               put_pixel_ = [this](void* pixels, int pitch, int x, int y, uint32_t color) {
                   detail::put_pixel_generic(pixels, pitch, x, y, color, format_);
               };
       }
   }
   ```

### Phase 2: DDA Line Drawing
**Priority**: High  
**Timeline**: Week 1-2

#### Methods to Implement:
```cpp
// Antialiased lines
template<point_like P1, point_like P2>
expected<void, std::string> draw_line_aa(const P1& start, const P2& end);

// Thick lines
template<point_like P1, point_like P2>
expected<void, std::string> draw_line_thick(const P1& start, const P2& end, float width);
```

#### Implementation Strategy:
- Adapt euler's iterators to use function pointer dispatch
- Implement efficient alpha blending for antialiased pixels
- Use batch processing where possible

### Phase 3: Circle and Ellipse Drawing
**Priority**: High  
**Timeline**: Week 2

#### Methods to Implement:
```cpp
// Circles
template<point_like P>
expected<void, std::string> draw_circle(const P& center, int radius);

template<point_like P>
expected<void, std::string> fill_circle(const P& center, int radius);

// Ellipses
template<point_like P>
expected<void, std::string> draw_ellipse(const P& center, int rx, int ry);

template<point_like P>
expected<void, std::string> fill_ellipse(const P& center, int rx, int ry);

template<point_like P>
expected<void, std::string> draw_ellipse_arc(const P& center, int rx, int ry, float start_angle, float end_angle);
```

#### Optimization Strategies:
- Horizontal span filling for filled shapes
- Batch pixel writes for outlines
- Clipping optimization for partially visible shapes

### Phase 4: Bezier and Spline Curves
**Priority**: Medium  
**Timeline**: Week 2-3

#### Methods to Implement:
```cpp
// Bezier curves
template<point_like P1, point_like P2, point_like P3>
expected<void, std::string> draw_bezier_quad(const P1& p0, const P2& p1, const P3& p2);

template<point_like P1, point_like P2, point_like P3, point_like P4>
expected<void, std::string> draw_bezier_cubic(const P1& p0, const P2& p1, const P3& p2, const P4& p3);

// Splines
template<typename Container>
expected<void, std::string> draw_bspline(const Container& control_points, int degree = 3);

template<typename Container>
expected<void, std::string> draw_catmull_rom(const Container& points, float tension = 0.5f);
```

### Phase 5: Polygon and Advanced Features
**Priority**: Medium  
**Timeline**: Week 3

#### Methods to Implement:
```cpp
// Polygons
template<typename Container>
expected<void, std::string> draw_polygon(const Container& vertices, bool close = true);

template<typename Container>
expected<void, std::string> fill_polygon(const Container& vertices);

template<typename Container>
expected<void, std::string> draw_polygon_aa(const Container& vertices, bool close = true);

// Parametric curves
template<typename CurveFunc>
expected<void, std::string> draw_curve(CurveFunc&& curve, float t_start = 0.0f, float t_end = 1.0f, int steps = 100);
```

#### Polygon Filling Strategy:
- Implement scanline algorithm with edge tables
- Use DDA for edge walking
- Optimize for convex polygons

### Phase 6: Blending and Clipping
**Priority**: High  
**Timeline**: Week 3-4

#### Features:
1. **Blend Mode Support**
   ```cpp
   expected<void, std::string> set_draw_blend_mode(blend_mode mode);
   expected<blend_mode, std::string> get_draw_blend_mode() const;
   ```

2. **Clipping Rectangle**
   ```cpp
   template<rect_like R>
   expected<void, std::string> set_clip_rect(const std::optional<R>& clip);
   
   template<rect_like R>
   expected<std::optional<R>, std::string> get_clip_rect() const;
   
   bool is_clip_enabled() const;
   ```

3. **Optimized Blend Functions**
   ```cpp
   namespace detail {
       // Alpha blending: out = src * alpha + dst * (1 - alpha)
       void blend_pixel_alpha(void* pixels, int pitch, int x, int y, uint32_t color, uint8_t alpha);
       
       // Additive blending: out = src + dst (clamped)
       void blend_pixel_add(void* pixels, int pitch, int x, int y, uint32_t color, uint8_t alpha);
       
       // Modulate blending: out = src * dst
       void blend_pixel_mod(void* pixels, int pitch, int x, int y, uint32_t color, uint8_t alpha);
       
       // Multiply blending: out = (src * dst) / 255
       void blend_pixel_mul(void* pixels, int pitch, int x, int y, uint32_t color, uint8_t alpha);
   }
   ```

#### Clipping Implementation Strategy:

1. **Point Clipping**
   ```cpp
   inline bool clip_point(int& x, int& y, const rect<int>& clip) {
       return x >= clip.x && x < clip.x + clip.w && 
              y >= clip.y && y < clip.y + clip.h;
   }
   ```

2. **Line Clipping (Cohen-Sutherland Algorithm)**
   ```cpp
   enum OutCode {
       INSIDE = 0,  // 0000
       LEFT   = 1,  // 0001
       RIGHT  = 2,  // 0010
       BOTTOM = 4,  // 0100
       TOP    = 8   // 1000
   };
   
   inline OutCode compute_outcode(float x, float y, const rect<int>& clip) {
       OutCode code = INSIDE;
       if (x < clip.x) code |= LEFT;
       else if (x >= clip.x + clip.w) code |= RIGHT;
       if (y < clip.y) code |= TOP;
       else if (y >= clip.y + clip.h) code |= BOTTOM;
       return code;
   }
   
   bool clip_line(float& x0, float& y0, float& x1, float& y1, const rect<int>& clip);
   ```

3. **Rectangle Clipping**
   ```cpp
   inline bool clip_rect(rect<int>& r, const rect<int>& clip) {
       int x0 = std::max(r.x, clip.x);
       int y0 = std::max(r.y, clip.y);
       int x1 = std::min(r.x + r.w, clip.x + clip.w);
       int y1 = std::min(r.y + r.h, clip.y + clip.h);
       
       if (x1 <= x0 || y1 <= y0) return false;
       
       r.x = x0;
       r.y = y0;
       r.w = x1 - x0;
       r.h = y1 - y0;
       return true;
   }
   ```

4. **Polygon Clipping (Sutherland-Hodgman Algorithm)**
   ```cpp
   template<typename Container>
   std::vector<point<float>> clip_polygon(const Container& vertices, const rect<int>& clip);
   ```

#### Pixel Operation Pipeline:

```cpp
class surface_renderer {
private:
    // Pixel operation that respects all render state
    inline void put_pixel_safe(int x, int y, uint32_t color, uint8_t alpha = 255) {
        // 1. Clipping check
        if (clip_rect_.has_value() && !clip_point(x, y, *clip_rect_)) {
            return;
        }
        
        // 2. Surface bounds check
        if (x < 0 || x >= surface_->w || y < 0 || y >= surface_->h) {
            return;
        }
        
        // 3. Apply blend mode
        void* pixels = static_cast<uint8_t*>(surface_->pixels) + y * surface_->pitch;
        
        switch (blend_mode_) {
            case blend_mode::none:
                put_pixel_(pixels, surface_->pitch, x, y, color);
                break;
                
            case blend_mode::blend:
                blend_pixel_alpha_(pixels, surface_->pitch, x, y, color, alpha);
                break;
                
            case blend_mode::add:
                blend_pixel_add_(pixels, surface_->pitch, x, y, color, alpha);
                break;
                
            case blend_mode::mod:
                blend_pixel_mod_(pixels, surface_->pitch, x, y, color, alpha);
                break;
                
            case blend_mode::mul:
                blend_pixel_mul_(pixels, surface_->pitch, x, y, color, alpha);
                break;
        }
    }
    
    // Fast path for no clipping/blending
    inline void put_pixel_fast(int x, int y, uint32_t color) {
        void* pixels = static_cast<uint8_t*>(surface_->pixels) + y * surface_->pitch;
        put_pixel_(pixels, surface_->pitch, x, y, color);
    }
    
    // Horizontal span operations for efficiency
    void fill_span(int x0, int x1, int y, uint32_t color, uint8_t alpha = 255);
    void fill_span_clipped(int x0, int x1, int y, uint32_t color, uint8_t alpha = 255);
};
```

#### DDA Iterator Integration:

```cpp
// Modified pixel batch processor for surface renderer
template<typename PixelType>
void process_pixel_batch(const euler::dda::pixel_batch<PixelType>& batch) {
    // Check if we need the slow path
    bool needs_clipping = clip_rect_.has_value();
    bool needs_blending = blend_mode_ != blend_mode::none;
    
    if (!needs_clipping && !needs_blending) {
        // Fast path - direct memory writes
        for (size_t i = 0; i < batch.count; ++i) {
            const auto& pixel = batch.pixels[i];
            if (pixel.pos.x >= 0 && pixel.pos.x < surface_->w &&
                pixel.pos.y >= 0 && pixel.pos.y < surface_->h) {
                put_pixel_fast(pixel.pos.x, pixel.pos.y, mapped_color_);
            }
        }
    } else {
        // Slow path - full pipeline
        for (size_t i = 0; i < batch.count; ++i) {
            const auto& pixel = batch.pixels[i];
            put_pixel_safe(pixel.pos.x, pixel.pos.y, mapped_color_);
        }
    }
}

// Specialized for antialiased pixels
template<>
void process_pixel_batch<euler::dda::aa_pixel<float>>(
    const euler::dda::pixel_batch<euler::dda::aa_pixel<float>>& batch) {
    
    for (size_t i = 0; i < batch.count; ++i) {
        const auto& pixel = batch.pixels[i];
        uint8_t alpha = static_cast<uint8_t>(pixel.coverage * 255.0f);
        put_pixel_safe(
            static_cast<int>(pixel.pos.x), 
            static_cast<int>(pixel.pos.y), 
            mapped_color_, 
            alpha
        );
    }
}
```

#### Optimized Shape Drawing with Clipping:

1. **Filled Rectangle with Clipping**
   ```cpp
   template<rect_like R>
   expected<void, std::string> fill_rect(const R& rect) {
       rect<int> r{
           static_cast<int>(get_x(rect)),
           static_cast<int>(get_y(rect)),
           static_cast<int>(get_width(rect)),
           static_cast<int>(get_height(rect))
       };
       
       // Apply clipping
       if (clip_rect_.has_value() && !clip_rect(r, *clip_rect_)) {
           return {}; // Completely clipped
       }
       
       // Fill using horizontal spans
       for (int y = r.y; y < r.y + r.h; ++y) {
           fill_span(r.x, r.x + r.w - 1, y, mapped_color_);
       }
       
       return {};
   }
   ```

2. **Circle Drawing with Clipping**
   ```cpp
   template<point_like P>
   expected<void, std::string> draw_circle(const P& center, int radius) {
       int cx = static_cast<int>(get_x(center));
       int cy = static_cast<int>(get_y(center));
       
       // Trivial rejection
       if (clip_rect_.has_value()) {
           rect<int> bounds{cx - radius, cy - radius, radius * 2, radius * 2};
           rect<int> clip_test = bounds;
           if (!clip_rect(clip_test, *clip_rect_)) {
               return {}; // Completely outside clip region
           }
       }
       
       // Use euler's circle iterator with clipping-aware pixel setting
       using namespace euler::dda;
       batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
           process_pixel_batch(batch);
       });
       
       for (auto pixel : circle_pixels(euler::point2<int>{cx, cy}, radius)) {
           writer.write(pixel);
       }
       
       return {};
   }
   ```

#### SDL Integration Options:

1. **Use SDL's Clipping Functions (if available)**
   ```cpp
   void surface_renderer::set_clip_rect_sdl(const rect<int>& clip) {
       SDL_Rect sdl_clip{clip.x, clip.y, clip.w, clip.h};
       SDL_SetSurfaceClipRect(surface_, &sdl_clip);
   }
   ```

2. **Use SDL's Blend Mode Functions**
   ```cpp
   void surface_renderer::set_blend_mode_sdl(blend_mode mode) {
       SDL_SetSurfaceBlendMode(surface_, static_cast<SDL_BlendMode>(mode));
   }
   ```

3. **Fallback to Manual Implementation**
   - When SDL functions are not available or for better control
   - Custom implementation can be more efficient for specific use cases

## Render State Management

### State Tracking and Optimization
```cpp
class surface_renderer {
private:
    // Render state flags for optimization
    struct render_state {
        bool has_clipping = false;
        bool has_blending = false;
        bool needs_bounds_check = true;
        bool is_opaque = true;  // Alpha == 255
        
        void update(blend_mode mode, const std::optional<rect<int>>& clip, uint8_t alpha) {
            has_clipping = clip.has_value();
            has_blending = mode != blend_mode::none;
            is_opaque = alpha == 255 && mode == blend_mode::none;
            needs_bounds_check = true; // Always true for safety
        }
    } state_;
    
    // Cached function pointers based on current state
    using pixel_op_func = void(*)(surface_renderer*, int x, int y, uint32_t color, uint8_t alpha);
    pixel_op_func current_pixel_op_;
    
    void update_pixel_op() {
        if (!state_.has_clipping && !state_.has_blending) {
            current_pixel_op_ = &pixel_op_fast;
        } else if (state_.has_clipping && !state_.has_blending) {
            current_pixel_op_ = &pixel_op_clipped;
        } else if (!state_.has_clipping && state_.has_blending) {
            current_pixel_op_ = &pixel_op_blended;
        } else {
            current_pixel_op_ = &pixel_op_full;
        }
    }
};
```

### Optimized Pixel Operations
```cpp
// Fast path - no clipping, no blending
static void pixel_op_fast(surface_renderer* sr, int x, int y, uint32_t color, uint8_t) {
    if (x >= 0 && x < sr->surface_->w && y >= 0 && y < sr->surface_->h) {
        void* pixels = static_cast<uint8_t*>(sr->surface_->pixels) + y * sr->surface_->pitch;
        sr->put_pixel_(pixels, sr->surface_->pitch, x, y, color);
    }
}

// Clipping only
static void pixel_op_clipped(surface_renderer* sr, int x, int y, uint32_t color, uint8_t) {
    if (sr->clip_rect_.has_value()) {
        const auto& clip = *sr->clip_rect_;
        if (x < clip.x || x >= clip.x + clip.w || y < clip.y || y >= clip.y + clip.h) {
            return;
        }
    }
    pixel_op_fast(sr, x, y, color, 255);
}

// Blending only
static void pixel_op_blended(surface_renderer* sr, int x, int y, uint32_t color, uint8_t alpha) {
    if (x >= 0 && x < sr->surface_->w && y >= 0 && y < sr->surface_->h) {
        void* pixels = static_cast<uint8_t*>(sr->surface_->pixels) + y * sr->surface_->pitch;
        sr->blend_funcs_[static_cast<int>(sr->blend_mode_)](pixels, sr->surface_->pitch, x, y, color, alpha);
    }
}

// Full pipeline - clipping and blending
static void pixel_op_full(surface_renderer* sr, int x, int y, uint32_t color, uint8_t alpha) {
    if (sr->clip_rect_.has_value()) {
        const auto& clip = *sr->clip_rect_;
        if (x < clip.x || x >= clip.x + clip.w || y < clip.y || y >= clip.y + clip.h) {
            return;
        }
    }
    pixel_op_blended(sr, x, y, color, alpha);
}
```

## Performance Optimizations

### Pixel Format Specializations
1. **32-bit Formats (RGBA8888, ARGB8888)**
   ```cpp
   inline void put_pixel_rgba8888(void* pixels, int pitch, int x, int y, uint32_t color) {
       uint32_t* p = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pixels) + x * 4);
       *p = color;
   }
   
   inline void blend_pixel_alpha_rgba8888(void* pixels, int pitch, int x, int y, uint32_t color, uint8_t alpha) {
       uint32_t* p = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pixels) + x * 4);
       uint32_t dst = *p;
       
       // Extract components
       uint8_t sr = (color >> 24) & 0xFF;
       uint8_t sg = (color >> 16) & 0xFF;
       uint8_t sb = (color >> 8) & 0xFF;
       uint8_t sa = alpha;
       
       uint8_t dr = (dst >> 24) & 0xFF;
       uint8_t dg = (dst >> 16) & 0xFF;
       uint8_t db = (dst >> 8) & 0xFF;
       uint8_t da = dst & 0xFF;
       
       // Alpha blend
       uint16_t inv_alpha = 255 - sa;
       uint8_t r = (sr * sa + dr * inv_alpha) >> 8;
       uint8_t g = (sg * sa + dg * inv_alpha) >> 8;
       uint8_t b = (sb * sa + db * inv_alpha) >> 8;
       uint8_t a = sa + (da * inv_alpha) >> 8;
       
       *p = (r << 24) | (g << 16) | (b << 8) | a;
   }
   ```

2. **24-bit Format (RGB888)**
   ```cpp
   inline void put_pixel_rgb888(void* pixels, int pitch, int x, int y, uint32_t color) {
       uint8_t* p = static_cast<uint8_t*>(pixels) + x * 3;
       p[0] = (color >> 16) & 0xFF;  // R
       p[1] = (color >> 8) & 0xFF;   // G
       p[2] = color & 0xFF;          // B
   }
   ```

3. **16-bit Format (RGB565)**
   ```cpp
   inline void put_pixel_rgb565(void* pixels, int pitch, int x, int y, uint32_t color) {
       uint16_t* p = reinterpret_cast<uint16_t*>(static_cast<uint8_t*>(pixels) + x * 2);
       uint8_t r = (color >> 19) & 0x1F;  // 5 bits
       uint8_t g = (color >> 10) & 0x3F;  // 6 bits
       uint8_t b = (color >> 3) & 0x1F;   // 5 bits
       *p = (r << 11) | (g << 5) | b;
   }
   ```

### Clipping Optimizations
1. **Trivial Rejection**: Quick bounds checking before drawing
   ```cpp
   inline bool trivially_reject_rect(const rect<int>& r, const rect<int>& clip) {
       return r.x >= clip.x + clip.w || r.x + r.w <= clip.x ||
              r.y >= clip.y + clip.h || r.y + r.h <= clip.y;
   }
   ```

2. **Cohen-Sutherland Line Clipping**: Efficient line clipping
3. **Sutherland-Hodgman Polygon Clipping**: For complex polygons
4. **Scanline Clipping**: Clip only at scanline boundaries for fills

### Memory Access Patterns
1. **Row-Major Processing**: Process pixels left-to-right for cache efficiency
2. **Span-Based Operations**: Group horizontal pixels
3. **Prefetching**: Hint next scanline data
4. **SIMD Optimizations**: Process multiple pixels at once

### Batch Processing Strategies
```cpp
// Process 4 pixels at once for RGBA8888
inline void put_pixels_rgba8888_x4(void* pixels, int pitch, int x, int y, 
                                   const uint32_t colors[4]) {
    uint32_t* p = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(pixels) + x * 4);
    // Use SIMD if available
    #ifdef __SSE2__
        __m128i vec = _mm_loadu_si128(reinterpret_cast<const __m128i*>(colors));
        _mm_storeu_si128(reinterpret_cast<__m128i*>(p), vec);
    #else
        p[0] = colors[0];
        p[1] = colors[1];
        p[2] = colors[2];
        p[3] = colors[3];
    #endif
}
```

## File Structure
```
include/sdlpp/video/
├── surface_renderer.hh      # Main surface renderer class
├── surface_renderer.inl     # Template implementations
└── pixel_ops.hh            # Pixel format operations

src/sdlpp/video/
├── surface_renderer.cc      # Core implementations
├── surface_renderer_dda.cc  # DDA drawing methods
└── pixel_ops.cc            # Format-specific pixel operations
```

## Testing Strategy

### Unit Tests
1. **Pixel Format Tests**: Verify correct pixel operations for each format
2. **Drawing Accuracy**: Compare output with reference images
3. **Performance Benchmarks**: Measure operations per second
4. **Edge Cases**: Zero-size shapes, clipping boundaries, etc.

### Integration Tests
1. **Format Conversion**: Test drawing across different surface formats
2. **Blend Mode Verification**: Ensure correct blending results
3. **Large Surface Handling**: Test with various surface sizes

## Example Usage

```cpp
// Create surface renderer
auto surface = SDL_CreateSurface(800, 600, SDL_PIXELFORMAT_RGBA8888);
surface_renderer sr(surface);

// Set drawing color
sr.set_draw_color({255, 0, 0, 255});

// Draw antialiased line
sr.draw_line_aa({10.5f, 20.5f}, {100.5f, 80.5f});

// Draw filled circle
sr.fill_circle({400, 300}, 50);

// Draw Bezier curve
sr.draw_bezier_cubic({0, 0}, {100, 200}, {200, 200}, {300, 0});

// Draw filled polygon
std::vector<point<float>> vertices = {{100, 100}, {200, 50}, {250, 150}, {150, 200}};
sr.fill_polygon(vertices);
```

## Dependencies
- SDL3 (for surface management)
- euler library (for DDA algorithms)
- No additional dependencies

## Performance Targets
- Point drawing: > 100M pixels/second
- Line drawing: > 10M pixels/second  
- Circle drawing: > 1M circles/second
- Polygon filling: > 100K polygons/second

These targets assume modern CPU, single-threaded operation, and common pixel formats (RGBA8888).