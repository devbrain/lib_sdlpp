---
title: Renderer Concepts
sidebar_label: Renderer Concepts
---

# Renderer Concepts

SDL++ provides C++20 concepts to create generic code that works with both hardware-accelerated renderer and software surface_renderer.

## Overview

The renderer concepts enable:
- Writing rendering code once that works with both backends
- Creating reusable drawing libraries
- Testing with software renderer, deploying with hardware
- Seamless renderer switching at runtime

## Concept Hierarchy

```cpp
basic_renderer
    ↓
primitive_renderer
    ↓
dda_renderer
    ↓
bezier_renderer
    ↓
renderer_like (full featured)
```

## Basic Renderer

The `basic_renderer` concept covers fundamental operations:

```cpp
template<sdlpp::basic_renderer R>
void clear_screen(R& renderer, const sdlpp::color& bg) {
    renderer.set_draw_color(bg);
    renderer.clear();
}

// Works with both:
sdlpp::renderer hw_renderer = /* ... */;
sdlpp::surface_renderer sw_renderer = /* ... */;

clear_screen(hw_renderer, {0, 0, 0, 255});
clear_screen(sw_renderer, {0, 0, 0, 255});
```

Required operations:
- `is_valid()` - Check renderer validity
- `clear()` - Clear the target
- `set_draw_color()` / `get_draw_color()` - Color management
- `set_draw_blend_mode()` / `get_draw_blend_mode()` - Blend mode

## Primitive Renderer

The `primitive_renderer` concept adds basic shape drawing:

```cpp
template<sdlpp::primitive_renderer R>
void draw_crosshair(R& renderer, int x, int y, int size) {
    renderer.draw_line(x - size, y, x + size, y);
    renderer.draw_line(x, y - size, x, y + size);
    renderer.draw_rect(x - 2, y - 2, 4, 4);
}
```

Required operations:
- `draw_point(x, y)` - Draw single pixel
- `draw_line(x1, y1, x2, y2)` - Draw line
- `draw_rect(x, y, w, h)` - Draw rectangle outline
- `fill_rect(x, y, w, h)` - Fill rectangle

## DDA Renderer

The `dda_renderer` concept adds smooth drawing algorithms:

```cpp
template<sdlpp::dda_renderer R>
void draw_smooth_ui(R& renderer) {
    // Antialiased lines
    renderer.draw_line_aa(0.5f, 0.5f, 99.5f, 99.5f);
    
    // Thick lines
    renderer.draw_line_thick(10, 10, 90, 90, 3.0f);
    
    // Circles and ellipses
    renderer.draw_circle(50, 50, 30);
    renderer.fill_ellipse(100, 100, 40, 20);
    
    // Arcs
    renderer.draw_ellipse_arc(200, 200, 50, 30, 0.0f, M_PI);
}
```

Required operations:
- `draw_line_aa()` - Antialiased lines
- `draw_line_thick()` - Variable width lines
- `draw_circle()` / `fill_circle()` - Circles
- `draw_ellipse()` / `fill_ellipse()` - Ellipses
- `draw_ellipse_arc()` - Elliptical arcs

## Bezier Renderer

The `bezier_renderer` concept adds curve support:

```cpp
template<sdlpp::bezier_renderer R>
void draw_curves(R& renderer) {
    // Quadratic Bezier
    renderer.draw_bezier_quad(0, 100, 50, 0, 100, 100);
    
    // Cubic Bezier
    renderer.draw_bezier_cubic(0, 0, 30, 100, 70, 100, 100, 0);
}
```

## Clipping Renderer

The `clipping_renderer` concept adds clipping support:

```cpp
template<sdlpp::clipping_renderer R>
void draw_clipped(R& renderer) {
    if (renderer.is_clip_enabled()) {
        auto clip = renderer.get_clip_rect();
        // Draw within clip bounds
    }
}
```

## Full Renderer

The `renderer_like` concept combines all capabilities:

```cpp
template<sdlpp::renderer_like R>
class DrawingLibrary {
    R& renderer;
    
public:
    explicit DrawingLibrary(R& r) : renderer(r) {}
    
    void draw_button(int x, int y, int w, int h) {
        // Rounded rectangle using all features
        renderer.fill_rect(x + 5, y, w - 10, h);
        renderer.fill_rect(x, y + 5, w, h - 10);
        renderer.fill_circle(x + 5, y + 5, 5);
        renderer.fill_circle(x + w - 5, y + 5, 5);
        renderer.fill_circle(x + 5, y + h - 5, 5);
        renderer.fill_circle(x + w - 5, y + h - 5, 5);
    }
};
```

