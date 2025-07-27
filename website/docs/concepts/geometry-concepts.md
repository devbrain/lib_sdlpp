---
title: Geometry Concepts
sidebar_label: Geometry Concepts
---

# Geometry Concepts

SDL++ provides C++20 concepts to enable generic programming with any geometry library. These concepts define interfaces for common geometric types like points, rectangles, and sizes.

## Overview

The geometry concepts allow SDL++ to work seamlessly with:
- Built-in SDL++ geometry types
- Euler geometry library
- Your custom geometry types
- Third-party geometry libraries

## Point-like Types

A type satisfies `point_like` if it has:
- A nested `value_type` type alias
- Members `x` and `y` convertible to `value_type`

```cpp
template<sdlpp::point_like P>
void draw_at_point(const P& pos) {
    auto x = sdlpp::get_x(pos);  // Generic accessor
    auto y = sdlpp::get_y(pos);
    renderer.draw_point(x, y);
}

// Works with any of these:
sdlpp::point<int> p1{100, 200};
euler::point2<float> p2{100.5f, 200.5f};
MyPoint p3{100, 200};  // Your custom type
```

## Rectangle-like Types

SDL++ supports two rectangle formats:

### SDL-style rectangles (`rect_like`)
Must have `x`, `y`, `w`, `h` members:

```cpp
template<sdlpp::rect_like R>
void fill_area(renderer& r, const R& rect) {
    r.fill_rect(rect);  // Renderer accepts any rect_like
}
```

### Alternative style (`rect_like_alt`)
Must have `left`, `top`, `width`, `height` members:

```cpp
struct MyRect {
    using value_type = int;
    int left, top, width, height;
};

// Works with either style
template<sdlpp::rectangle_like R>  // Accepts both formats
bool is_empty_rect(const R& rect) {
    return sdlpp::is_empty(rect);
}
```

## Size-like Types

Types with `width` and `height` members:

```cpp
template<sdlpp::size_like S>
auto calculate_area(const S& size) {
    return sdlpp::get_area(size);
}

sdlpp::size<int> s1{800, 600};
MySize s2{1920, 1080};
auto area = calculate_area(s2);  // Works with any size_like
```

## Other Geometric Concepts

### Line-like Types
```cpp
template<sdlpp::line_like L>
void draw_line_segment(const L& line) {
    renderer.draw_line(line.x1, line.y1, line.x2, line.y2);
}
```

### Circle-like Types
```cpp
template<sdlpp::circle_like C>
void draw_circle_shape(const C& circle) {
    renderer.draw_circle(circle.x, circle.y, circle.radius);
}
```

### Triangle-like Types
```cpp
template<sdlpp::triangle_like T>
void draw_triangle(const T& tri) {
    renderer.draw_line(tri.a, tri.b);
    renderer.draw_line(tri.b, tri.c);
    renderer.draw_line(tri.c, tri.a);
}
```

### Polygon-like Types
```cpp
template<sdlpp::polygon_like P>
void draw_polygon(const P& poly) {
    for (size_t i = 0; i < poly.size(); ++i) {
        auto next = (i + 1) % poly.size();
        renderer.draw_line(poly[i], poly[next]);
    }
}
```

## Generic Accessor Functions

SDL++ provides generic functions that work with all geometric types:

### Position and Size Accessors
```cpp
// Get components
auto x = sdlpp::get_x(point);
auto y = sdlpp::get_y(point);
auto w = sdlpp::get_width(rect_or_size);
auto h = sdlpp::get_height(rect_or_size);

// Calculate area
auto area = sdlpp::get_area(rect_or_size);

// Check if empty
bool empty = sdlpp::is_empty(rect_or_size);
```

### Geometric Tests
```cpp
// Point in rectangle
if (sdlpp::contains(rect, point)) {
    // Point is inside rectangle
}

// Rectangle intersection
if (sdlpp::intersects(rect1, rect2)) {
    // Rectangles overlap
}
```

### Type Conversions
```cpp
// Extract position from rectangle
auto pos = sdlpp::get_position<sdlpp::point<int>>(rect);

// Extract size from rectangle
auto size = sdlpp::get_size<sdlpp::size<int>>(rect);

// Create rectangle from point and size
auto rect = sdlpp::make_rect<sdlpp::rect<int>>(point, size);
```

## Arithmetic Concepts

For types with arithmetic `value_type`:

```cpp
template<sdlpp::arithmetic_point_like P>
P interpolate(const P& a, const P& b, float t) {
    using T = typename P::value_type;
    return P{
        static_cast<T>(a.x + (b.x - a.x) * t),
        static_cast<T>(a.y + (b.y - a.y) * t)
    };
}
```

## Creating Compatible Types

To make your types work with SDL++ concepts:

```cpp
struct MyPoint {
    using value_type = float;  // Required type alias
    float x, y;                // Required members
};

struct MyRect {
    using value_type = int;
    int x, y, w, h;            // SDL-style
    // OR
    int left, top, width, height;  // Alternative style
};

struct MyPolygon {
    using value_type = MyPoint;
    
    size_t size() const { return points.size(); }
    const MyPoint& operator[](size_t i) const { return points[i]; }
    
private:
    std::vector<MyPoint> points;
};
```

## Usage with Renderer

All SDL++ rendering functions accept geometry concepts:

```cpp
// Any point_like type
renderer.draw_point(my_point);
renderer.draw_line(point1, point2);

// Any rect_like type
renderer.draw_rect(my_rect);
renderer.fill_rect(my_rect);

// Mixed types work too
euler::point2<float> p1{0, 0};
MyPoint p2{100, 100};
renderer.draw_line(p1, p2);  // Different point types!
```

## Best Practices

1. **Use concepts in templates** - Make your functions generic:
   ```cpp
   template<sdlpp::rect_like R>
   void process_rectangle(const R& rect) { /* ... */ }
   ```

2. **Leverage type deduction** - Let the compiler figure out types:
   ```cpp
   auto area = sdlpp::get_area(some_rect);
   ```

3. **Mix and match** - SDL++ handles conversions between compatible types:
   ```cpp
   sdlpp::rect<int> sdl_rect{0, 0, 100, 100};
   euler::rectangle<float> euler_rect{0, 0, 100, 100};
   if (sdlpp::intersects(sdl_rect, euler_rect)) { /* ... */ }
   ```

4. **Prefer generic accessors** - Use `get_x()`, `get_width()` etc. for maximum compatibility

## See Also

- [Renderer Concepts](renderer-concepts.md) - Concepts for renderer interfaces
- [Geometry Types](../api/utility/geometry.md) - Built-in geometry types
- [Renderer API](../api/video/renderer.md) - Using geometry with rendering