---
sidebar_position: 1
---

# Event Queue

The event queue module provides the main interface for handling SDL events in a type-safe manner. SDL++ wraps SDL's event system with modern C++ features like variants and visitor patterns.

## Basic Usage

### Polling Events

```cpp
#include <sdlpp/events/events.hh>

// Poll single event
while (auto event = sdlpp::event_queue::poll()) {
    // Process event
    if (event->type() == sdlpp::event_type::quit) {
        break;
    }
}

// Poll all available events
while (running) {
    while (auto event = sdlpp::event_queue::poll()) {
        // Process each event
        handle_event(*event);
    }
    
    // Update game state
    // Render frame
}
```

### Waiting for Events

```cpp
// Wait indefinitely for next event
auto event = sdlpp::event_queue::wait();
if (event) {
    handle_event(*event);
}

// Wait with timeout
auto event = sdlpp::event_queue::wait_timeout(std::chrono::milliseconds(100));
if (event) {
    handle_event(*event);
} else {
    // Timeout occurred
}
```

## Event Processing

### Using the Visitor Pattern

```cpp
void handle_event(const sdlpp::event& event) {
    event.visit([](auto&& e) {
        using T = std::decay_t<decltype(e)>;
        
        if constexpr (std::is_same_v<T, sdlpp::quit_event>) {
            std::cout << "Application quit requested\n";
        }
        else if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
            if (e.is_pressed()) {
                std::cout << "Key pressed: " << e.key << "\n";
            }
        }
        else if constexpr (std::is_same_v<T, sdlpp::mouse_button_event>) {
            std::cout << "Mouse button " << e.button << " "
                      << (e.is_pressed() ? "pressed" : "released") << "\n";
        }
        else if constexpr (std::is_same_v<T, sdlpp::mouse_motion_event>) {
            std::cout << "Mouse moved to " << e.x << ", " << e.y << "\n";
        }
        // Handle other event types...
    });
}
```

### Using Type Checking

```cpp
void handle_event(const sdlpp::event& event) {
    switch (event.type()) {
        case sdlpp::event_type::quit:
            running = false;
            break;
            
        case sdlpp::event_type::key_down:
        case sdlpp::event_type::key_up:
            if (auto* key_event = event.as<sdlpp::keyboard_event>()) {
                handle_keyboard(*key_event);
            }
            break;
            
        case sdlpp::event_type::mouse_button_down:
        case sdlpp::event_type::mouse_button_up:
            if (auto* mouse_event = event.as<sdlpp::mouse_button_event>()) {
                handle_mouse_button(*mouse_event);
            }
            break;
            
        default:
            break;
    }
}
```

## Event Filtering

### Push Event Filter

```cpp
// Filter that blocks all keyboard events
auto keyboard_filter = [](const sdlpp::event& event) -> bool {
    return event.type() != sdlpp::event_type::key_down &&
           event.type() != sdlpp::event_type::key_up;
};

// Add filter
sdlpp::event_queue::add_filter(keyboard_filter);

// Remove filter later
sdlpp::event_queue::remove_filter(keyboard_filter);
```

### Event State Control

```cpp
// Disable specific event types
sdlpp::event_queue::set_state(sdlpp::event_type::mouse_motion, false);

// Re-enable events
sdlpp::event_queue::set_state(sdlpp::event_type::mouse_motion, true);

// Query current state
bool motion_enabled = sdlpp::event_queue::get_state(sdlpp::event_type::mouse_motion);
```

## Custom Events

### Registering Custom Events

```cpp
// Register custom event type
auto custom_type = sdlpp::event_queue::register_events(1);
if (!custom_type) {
    std::cerr << "Failed to register custom event\n";
    return;
}

// Store the custom event type
constexpr uint32_t MY_CUSTOM_EVENT = *custom_type;
```

### Pushing Custom Events

