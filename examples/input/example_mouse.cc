#include <sdlpp/core/core.hh>
#include <../include/sdlpp/input/mouse.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <../include/sdlpp/core/timer.hh>
#include <../include/sdlpp/core/log.hh>
#include <sdlpp/utility/geometry.hh>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <memory>

using namespace sdlpp;

// Helper to draw a circle (for visualizing mouse position)
void draw_circle(renderer& ren, int cx, int cy, int radius) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x*x + y*y <= radius*radius) {
                auto result = ren.draw_point(static_cast<float>(cx + x), static_cast<float>(cy + y));
                // Ignore result for performance
                (void)result;
            }
        }
    }
}

int main() {
    // Initialize SDL
    auto init = sdlpp::init(init_flags::video | init_flags::events);
    if (!init) {
        logger::error(log_category::application, std::source_location::current(), "Failed to initialize SDL");
        return 1;
    }
    
    // Create window
    auto window_result = window::create("Mouse Example - Try all mouse features!", 800, 600);
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
    
    // Print mouse information
    std::cout << "\n=== SDL++ Mouse Example ===\n\n";
    std::cout << "Mouse Information:\n";
    std::cout << "- Has mouse: " << (has_mouse() ? "Yes" : "No") << "\n";
    
    auto mice = get_mice();
    std::cout << "- Number of mice: " << mice.size() << "\n";
    for (size_t i = 0; i < mice.size(); ++i) {
        auto name = get_mouse_name(mice[i]);
        std::cout << "  Mouse " << i << ": " << (name.empty() ? "(unnamed)" : name) 
                  << " (ID: " << mice[i] << ")\n";
    }
    
    // Check for special mouse IDs
    for (const auto& id : mice) {
        if (id == mouse_special_id::touch) {
            std::cout << "  - Touch device detected as mouse\n";
        }
        if (id == mouse_special_id::pen) {
            std::cout << "  - Pen device detected as mouse\n";
        }
    }
    
    std::cout << "\n";
    
    // Create custom cursors
    auto arrow_cursor = cursor::create_system(system_cursor::default_cursor);
    auto hand_cursor = cursor::create_system(system_cursor::pointer);
    auto crosshair_cursor = cursor::create_system(system_cursor::crosshair);
    auto wait_cursor = cursor::create_system(system_cursor::wait);
    
    if (!arrow_cursor || !hand_cursor || !crosshair_cursor || !wait_cursor) {
        std::cerr << "Failed to create system cursors\n";
    }
    
    // Instructions
    std::cout << "Instructions:\n";
    std::cout << "- Move mouse to see position and trail\n";
    std::cout << "- Click buttons to see button state\n";
    std::cout << "- Press 'R' to toggle relative mouse mode\n";
    std::cout << "- Press 'C' to cycle through cursors\n";
    std::cout << "- Press 'H' to hide/show cursor\n";
    std::cout << "- Press 'W' to warp mouse to center\n";
    std::cout << "- Press 'M' to toggle mouse capture\n";
    std::cout << "- Press SPACE to clear trail\n";
    std::cout << "- Press ESC to quit\n\n";
    
    // Event loop
    event_queue events;
    bool running = true;
    bool relative_mode = false;
    bool mouse_captured = false;
    bool cursor_hidden = false;
    int current_cursor = 0;
    
    // Mouse trail storage
    struct trail_point {
        int x, y;
        Uint64 time;
    };
    std::vector<trail_point> mouse_trail;
    const size_t max_trail_points = 100;
    
    // Frame limiter
    frame_limiter limiter(60.0);
    
    while (running) {
        // Clear screen
        auto color_result = ren.set_draw_color(color{20, 20, 30, 255});
        if (!color_result) {
            logger::error(log_category::application, std::source_location::current(), "Failed to set color: ", color_result.error());
        }
        auto clear_result = ren.clear();
        if (!clear_result) {
            logger::error(log_category::application, std::source_location::current(), "Failed to clear: ", clear_result.error());
        }
        
        // Get current mouse state
        mouse_state_helper mouse;
        
        // Add to trail
        Uint64 current_time = static_cast<Uint64>(sdlpp::timer::elapsed().count());
        mouse_trail.push_back({mouse.x(), mouse.y(), current_time});
        if (mouse_trail.size() > max_trail_points) {
            mouse_trail.erase(mouse_trail.begin());
        }
        
        // Draw mouse trail
        auto trail_color = ren.set_draw_color(color{100, 100, 255, 255});
        (void)trail_color;
        for (size_t i = 1; i < mouse_trail.size(); ++i) {
            auto& prev = mouse_trail[i-1];
            auto& curr = mouse_trail[i];
            
            // Fade older points
            Uint64 age = current_time - curr.time;
            if (age < 1000) { // Show trail for 1 second
                int alpha = static_cast<int>(255 * (1.0f - static_cast<float>(age) / 1000.0f));
                auto trail_fade_result = ren.set_draw_color(color{100, 100, 255, static_cast<Uint8>(alpha)});
                (void)trail_fade_result;
                auto line_result = ren.draw_line(point{prev.x, prev.y}, point{curr.x, curr.y});
                (void)line_result;
            }
        }
        
        // Draw mouse position
        auto white_result = ren.set_draw_color(color{255, 255, 255, 255});
        (void)white_result;
        draw_circle(ren, mouse.x(), mouse.y(), 5);
        
        // Draw button indicators
        int indicator_y = 50;
        auto indicator_color = ren.set_draw_color(color{255, 255, 255, 255});
        (void)indicator_color;
        
        if (mouse.is_left_pressed()) {
            auto red_result = ren.set_draw_color(color{255, 100, 100, 255});
            (void)red_result;
            auto fill_result = ren.fill_rect(rect_i{50, indicator_y, 100, 30});
            (void)fill_result;
        }
        auto white_result2 = ren.set_draw_color(color{255, 255, 255, 255});
        (void)white_result2;
        auto rect_result = ren.draw_rect(rect_i{50, indicator_y, 100, 30});
        (void)rect_result;
        
        if (mouse.is_middle_pressed()) {
            auto green_result = ren.set_draw_color(color{100, 255, 100, 255});
            (void)green_result;
            auto fill_result = ren.fill_rect(rect_i{160, indicator_y, 100, 30});
            (void)fill_result;
        }
        auto white_result3 = ren.set_draw_color(color{255, 255, 255, 255});
        (void)white_result3;
        auto rect_result2 = ren.draw_rect(rect_i{160, indicator_y, 100, 30});
        (void)rect_result2;
        
        if (mouse.is_right_pressed()) {
            auto blue_result = ren.set_draw_color(color{100, 100, 255, 255});
            (void)blue_result;
            auto fill_result = ren.fill_rect(rect_i{270, indicator_y, 100, 30});
            (void)fill_result;
        }
        auto white_result4 = ren.set_draw_color(color{255, 255, 255, 255});
        (void)white_result4;
        auto rect_result3 = ren.draw_rect(rect_i{270, indicator_y, 100, 30});
        (void)rect_result3;
        
        // Poll events
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
                        else if (e.scan == scancode::r) {
                            // Toggle relative mode
                            relative_mode = !relative_mode;
                            auto result = set_window_relative_mouse_mode(win, relative_mode);
                            if (result) {
                                std::cout << "Relative mouse mode: " << (relative_mode ? "ON" : "OFF") << "\n";
                            } else {
                                std::cout << "Failed to set relative mode: " << result.error() << "\n";
                                relative_mode = !relative_mode; // Revert
                            }
                        }
                        else if (e.scan == scancode::c) {
                            // Cycle cursors
                            current_cursor = (current_cursor + 1) % 4;
                            switch (current_cursor) {
                                case 0: 
                                    if (arrow_cursor) {
                                        auto result = arrow_cursor->set();
                                        if (!result) {
                                            std::cerr << "Failed to set cursor: " << result.error() << "\n";
                                        }
                                    }
                                    std::cout << "Cursor: Arrow\n";
                                    break;
                                case 1: 
                                    if (hand_cursor) {
                                        auto result = hand_cursor->set();
                                        if (!result) {
                                            std::cerr << "Failed to set cursor: " << result.error() << "\n";
                                        }
                                    }
                                    std::cout << "Cursor: Hand\n";
                                    break;
                                case 2: 
                                    if (crosshair_cursor) {
                                        auto result = crosshair_cursor->set();
                                        if (!result) {
                                            std::cerr << "Failed to set cursor: " << result.error() << "\n";
                                        }
                                    }
                                    std::cout << "Cursor: Crosshair\n";
                                    break;
                                case 3: 
                                    if (wait_cursor) {
                                        auto result = wait_cursor->set();
                                        if (!result) {
                                            std::cerr << "Failed to set cursor: " << result.error() << "\n";
                                        }
                                    }
                                    std::cout << "Cursor: Wait\n";
                                    break;
                            }
                        }
                        else if (e.scan == scancode::h) {
                            // Toggle cursor visibility
                            cursor_hidden = !cursor_hidden;
                            if (cursor_hidden) {
                                auto hide_result = hide_cursor();
                                if (!hide_result) {
                                    std::cerr << "Failed to hide cursor: " << hide_result.error() << "\n";
                                }
                                std::cout << "Cursor: HIDDEN\n";
                            } else {
                                auto show_result = show_cursor();
                                if (!show_result) {
                                    std::cerr << "Failed to show cursor: " << show_result.error() << "\n";
                                }
                                std::cout << "Cursor: VISIBLE\n";
                            }
                        }
                        else if (e.scan == scancode::w) {
                            // Warp to center
                            int center_x = 400;
                            int center_y = 300;
                            warp_mouse_in_window(win, center_x, center_y);
                            std::cout << "Warped mouse to center (400, 300)\n";
                        }
                        else if (e.scan == scancode::m) {
                            // Toggle mouse capture
                            mouse_captured = !mouse_captured;
                            auto result = capture_mouse(mouse_captured);
                            if (result) {
                                std::cout << "Mouse capture: " << (mouse_captured ? "ON" : "OFF") << "\n";
                            } else {
                                std::cout << "Failed to set mouse capture: " << result.error() << "\n";
                                mouse_captured = !mouse_captured; // Revert
                            }
                        }
                        else if (e.scan == scancode::space) {
                            // Clear trail
                            mouse_trail.clear();
                            std::cout << "Mouse trail cleared\n";
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, mouse_motion_event>) {
                    if (relative_mode) {
                        std::cout << "Relative motion: dx=" << e.xrel << " dy=" << e.yrel << "\n";
                    }
                }
                else if constexpr (std::is_same_v<T, mouse_button_event>) {
                    std::string button_name;
                    switch (static_cast<int>(e.button)) {
                        case static_cast<int>(mouse_button::left): button_name = "LEFT"; break;
                        case static_cast<int>(mouse_button::middle): button_name = "MIDDLE"; break;
                        case static_cast<int>(mouse_button::right): button_name = "RIGHT"; break;
                        case static_cast<int>(mouse_button::x1): button_name = "X1"; break;
                        case static_cast<int>(mouse_button::x2): button_name = "X2"; break;
                        default: button_name = "UNKNOWN"; break;
                    }
                    
                    std::cout << "Mouse " << button_name << " button " 
                              << (e.is_pressed() ? "DOWN" : "UP")
                              << " at (" << e.x << ", " << e.y << ")";
                    
                    if (e.clicks > 1) {
                        std::cout << " [" << static_cast<int>(e.clicks) << " clicks]";
                    }
                    
                    std::cout << "\n";
                }
                else if constexpr (std::is_same_v<T, mouse_wheel_event>) {
                    std::cout << "Mouse wheel: x=" << e.x << " y=" << e.y;
                    if (e.direction == mouse_wheel_direction::flipped) {
                        std::cout << " (flipped)";
                    }
                    std::cout << " at mouse (" << e.mouse_x << ", " << e.mouse_y << ")\n";
                }
            });
        }
        
        // Display current state
        static int frame_count = 0;
        if (++frame_count % 60 == 0) { // Every second
            std::stringstream ss;
            ss << "Mouse: (" << mouse.x() << ", " << mouse.y() << ")";
            
            if (mouse.any_button_pressed()) {
                ss << " Buttons:";
                if (mouse.is_left_pressed()) ss << " L";
                if (mouse.is_middle_pressed()) ss << " M";
                if (mouse.is_right_pressed()) ss << " R";
                if (mouse.is_x1_pressed()) ss << " X1";
                if (mouse.is_x2_pressed()) ss << " X2";
            }
            
            if (relative_mode) ss << " [REL]";
            if (mouse_captured) ss << " [CAP]";
            if (cursor_hidden) ss << " [HIDDEN]";
            
            std::cout << "\r" << ss.str() << "          " << std::flush;
        }
        
        // Present
        auto present_result = ren.present();
        if (!present_result) {
            logger::error(log_category::application, std::source_location::current(), "Failed to present: ", present_result.error());
        }
        
        // Frame limiting
        limiter.wait_for_next_frame();
    }
    
    std::cout << "\n\nGoodbye!\n";
    return 0;
}