---
title: Event Types
sidebar_label: Event Types
---

# SDL++ Event Types

SDL++ provides a comprehensive type-safe event system that covers all SDL events with modern C++ features.

## Event Base Type

All events in SDL++ derive from a common base:

```cpp
class event {
public:
    event_type type() const;
    Uint32 timestamp() const;
    
    // Visit pattern for type-safe handling
    template<typename Visitor>
    auto visit(Visitor&& visitor) const;
};
```

## Event Categories

### Application Events

```cpp
// Application terminating (mobile)
struct app_terminating_event : event_base {
    // App is being terminated by OS
};

// Low memory warning (mobile)
struct app_low_memory_event : event_base {
    // System is low on memory
};

// App lifecycle (mobile)
struct app_will_enter_background_event : event_base {};
struct app_did_enter_background_event : event_base {};
struct app_will_enter_foreground_event : event_base {};
struct app_did_enter_foreground_event : event_base {};
```

### Window Events

```cpp
struct window_event : event_base {
    SDL_WindowID window_id;
    
    // Specific window events
    bool is_shown() const;
    bool is_hidden() const;
    bool is_exposed() const;
    bool is_moved() const;
    bool is_resized() const;
    bool is_size_changed() const;
    bool is_minimized() const;
    bool is_maximized() const;
    bool is_restored() const;
    bool is_enter() const;      // Mouse entered
    bool is_leave() const;      // Mouse left
    bool is_focus_gained() const;
    bool is_focus_lost() const;
    bool is_close() const;
    bool is_take_focus() const;
    bool is_hit_test() const;
    
    // Get data for specific events
    point_i get_position() const;    // For moved events
    size_i get_size() const;         // For resized events
};
```

### Keyboard Events

```cpp
struct keyboard_event : event_base {
    SDL_WindowID window_id;
    bool state;           // true = pressed, false = released
    bool repeat;          // Key repeat
    
    scancode scan;        // Physical key
    keycode key;          // Logical key
    key_mod_state mods;   // Modifier state
    
    // Helpers
    bool is_pressed() const { return state; }
    bool is_released() const { return !state; }
    std::string key_name() const;
};

// Text input events
struct text_input_event : event_base {
    SDL_WindowID window_id;
    std::string text;     // UTF-8 text
};

struct text_editing_event : event_base {
    SDL_WindowID window_id;
    std::string text;     // Composition text
    Sint32 start;         // Selection start
    Sint32 length;        // Selection length
};
```

### Mouse Events

```cpp
struct mouse_button_event : event_base {
    SDL_WindowID window_id;
    SDL_MouseID which;    // Mouse instance
    mouse_button button;
    bool state;           // true = pressed
    Uint8 clicks;         // 1 = single, 2 = double
    int x, y;             // Window coordinates
    
    bool is_pressed() const { return state; }
    bool is_released() const { return !state; }
    bool is_double_click() const { return clicks == 2; }
};

struct mouse_motion_event : event_base {
    SDL_WindowID window_id;
    SDL_MouseID which;
    mouse_button_state buttons;  // Current button states
    int x, y;             // Window coordinates
    int xrel, yrel;       // Relative motion
};

struct mouse_wheel_event : event_base {
    SDL_WindowID window_id;
    SDL_MouseID which;
    float x, y;           // Scroll amount
    mouse_wheel_direction direction;
    float precise_x, precise_y;  // High-precision scroll
};
```

### Touch Events

```cpp
struct touch_finger_event : event_base {
    SDL_TouchID touch_id;  // Touch device
    SDL_FingerID finger_id;
    float x, y;           // Normalized [0,1]
    float dx, dy;         // Delta movement
    float pressure;       // Normalized [0,1]
    
    bool is_down() const;
    bool is_up() const;
    bool is_motion() const;
};

// Multi-gesture events
struct multi_gesture_event : event_base {
    SDL_TouchID touch_id;
    float dtheta;         // Rotation
    float ddist;          // Pinch
    float x, y;           // Center
    Uint16 num_fingers;
};

// Dollar gesture ($1 recognition)
struct dollar_gesture_event : event_base {
    SDL_TouchID touch_id;
    SDL_GestureID gesture_id;
    Uint32 num_fingers;
    float error;          // Difference from template
    float x, y;           // Center
};
```

### Game Controller Events

