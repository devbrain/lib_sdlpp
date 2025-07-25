---
sidebar_position: 3
---

# Drawing Shapes

Learn how to draw basic 2D shapes using SDL++'s renderer API.

## Complete Code

```cpp
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/timer/timer.hh>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

int main() {
    // Initialize SDL
    auto init = sdlpp::init::create(sdlpp::init_flags::video);
    if (!init) {
        std::cerr << "Failed to initialize SDL: " << init.error() << "\n";
        return 1;
    }
    
    // Create window and renderer
    auto window = sdlpp::window::create("Drawing Shapes", 800, 600);
    if (!window) {
        std::cerr << "Failed to create window: " << window.error() << "\n";
        return 1;
    }
    
    auto renderer = sdlpp::renderer::create(*window,
        sdlpp::renderer_flags::accelerated | 
        sdlpp::renderer_flags::presentvsync);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << renderer.error() << "\n";
        return 1;
    }
    
    // Enable alpha blending
    renderer->set_draw_blend_mode(sdlpp::blend_mode::blend);
    
    // Random number generation
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis_pos(50, 750);
    std::uniform_int_distribution<> dis_size(20, 100);
    std::uniform_int_distribution<> dis_color(0, 255);
    
    bool running = true;
    float time = 0.0f;
    
    while (running) {
        auto frame_start = sdlpp::timer::get_performance_counter();
        
        // Handle events
        while (auto event = sdlpp::event_queue::poll()) {
            if (event->type() == sdlpp::event_type::quit) {
                running = false;
            }
            
            event->visit([&](auto&& e) {
                using T = std::decay_t<decltype(e)>;
                
                if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
                    if (e.is_pressed()) {
                        switch (e.scan) {
                            case sdlpp::scancode::escape:
                                running = false;
                                break;
                            case sdlpp::scancode::space:
                                // Clear screen on space
                                renderer->set_draw_color({0, 0, 0, 255});
                                renderer->clear();
                                break;
                            default:
                                break;
                        }
                    }
                }
            });
        }
        
        // Clear screen with dark gray
        renderer->set_draw_color({32, 32, 32, 255});
        renderer->clear();
        
        // Draw points in a sine wave pattern
        renderer->set_draw_color({255, 255, 0, 255}); // Yellow
        std::vector<sdlpp::point> points;
        for (int x = 0; x < 800; x += 2) {
            int y = 300 + static_cast<int>(50 * std::sin(x * 0.02f + time));
            points.push_back({x, y});
        }
        renderer->draw_points(points);
        
        // Draw lines forming a star
        renderer->set_draw_color({0, 255, 255, 255}); // Cyan
        const int num_points = 5;
        const float radius = 100;
        std::vector<sdlpp::point> star_points;
        
        for (int i = 0; i < num_points * 2; ++i) {
            float angle = (i * M_PI) / num_points + time * 0.5f;
            float r = (i % 2 == 0) ? radius : radius * 0.5f;
            int x = 400 + static_cast<int>(r * std::cos(angle));
            int y = 150 + static_cast<int>(r * std::sin(angle));
            star_points.push_back({x, y});
        }
        
        // Connect points to form star
        for (size_t i = 0; i < star_points.size(); ++i) {
            size_t next = (i + 1) % star_points.size();
            renderer->draw_line(star_points[i], star_points[next]);
        }
        
        // Draw filled rectangles with transparency
        for (int i = 0; i < 5; ++i) {
            int alpha = 255 - i * 40;
            renderer->set_draw_color({255, 0, 0, static_cast<uint8_t>(alpha)});
            
            int size = 80 + i * 20;
            int x = 100 + i * 30;
            int y = 350;
            renderer->fill_rect({{x, y}, {size, size}});
        }
        
        // Draw outlined rectangles
        renderer->set_draw_color({0, 255, 0, 255}); // Green
        for (int i = 0; i < 3; ++i) {
            sdlpp::rect r{{500 + i * 60, 350 + i * 30}, {100 - i * 20, 80 - i * 15}};
            renderer->draw_rect(r);
        }
        
        // Draw a grid of small rectangles
        renderer->set_draw_color({128, 128, 255, 255}); // Light blue
        for (int y = 450; y < 550; y += 10) {
            for (int x = 50; x < 750; x += 10) {
                if ((x + y) % 20 == 0) {
                    renderer->fill_rect({{x, y}, {8, 8}});
                }
            }
        }
        
        // Draw thick lines by drawing multiple lines
        auto draw_thick_line = [&](sdlpp::point p1, sdlpp::point p2, int thickness) {
            for (int i = -thickness/2; i <= thickness/2; ++i) {
                renderer->draw_line({p1.x + i, p1.y}, {p2.x + i, p2.y});
                renderer->draw_line({p1.x, p1.y + i}, {p2.x, p2.y + i});
            }
        };
        
        renderer->set_draw_color({255, 0, 255, 255}); // Magenta
        draw_thick_line({600, 50}, {700, 150}, 5);
        
        // Draw random shapes on mouse click
        static std::vector<std::tuple<sdlpp::rect, sdlpp::color>> random_rects;
        
        while (auto event = sdlpp::event_queue::poll()) {
            event->visit([&](auto&& e) {
                using T = std::decay_t<decltype(e)>;
                
                if constexpr (std::is_same_v<T, sdlpp::mouse_button_event>) {
                    if (e.is_pressed() && e.button == 1) { // Left click
                        sdlpp::rect r{
                            {e.x - dis_size(gen)/2, e.y - dis_size(gen)/2},
                            {dis_size(gen), dis_size(gen)}
                        };
                        sdlpp::color c{
                            static_cast<uint8_t>(dis_color(gen)),
                            static_cast<uint8_t>(dis_color(gen)),
                            static_cast<uint8_t>(dis_color(gen)),
                            200
                        };
                        random_rects.push_back({r, c});
                        
                        // Keep only last 20 shapes
                        if (random_rects.size() > 20) {
                            random_rects.erase(random_rects.begin());
                        }
                    }
                }
            });
        }
        
        // Draw random rectangles
        for (const auto& [rect, color] : random_rects) {
            renderer->set_draw_color(color);
            renderer->fill_rect(rect);
        }
        
        // Present the frame
        renderer->present();
        
        // Update time for animation
        time += 0.016f; // Approximately 60 FPS
        
        // Calculate frame time
        auto frame_end = sdlpp::timer::get_performance_counter();
        auto frame_time = sdlpp::timer::performance_to_seconds(frame_end - frame_start);
        
        // Display FPS in window title
        static int frame_count = 0;
        static float fps_timer = 0.0f;
        fps_timer += frame_time;
        frame_count++;
        
        if (fps_timer >= 1.0f) {
            float fps = frame_count / fps_timer;
            window->set_title("Drawing Shapes - FPS: " + std::to_string(static_cast<int>(fps)));
            frame_count = 0;
            fps_timer = 0.0f;
        }
    }
    
    return 0;
}
```

