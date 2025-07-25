---
title: Gamepad Input
sidebar_label: Gamepad
---

# Gamepad Input

SDL++ provides a high-level gamepad API that works with modern game controllers, mapping their inputs to a standard layout.

## Gamepad vs Joystick

SDL provides two APIs for game controllers:
- **Gamepad API**: High-level API with standard button/axis mapping (recommended)
- **Joystick API**: Low-level API with direct hardware access

Use the Gamepad API unless you need to support non-standard controllers.

## Opening Gamepads

```cpp
#include <sdlpp/input/gamepad.hh>

// List available gamepads
int num_joysticks = sdlpp::joystick::count();
for (int i = 0; i < num_joysticks; ++i) {
    if (sdlpp::gamepad::is_gamepad(i)) {
        std::cout << "Gamepad " << i << ": " 
                  << sdlpp::gamepad::name_for_index(i) << std::endl;
    }
}

// Open first gamepad
auto gamepad_result = sdlpp::gamepad::open(0);
if (!gamepad_result) {
    std::cerr << "Failed to open gamepad: " << gamepad_result.error() << std::endl;
    return;
}

auto& gamepad = gamepad_result.value();
```

## Gamepad Information

```cpp
// Get gamepad details
std::string name = gamepad.get_name();
std::string serial = gamepad.get_serial();  // May be empty
sdlpp::joystick_type type = gamepad.get_type();

// Get associated joystick
sdlpp::joystick& joy = gamepad.get_joystick();
auto instance_id = joy.get_instance_id();

// Check if still connected
if (!gamepad.is_attached()) {
    std::cout << "Gamepad disconnected!" << std::endl;
}
```

## Button Input

```cpp
// Standard gamepad buttons
enum class gamepad_button : Sint8 {
    a = SDL_GAMEPAD_BUTTON_SOUTH,              // Cross on PS
    b = SDL_GAMEPAD_BUTTON_EAST,               // Circle on PS
    x = SDL_GAMEPAD_BUTTON_WEST,               // Square on PS
    y = SDL_GAMEPAD_BUTTON_NORTH,              // Triangle on PS
    back = SDL_GAMEPAD_BUTTON_BACK,            // Select
    guide = SDL_GAMEPAD_BUTTON_GUIDE,          // Home/PS/Xbox button
    start = SDL_GAMEPAD_BUTTON_START,
    left_stick = SDL_GAMEPAD_BUTTON_LEFT_STICK,   // L3
    right_stick = SDL_GAMEPAD_BUTTON_RIGHT_STICK, // R3
    left_shoulder = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,    // L1/LB
    right_shoulder = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,  // R1/RB
    dpad_up = SDL_GAMEPAD_BUTTON_DPAD_UP,
    dpad_down = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
    dpad_left = SDL_GAMEPAD_BUTTON_DPAD_LEFT,
    dpad_right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
    misc1 = SDL_GAMEPAD_BUTTON_MISC1,         // Share on PS5
    paddle1 = SDL_GAMEPAD_BUTTON_PADDLE1,     // Xbox Elite paddle
    paddle2 = SDL_GAMEPAD_BUTTON_PADDLE2,
    paddle3 = SDL_GAMEPAD_BUTTON_PADDLE3,
    paddle4 = SDL_GAMEPAD_BUTTON_PADDLE4,
    touchpad = SDL_GAMEPAD_BUTTON_TOUCHPAD     // PS4/PS5
};

// Check button state
if (gamepad.get_button(sdlpp::gamepad_button::a)) {
    player.jump();
}

// Check multiple buttons
bool sprint = gamepad.get_button(sdlpp::gamepad_button::left_shoulder);
bool crouch = gamepad.get_button(sdlpp::gamepad_button::right_shoulder);
```

## Analog Input

