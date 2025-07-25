---
title: Keyboard Input
sidebar_label: Keyboard
---

# Keyboard Input

SDL++ provides comprehensive keyboard input handling with type-safe key codes and modern C++ features.

## Basic Keyboard State

Check the current state of keyboard keys:

```cpp
#include <sdlpp/input/keyboard.hh>

// Get current keyboard state
auto keyboard = sdlpp::keyboard::get_state();

// Check if a key is pressed
if (keyboard[sdlpp::scancode::space]) {
    player.jump();
}

// Check multiple keys
if (keyboard[sdlpp::scancode::left_shift] || keyboard[sdlpp::scancode::right_shift]) {
    player.run();
}

// Movement with arrow keys
if (keyboard[sdlpp::scancode::left]) {
    player.move_left();
}
if (keyboard[sdlpp::scancode::right]) {
    player.move_right();
}
```

## Keyboard Events

Handle keyboard events in your event loop:

```cpp
void handle_event(const sdlpp::event& e) {
    e.visit([](auto&& event) {
        using T = std::decay_t<decltype(event)>;
        
        if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
            if (event.is_pressed()) {
                std::cout << "Key pressed: " << event.key_name() << std::endl;
                
                // Check specific keys
                switch (event.scan) {
                    case sdlpp::scancode::escape:
                        quit_game();
                        break;
                    case sdlpp::scancode::f11:
                        toggle_fullscreen();
                        break;
                }
            } else {
                std::cout << "Key released: " << event.key_name() << std::endl;
            }
            
            // Check for key repeat
            if (event.repeat) {
                std::cout << "Key is repeating" << std::endl;
            }
        }
    });
}
```

## Scancode vs Keycode

SDL++ distinguishes between scancodes (physical keys) and keycodes (logical keys):

```cpp
// Scancode: Physical key position (layout-independent)
// Use for game controls
if (event.scan == sdlpp::scancode::w) {
    // Physical W key, regardless of keyboard layout
}

// Keycode: Logical key (layout-dependent)
// Use for text input
if (event.key == sdlpp::keycode::w) {
    // Logical 'w' character (might be different physical key on AZERTY)
}

// Convert between them
auto keycode = sdlpp::scancode_to_keycode(sdlpp::scancode::a);
auto scancode = sdlpp::keycode_to_scancode(sdlpp::keycode::a);
```

## Key Modifiers

Check and use keyboard modifiers:

```cpp
// In event handler
if (event.mods.has(sdlpp::key_mod::ctrl)) {
    if (event.scan == sdlpp::scancode::s) {
        save_file();  // Ctrl+S
    }
}

// Check multiple modifiers
if (event.mods.has(sdlpp::key_mod::ctrl | sdlpp::key_mod::shift)) {
    // Ctrl+Shift+something
}

// Get current modifier state
auto current_mods = sdlpp::keyboard::get_mod_state();
if (current_mods.has(sdlpp::key_mod::alt)) {
    // Alt is currently pressed
}

// Common modifier checks
bool ctrl_pressed = event.mods.ctrl();
bool shift_pressed = event.mods.shift();
bool alt_pressed = event.mods.alt();
bool gui_pressed = event.mods.gui();  // Windows/Command key
```

## Text Input

For text input, use SDL's text input mode:

```cpp
class text_input_handler {
    std::string text;
    bool active = false;
    
public:
    void start() {
        sdlpp::start_text_input();
        active = true;
        text.clear();
    }
    
    void stop() {
        sdlpp::stop_text_input();
        active = false;
    }
    
    void handle_event(const sdlpp::event& e) {
        e.visit([this](auto&& event) {
            using T = std::decay_t<decltype(event)>;
            
            if constexpr (std::is_same_v<T, sdlpp::text_input_event>) {
                // Append UTF-8 text
                text += event.text;
            }
            else if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
                if (event.is_pressed()) {
                    // Handle special keys
                    if (event.scan == sdlpp::scancode::backspace && !text.empty()) {
                        // Remove last UTF-8 character properly
                        auto it = text.end();
                        while (it != text.begin()) {
                            --it;
                            if ((*it & 0xC0) != 0x80) {
                                text.erase(it, text.end());
                                break;
                            }
                        }
                    }
                    else if (event.scan == sdlpp::scancode::return_) {
                        // Submit text
                        submit(text);
                        text.clear();
                    }
                }
            }
        });
    }
};
```

## Key Names and Strings

Get human-readable key names:

