# Triangle and Geometry Rendering Usage Guide

## Overview

This guide covers the triangle support in `geometry.hh` and the render geometry API wrapper for SDL3's `SDL_RenderGeometry` functionality. These features enable efficient rendering of textured and colored triangles for advanced 2D graphics.

## Triangle Support in Geometry

### Basic Triangle Types

```cpp
#include "sdlpp/geometry.hh"

// Integer triangle
sdlpp::triangle tri(0, 0, 100, 0, 50, 100);

// Floating-point triangle  
sdlpp::ftriangle ftri(0.0f, 0.0f, 100.0f, 0.0f, 50.0f, 100.0f);

// From points
sdlpp::point a(0, 0), b(100, 0), c(50, 100);
sdlpp::triangle tri2(a, b, c);
```

### Triangle Properties

```cpp
// Centroid (center of mass)
auto center = tri.centroid();

// Area calculation
auto area = tri.area();          // Always positive
auto signed = tri.signed_area(); // Positive if CCW, negative if CW

// Winding order
bool ccw = tri.is_ccw();

// Perimeter
auto perimeter = tri.perimeter();

// Bounding box
auto bounds = tri.bounds();
```

### Point Containment

```cpp
// Check if point is inside triangle
sdlpp::point p(50, 50);
bool inside = tri.contains(p);

// Works with vertices too
bool has_vertex = tri.contains(tri.a); // true
```

### Triangle Transformations

```cpp
// Translation
auto moved = tri.translated(sdlpp::point(10, 20));

// Scaling from origin
auto bigger = tri.scaled(2);

// Scaling from a point
auto scaled = tri.scaled_from(tri.centroid(), 1.5);

// Rotation (angles in radians)
sdlpp::ftriangle ftri(0, 0, 10, 0, 5, 10);
auto rotated = ftri.rotated(M_PI / 4);  // 45 degrees

// Rotation around a point
auto rotated2 = ftri.rotated_around(ftri.centroid(), M_PI / 2);
```

### Utility Functions

```cpp
// Create special triangles
auto equilateral = sdlpp::make_equilateral_triangle(100.0f);
auto right = sdlpp::make_right_triangle(3, 4); // 3-4-5 triangle

// Check collinearity
bool collinear = sdlpp::are_collinear(p1, p2, p3);
```

## Geometry Rendering API

### Basic Triangle Rendering

```cpp
// Create vertices for a colored triangle
SDL_Vertex v0 = sdlpp::renderer::make_vertex(
    sdlpp::fpoint{10.0f, 10.0f}, 
    sdlpp::colors::red
);
SDL_Vertex v1 = sdlpp::renderer::make_vertex(
    sdlpp::fpoint{50.0f, 10.0f}, 
    sdlpp::colors::green
);
SDL_Vertex v2 = sdlpp::renderer::make_vertex(
    sdlpp::fpoint{30.0f, 50.0f}, 
    sdlpp::colors::blue
);

// Render the triangle
renderer.render_triangle(v0, v1, v2);
```

### Using Triangle Types

```cpp
// Render triangle from geometry types
sdlpp::ftriangle tri(10.0f, 10.0f, 50.0f, 10.0f, 30.0f, 50.0f);
renderer.render_triangle(tri, sdlpp::colors::white);
```

### Textured Triangles

```cpp
// Create or load a texture
auto texture = sdlpp::texture::create(renderer, surface);

// Define triangle positions
sdlpp::ftriangle tri(10.0f, 10.0f, 50.0f, 10.0f, 30.0f, 50.0f);

// Define texture coordinates (0-1 range)
sdlpp::ftriangle tex_coords(0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 1.0f);

// Render textured triangle
renderer.render_textured_triangle(
    texture->get(), 
    tri, 
    sdlpp::colors::white,  // Color modulation
    tex_coords
);
```

### Batch Rendering

```cpp
// Create vertices for multiple triangles
std::vector<SDL_Vertex> vertices = {
    // First quad (two triangles)
    renderer::make_vertex({0, 0}, colors::red, {0, 0}),
    renderer::make_vertex({100, 0}, colors::green, {1, 0}),
    renderer::make_vertex({100, 100}, colors::blue, {1, 1}),
    renderer::make_vertex({0, 100}, colors::white, {0, 1})
};

// Define triangle indices
std::vector<int> indices = {
    0, 1, 2,  // First triangle
    0, 2, 3   // Second triangle
};

// Render all triangles at once
renderer.render_geometry(texture->get(), vertices, indices);
```

### Vertex Creation Helper