```cpp
// Create custom event data
struct my_event_data {
    int value;
    std::string message;
};

// Push custom event
sdlpp::user_event event;
event.type = MY_CUSTOM_EVENT;
event.code = 42;
event.data1 = new my_event_data{100, "Hello"};
event.data2 = nullptr;

auto result = sdlpp::event_queue::push(event);
if (!result) {
    std::cerr << "Failed to push event: " << result.error() << "\n";
    delete static_cast<my_event_data*>(event.data1);
}
```

### Handling Custom Events

```cpp
while (auto event = sdlpp::event_queue::poll()) {
    event->visit([](auto&& e) {
        using T = std::decay_t<decltype(e)>;
        
        if constexpr (std::is_same_v<T, sdlpp::user_event>) {
            if (e.type == MY_CUSTOM_EVENT) {
                auto* data = static_cast<my_event_data*>(e.data1);
                std::cout << "Custom event: " << data->message << "\n";
                delete data;
            }
        }
    });
}
```

## Event Queue Management

### Flushing Events

```cpp
// Flush all events
sdlpp::event_queue::flush();

// Flush specific event type
sdlpp::event_queue::flush(sdlpp::event_type::mouse_motion);

// Flush range of event types
sdlpp::event_queue::flush(
    sdlpp::event_type::key_down,
    sdlpp::event_type::key_up
);
```

### Peeking at Events

```cpp
// Peek at events without removing them
std::vector<sdlpp::event> events;
auto count = sdlpp::event_queue::peek(events, 10); // Peek up to 10 events

// Peek at specific event types
auto key_count = sdlpp::event_queue::peek(
    events,
    sdlpp::event_type::key_down,
    sdlpp::event_type::key_up,
    10
);
```

### Checking Event Queue

```cpp
// Check if any events are waiting
bool has_events = sdlpp::event_queue::has_events();

// Check for specific event type
bool has_quit = sdlpp::event_queue::has_event(sdlpp::event_type::quit);

// Check for range of event types
bool has_mouse = sdlpp::event_queue::has_events(
    sdlpp::event_type::mouse_button_down,
    sdlpp::event_type::mouse_wheel
);
```

## Common Event Patterns

### Game Input Handler

```cpp
class input_handler {
    std::unordered_set<sdlpp::scancode> pressed_keys;
    std::unordered_set<uint8_t> pressed_buttons;
    sdlpp::point mouse_pos;
    
public:
    void process_event(const sdlpp::event& event) {
        event.visit([this](auto&& e) {
            using T = std::decay_t<decltype(e)>;
            
            if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
                if (e.is_pressed()) {
                    pressed_keys.insert(e.scan);
                } else {
                    pressed_keys.erase(e.scan);
                }
            }
            else if constexpr (std::is_same_v<T, sdlpp::mouse_button_event>) {
                if (e.is_pressed()) {
                    pressed_buttons.insert(e.button);
                } else {
                    pressed_buttons.erase(e.button);
                }
            }
            else if constexpr (std::is_same_v<T, sdlpp::mouse_motion_event>) {
                mouse_pos = {e.x, e.y};
            }
        });
    }
    
    bool is_key_pressed(sdlpp::scancode key) const {
        return pressed_keys.count(key) > 0;
    }
    
    bool is_button_pressed(uint8_t button) const {
        return pressed_buttons.count(button) > 0;
    }
    
    sdlpp::point get_mouse_pos() const {
        return mouse_pos;
    }
};
```

### Event Recording and Playback

```cpp
class event_recorder {
    struct timed_event {
        std::chrono::milliseconds timestamp;
        sdlpp::event event;
    };
    
    std::vector<timed_event> recorded_events;
    std::chrono::steady_clock::time_point start_time;
    bool recording = false;
    
public:
    void start_recording() {
        recorded_events.clear();
        start_time = std::chrono::steady_clock::now();
        recording = true;
    }
    
    void record_event(const sdlpp::event& event) {
        if (!recording) return;
        
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - start_time
        );
        
        recorded_events.push_back({elapsed, event});
    }
    
    void playback() {
        auto playback_start = std::chrono::steady_clock::now();
        
        for (const auto& [timestamp, event] : recorded_events) {
            // Wait until the right time
            auto target_time = playback_start + timestamp;
            std::this_thread::sleep_until(target_time);
            
            // Push the recorded event
            sdlpp::event_queue::push(event);
        }
    }
};
```