## Using Renderer Concepts

### Generic Drawing Functions

```cpp
// Works with any renderer that supports primitives
template<sdlpp::primitive_renderer R>
void draw_grid(R& renderer, int cols, int rows, int cell_size) {
    for (int i = 0; i <= cols; ++i) {
        renderer.draw_line(i * cell_size, 0, 
                          i * cell_size, rows * cell_size);
    }
    for (int j = 0; j <= rows; ++j) {
        renderer.draw_line(0, j * cell_size, 
                          cols * cell_size, j * cell_size);
    }
}
```

### Renderer-Agnostic Libraries

```cpp
template<sdlpp::dda_renderer R>
class Chart {
    R& renderer;
    
public:
    explicit Chart(R& r) : renderer(r) {}
    
    void draw_pie_chart(int cx, int cy, int radius, 
                       const std::vector<float>& values) {
        float total = std::accumulate(values.begin(), values.end(), 0.0f);
        float start_angle = 0.0f;
        
        for (float value : values) {
            float end_angle = start_angle + (value / total) * 2 * M_PI;
            
            // Draw slice
            renderer.draw_ellipse_arc(cx, cy, radius, radius, 
                                     start_angle, end_angle);
            
            start_angle = end_angle;
        }
    }
};
```

### Runtime Renderer Selection

```cpp
void render_scene(bool use_hardware) {
    if (use_hardware) {
        sdlpp::renderer hw_renderer = /* ... */;
        draw_everything(hw_renderer);
    } else {
        sdlpp::surface surf(800, 600, SDL_PIXELFORMAT_ARGB8888);
        sdlpp::surface_renderer sw_renderer(surf);
        draw_everything(sw_renderer);
    }
}

template<sdlpp::renderer_like R>
void draw_everything(R& renderer) {
    // Same code works for both renderers
    renderer.clear();
    renderer.draw_circle({400, 300}, 100);
    // ...
}
```

## Euler Angle Support

The `euler_angle_renderer` concept adds type-safe angle support:

```cpp
template<sdlpp::euler_angle_renderer R>
void draw_with_angles(R& renderer) {
    // Type-safe angles
    renderer.draw_ellipse_arc(100, 100, 50, 30,
                             euler::radian<float>(0.0f),
                             euler::radian<float>(M_PI_2));
}
```

## Creating Renderer-Compatible Types

To make your renderer work with these concepts:

```cpp
class MyRenderer {
public:
    // Basic operations
    bool is_valid() const { return true; }
    expected<void, std::string> clear() { /* ... */ }
    expected<void, std::string> set_draw_color(const color& c) { /* ... */ }
    expected<color, std::string> get_draw_color() const { /* ... */ }
    
    // Primitive operations
    expected<void, std::string> draw_point(int x, int y) { /* ... */ }
    expected<void, std::string> draw_line(int x1, int y1, int x2, int y2) { /* ... */ }
    
    // DDA operations (if supported)
    expected<void, std::string> draw_circle(int x, int y, int r) { /* ... */ }
    // ...
};

static_assert(sdlpp::primitive_renderer<MyRenderer>);
```

## Performance Considerations

1. **Template instantiation** - Concepts generate code for each renderer type
2. **Virtual dispatch alternative** - Concepts avoid virtual function overhead
3. **Compile-time polymorphism** - All dispatch happens at compile time
4. **Inlining opportunities** - Compilers can inline across concept boundaries

## Best Practices

1. **Use the most general concept** - Don't require `renderer_like` if `primitive_renderer` suffices
2. **Combine with geometry concepts** - For maximum flexibility:
   ```cpp
   template<sdlpp::primitive_renderer R, sdlpp::rect_like Rect>
   void fill_area(R& renderer, const Rect& rect) {
       renderer.fill_rect(rect);
   }
   ```

3. **Provide overloads** - For common concrete types:
   ```cpp
   // Generic version
   template<sdlpp::renderer_like R>
   void draw(R& renderer);
   
   // Optimized for hardware
   void draw(sdlpp::renderer& renderer) {
       // Hardware-specific optimizations
   }
   ```

4. **Document requirements** - Be clear about which concept you need:
   ```cpp
   // Requires: R models sdlpp::dda_renderer
   template<typename R>
   void smooth_draw(R& renderer);
   ```

## See Also

- [Geometry Concepts](geometry-concepts.md) - Concepts for geometric types
- [Hardware Renderer](../api/video/renderer.md) - Hardware-accelerated rendering
- [Surface Renderer](../api/video/surface-renderer.md) - Software rendering
- [DDA Algorithms](../guides/dda-rendering.md) - Digital Differential Analyzer guide