## Key Concepts

### Drawing Primitives

SDL++ provides several functions for drawing basic shapes:

#### Points
```cpp
// Single point
renderer->draw_point({100, 100});

// Multiple points
std::vector<sdlpp::point> points = {{10, 10}, {20, 20}, {30, 30}};
renderer->draw_points(points);
```

#### Lines
```cpp
// Single line
renderer->draw_line({0, 0}, {100, 100});

// Connected lines
std::vector<sdlpp::point> line_strip = {{0, 0}, {50, 100}, {100, 50}};
renderer->draw_lines(line_strip);
```

#### Rectangles
```cpp
// Outlined rectangle
sdlpp::rect r{{10, 10}, {100, 50}};
renderer->draw_rect(r);

// Filled rectangle
renderer->fill_rect(r);

// Multiple rectangles
std::vector<sdlpp::rect> rects = {{{10, 10}, {50, 50}}, {{70, 10}, {50, 50}}};
renderer->draw_rects(rects);
renderer->fill_rects(rects);
```

### Color and Transparency

```cpp
// Set draw color (RGBA)
renderer->set_draw_color({255, 0, 0, 255}); // Opaque red
renderer->set_draw_color({0, 255, 0, 128}); // Semi-transparent green

// Enable blending for transparency
renderer->set_draw_blend_mode(sdlpp::blend_mode::blend);
```

