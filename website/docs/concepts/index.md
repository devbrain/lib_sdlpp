---
title: Concepts Overview
sidebar_label: Overview
---

# SDL++ Concepts

SDL++ leverages C++20 concepts to provide flexible, type-safe interfaces that work with various implementations and libraries.

## What are Concepts?

Concepts are a C++20 feature that define requirements for template arguments. They enable:
- **Compile-time polymorphism** - No runtime overhead
- **Better error messages** - Clear requirements for template parameters
- **Generic programming** - Write code once, use with multiple types
- **Library interoperability** - Work with different geometry/math libraries

## Available Concepts

### [Geometry Concepts](geometry-concepts.md)

Define interfaces for geometric types:
- `point_like` - Types with x, y coordinates
- `rect_like` - Rectangle types (SDL or alternative style)
- `size_like` - Types with width and height
- `circle_like`, `line_like`, `triangle_like`, `polygon_like`

```cpp
template<sdlpp::point_like P>
void draw_at(const P& pos) {
    renderer.draw_point(sdlpp::get_x(pos), sdlpp::get_y(pos));
}
```

### [Renderer Concepts](renderer-concepts.md)

Define interfaces for rendering backends:
- `basic_renderer` - Core operations (clear, color)
- `primitive_renderer` - Basic shapes (points, lines, rectangles)
- `dda_renderer` - Smooth algorithms (antialiased lines, circles)
- `renderer_like` - Full-featured renderer

```cpp
template<sdlpp::renderer_like R>
void render_scene(R& renderer) {
    // Works with both hardware and software renderers
    renderer.clear();
    renderer.draw_circle({100, 100}, 50);
}
```

## Benefits

### 1. Type Safety

```cpp
// Compile-time error if T doesn't have x, y members
template<sdlpp::point_like T>
void process_point(const T& p);

// Clear requirements in function signatures
void draw_shape(sdlpp::renderer_like auto& r, 
                sdlpp::rect_like auto const& bounds);
```

### 2. Library Agnostic

```cpp
// Works with SDL++ types
sdlpp::point<int> p1{100, 200};
renderer.draw_point(p1);

// Works with Euler library
euler::point2<float> p2{100.5f, 200.5f};
renderer.draw_point(p2);

// Works with your types
MyPoint p3{100, 200};
renderer.draw_point(p3);
```

### 3. Zero Overhead

Concepts use compile-time polymorphism:
- No virtual function calls
- No runtime type checks
- Full inlining opportunities
- Same performance as hand-written code

### 4. Better Error Messages

Without concepts:
```
error: no matching function for call to 'draw_point'
note: candidate template ignored: substitution failure [with T = MyBadType]:
      no member named 'x' in 'MyBadType'
```

With concepts:
```
error: no matching function for call to 'draw_point'
note: candidate template ignored: constraints not satisfied [with T = MyBadType]
note: because 'MyBadType' does not satisfy 'point_like'
note: because 't.x' would be invalid: no member named 'x' in 'MyBadType'
```

## Usage Patterns

### Generic Functions

```cpp
// Accept any point-like type
template<sdlpp::point_like P>
float distance(const P& a, const P& b) {
    auto dx = sdlpp::get_x(b) - sdlpp::get_x(a);
    auto dy = sdlpp::get_y(b) - sdlpp::get_y(a);
    return std::sqrt(dx * dx + dy * dy);
}
```

### Constrained Auto

```cpp
// C++20 abbreviated function template
void draw(sdlpp::renderer_like auto& r, 
          sdlpp::rect_like auto const& rect) {
    r.fill_rect(rect);
}
```

### Concept Combinations

```cpp
template<typename T>
concept drawable_shape = sdlpp::rect_like<T> || 
                        sdlpp::circle_like<T> ||
                        sdlpp::polygon_like<T>;

void draw_any_shape(sdlpp::renderer_like auto& r,
                    drawable_shape auto const& shape) {
    // Implementation...
}
```

### SFINAE Replacement

```cpp
// Old way (SFINAE)
template<typename T>
std::enable_if_t<has_x_and_y<T>::value, void>
process(const T& point);

// New way (Concepts)
template<sdlpp::point_like T>
void process(const T& point);
```

## Creating Concept-Compatible Types

Make your types work with SDL++ concepts:

```cpp
// Point-like type
struct MyPoint {
    using value_type = float;  // Required
    float x, y;                // Required members
};

// Rectangle-like type
struct MyRect {
    using value_type = int;
    int x, y, w, h;            // SDL style
};

// Now they work with SDL++
MyPoint p{10.5f, 20.5f};
MyRect r{0, 0, 100, 100};

renderer.draw_point(p);
renderer.fill_rect(r);
```

## Best Practices

1. **Use appropriate concepts** - Don't over-constrain templates
2. **Provide generic accessors** - Use `get_x()`, `get_width()` for compatibility
3. **Document requirements** - Make concept requirements clear
4. **Test with multiple types** - Ensure your templates work with various implementations
5. **Combine concepts** - Create domain-specific concept combinations

## See Also

- [C++20 Concepts](https://en.cppreference.com/w/cpp/language/constraints) - Language reference
- [Generic Programming Guide](../guides/generic-programming.md) - Using concepts effectively
- [API Reference](../api/overview.md) - Full API documentation