```cpp
// Analog axes
enum class gamepad_axis : Sint8 {
    left_x = SDL_GAMEPAD_AXIS_LEFTX,
    left_y = SDL_GAMEPAD_AXIS_LEFTY,
    right_x = SDL_GAMEPAD_AXIS_RIGHTX,
    right_y = SDL_GAMEPAD_AXIS_RIGHTY,
    left_trigger = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
    right_trigger = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER
};

// Get axis values (-32768 to 32767 for sticks, 0 to 32767 for triggers)
Sint16 left_x = gamepad.get_axis(sdlpp::gamepad_axis::left_x);
Sint16 left_y = gamepad.get_axis(sdlpp::gamepad_axis::left_y);

// Convert to normalized float
float norm_x = left_x / 32767.0f;
float norm_y = left_y / 32767.0f;

// Apply deadzone
const float DEADZONE = 0.15f;
if (std::abs(norm_x) < DEADZONE) norm_x = 0.0f;
if (std::abs(norm_y) < DEADZONE) norm_y = 0.0f;

// Get trigger values
Sint16 left_trigger = gamepad.get_axis(sdlpp::gamepad_axis::left_trigger);
float trigger_value = left_trigger / 32767.0f;  // 0.0 to 1.0
```

## Gamepad Events

```cpp
void handle_event(const sdlpp::event& e) {
    e.visit([](auto&& event) {
        using T = std::decay_t<decltype(event)>;
        
        if constexpr (std::is_same_v<T, sdlpp::gamepad_button_event>) {
            std::cout << "Gamepad " << event.which 
                     << " button " << static_cast<int>(event.button)
                     << (event.is_pressed() ? " pressed" : " released")
                     << std::endl;
                     
            if (event.button == sdlpp::gamepad_button::start && event.is_pressed()) {
                pause_game();
            }
        }
        else if constexpr (std::is_same_v<T, sdlpp::gamepad_axis_event>) {
            std::cout << "Gamepad " << event.which
                     << " axis " << static_cast<int>(event.axis)
                     << " = " << event.value << std::endl;
        }
        else if constexpr (std::is_same_v<T, sdlpp::gamepad_added_event>) {
            std::cout << "Gamepad connected: " << event.which << std::endl;
            open_gamepad(event.which);
        }
        else if constexpr (std::is_same_v<T, sdlpp::gamepad_removed_event>) {
            std::cout << "Gamepad disconnected: " << event.which << std::endl;
            close_gamepad(event.which);
        }
    });
}
```

## Rumble/Haptic Feedback

```cpp
// Simple rumble (0-65535 strength, duration in milliseconds)
auto result = gamepad.rumble(32768, 32768, 250);  // 50% strength, 250ms

// Trigger rumble (PS5, Xbox One)
gamepad.rumble_triggers(16384, 16384, 100);  // 25% strength, 100ms

// Stop rumble
gamepad.rumble(0, 0, 0);
```

## LED Control

```cpp
// Set LED color (PS4/PS5, some others)
gamepad.set_led(255, 0, 0);  // Red

// Check if LED control is supported
if (gamepad.has_led()) {
    // Pulse between colors
    animate_led(gamepad);
}
```

## Touchpad (PS4/PS5)

```cpp
// Check touchpad support
if (gamepad.get_touchpad_count() > 0) {
    // Get touchpad state
    auto finger_count = gamepad.get_touchpad_finger_count(0);
    
    for (int i = 0; i < finger_count; ++i) {
        Uint8 state;
        float x, y, pressure;
        
        if (gamepad.get_touchpad_finger(0, i, state, x, y, pressure)) {
            std::cout << "Finger " << i << ": "
                     << "(" << x << ", " << y << ") "
                     << "pressure: " << pressure << std::endl;
        }
    }
}
```

## Common Patterns

### Gamepad Manager

