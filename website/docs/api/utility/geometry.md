---
title: Geometry System
sidebar_label: Geometry
---

# Geometry System

SDL++ provides a flexible geometry system with C++20 concepts that allows integration with external math libraries while providing built-in types for common use cases.

## Geometry Concepts

SDL++ uses concepts to define what makes a valid geometric type:

```cpp
#include <sdlpp/utility/geometry.hh>

// Point concept - requires x and y members
template<typename T>
concept point_like = requires(T t) {
    { t.x } -> std::convertible_to<int>;
    { t.y } -> std::convertible_to<int>;
};

// Size concept - requires width and height members  
template<typename T>
concept size_like = requires(T t) {
    { t.width } -> std::convertible_to<int>;
    { t.height } -> std::convertible_to<int>;
};

// Rectangle concept - requires x, y, width, height
template<typename T>
concept rect_like = requires(T t) {
    { t.x } -> std::convertible_to<int>;
    { t.y } -> std::convertible_to<int>;
    { t.width } -> std::convertible_to<int>;
    { t.height } -> std::convertible_to<int>;
};
```

## Built-in Types

SDL++ provides standard geometry types:

```cpp
// Integer types (most common)
sdlpp::point_i p{100, 200};         // Integer point
sdlpp::size_i s{800, 600};          // Integer size  
sdlpp::rect_i r{10, 20, 100, 50};   // Integer rectangle

// Float types (for smooth animation)
sdlpp::point_f pf{100.5f, 200.7f};  // Float point
sdlpp::size_f sf{800.0f, 600.0f};   // Float size
sdlpp::rect_f rf{10.5f, 20.5f, 100.0f, 50.0f}; // Float rectangle

// Type aliases for convenience
using point = point_i;  // Default to integer
using size = size_i;
using rect = rect_i;
```

## Point Operations

```cpp
// Creation
sdlpp::point_i p1{100, 200};
sdlpp::point_i p2{50, 75};

// Arithmetic
auto p3 = p1 + p2;  // {150, 275}
auto p4 = p1 - p2;  // {50, 125}
auto p5 = p1 * 2;   // {200, 400}
auto p6 = p1 / 2;   // {50, 100}

// Comparison
if (p1 == p2) { /* ... */ }
if (p1 != p2) { /* ... */ }

// Distance
float dist = p1.distance_to(p2);
float dist_squared = p1.distance_squared_to(p2);  // Faster

// Utility
p1.clamp(sdlpp::rect_i{0, 0, 800, 600});  // Clamp to bounds
```

## Size Operations

```cpp
// Creation
sdlpp::size_i size{1920, 1080};

// Properties
int area = size.area();              // 1920 * 1080
float aspect = size.aspect_ratio();  // 1920.0 / 1080.0 ≈ 1.78
bool empty = size.is_empty();        // false (both > 0)

// Scaling
auto scaled = size.scaled(0.5f);     // {960, 540}
auto fitted = size.scaled_to_fit(sdlpp::size_i{800, 600}); // Maintain aspect

// Arithmetic
auto doubled = size * 2;             // {3840, 2160}
auto halved = size / 2;              // {960, 540}
```

## Rectangle Operations

```cpp
// Creation
sdlpp::rect_i rect{100, 100, 200, 150};

// Position and size
auto pos = rect.position();          // point_i{100, 100}
auto size = rect.size();             // size_i{200, 150}

// Corners
auto tl = rect.top_left();          // {100, 100}
auto br = rect.bottom_right();      // {300, 250}
auto center = rect.center();        // {200, 175}

// Edges
int left = rect.left();             // 100
int right = rect.right();           // 300
int top = rect.top();               // 100  
int bottom = rect.bottom();         // 250

// Contains
bool contains_point = rect.contains(sdlpp::point_i{150, 125});
bool contains_rect = rect.contains(sdlpp::rect_i{120, 120, 50, 50});

// Intersection
auto other = sdlpp::rect_i{150, 125, 200, 200};
bool intersects = rect.intersects(other);
auto intersection = rect.intersection(other);  // Optional<rect_i>

// Union
auto united = rect.united(other);   // Bounding rect

// Inflation/Deflation
auto bigger = rect.inflated(10);    // 10 pixels larger on all sides
auto smaller = rect.deflated(5);    // 5 pixels smaller
```