```cpp
struct gamepad_axis_event : event_base {
    SDL_JoystickID which;
    gamepad_axis axis;
    Sint16 value;         // -32768 to 32767
    
    float normalized() const {
        return value / 32767.0f;
    }
};

struct gamepad_button_event : event_base {
    SDL_JoystickID which;
    gamepad_button button;
    bool state;
    
    bool is_pressed() const { return state; }
    bool is_released() const { return !state; }
};

struct gamepad_added_event : event_base {
    Sint32 which;         // Device index
};

struct gamepad_removed_event : event_base {
    SDL_JoystickID which; // Instance ID
};

struct gamepad_remapped_event : event_base {
    SDL_JoystickID which;
};

// Touchpad events (PS4/PS5 controllers)
struct gamepad_touchpad_event : event_base {
    SDL_JoystickID which;
    Sint32 touchpad;      // Touchpad index
    Sint32 finger;        // Finger index
    float x, y;           // Normalized [0,1]
    float pressure;
    
    bool is_down() const;
    bool is_up() const;
    bool is_motion() const;
};

// Sensor events (controller gyro/accel)
struct gamepad_sensor_event : event_base {
    SDL_JoystickID which;
    float data[3];        // x, y, z
    Uint64 timestamp_us;  // Microseconds
};
```

### Audio Device Events

```cpp
struct audio_device_event : event_base {
    SDL_AudioDeviceID which;
    bool is_capture;      // true = recording device
    
    bool is_added() const;
    bool is_removed() const;
};
```

### Sensor Events

```cpp
struct sensor_event : event_base {
    SDL_SensorID which;
    float data[6];        // Up to 6 values
    Uint64 timestamp_us;
};
```

### Display Events

```cpp
struct display_event : event_base {
    SDL_DisplayID display;
    
    bool is_connected() const;
    bool is_disconnected() const;
    bool is_orientation() const;
    
    display_orientation get_orientation() const;
};
```

### Drag and Drop Events

```cpp
struct drop_event : event_base {
    SDL_WindowID window_id;
    
    bool is_file() const;
    bool is_text() const;
    bool is_begin() const;
    bool is_complete() const;
    
    std::string get_file() const;  // For file drops
    std::string get_text() const;  // For text drops
    point_i get_position() const;   // Drop position
};
```

### Clipboard Events

```cpp
struct clipboard_event : event_base {
    // Clipboard content changed
};
```

### Render Events

```cpp
struct render_targets_reset_event : event_base {
    // All render targets need to be recreated
};

struct render_device_reset_event : event_base {
    // Render device was reset
};
```

## Custom Events

Register and use custom events:

```cpp
// Register custom event type
Uint32 my_event_type = sdlpp::register_events(1);

// Custom event data
struct my_custom_data {
    int value1;
    float value2;
    std::string message;
};

// Push custom event
void send_custom_event(const my_custom_data& data) {
    sdlpp::user_event event;
    event.type = my_event_type;
    event.code = 0;
    event.data1 = new my_custom_data(data);
    event.data2 = nullptr;
    
    sdlpp::push_event(event);
}

// Handle custom event
void handle_event(const sdlpp::event& e) {
    if (e.type() == my_event_type) {
        auto& user = static_cast<const sdlpp::user_event&>(e);
        auto* data = static_cast<my_custom_data*>(user.data1);
        
        // Process custom data
        process_custom_event(*data);
        
        // Clean up
        delete data;
    }
}
```

## Event Filtering

Filter events before they enter the queue:

```cpp
// Event filter function
sdlpp::event_filter_result my_filter(const sdlpp::event& event) {
    // Block all mouse motion events
    if (event.type() == sdlpp::event_type::mouse_motion) {
        return sdlpp::event_filter_result::drop;
    }
    
    // Monitor but don't block keyboard events
    if (event.type() == sdlpp::event_type::key_down) {
        log_key_press(static_cast<const sdlpp::keyboard_event&>(event));
    }
    
    return sdlpp::event_filter_result::keep;
}

// Set filter
sdlpp::set_event_filter(my_filter);

// Add additional filter
auto filter_id = sdlpp::add_event_watch(my_filter);

// Remove filter (automatic with RAII)
```

## Common Event Patterns

### Event Dispatcher