```cpp
class gamepad_manager {
    std::unordered_map<SDL_JoystickID, sdlpp::gamepad> gamepads;
    
public:
    void init() {
        // Open all connected gamepads
        int count = sdlpp::joystick::count();
        for (int i = 0; i < count; ++i) {
            open_gamepad(i);
        }
    }
    
    void open_gamepad(int device_index) {
        if (!sdlpp::gamepad::is_gamepad(device_index)) {
            return;  // Not a gamepad
        }
        
        auto gamepad_result = sdlpp::gamepad::open(device_index);
        if (gamepad_result) {
            auto id = gamepad_result->get_joystick().get_instance_id();
            gamepads[id] = std::move(gamepad_result.value());
            
            std::cout << "Opened gamepad: " 
                     << gamepads[id].get_name() << std::endl;
        }
    }
    
    void close_gamepad(SDL_JoystickID instance_id) {
        gamepads.erase(instance_id);
    }
    
    sdlpp::gamepad* get_gamepad(int player_index) {
        if (player_index >= gamepads.size()) return nullptr;
        
        auto it = gamepads.begin();
        std::advance(it, player_index);
        return &it->second;
    }
    
    void handle_event(const sdlpp::event& e) {
        e.visit([this](auto&& event) {
            using T = std::decay_t<decltype(event)>;
            
            if constexpr (std::is_same_v<T, sdlpp::gamepad_added_event>) {
                open_gamepad(event.which);
            }
            else if constexpr (std::is_same_v<T, sdlpp::gamepad_removed_event>) {
                close_gamepad(event.which);
            }
        });
    }
};
```

### Input Mapping

```cpp
class gamepad_input {
    struct stick_state {
        float x = 0.0f;
        float y = 0.0f;
        
        float magnitude() const {
            return std::sqrt(x * x + y * y);
        }
        
        void apply_deadzone(float deadzone) {
            float mag = magnitude();
            if (mag < deadzone) {
                x = y = 0.0f;
            } else {
                // Scale to maintain full range beyond deadzone
                float scale = (mag - deadzone) / (1.0f - deadzone);
                x = (x / mag) * scale;
                y = (y / mag) * scale;
            }
        }
    };
    
    sdlpp::gamepad* gamepad;
    float deadzone = 0.15f;
    float trigger_threshold = 0.1f;
    
public:
    stick_state get_left_stick() {
        if (!gamepad) return {};
        
        stick_state state;
        state.x = gamepad->get_axis(sdlpp::gamepad_axis::left_x) / 32767.0f;
        state.y = gamepad->get_axis(sdlpp::gamepad_axis::left_y) / 32767.0f;
        state.apply_deadzone(deadzone);
        
        return state;
    }
    
    stick_state get_right_stick() {
        if (!gamepad) return {};
        
        stick_state state;
        state.x = gamepad->get_axis(sdlpp::gamepad_axis::right_x) / 32767.0f;
        state.y = gamepad->get_axis(sdlpp::gamepad_axis::right_y) / 32767.0f;
        state.apply_deadzone(deadzone);
        
        return state;
    }
    
    float get_left_trigger() {
        if (!gamepad) return 0.0f;
        
        float value = gamepad->get_axis(sdlpp::gamepad_axis::left_trigger) / 32767.0f;
        return value > trigger_threshold ? value : 0.0f;
    }
    
    float get_right_trigger() {
        if (!gamepad) return 0.0f;
        
        float value = gamepad->get_axis(sdlpp::gamepad_axis::right_trigger) / 32767.0f;
        return value > trigger_threshold ? value : 0.0f;
    }
};
```

### Button Combos