### Animation Techniques

The example shows several animation techniques:

1. **Time-based animation**: Using a time variable for smooth motion
2. **Sine wave motion**: Creating smooth periodic movement
3. **Rotation**: Rotating points around a center
4. **Alpha fading**: Creating transparency effects

### Performance Optimization

```cpp
// Draw multiple primitives at once
std::vector<sdlpp::point> many_points;
// ... fill vector ...
renderer->draw_points(many_points); // Single draw call

// Use VSync to prevent tearing
auto renderer = sdlpp::renderer::create(*window,
    sdlpp::renderer_flags::presentvsync);
```

## Exercises

### 1. Draw a Circle

Since SDL doesn't provide a circle primitive, implement one using points:

```cpp
void draw_circle(sdlpp::renderer& renderer, sdlpp::point center, int radius) {
    std::vector<sdlpp::point> points;
    
    for (int angle = 0; angle < 360; ++angle) {
        float rad = angle * M_PI / 180.0f;
        int x = center.x + static_cast<int>(radius * std::cos(rad));
        int y = center.y + static_cast<int>(radius * std::sin(rad));
        points.push_back({x, y});
    }
    
    renderer.draw_points(points);
}
```

### 2. Create a Drawing App

Add mouse drawing functionality:

```cpp
std::vector<sdlpp::point> drawing;
bool is_drawing = false;

// In event loop
if constexpr (std::is_same_v<T, sdlpp::mouse_button_event>) {
    if (e.button == 1) { // Left button
        is_drawing = e.is_pressed();
        if (is_drawing) {
            drawing.clear();
        }
    }
}
else if constexpr (std::is_same_v<T, sdlpp::mouse_motion_event>) {
    if (is_drawing) {
        drawing.push_back({e.x, e.y});
    }
}

// In render loop
if (drawing.size() > 1) {
    renderer->draw_lines(drawing);
}
```

### 3. Gradient Effect

Create a gradient by drawing rectangles with changing colors:

```cpp
for (int y = 0; y < 600; ++y) {
    uint8_t intensity = static_cast<uint8_t>((y * 255) / 600);
    renderer->set_draw_color({intensity, 0, 255 - intensity, 255});
    renderer->draw_line({0, y}, {800, y});
}
```

## Common Patterns

### Shape Batching

For better performance, batch similar operations:

```cpp
// Inefficient
for (const auto& rect : rectangles) {
    renderer->fill_rect(rect);
}

// Better - use fill_rects
renderer->fill_rects(rectangles);
```

### State Management

Minimize state changes:

```cpp
// Group by color
renderer->set_draw_color({255, 0, 0, 255});
// Draw all red shapes

renderer->set_draw_color({0, 255, 0, 255});
// Draw all green shapes
```

## Next Steps

- Learn about [textures and images](loading-images) for more complex graphics
- Explore [render targets](render-targets) for advanced effects
- Try [OpenGL integration](opengl-integration) for 3D graphics

## Building This Example

```bash
g++ -std=c++20 drawing_shapes.cpp -lsdlpp -lSDL3 -lm -o drawing_shapes
./drawing_shapes
```

This example demonstrates the fundamental drawing operations in SDL++. These simple primitives can be combined to create complex graphics and visual effects for games and applications.