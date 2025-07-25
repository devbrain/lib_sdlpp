#include <sdlpp/core/core.hh>
#include <sdlpp/input/joystick.hh>
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
#include <sstream>
#include <algorithm>

using namespace sdlpp;

// Helper to draw a simple progress bar
void draw_bar(renderer& ren, int x, int y, int width, int height, float value, const color& bar_color) {
    // Draw background
    auto bg_result = ren.set_draw_color(color{50, 50, 50, 255});
    (void)bg_result;
    auto bg_rect_result = ren.fill_rect(rect_i{x, y, width, height});
    (void)bg_rect_result;
    
    // Draw filled portion
    if (value > 0.0f) {
        auto bar_result = ren.set_draw_color(bar_color);
        (void)bar_result;
        int filled_width = static_cast<int>(static_cast<float>(width) * value);
        auto fill_result = ren.fill_rect(rect_i{x, y, filled_width, height});
        (void)fill_result;
    }
    
    // Draw border
    auto border_result = ren.set_draw_color(color{255, 255, 255, 255});
    (void)border_result;
    auto rect_result = ren.draw_rect(rect_i{x, y, width, height});
    (void)rect_result;
}

// Helper to draw a direction indicator (for hats)
void draw_hat(renderer& ren, int cx, int cy, hat_position pos) {
    const int size = 20;
    
    // Draw center
    auto center_result = ren.set_draw_color(color{100, 100, 100, 255});
    (void)center_result;
    auto center_rect = ren.fill_rect(rect_i{cx - 2, cy - 2, 4, 4});
    (void)center_rect;
    
    // Draw direction indicator
    auto dir_result = ren.set_draw_color(color{255, 255, 0, 255});
    (void)dir_result;
    
    int dx = 0, dy = 0;
    
    if (static_cast<uint8_t>(pos) & static_cast<uint8_t>(hat_position::up)) dy = -1;
    if (static_cast<uint8_t>(pos) & static_cast<uint8_t>(hat_position::down)) dy = 1;
    if (static_cast<uint8_t>(pos) & static_cast<uint8_t>(hat_position::left)) dx = -1;
    if (static_cast<uint8_t>(pos) & static_cast<uint8_t>(hat_position::right)) dx = 1;
    
    if (dx != 0 || dy != 0) {
        auto line_result = ren.draw_line(point{cx, cy}, point{cx + dx * size, cy + dy * size});
        (void)line_result;
    }
}

// Structure to hold joystick info and state
struct joystick_info {
    joystick joy;
    std::string name;
    joystick_id id;
    int player_index;
    std::vector<int16_t> axes;
    std::vector<bool> buttons;
    std::vector<hat_position> hats;
    std::vector<std::pair<int, int>> balls;
    bool rumbling = false;
    Uint64 rumble_end_time = 0;
};