## Using External Geometry Types

SDL++ can work with your existing math library:

```cpp
// Example: Using GLM
#include <glm/glm.hpp>

// GLM types satisfy SDL++ concepts
glm::ivec2 point{100, 200};
glm::vec2 size{800.0f, 600.0f};

// Can be used with SDL++ functions
void draw_at_point(sdlpp::renderer& r, const auto& p) 
    requires sdlpp::point_like<decltype(p)> {
    r.draw_point(static_cast<float>(p.x), static_cast<float>(p.y));
}

// Works with both SDL++ and GLM types
draw_at_point(renderer, sdlpp::point_i{100, 100});
draw_at_point(renderer, glm::ivec2{200, 200});
```

## Geometry Algorithms

```cpp
// Line intersection
auto line1 = sdlpp::line_i{{0, 0}, {100, 100}};
auto line2 = sdlpp::line_i{{0, 100}, {100, 0}};
auto intersection = line1.intersection(line2);  // Optional<point_i>

// Point in polygon
std::vector<sdlpp::point_i> polygon = {
    {0, 0}, {100, 0}, {100, 100}, {0, 100}
};
bool inside = sdlpp::point_in_polygon({50, 50}, polygon);

// Convex hull
auto hull = sdlpp::convex_hull(points);

// Bounding box
auto bounds = sdlpp::bounding_box(points);
```

## Common Patterns

### Camera/Viewport System

```cpp
class camera {
    sdlpp::rect_f view;      // World coordinates
    sdlpp::size_i viewport;  // Screen size
    float zoom = 1.0f;
    
public:
    // World to screen transformation
    sdlpp::point_i world_to_screen(const sdlpp::point_f& world) const {
        auto normalized = (world - view.position()) / view.size();
        return {
            static_cast<int>(normalized.x * viewport.width),
            static_cast<int>(normalized.y * viewport.height)
        };
    }
    
    // Screen to world transformation
    sdlpp::point_f screen_to_world(const sdlpp::point_i& screen) const {
        auto normalized = sdlpp::point_f{
            static_cast<float>(screen.x) / viewport.width,
            static_cast<float>(screen.y) / viewport.height
        };
        return view.position() + normalized * view.size();
    }
    
    void pan(const sdlpp::point_f& delta) {
        view.x += delta.x;
        view.y += delta.y;
    }
    
    void zoom_at(const sdlpp::point_f& center, float factor) {
        auto old_size = view.size();
        auto new_size = old_size / factor;
        auto size_delta = old_size - new_size;
        
        // Zoom towards center point
        auto center_norm = (center - view.position()) / old_size;
        view.x += size_delta.width * center_norm.x;
        view.y += size_delta.height * center_norm.y;
        view.width = new_size.width;
        view.height = new_size.height;
        
        zoom *= factor;
    }
};
```

### Collision Detection

```cpp
class collision_system {
public:
    // AABB collision
    static bool aabb_collision(const sdlpp::rect_f& a, const sdlpp::rect_f& b) {
        return a.intersects(b);
    }
    
    // Circle collision
    static bool circle_collision(const sdlpp::point_f& c1, float r1,
                                const sdlpp::point_f& c2, float r2) {
        float dist_sq = c1.distance_squared_to(c2);
        float radius_sum = r1 + r2;
        return dist_sq <= radius_sum * radius_sum;
    }
    
    // Circle-rectangle collision
    static bool circle_rect_collision(const sdlpp::point_f& center, float radius,
                                     const sdlpp::rect_f& rect) {
        // Find closest point on rectangle
        float closest_x = std::clamp(center.x, rect.left(), rect.right());
        float closest_y = std::clamp(center.y, rect.top(), rect.bottom());
        
        // Check distance
        sdlpp::point_f closest{closest_x, closest_y};
        return center.distance_squared_to(closest) <= radius * radius;
    }
};
```

### Grid/Tile System

