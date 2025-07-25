---
title: Mouse Input
sidebar_label: Mouse
---

# Mouse Input

SDL++ provides comprehensive mouse input handling including position tracking, button states, wheel events, and cursor management.

## Basic Mouse State

Get current mouse position and button states:

```cpp
#include <sdlpp/input/mouse.hh>

// Get current mouse state
auto [x, y, buttons] = sdlpp::mouse::get_state();

// Check button states
if (buttons.left()) {
    std::cout << "Left button pressed at: " << x << ", " << y << std::endl;
}

if (buttons.right()) {
    // Right button is pressed
}

if (buttons.middle()) {
    // Middle button is pressed
}

// Get position relative to a window
auto [win_x, win_y] = sdlpp::mouse::get_position_in_window(window);

// Get global position
auto [global_x, global_y] = sdlpp::mouse::get_global_position();
```

## Mouse Events

Handle mouse events in your event loop:

```cpp
void handle_event(const sdlpp::event& e) {
    e.visit([](auto&& event) {
        using T = std::decay_t<decltype(event)>;
        
        if constexpr (std::is_same_v<T, sdlpp::mouse_button_event>) {
            if (event.is_pressed()) {
                std::cout << "Mouse button " << static_cast<int>(event.button) 
                         << " pressed at: " << event.x << ", " << event.y << std::endl;
                
                // Check for double click
                if (event.clicks == 2) {
                    std::cout << "Double click!" << std::endl;
                }
            }
        }
        else if constexpr (std::is_same_v<T, sdlpp::mouse_motion_event>) {
            std::cout << "Mouse moved to: " << event.x << ", " << event.y 
                     << " (delta: " << event.xrel << ", " << event.yrel << ")"
                     << std::endl;
        }
        else if constexpr (std::is_same_v<T, sdlpp::mouse_wheel_event>) {
            std::cout << "Mouse wheel: " << event.x << ", " << event.y << std::endl;
            
            if (event.direction == sdlpp::mouse_wheel_direction::flipped) {
                // Natural scrolling
            }
        }
    });
}
```

## Mouse Buttons

SDL++ supports multiple mouse buttons:

```cpp
// Button enumeration
enum class mouse_button : Uint8 {
    left   = SDL_BUTTON_LEFT,
    middle = SDL_BUTTON_MIDDLE,
    right  = SDL_BUTTON_RIGHT,
    x1     = SDL_BUTTON_X1,      // Extra button 1
    x2     = SDL_BUTTON_X2       // Extra button 2
};

// Check specific button in event
if (event.button == sdlpp::mouse_button::left) {
    handle_left_click(event.x, event.y);
}

// Button state helper
class mouse_button_state {
public:
    bool left() const;
    bool middle() const;
    bool right() const;
    bool x1() const;
    bool x2() const;
    bool any() const;  // Any button pressed
    int count() const; // Number of buttons pressed
};
```

## Cursor Management

### System Cursors

```cpp
// Create system cursor
auto cursor = sdlpp::cursor::create_system(sdlpp::system_cursor::hand);
if (cursor) {
    // Set as active cursor
    cursor->set();
}

// Available system cursors:
// - arrow (default)
// - ibeam (text input)
// - wait (loading)
// - crosshair
// - wait_arrow (loading with arrow)
// - size_nwse, size_nesw (diagonal resize)
// - size_we, size_ns (horizontal/vertical resize)
// - size_all (move)
// - no (not allowed)
// - hand (clickable)
```

### Custom Cursors

```cpp
// Create cursor from surface
auto surface = load_cursor_image("cursor.png");
auto custom_cursor = sdlpp::cursor::create(surface, 
                                          hot_x, hot_y);  // Hotspot
if (custom_cursor) {
    custom_cursor->set();
}

// Create color cursor (SDL 2.0.0+)
auto color_cursor = sdlpp::cursor::create_color(surface, hot_x, hot_y);
```

### Cursor Visibility

```cpp
// Hide cursor
sdlpp::mouse::show_cursor(false);

// Show cursor
sdlpp::mouse::show_cursor(true);

// Check visibility
bool visible = sdlpp::mouse::is_cursor_visible();

// Hide cursor and capture in window
sdlpp::mouse::set_relative_mode(true);
```

## Relative Mouse Mode

For FPS-style mouse control:

```cpp
// Enable relative mode (hides cursor, infinite movement)
auto result = sdlpp::mouse::set_relative_mode(true);
if (!result) {
    std::cerr << "Relative mode not supported: " << result.error() << std::endl;
}

// In relative mode, use relative movement
void handle_mouse_motion(const sdlpp::mouse_motion_event& e) {
    if (sdlpp::mouse::get_relative_mode()) {
        // e.xrel and e.yrel contain movement delta
        camera.rotate(e.xrel * sensitivity, e.yrel * sensitivity);
    }
}

// Disable relative mode
sdlpp::mouse::set_relative_mode(false);
```

## Mouse Capture

Keep mouse events even when outside window:

```cpp
// Capture mouse (get events even outside window)
auto result = sdlpp::mouse::capture(true);

// Release capture
sdlpp::mouse::capture(false);

// Useful for:
// - Drag operations that may leave window
// - Color picker tools
// - Window resizing
```

## Mouse Warping

Move mouse cursor programmatically:

```cpp
// Warp to window position
sdlpp::mouse::warp_in_window(window, 400, 300);

// Warp to global position
sdlpp::mouse::warp_global(1920 / 2, 1080 / 2);

// Note: Warping generates a mouse motion event
```

## Common Mouse Patterns