```cpp
class event_dispatcher {
    using handler_func = std::function<void(const sdlpp::event&)>;
    std::unordered_map<event_type, std::vector<handler_func>> handlers;
    
public:
    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> handler) {
        auto type = EventType::static_type();
        handlers[type].push_back(
            [handler](const sdlpp::event& e) {
                handler(static_cast<const EventType&>(e));
            }
        );
    }
    
    void dispatch(const sdlpp::event& event) {
        if (auto it = handlers.find(event.type()); it != handlers.end()) {
            for (auto& handler : it->second) {
                handler(event);
            }
        }
    }
};

// Usage
event_dispatcher dispatcher;

dispatcher.subscribe<sdlpp::keyboard_event>(
    [](const auto& e) {
        if (e.is_pressed()) {
            std::cout << "Key pressed: " << e.key_name() << std::endl;
        }
    }
);
```

### Event Recording/Playback

```cpp
class event_recorder {
    struct timed_event {
        Uint32 timestamp;
        std::unique_ptr<sdlpp::event> event;
    };
    
    std::vector<timed_event> recorded_events;
    Uint32 start_time = 0;
    bool recording = false;
    
public:
    void start_recording() {
        recorded_events.clear();
        start_time = sdlpp::get_ticks();
        recording = true;
    }
    
    void stop_recording() {
        recording = false;
    }
    
    void record_event(const sdlpp::event& e) {
        if (!recording) return;
        
        recorded_events.push_back({
            e.timestamp() - start_time,
            e.clone()  // Deep copy
        });
    }
    
    void playback() {
        Uint32 playback_start = sdlpp::get_ticks();
        
        for (const auto& [timestamp, event] : recorded_events) {
            // Wait until event time
            while (sdlpp::get_ticks() - playback_start < timestamp) {
                sdlpp::delay(1);
            }
            
            // Push event
            sdlpp::push_event(*event);
        }
    }
};
```

## Best Practices

1. **Use Visit Pattern**: Leverage the visitor pattern for type-safe event handling
2. **Filter Early**: Use event filters to drop unwanted events before processing
3. **Batch Processing**: Process all pending events in one frame
4. **Custom Events**: Use custom events for game-specific logic
5. **Avoid Blocking**: Don't block in event handlers

## Example: Complete Event System

```cpp
class event_system {
    // Event statistics
    std::unordered_map<event_type, size_t> event_counts;
    
    // Frame event lists
    std::vector<sdlpp::keyboard_event> frame_key_events;
    std::vector<sdlpp::mouse_button_event> frame_mouse_events;
    
public:
    void process_events() {
        // Clear frame lists
        frame_key_events.clear();
        frame_mouse_events.clear();
        
        // Process all events
        sdlpp::event_queue queue;
        while (auto event = queue.poll()) {
            // Statistics
            event_counts[event->type()]++;
            
            // Dispatch by type
            event->visit([this](auto&& e) {
                handle_specific_event(e);
            });
        }
    }
    
private:
    void handle_specific_event(const sdlpp::keyboard_event& e) {
        frame_key_events.push_back(e);
        
        // Immediate handling for system keys
        if (e.is_pressed() && e.scan == sdlpp::scancode::f11) {
            toggle_fullscreen();
        }
    }
    
    void handle_specific_event(const sdlpp::mouse_button_event& e) {
        frame_mouse_events.push_back(e);
    }
    
    void handle_specific_event(const sdlpp::window_event& e) {
        if (e.is_close()) {
            request_quit();
        } else if (e.is_resized()) {
            handle_resize(e.get_size());
        } else if (e.is_focus_lost()) {
            pause_game();
        }
    }
    
    void handle_specific_event(const sdlpp::quit_event& e) {
        force_quit();
    }
    
    // Default handler
    template<typename T>
    void handle_specific_event(const T& e) {
        // Log unhandled event types in debug
        #ifdef DEBUG
        std::cout << "Unhandled event type: " 
                  << static_cast<int>(e.type()) << std::endl;
        #endif
    }
    
public:
    // Query frame events
    bool was_key_pressed(sdlpp::scancode key) const {
        return std::any_of(frame_key_events.begin(), frame_key_events.end(),
            [key](const auto& e) {
                return e.is_pressed() && e.scan == key;
            });
    }
    
    bool was_mouse_clicked(sdlpp::mouse_button btn) const {
        return std::any_of(frame_mouse_events.begin(), frame_mouse_events.end(),
            [btn](const auto& e) {
                return e.is_pressed() && e.button == btn;
            });
    }
    
    void log_statistics() const {
        std::cout << "Event Statistics:\n";
        for (const auto& [type, count] : event_counts) {
            std::cout << "  " << event_type_name(type) 
                     << ": " << count << "\n";
        }
    }
};
```