```cpp
class tile_map {
    sdlpp::size_i grid_size;
    sdlpp::size_i tile_size;
    
public:
    // Convert world position to grid coordinates
    sdlpp::point_i world_to_grid(const sdlpp::point_f& world) const {
        return {
            static_cast<int>(world.x / tile_size.width),
            static_cast<int>(world.y / tile_size.height)
        };
    }
    
    // Get tile bounds in world coordinates
    sdlpp::rect_f get_tile_bounds(int grid_x, int grid_y) const {
        return {
            static_cast<float>(grid_x * tile_size.width),
            static_cast<float>(grid_y * tile_size.height),
            static_cast<float>(tile_size.width),
            static_cast<float>(tile_size.height)
        };
    }
    
    // Get visible tiles for rendering
    std::vector<sdlpp::point_i> get_visible_tiles(const sdlpp::rect_f& view) const {
        auto start = world_to_grid(view.top_left());
        auto end = world_to_grid(view.bottom_right());
        
        std::vector<sdlpp::point_i> visible;
        for (int y = start.y; y <= end.y; ++y) {
            for (int x = start.x; x <= end.x; ++x) {
                if (x >= 0 && x < grid_size.width && 
                    y >= 0 && y < grid_size.height) {
                    visible.push_back({x, y});
                }
            }
        }
        
        return visible;
    }
};
```

## Best Practices

1. **Use Integer Types**: For pixel-perfect positioning
2. **Use Float Types**: For smooth animation and physics
3. **Validate Sizes**: Ensure width/height are non-negative
4. **Cache Calculations**: Store frequently used values like center
5. **Concept Constraints**: Use concepts for generic geometry functions

## Example: Complete Geometry System

```cpp
class geometry_demo {
    // Player
    sdlpp::rect_f player{400, 300, 32, 32};
    sdlpp::point_f velocity{0, 0};
    
    // Camera
    camera cam;
    
    // Obstacles
    std::vector<sdlpp::rect_f> obstacles;
    
public:
    void update(float dt) {
        // Update player position
        auto new_pos = player.position() + velocity * dt;
        auto new_rect = sdlpp::rect_f{new_pos.x, new_pos.y, 
                                      player.width, player.height};
        
        // Check collisions
        bool collided = false;
        for (const auto& obstacle : obstacles) {
            if (new_rect.intersects(obstacle)) {
                collided = true;
                
                // Resolve collision
                auto overlap = new_rect.intersection(obstacle);
                if (overlap) {
                    resolve_collision(new_rect, obstacle, *overlap);
                }
            }
        }
        
        if (!collided) {
            player = new_rect;
        }
        
        // Update camera to follow player
        cam.view = sdlpp::rect_f{
            player.center().x - cam.viewport.width / 2,
            player.center().y - cam.viewport.height / 2,
            static_cast<float>(cam.viewport.width),
            static_cast<float>(cam.viewport.height)
        };
    }
    
    void render(sdlpp::renderer& r) {
        // Render obstacles
        for (const auto& obstacle : obstacles) {
            auto screen_rect = world_to_screen(obstacle);
            r.fill_rect(screen_rect);
        }
        
        // Render player
        auto player_screen = world_to_screen(player);
        r.fill_rect(player_screen);
    }
    
private:
    sdlpp::rect_i world_to_screen(const sdlpp::rect_f& world) {
        auto tl = cam.world_to_screen(world.top_left());
        auto br = cam.world_to_screen(world.bottom_right());
        
        return {tl.x, tl.y, br.x - tl.x, br.y - tl.y};
    }
    
    void resolve_collision(sdlpp::rect_f& moving, const sdlpp::rect_f& static_obj,
                          const sdlpp::rect_f& overlap) {
        // Simple AABB resolution
        if (overlap.width < overlap.height) {
            // Resolve horizontally
            if (moving.center().x < static_obj.center().x) {
                moving.x -= overlap.width;
            } else {
                moving.x += overlap.width;
            }
            velocity.x = 0;
        } else {
            // Resolve vertically
            if (moving.center().y < static_obj.center().y) {
                moving.y -= overlap.height;
            } else {
                moving.y += overlap.height;
            }
            velocity.y = 0;
        }
    }
};
```