### Drag and Drop Handler
```cpp
class drag_handler {
    bool dragging = false;
    sdlpp::point_i drag_start;
    sdlpp::point_i drag_offset;
    
public:
    void handle_button(const sdlpp::mouse_button_event& e) {
        if (e.button == sdlpp::mouse_button::left) {
            if (e.is_pressed()) {
                // Start drag
                dragging = true;
                drag_start = {e.x, e.y};
                sdlpp::mouse::capture(true);
            } else {
                // End drag
                dragging = false;
                sdlpp::mouse::capture(false);
            }
        }
    }
    
    void handle_motion(const sdlpp::mouse_motion_event& e) {
        if (dragging) {
            drag_offset = {e.x - drag_start.x, e.y - drag_start.y};
            // Update dragged object position
        }
    }
};
```

### Mouse Hover Detection
```cpp
class hover_detector {
    struct hoverable {
        sdlpp::rect_i bounds;
        std::function<void()> on_enter;
        std::function<void()> on_leave;
        bool hovered = false;
    };
    
    std::vector<hoverable> items;
    
public:
    void add_hoverable(sdlpp::rect_i bounds, 
                      std::function<void()> on_enter,
                      std::function<void()> on_leave) {
        items.push_back({bounds, on_enter, on_leave, false});
    }
    
    void update(int mouse_x, int mouse_y) {
        sdlpp::point_i mouse_pos{mouse_x, mouse_y};
        
        for (auto& item : items) {
            bool now_hovered = item.bounds.contains(mouse_pos);
            
            if (now_hovered && !item.hovered) {
                item.on_enter();
            } else if (!now_hovered && item.hovered) {
                item.on_leave();
            }
            
            item.hovered = now_hovered;
        }
    }
};
```

### Click Handler with Tolerance
```cpp
class click_handler {
    static constexpr int CLICK_TOLERANCE = 5;
    static constexpr auto CLICK_TIME = std::chrono::milliseconds(500);
    
    sdlpp::point_i mouse_down_pos;
    std::chrono::steady_clock::time_point mouse_down_time;
    bool mouse_pressed = false;
    
public:
    void handle_button(const sdlpp::mouse_button_event& e) {
        if (e.button != sdlpp::mouse_button::left) return;
        
        if (e.is_pressed()) {
            mouse_pressed = true;
            mouse_down_pos = {e.x, e.y};
            mouse_down_time = std::chrono::steady_clock::now();
        } else if (mouse_pressed) {
            auto elapsed = std::chrono::steady_clock::now() - mouse_down_time;
            auto distance = std::hypot(e.x - mouse_down_pos.x, 
                                     e.y - mouse_down_pos.y);
            
            if (elapsed < CLICK_TIME && distance < CLICK_TOLERANCE) {
                on_click(e.x, e.y, e.clicks);
            }
            
            mouse_pressed = false;
        }
    }
    
    virtual void on_click(int x, int y, int clicks) = 0;
};
```

## Touch Mouse Events

On touch devices, touches can generate mouse events:

```cpp
// Check if event is from touch
if (event.which == SDL_TOUCH_MOUSEID) {
    // This "mouse" event came from touch input
    // Handle differently if needed
}

// Enable/disable touch-to-mouse events
SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");  // Disable
SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");  // Enable
```

## Best Practices

1. **Capture During Drag**: Use mouse capture for drag operations
2. **Relative Mode for FPS**: Use relative mode for first-person cameras
3. **Check Touch Events**: Distinguish between real mouse and touch-generated events
4. **Custom Cursors**: Provide visual feedback with appropriate cursors
5. **Handle All Buttons**: Don't assume users only have left/right buttons

## Example: Complete Mouse Input System

```cpp
class mouse_input_system {
    // State tracking
    sdlpp::point_i position;
    sdlpp::point_i last_position;
    sdlpp::mouse_button_state buttons;
    std::unordered_set<sdlpp::mouse_button> just_pressed;
    std::unordered_set<sdlpp::mouse_button> just_released;
    
    // Cursor management
    std::unordered_map<std::string, sdlpp::cursor> cursors;
    
public:
    void init() {
        // Load cursors
        cursors["default"] = sdlpp::cursor::create_system(
            sdlpp::system_cursor::arrow);
        cursors["hand"] = sdlpp::cursor::create_system(
            sdlpp::system_cursor::hand);
        cursors["crosshair"] = sdlpp::cursor::create_system(
            sdlpp::system_cursor::crosshair);
    }
    
    void update() {
        just_pressed.clear();
        just_released.clear();
        last_position = position;
        
        auto [x, y, btn_state] = sdlpp::mouse::get_state();
        position = {x, y};
        buttons = btn_state;
    }
    
    void handle_event(const sdlpp::event& e) {
        e.visit([this](auto&& event) {
            using T = std::decay_t<decltype(event)>;
            
            if constexpr (std::is_same_v<T, sdlpp::mouse_button_event>) {
                if (event.is_pressed()) {
                    just_pressed.insert(event.button);
                } else {
                    just_released.insert(event.button);
                }
            }
        });
    }
    
    // Query functions
    sdlpp::point_i get_position() const { return position; }
    sdlpp::point_i get_delta() const { 
        return {position.x - last_position.x, 
                position.y - last_position.y}; 
    }
    
    bool is_button_pressed(sdlpp::mouse_button btn) const {
        switch (btn) {
            case sdlpp::mouse_button::left: return buttons.left();
            case sdlpp::mouse_button::middle: return buttons.middle();
            case sdlpp::mouse_button::right: return buttons.right();
            default: return false;
        }
    }
    
    bool was_button_just_pressed(sdlpp::mouse_button btn) const {
        return just_pressed.count(btn) > 0;
    }
    
    void set_cursor(const std::string& name) {
        if (auto it = cursors.find(name); it != cursors.end()) {
            it->second.set();
        }
    }
};
```