### Event Dispatcher

```cpp
template<typename... Handlers>
class event_dispatcher {
    std::tuple<Handlers...> handlers;
    
public:
    explicit event_dispatcher(Handlers... h) : handlers(h...) {}
    
    void dispatch(const sdlpp::event& event) {
        event.visit([this](auto&& e) {
            dispatch_to_handlers(e, std::index_sequence_for<Handlers...>{});
        });
    }
    
private:
    template<typename Event, size_t... Is>
    void dispatch_to_handlers(Event&& e, std::index_sequence<Is...>) {
        (try_dispatch<Is>(std::forward<Event>(e)), ...);
    }
    
    template<size_t I, typename Event>
    void try_dispatch(Event&& e) {
        auto& handler = std::get<I>(handlers);
        if constexpr (std::is_invocable_v<decltype(handler), Event>) {
            handler(std::forward<Event>(e));
        }
    }
};

// Usage
auto dispatcher = event_dispatcher{
    [](const sdlpp::keyboard_event& e) {
        std::cout << "Key event\n";
    },
    [](const sdlpp::mouse_button_event& e) {
        std::cout << "Mouse button event\n";
    },
    [](const sdlpp::quit_event& e) {
        std::cout << "Quit event\n";
    }
};

while (auto event = sdlpp::event_queue::poll()) {
    dispatcher.dispatch(*event);
}
```

## Thread Safety

### Event Queue and Threads

```cpp
// Events can be safely pushed from any thread
std::thread worker([&]() {
    // Do some work...
    
    // Notify main thread
    sdlpp::user_event event;
    event.type = WORK_COMPLETE_EVENT;
    event.code = 0;
    event.data1 = new work_result{...};
    
    sdlpp::event_queue::push(event);
});

// Main thread polls events normally
while (auto event = sdlpp::event_queue::poll()) {
    // Handle events including those from worker thread
}
```

## Best Practices

1. **Process All Events**: Always drain the event queue each frame
2. **Use Visitors**: Prefer the visitor pattern for type-safe event handling
3. **Filter Early**: Use event filters to reduce processing overhead
4. **Clean Up Custom Events**: Always free custom event data
5. **Avoid Blocking**: Don't perform long operations in event handlers
6. **Thread Safety**: Only poll events from the main thread
7. **State Management**: Track input state for game logic

## Performance Considerations

```cpp
// Efficient event processing
void process_events() {
    // Pre-allocate for peeking
    static std::vector<sdlpp::event> event_buffer;
    event_buffer.reserve(64);
    
    // Process in batches
    while (sdlpp::event_queue::has_events()) {
        auto count = sdlpp::event_queue::peek(event_buffer, 64);
        
        for (size_t i = 0; i < count; ++i) {
            handle_event(event_buffer[i]);
        }
        
        // Remove processed events
        for (size_t i = 0; i < count; ++i) {
            sdlpp::event_queue::poll();
        }
    }
}
```

## API Reference

### Free Functions

- `event_queue::poll()` - Get next event
- `event_queue::wait()` - Wait for next event
- `event_queue::wait_timeout()` - Wait with timeout
- `event_queue::push()` - Add event to queue
- `event_queue::peek()` - Look at events without removing
- `event_queue::flush()` - Remove events from queue
- `event_queue::has_events()` - Check if events are waiting
- `event_queue::add_filter()` - Add event filter
- `event_queue::set_state()` - Enable/disable event types
- `event_queue::register_events()` - Register custom event types

### Classes

- `event` - Base event type (variant)
- `event_filter` - Event filter callback

### Event Types

See [Event Types](event-types) for the complete list of event types and their properties.