```cpp
// Get name from scancode
std::string key_name = sdlpp::get_scancode_name(sdlpp::scancode::space);
// Returns "Space"

// Get name from keycode
std::string key_char = sdlpp::get_key_name(sdlpp::keycode::a);
// Returns "A"

// Parse from string
auto scan = sdlpp::get_scancode_from_name("Space");
if (scan != sdlpp::scancode::unknown) {
    // Valid scancode
}
```

## Common Keyboard Patterns

### Movement Handler
```cpp
class movement_handler {
    struct {
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;
    } keys;
    
public:
    void handle_event(const sdlpp::keyboard_event& e) {
        bool pressed = e.is_pressed();
        
        switch (e.scan) {
            case sdlpp::scancode::w:
            case sdlpp::scancode::up:
                keys.up = pressed;
                break;
            case sdlpp::scancode::s:
            case sdlpp::scancode::down:
                keys.down = pressed;
                break;
            case sdlpp::scancode::a:
            case sdlpp::scancode::left:
                keys.left = pressed;
                break;
            case sdlpp::scancode::d:
            case sdlpp::scancode::right:
                keys.right = pressed;
                break;
        }
    }
    
    vec2 get_direction() const {
        vec2 dir{0, 0};
        if (keys.up) dir.y -= 1;
        if (keys.down) dir.y += 1;
        if (keys.left) dir.x -= 1;
        if (keys.right) dir.x += 1;
        return normalize(dir);
    }
};
```

### Hotkey System
```cpp
class hotkey_system {
    struct hotkey {
        sdlpp::scancode key;
        sdlpp::key_mod mods;
        std::function<void()> action;
    };
    
    std::vector<hotkey> hotkeys;
    
public:
    void register_hotkey(sdlpp::scancode key, sdlpp::key_mod mods, 
                        std::function<void()> action) {
        hotkeys.push_back({key, mods, std::move(action)});
    }
    
    void handle_event(const sdlpp::keyboard_event& e) {
        if (!e.is_pressed() || e.repeat) return;
        
        for (const auto& hk : hotkeys) {
            if (e.scan == hk.key && e.mods.has_all(hk.mods)) {
                hk.action();
                break;
            }
        }
    }
};

// Usage
hotkey_system hotkeys;
hotkeys.register_hotkey(sdlpp::scancode::s, 
                       sdlpp::key_mod::ctrl, 
                       []{ save_game(); });
hotkeys.register_hotkey(sdlpp::scancode::l, 
                       sdlpp::key_mod::ctrl, 
                       []{ load_game(); });
```

## Virtual Keyboard (Mobile)

For mobile platforms:

```cpp
// Check if on-screen keyboard is supported
if (sdlpp::has_screen_keyboard_support()) {
    // Show virtual keyboard
    sdlpp::show_screen_keyboard(window);
    
    // Hide virtual keyboard
    sdlpp::hide_screen_keyboard(window);
    
    // Check if visible
    if (sdlpp::is_screen_keyboard_shown(window)) {
        // Adjust UI layout
    }
}

// Set text input area (helps position virtual keyboard)
sdlpp::set_text_input_rect(sdlpp::rect_i{100, 400, 200, 50});
```

## Best Practices

1. **Use Scancodes for Games**: Physical key positions don't change with layout
2. **Use Keycodes for Text**: Respects user's keyboard layout
3. **Handle Repeat**: Check `event.repeat` to ignore held keys when needed
4. **Text Input Mode**: Use text input events for actual text entry
5. **Modifier Keys**: Always check modifiers for shortcuts

## Example: Complete Input System

```cpp
class input_system {
    // Current state
    sdlpp::keyboard_state keyboard;
    std::unordered_set<sdlpp::scancode> just_pressed;
    std::unordered_set<sdlpp::scancode> just_released;
    
public:
    void update() {
        // Clear frame-specific sets
        just_pressed.clear();
        just_released.clear();
        
        // Update keyboard state
        keyboard = sdlpp::keyboard::get_state();
    }
    
    void handle_event(const sdlpp::keyboard_event& e) {
        if (e.is_pressed() && !e.repeat) {
            just_pressed.insert(e.scan);
        } else if (e.is_released()) {
            just_released.insert(e.scan);
        }
    }
    
    bool is_pressed(sdlpp::scancode key) const {
        return keyboard[key];
    }
    
    bool was_just_pressed(sdlpp::scancode key) const {
        return just_pressed.count(key) > 0;
    }
    
    bool was_just_released(sdlpp::scancode key) const {
        return just_released.count(key) > 0;
    }
    
    // Chord detection
    bool is_chord_pressed(std::initializer_list<sdlpp::scancode> keys) const {
        return std::all_of(keys.begin(), keys.end(),
            [this](auto key) { return is_pressed(key); });
    }
};
```