```cpp
// Basic vertex (position and color)
auto v = renderer::make_vertex(
    sdlpp::fpoint{100.0f, 200.0f},
    sdlpp::colors::red
);

// With texture coordinates
auto v_tex = renderer::make_vertex(
    sdlpp::fpoint{100.0f, 200.0f},
    sdlpp::colors::white,
    sdlpp::fpoint{0.5f, 0.5f}  // UV coordinates
);

// Color is automatically converted from 0-255 to 0.0-1.0 range
sdlpp::color semi_transparent{255, 128, 64, 128};
auto v_alpha = renderer::make_vertex({0, 0}, semi_transparent);
```

## Advanced Examples

### Render a Textured Mesh

```cpp
// Create a simple mesh (grid of triangles)
std::vector<SDL_Vertex> vertices;
std::vector<int> indices;

const int grid_size = 10;
const float cell_size = 50.0f;

// Generate vertices
for (int y = 0; y <= grid_size; ++y) {
    for (int x = 0; x <= grid_size; ++x) {
        vertices.push_back(renderer::make_vertex(
            {x * cell_size, y * cell_size},
            sdlpp::colors::white,
            {float(x) / grid_size, float(y) / grid_size}
        ));
    }
}

// Generate triangle indices
for (int y = 0; y < grid_size; ++y) {
    for (int x = 0; x < grid_size; ++x) {
        int top_left = y * (grid_size + 1) + x;
        int top_right = top_left + 1;
        int bottom_left = top_left + (grid_size + 1);
        int bottom_right = bottom_left + 1;
        
        // First triangle
        indices.push_back(top_left);
        indices.push_back(top_right);
        indices.push_back(bottom_left);
        
        // Second triangle
        indices.push_back(top_right);
        indices.push_back(bottom_right);
        indices.push_back(bottom_left);
    }
}

// Render the mesh
renderer.render_geometry(texture->get(), vertices, indices);
```

### Gradient Triangle

```cpp
// Create triangle with different colors at each vertex
auto v0 = renderer::make_vertex({100, 50}, {255, 0, 0, 255});    // Red
auto v1 = renderer::make_vertex({150, 150}, {0, 255, 0, 255});  // Green
auto v2 = renderer::make_vertex({50, 150}, {0, 0, 255, 255});   // Blue

// Render - colors will be interpolated across the triangle
renderer.render_triangle(v0, v1, v2);
```

### Triangle Strip Rendering

```cpp
// Create a triangle strip (efficient for connected triangles)
std::vector<SDL_Vertex> strip_verts;
std::vector<int> strip_indices;

// Create vertices for a zigzag pattern
for (int i = 0; i < 10; ++i) {
    float x = i * 30.0f;
    float y1 = (i % 2 == 0) ? 50.0f : 100.0f;
    float y2 = (i % 2 == 0) ? 100.0f : 50.0f;
    
    strip_verts.push_back(renderer::make_vertex({x, y1}, colors::red));
    strip_verts.push_back(renderer::make_vertex({x, y2}, colors::blue));
}

// Generate indices for triangle strip
for (int i = 0; i < strip_verts.size() - 2; ++i) {
    if (i % 2 == 0) {
        strip_indices.push_back(i);
        strip_indices.push_back(i + 1);
        strip_indices.push_back(i + 2);
    } else {
        strip_indices.push_back(i + 1);
        strip_indices.push_back(i);
        strip_indices.push_back(i + 2);
    }
}

renderer.render_geometry(strip_verts, strip_indices);
```

## Performance Tips

1. **Batch Rendering**: Use `render_geometry` with multiple triangles instead of individual `render_triangle` calls
2. **Index Buffers**: Reuse vertices with index buffers to reduce memory usage
3. **Texture Atlases**: Combine multiple textures into one to reduce texture switches
4. **Pre-transform**: Transform vertices on CPU if the transformation is static
5. **Color Modulation**: Use white color when not needed - the GPU can skip the multiplication

## Error Handling

```cpp
// All rendering functions return sdlpp::expected
auto result = renderer.render_triangle(tri, colors::white);
if (!result) {
    std::cerr << "Render error: " << result.error() << std::endl;
}

// Index validation
std::vector<int> indices = {0, 1, 2, 3}; // Not multiple of 3!
auto result2 = renderer.render_geometry(vertices, indices);
if (!result2) {
    // Error: "Index count must be multiple of 3 for triangles"
}
```

## Limitations

- SDL_RenderGeometry only supports triangles (no quads, lines, or points)
- Texture coordinates are in 0.0-1.0 range
- Colors are specified per-vertex (no per-pixel shading)
- No built-in support for triangle fans or other primitive types
- Performance varies by backend (OpenGL, Direct3D, Metal, etc.)