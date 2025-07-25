#include <sdlpp/core/core.hh>
#include <sdlpp/input/gamepad.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <../include/sdlpp/core/timer.hh>
#include <../include/sdlpp/core/log.hh>
#include <sdlpp/utility/geometry.hh>

#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <map>
#include <cmath>
#include <algorithm>

using namespace sdlpp;

// Helper to draw a stick position
void draw_stick(renderer& ren, int cx, int cy, int radius, float x, float y) {
    // Draw stick boundary
    auto circle_color = ren.set_draw_color(color{100, 100, 100, 255});
    (void)circle_color;
    
    // Simple circle approximation
    const int segments = 32;
    for (int i = 0; i < segments; ++i) {
        float angle1 = (static_cast<float>(i) * 2.0f * 3.14159f) / static_cast<float>(segments);
        float angle2 = (static_cast<float>(i + 1) * 2.0f * 3.14159f) / static_cast<float>(segments);
        
        int x1 = cx + static_cast<int>(static_cast<float>(radius) * std::cos(angle1));
        int y1 = cy + static_cast<int>(static_cast<float>(radius) * std::sin(angle1));
        int x2 = cx + static_cast<int>(static_cast<float>(radius) * std::cos(angle2));
        int y2 = cy + static_cast<int>(static_cast<float>(radius) * std::sin(angle2));
        
        auto line_result = ren.draw_line(point_i{x1, y1}, point_i{x2, y2});
        (void)line_result;
    }
    
    // Draw crosshair
    auto cross_result1 = ren.draw_line(point_i{cx - radius, cy}, point_i{cx + radius, cy});
    (void)cross_result1;
    auto cross_result2 = ren.draw_line(point_i{cx, cy - radius}, point_i{cx, cy + radius});
    (void)cross_result2;
    
    // Draw stick position
    int stick_x = cx + static_cast<int>((x / 32767.0f) * static_cast<float>(radius));
    int stick_y = cy + static_cast<int>((y / 32767.0f) * static_cast<float>(radius));
    
    auto stick_color = ren.set_draw_color(color{255, 255, 0, 255});
    (void)stick_color;
    
    // Draw filled circle for stick position
    const int stick_radius = 8;
    for (int dy = -stick_radius; dy <= stick_radius; dy++) {
        for (int dx = -stick_radius; dx <= stick_radius; dx++) {
            if (dx*dx + dy*dy <= stick_radius*stick_radius) {
                auto point_result = ren.draw_point(static_cast<float>(stick_x + dx), 
                                                  static_cast<float>(stick_y + dy));
                (void)point_result;
            }
        }
    }
}

// Helper to draw a trigger
void draw_trigger(renderer& ren, int x, int y, int width, int height, float value) {
    // Draw background
    auto bg_color = ren.set_draw_color(color{50, 50, 50, 255});
    (void)bg_color;
    auto bg_result = ren.fill_rect(rect_i{x, y, width, height});
    (void)bg_result;
    
    // Draw filled portion
    if (value > 0.0f) {
        auto fill_color = ren.set_draw_color(color{200, 100, 100, 255});
        (void)fill_color;
        int filled_height = static_cast<int>((value / 32767.0f) * static_cast<float>(height));
        auto fill_result = ren.fill_rect(rect_i{x, y + height - filled_height, width, filled_height});
        (void)fill_result;
    }
    
    // Draw border
    auto border_color = ren.set_draw_color(color{255, 255, 255, 255});
    (void)border_color;
    auto border_result = ren.draw_rect(rect_i{x, y, width, height});
    (void)border_result;
}

// Helper to draw a button
void draw_button(renderer& ren, int x, int y, bool pressed, [[maybe_unused]] const std::string& label = "") {
    const int size = 40;
    
    auto button_color = pressed ? color{255, 100, 100, 255} : color{100, 100, 100, 255};
    auto color_result = ren.set_draw_color(button_color);
    (void)color_result;
    auto fill_result = ren.fill_rect(rect_i{x, y, size, size});
    (void)fill_result;
    
    auto border_color = ren.set_draw_color(color{255, 255, 255, 255});
    (void)border_color;
    auto border_result = ren.draw_rect(rect_i{x, y, size, size});
    (void)border_result;
}

// Structure to hold gamepad info
struct gamepad_info {
    gamepad pad;
    std::string name;
    gamepad_type type;
    int player_index;
    bool rumbling = false;
    Uint64 rumble_end_time = 0;
    
    // Sensor data
    bool has_accel = false;
    bool has_gyro = false;
    std::vector<float> accel_data{0, 0, 0};
    std::vector<float> gyro_data{0, 0, 0};
};