int main() {
    // Initialize SDL
    auto init = sdlpp::init(init_flags::video | init_flags::events | init_flags::joystick);
    if (!init) {
        logger::error(log_category::application, std::source_location::current(), "Failed to initialize SDL");
        return 1;
    }
    
    // Create window
    auto window_result = window::create("Joystick Example - Connect controllers to test!", 1024, 768);
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
    
    // Print joystick information
    std::cout << "\n=== SDL++ Joystick Example ===\n\n";
    std::cout << "Instructions:\n";
    std::cout << "- Connect/disconnect joysticks to see them appear/disappear\n";
    std::cout << "- Move axes and press buttons to see state\n";
    std::cout << "- Press number key (1-9) to rumble that joystick\n";
    std::cout << "- Press Shift + number (1-9) to toggle LED\n";
    std::cout << "- Press 'V' to create a virtual joystick\n";
    std::cout << "- Press ESC to quit\n\n";
    
    // Map to store opened joysticks
    std::map<joystick_id, std::unique_ptr<joystick_info>> joysticks;
    joystick_id virtual_joystick_id = 0;
    
    // Function to open a joystick
    auto open_joystick = [&](joystick_id id) {
        auto joy_result = joystick::open(id);
        if (joy_result) {
            auto info = std::make_unique<joystick_info>();
            info->joy = std::move(joy_result.value());
            info->id = id;
            info->name = info->joy.get_name();
            info->player_index = info->joy.get_player_index();
            
            // Initialize state vectors
            info->axes.resize(info->joy.get_num_axes(), 0);
            info->buttons.resize(info->joy.get_num_buttons(), false);
            info->hats.resize(info->joy.get_num_hats(), hat_position::centered);
            info->balls.resize(info->joy.get_num_balls(), std::make_pair(0, 0));
            
            std::cout << "Opened joystick: " << info->name 
                      << " (ID: " << id 
                      << ", Player: " << (info->player_index >= 0 ? std::to_string(info->player_index) : "none")
                      << ")\n";
            std::cout << "  Type: ";
            switch (info->joy.get_type()) {
                case joystick_type::gamepad: std::cout << "Gamepad"; break;
                case joystick_type::wheel: std::cout << "Wheel"; break;
                case joystick_type::arcade_stick: std::cout << "Arcade Stick"; break;
                case joystick_type::flight_stick: std::cout << "Flight Stick"; break;
                case joystick_type::dance_pad: std::cout << "Dance Pad"; break;
                case joystick_type::guitar: std::cout << "Guitar"; break;
                case joystick_type::drum_kit: std::cout << "Drum Kit"; break;
                case joystick_type::arcade_pad: std::cout << "Arcade Pad"; break;
                case joystick_type::throttle: std::cout << "Throttle"; break;
                default: std::cout << "Unknown"; break;
            }
            std::cout << "\n";
            std::cout << "  Axes: " << info->axes.size() 
                      << ", Buttons: " << info->buttons.size()
                      << ", Hats: " << info->hats.size()
                      << ", Balls: " << info->balls.size() << "\n";
            
            joysticks[id] = std::move(info);
        } else {
            std::cerr << "Failed to open joystick " << id << ": " << joy_result.error() << "\n";
        }
    };
    
    // Open all initially connected joysticks
    auto initial_joysticks = get_joysticks();
    for (auto id : initial_joysticks) {
        open_joystick(id);
    }
    
    // Event loop
    event_queue events;
    bool running = true;
    frame_limiter limiter(60.0);
    
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
                    if (e.is_pressed() && !e.repeat) {
                        if (e.scan == scancode::escape) {
                            running = false;
                        }
                        else if (e.scan == scancode::v) {
                            // Create virtual joystick
                            virtual_joystick_desc desc{};
                            desc.vendor_id = 0xDEAD;
                            desc.product_id = 0xBEEF;
                            desc.naxes = 2;
                            desc.nbuttons = 4;
                            desc.nhats = 1;
                            desc.name = "SDL++ Virtual Joystick";
                            
                            auto result = attach_virtual_joystick(desc);
                            if (result) {
                                virtual_joystick_id = result.value();
                                std::cout << "Created virtual joystick with ID " << virtual_joystick_id << "\n";
                            } else {
                                std::cerr << "Failed to create virtual joystick: " << result.error() << "\n";
                            }
                        }
                        else if (e.scan >= scancode::num_1 && e.scan <= scancode::num_9) {
                            // Rumble or LED control
                            int index = static_cast<int>(e.scan) - static_cast<int>(scancode::num_1);
                            auto it = joysticks.begin();
                            std::advance(it, std::min(index, static_cast<int>(joysticks.size()) - 1));
                            
                            if (it != joysticks.end()) {
                                auto& info = it->second;
                                
                                if (static_cast<Uint16>(e.mod & keymod::shift) != 0) {
                                    // Toggle LED
                                    static bool led_on = false;
                                    led_on = !led_on;
                                    auto led_result = info->joy.set_led(
                                        led_on ? 255 : 0,
                                        led_on ? 0 : 0,
                                        led_on ? 0 : 0
                                    );
                                    if (led_result) {
                                        std::cout << "LED " << (led_on ? "ON" : "OFF") 
                                                  << " for " << info->name << "\n";
                                    }
                                } else {
                                    // Rumble
                                    auto rumble_result = info->joy.rumble(32000, 32000, 500);
                                    if (rumble_result) {
                                        info->rumbling = true;
                                        info->rumble_end_time = static_cast<Uint64>(sdlpp::timer::elapsed().count() + 500);
                                        std::cout << "Rumbling " << info->name << "\n";
                                    }
                                }
                            }
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, joystick_device_event>) {
                    if (e.is_added()) {
                        std::cout << "Joystick connected: ID " << e.which << "\n";
                        open_joystick(e.which);
                    } else if (e.is_removed()) {
                        std::cout << "Joystick disconnected: ID " << e.which << "\n";
                        joysticks.erase(e.which);
                    }
                }
                else if constexpr (std::is_same_v<T, joystick_axis_event>) {
                    auto it = joysticks.find(e.which);
                    if (it != joysticks.end() && e.axis < it->second->axes.size()) {
                        it->second->axes[e.axis] = e.value;
                    }
                }
                else if constexpr (std::is_same_v<T, joystick_button_event>) {
                    auto it = joysticks.find(e.which);
                    if (it != joysticks.end() && e.button < it->second->buttons.size()) {
                        it->second->buttons[e.button] = e.is_pressed();
                        if (e.is_pressed()) {
                            std::cout << "Button " << static_cast<int>(e.button) 
                                      << " pressed on " << it->second->name << "\n";
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, joystick_hat_event>) {
                    auto it = joysticks.find(e.which);
                    if (it != joysticks.end() && e.hat < it->second->hats.size()) {
                        it->second->hats[e.hat] = static_cast<hat_position>(e.value);
                    }
                }
                else if constexpr (std::is_same_v<T, joystick_ball_event>) {
                    auto it = joysticks.find(e.which);
                    if (it != joysticks.end() && e.ball < it->second->balls.size()) {
                        it->second->balls[e.ball].first += e.xrel;
                        it->second->balls[e.ball].second += e.yrel;
                    }
                }
            });
        }
        
        // Check rumble timeouts
        auto current_time = static_cast<Uint64>(sdlpp::timer::elapsed().count());
        for (auto& [id, info] : joysticks) {
            if (info->rumbling && current_time >= info->rumble_end_time) {
                info->rumbling = false;
            }
        }
        
        // Draw joystick states
        int y_offset = 20;
        const int joystick_height = 150;
        
        for (const auto& [id, info] : joysticks) {
            // Draw joystick name and info
            auto text_color = ren.set_draw_color(color{255, 255, 255, 255});
            (void)text_color;
            
            // Draw a box for this joystick
            auto box_color = ren.set_draw_color(color{60, 60, 60, 255});
            (void)box_color;
            auto box_result = ren.fill_rect(rect_i{10, y_offset - 5, 1004, joystick_height});
            (void)box_result;
            
            // Draw axes
            int x_offset = 20;
            for (size_t i = 0; i < info->axes.size(); ++i) {
                float normalized = (info->axes[i] + 32768.0f) / 65535.0f;
                draw_bar(ren, x_offset, y_offset + 30, 60, 20, normalized, color{100, 200, 100, 255});
                x_offset += 70;
            }
            
            // Draw buttons
            x_offset = 20;
            int button_y = y_offset + 60;
            for (size_t i = 0; i < info->buttons.size(); ++i) {
                auto btn_color = info->buttons[i] ? color{255, 100, 100, 255} : color{100, 100, 100, 255};
                auto btn_result = ren.set_draw_color(btn_color);
                (void)btn_result;
                auto btn_rect = ren.fill_rect(rect_i{x_offset, button_y, 20, 20});
                (void)btn_rect;
                
                x_offset += 25;
                if ((i + 1) % 16 == 0) {
                    x_offset = 20;
                    button_y += 25;
                }
            }
            
            // Draw hats
            x_offset = 600;
            for (size_t i = 0; i < info->hats.size(); ++i) {
                draw_hat(ren, x_offset + 30, y_offset + 50, info->hats[i]);
                x_offset += 70;
            }
            
            // Draw connection state and power level
            std::stringstream status;
            status << info->name << " (ID: " << id << ")";
            
            auto conn_state = info->joy.get_connection_state();
            switch (conn_state) {
                case joystick_connection_state::wired:
                    status << " [WIRED]";
                    break;
                case joystick_connection_state::wireless:
                    status << " [WIRELESS]";
                    break;
                default:
                    break;
            }
            
            int battery_percent = -1;
            auto power = info->joy.get_power_info(&battery_percent);
            switch (power) {
                case power_state::on_battery:
                    status << " [BATTERY";
                    if (battery_percent >= 0) {
                        status << ": " << battery_percent << "%";
                    }
                    status << "]";
                    break;
                case power_state::no_battery:
                    status << " [NO BATTERY]";
                    break;
                case power_state::charging:
                    status << " [CHARGING";
                    if (battery_percent >= 0) {
                        status << ": " << battery_percent << "%";
                    }
                    status << "]";
                    break;
                case power_state::charged:
                    status << " [CHARGED]";
                    break;
                default:
                    break;
            }
            
            if (info->rumbling) {
                status << " [RUMBLING]";
            }
            
            // We can't actually draw text without a font system, so just use the status for console
            if (y_offset == 20) { // Only print status for first joystick to avoid spam
                std::cout << "\r" << status.str() << "          " << std::flush;
            }
            
            y_offset += joystick_height + 10;
        }
        
        // Present
        auto present_result = ren.present();
        (void)present_result;
        
        // Frame limiting
        limiter.wait_for_next_frame();
    }
    
    // Clean up virtual joystick if created
    if (virtual_joystick_id != 0) {
        auto detach_result = detach_virtual_joystick(virtual_joystick_id);
        if (!detach_result) {
            std::cerr << "Failed to detach virtual joystick: " << detach_result.error() << "\n";
        }
    }
    
    std::cout << "\n\nGoodbye!\n";
    return 0;
}