```cpp
class combo_detector {
    struct combo {
        std::vector<sdlpp::gamepad_button> sequence;
        std::chrono::milliseconds max_time{500};
        std::function<void()> action;
    };
    
    std::vector<combo> combos;
    std::vector<sdlpp::gamepad_button> input_buffer;
    std::chrono::steady_clock::time_point last_input;
    
public:
    void register_combo(std::vector<sdlpp::gamepad_button> seq,
                       std::function<void()> action) {
        combos.push_back({seq, {}, action});
    }
    
    void handle_button(sdlpp::gamepad_button button) {
        auto now = std::chrono::steady_clock::now();
        
        // Clear buffer if too much time passed
        if (now - last_input > std::chrono::milliseconds(500)) {
            input_buffer.clear();
        }
        
        input_buffer.push_back(button);
        last_input = now;
        
        // Check for matching combos
        for (const auto& combo : combos) {
            if (check_combo(combo)) {
                combo.action();
                input_buffer.clear();
                break;
            }
        }
        
        // Limit buffer size
        if (input_buffer.size() > 10) {
            input_buffer.erase(input_buffer.begin());
        }
    }
    
private:
    bool check_combo(const combo& c) {
        if (input_buffer.size() < c.sequence.size()) return false;
        
        return std::equal(c.sequence.begin(), c.sequence.end(),
                         input_buffer.end() - c.sequence.size());
    }
};
```

## Best Practices

1. **Use Gamepad API**: For standard controllers, not joystick API
2. **Apply Deadzones**: Prevent drift from worn analog sticks
3. **Support Hot-Plugging**: Handle connect/disconnect events
4. **Normalize Inputs**: Convert to -1 to 1 or 0 to 1 ranges
5. **Test Multiple Controllers**: Xbox, PlayStation, Switch Pro have differences

## Example: Complete Gamepad System

```cpp
class player_controller {
    sdlpp::gamepad* gamepad = nullptr;
    
    // Input settings
    float move_deadzone = 0.2f;
    float aim_deadzone = 0.15f;
    float trigger_threshold = 0.1f;
    
    // Rumble state
    bool rumble_enabled = true;
    
public:
    void set_gamepad(sdlpp::gamepad* gp) {
        gamepad = gp;
    }
    
    void update(player& p, float dt) {
        if (!gamepad || !gamepad->is_attached()) return;
        
        // Movement (left stick)
        float move_x = gamepad->get_axis(sdlpp::gamepad_axis::left_x) / 32767.0f;
        float move_y = gamepad->get_axis(sdlpp::gamepad_axis::left_y) / 32767.0f;
        
        apply_deadzone(move_x, move_y, move_deadzone);
        p.move(move_x, move_y, dt);
        
        // Aiming (right stick)
        float aim_x = gamepad->get_axis(sdlpp::gamepad_axis::right_x) / 32767.0f;
        float aim_y = gamepad->get_axis(sdlpp::gamepad_axis::right_y) / 32767.0f;
        
        apply_deadzone(aim_x, aim_y, aim_deadzone);
        if (aim_x != 0 || aim_y != 0) {
            p.aim(std::atan2(aim_y, aim_x));
        }
        
        // Actions
        if (gamepad->get_button(sdlpp::gamepad_button::a)) {
            p.jump();
        }
        
        if (gamepad->get_button(sdlpp::gamepad_button::x)) {
            p.attack();
        }
        
        // Shooting (right trigger)
        float trigger = gamepad->get_axis(sdlpp::gamepad_axis::right_trigger) / 32767.0f;
        if (trigger > trigger_threshold) {
            p.shoot(trigger);  // Variable power
        }
        
        // Sprint (left shoulder)
        p.set_sprinting(gamepad->get_button(sdlpp::gamepad_button::left_shoulder));
    }
    
    void on_damage(float amount) {
        if (gamepad && rumble_enabled) {
            // Rumble based on damage
            Uint16 strength = static_cast<Uint16>(
                std::min(amount * 1000, 65535.0f)
            );
            gamepad->rumble(strength, strength, 200);
        }
    }
    
private:
    void apply_deadzone(float& x, float& y, float deadzone) {
        float magnitude = std::sqrt(x * x + y * y);
        
        if (magnitude < deadzone) {
            x = y = 0.0f;
        } else {
            // Rescale to maintain full range
            float scale = (magnitude - deadzone) / (1.0f - deadzone);
            x = (x / magnitude) * scale;
            y = (y / magnitude) * scale;
        }
    }
};
```