int main() {
    // Initialize SDL
    auto init = sdlpp::init(init_flags::video | init_flags::events | init_flags::joystick | init_flags::gamepad);
    if (!init) {
        logger::error(log_category::application, std::source_location::current(), "Failed to initialize SDL");
        return 1;
    }
    
    // Create window
    auto window_result = window::create("Gamepad Example - Connect controllers to test!", 1200, 800);
    if (!window_result) {
        logger::error(log_category::application, std::source_location::current(), "Failed to create window: ", window_result.error());
        return 1;
    }
    auto& win = window_result.value();
    
    // Create renderer
    auto renderer_result = renderer::create(win);
    if (!renderer_result) {
        logger::error(log_category::application, std::source_location::current(), "Failed to create renderer: ", renderer_result.error());
        return 1;
    }
    auto& ren = renderer_result.value();
    
    // Print gamepad information
    std::cout << "\n=== SDL++ Gamepad Example ===\n\n";
    std::cout << "Instructions:\n";
    std::cout << "- Connect/disconnect gamepads to see them appear/disappear\n";
    std::cout << "- Use sticks, triggers, and buttons to see state\n";
    std::cout << "- Press A/X (South) to rumble\n";
    std::cout << "- Press B/Circle (East) to rumble triggers\n";
    std::cout << "- Press X/Square (West) to cycle LED colors\n";
    std::cout << "- Press Y/Triangle (North) to toggle sensors\n";
    std::cout << "- Press ESC to quit\n\n";
    
    // Map to store opened gamepads
    std::map<joystick_id, std::unique_ptr<gamepad_info>> gamepads;
    
    // Function to open a gamepad
    auto open_gamepad = [&](joystick_id id) {
        auto pad_result = gamepad::open(id);
        if (pad_result) {
            auto info = std::make_unique<gamepad_info>();
            info->pad = std::move(pad_result.value());
            info->name = info->pad.get_name();
            info->type = info->pad.get_type();
            info->player_index = info->pad.get_player_index();
            
            std::cout << "Opened gamepad: " << info->name 
                      << " (ID: " << id 
                      << ", Player: " << (info->player_index >= 0 ? std::to_string(info->player_index) : "none")
                      << ")\n";
            std::cout << "  Type: " << get_gamepad_type_string(info->type) << "\n";
            std::cout << "  Vendor: 0x" << std::hex << info->pad.get_vendor() 
                      << " Product: 0x" << info->pad.get_product() << std::dec << "\n";
            
            // Check for sensors
            info->has_accel = info->pad.has_sensor(gamepad_sensor_type::accel);
            info->has_gyro = info->pad.has_sensor(gamepad_sensor_type::gyro);
            
            if (info->has_accel || info->has_gyro) {
                std::cout << "  Sensors:";
                if (info->has_accel) std::cout << " Accelerometer";
                if (info->has_gyro) std::cout << " Gyroscope";
                std::cout << "\n";
            }
            
            // Check for touchpads
            int num_touchpads = static_cast<int>(info->pad.get_num_touchpads());
            if (num_touchpads > 0) {
                std::cout << "  Touchpads: " << num_touchpads << "\n";
            }
            
            gamepads[id] = std::move(info);
        } else {
            std::cerr << "Failed to open gamepad " << id << ": " << pad_result.error() << "\n";
        }
    };
    
    // Open all initially connected gamepads
    auto initial_gamepads = get_gamepads();
    std::cout << "Found " << initial_gamepads.size() << " gamepad(s)\n";
    for (auto id : initial_gamepads) {
        open_gamepad(id);
    }
    
    // Event loop
    event_queue events;
    bool running = true;
    frame_limiter limiter(60.0);
    
    // LED color cycle
    const std::vector<std::tuple<uint8_t, uint8_t, uint8_t>> led_colors = {
        {255, 0, 0},     // Red
        {0, 255, 0},     // Green
        {0, 0, 255},     // Blue
        {255, 255, 0},   // Yellow
        {255, 0, 255},   // Magenta
        {0, 255, 255},   // Cyan
        {255, 255, 255}, // White
        {0, 0, 0}        // Off
    };
    size_t led_color_index = 0;
    
    while (running) {
        // Clear screen
        auto clear_color = ren.set_draw_color(color{30, 30, 40, 255});
        (void)clear_color;
        auto clear_result = ren.clear();
        (void)clear_result;
        
        // Process events
        while (auto event = events.poll()) {
            event->visit([&](auto&& e) {
                using T = std::decay_t<decltype(e)>;
                
                if constexpr (std::is_same_v<T, quit_event>) {
                    running = false;
                }
                else if constexpr (std::is_same_v<T, keyboard_event>) {
                    if (e.is_pressed() && !e.repeat && e.scan == scancode::escape) {
                        running = false;
                    }
                }
                else if constexpr (std::is_same_v<T, gamepad_device_event>) {
                    if (e.is_added()) {
                        std::cout << "Gamepad connected: ID " << e.which << "\n";
                        open_gamepad(e.which);
                    } else if (e.is_removed()) {
                        std::cout << "Gamepad disconnected: ID " << e.which << "\n";
                        gamepads.erase(e.which);
                    } else if (e.is_remapped()) {
                        std::cout << "Gamepad remapped: ID " << e.which << "\n";
                    }
                }
                else if constexpr (std::is_same_v<T, gamepad_button_event>) {
                    auto it = gamepads.find(e.which);
                    if (it != gamepads.end() && e.is_pressed()) {
                        auto& info = it->second;
                        
                        std::string button_name = get_gamepad_button_name(static_cast<gamepad_button>(e.button));
                        std::cout << "Button pressed: " << button_name << " on " << info->name << "\n";
                        
                        // Handle button actions
                        if (e.button == static_cast<Uint8>(gamepad_button::south)) {
                            // Rumble
                            auto rumble_result = info->pad.rumble(32000, 32000, 500);
                            if (rumble_result) {
                                info->rumbling = true;
                                info->rumble_end_time = static_cast<Uint64>(sdlpp::timer::elapsed().count() + 500);
                            }
                        }
                        else if (e.button == static_cast<Uint8>(gamepad_button::east)) {
                            // Rumble triggers
                            auto rumble_result = info->pad.rumble_triggers(32000, 32000, 500);
                            if (!rumble_result) {
                                std::cout << "Trigger rumble not supported\n";
                            }
                        }
                        else if (e.button == static_cast<Uint8>(gamepad_button::west)) {
                            // Cycle LED color
                            auto [r, g, b] = led_colors[led_color_index];
                            auto led_result = info->pad.set_led(r, g, b);
                            if (led_result) {
                                std::cout << "LED color changed\n";
                            } else {
                                std::cout << "LED control not supported\n";
                            }
                            led_color_index = (led_color_index + 1) % led_colors.size();
                        }
                        else if (e.button == static_cast<Uint8>(gamepad_button::north)) {
                            // Toggle sensors
                            if (info->has_accel) {
                                bool enabled = info->pad.is_sensor_enabled(gamepad_sensor_type::accel);
                                auto result = info->pad.set_sensor_enabled(gamepad_sensor_type::accel, !enabled);
                                if (result) {
                                    std::cout << "Accelerometer " << (!enabled ? "enabled" : "disabled") << "\n";
                                }
                            }
                            if (info->has_gyro) {
                                bool enabled = info->pad.is_sensor_enabled(gamepad_sensor_type::gyro);
                                auto result = info->pad.set_sensor_enabled(gamepad_sensor_type::gyro, !enabled);
                                if (result) {
                                    std::cout << "Gyroscope " << (!enabled ? "enabled" : "disabled") << "\n";
                                }
                            }
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, gamepad_sensor_event>) {
                    auto it = gamepads.find(e.which);
                    if (it != gamepads.end()) {
                        auto& info = it->second;
                        
                        if (e.sensor == sensor_type::accel) {
                            info->accel_data = {e.data[0], e.data[1], e.data[2]};
                        } else if (e.sensor == sensor_type::gyro) {
                            info->gyro_data = {e.data[0], e.data[1], e.data[2]};
                        }
                    }
                }
            });
        }
        
        // Check rumble timeouts
        auto current_time = static_cast<Uint64>(sdlpp::timer::elapsed().count());
        for (auto& [id, info] : gamepads) {
            if (info->rumbling && current_time >= info->rumble_end_time) {
                info->rumbling = false;
            }
        }
        
        // Draw gamepad states
        int y_offset = 20;
        const int gamepad_height = 300;
        
        for (const auto& [id, info] : gamepads) {
            gamepad_state state(info->pad);
            
            // Draw gamepad background
            auto bg_color = ren.set_draw_color(color{60, 60, 60, 255});
            (void)bg_color;
            auto bg_result = ren.fill_rect(rect_i{10, y_offset - 5, 1180, gamepad_height});
            (void)bg_result;
            
            // Draw gamepad name and info
            // (We can't draw text without a font system, so we'll just use shapes)
            
            // Draw sticks
            draw_stick(ren, 100, y_offset + 100, 60, state.left_x(), state.left_y());
            draw_stick(ren, 300, y_offset + 100, 60, state.right_x(), state.right_y());
            
            // Draw triggers
            draw_trigger(ren, 450, y_offset + 50, 40, 100, state.left_trigger());
            draw_trigger(ren, 500, y_offset + 50, 40, 100, state.right_trigger());
            
            // Draw face buttons
            draw_button(ren, 650, y_offset + 100, state.a(), "A");  // South
            draw_button(ren, 700, y_offset + 50, state.b(), "B");   // East
            draw_button(ren, 600, y_offset + 50, state.x(), "X");   // West
            draw_button(ren, 650, y_offset + 0, state.y(), "Y");    // North
            
            // Draw D-pad
            draw_button(ren, 200, y_offset + 200, state.dpad_up());
            draw_button(ren, 200, y_offset + 250, state.dpad_down());
            draw_button(ren, 150, y_offset + 225, state.dpad_left());
            draw_button(ren, 250, y_offset + 225, state.dpad_right());
            
            // Draw shoulder buttons
            draw_button(ren, 450, y_offset + 10, state.left_shoulder(), "L1");
            draw_button(ren, 500, y_offset + 10, state.right_shoulder(), "R1");
            
            // Draw stick buttons
            draw_button(ren, 100 - 20, y_offset + 170, state.left_stick(), "L3");
            draw_button(ren, 300 - 20, y_offset + 170, state.right_stick(), "R3");
            
            // Draw menu buttons
            draw_button(ren, 350, y_offset + 200, state.back(), "Back");
            draw_button(ren, 400, y_offset + 200, state.start(), "Start");
            draw_button(ren, 375, y_offset + 250, state.guide(), "Guide");
            
            // Draw touchpad button if available
            if (info->pad.has_button(gamepad_button::touchpad)) {
                draw_button(ren, 800, y_offset + 100, state.touchpad(), "Touch");
            }
            
            // Draw sensor data if available
            if (info->pad.is_sensor_enabled(gamepad_sensor_type::accel)) {
                auto accel_result = info->pad.get_sensor_data(gamepad_sensor_type::accel, info->accel_data);
                (void)accel_result;
                // Draw accelerometer visualization
                int accel_x = 900;
                int accel_y = y_offset + 50;
                
                auto accel_color = ren.set_draw_color(color{100, 200, 100, 255});
                (void)accel_color;
                
                // Draw X, Y, Z bars
                for (int i = 0; i < 3; ++i) {
                    int bar_height = static_cast<int>(std::abs(info->accel_data[static_cast<size_t>(i)]) * 10);
                    bar_height = std::min(bar_height, 50);
                    auto bar_result = ren.fill_rect(rect_i{accel_x + i * 30, accel_y - bar_height/2, 20, bar_height});
                    (void)bar_result;
                }
            }
            
            if (info->pad.is_sensor_enabled(gamepad_sensor_type::gyro)) {
                auto gyro_result = info->pad.get_sensor_data(gamepad_sensor_type::gyro, info->gyro_data);
                (void)gyro_result;
                // Draw gyroscope visualization
                int gyro_x = 900;
                int gyro_y = y_offset + 150;
                
                auto gyro_color = ren.set_draw_color(color{200, 100, 100, 255});
                (void)gyro_color;
                
                // Draw X, Y, Z bars
                for (int i = 0; i < 3; ++i) {
                    int bar_height = static_cast<int>(std::abs(info->gyro_data[static_cast<size_t>(i)]) * 0.1f);
                    bar_height = std::min(bar_height, 50);
                    auto bar_result = ren.fill_rect(rect_i{gyro_x + i * 30, gyro_y - bar_height/2, 20, bar_height});
                    (void)bar_result;
                }
            }
            
            // Draw connection and power status indicators
            int status_x = 1050;
            auto conn_state = info->pad.get_connection_state();
            if (conn_state == joystick_connection_state::wireless) {
                auto wireless_color = ren.set_draw_color(color{100, 100, 255, 255});
                (void)wireless_color;
                auto wireless_result = ren.fill_rect(rect_i{status_x, y_offset + 10, 20, 20});
                (void)wireless_result;
            }
            
            int battery_percent = -1;
            [[maybe_unused]] auto power = info->pad.get_power_info(&battery_percent);
            if (battery_percent >= 0) {
                // Draw battery indicator
                auto battery_color = battery_percent > 20 ? color{100, 255, 100, 255} : color{255, 100, 100, 255};
                auto bat_color_result = ren.set_draw_color(battery_color);
                (void)bat_color_result;
                int battery_width = (battery_percent * 40) / 100;
                auto battery_result = ren.fill_rect(rect_i{status_x, y_offset + 40, battery_width, 10});
                (void)battery_result;
            }
            
            if (info->rumbling) {
                auto rumble_color = ren.set_draw_color(color{255, 255, 0, 255});
                (void)rumble_color;
                auto rumble_result = ren.fill_rect(rect_i{status_x, y_offset + 60, 40, 10});
                (void)rumble_result;
            }
            
            y_offset += gamepad_height + 10;
        }
        
        // Present
        auto present_result = ren.present();
        (void)present_result;
        
        // Frame limiting
        limiter.wait_for_next_frame();
    }
    
    std::cout << "\n\nGoodbye!\n";
    return 0;
}