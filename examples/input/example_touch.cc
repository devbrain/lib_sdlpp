#include <sdlpp/core/core.hh>
#include <sdlpp/core/timer.hh>
#include <sdlpp/input/touch.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <../include/sdlpp/core/timer.hh>
#include <../include/sdlpp/core/log.hh>
#include <sdlpp/utility/geometry.hh>

#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <cmath>

using namespace sdlpp;

// Structure to track finger trails
struct finger_trail {
    struct point {
        float x, y;
        Uint64 timestamp;
    };
    std::vector<point> points;
    color trail_color;
};

// Helper to generate a color from finger ID
color finger_id_to_color(finger_id id) {
    // Use finger ID to generate a unique color
    uint32_t hash = static_cast<uint32_t>(id);
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = ((hash >> 16) ^ hash) * 0x45d9f3b;
    hash = (hash >> 16) ^ hash;
    
    return color{
        static_cast<uint8_t>((hash & 0xFF) | 0x80),           // R (ensure bright)
        static_cast<uint8_t>(((hash >> 8) & 0xFF) | 0x80),    // G
        static_cast<uint8_t>(((hash >> 16) & 0xFF) | 0x80),   // B
        255                                                     // A
    };
}

// Helper to draw a filled circle
void draw_filled_circle(renderer& ren, float cx, float cy, float radius) {
    int r = static_cast<int>(radius);
    for (int dy = -r; dy <= r; dy++) {
        for (int dx = -r; dx <= r; dx++) {
            if (dx*dx + dy*dy <= r*r) {
                auto result = ren.draw_point(cx + static_cast<float>(dx), cy + static_cast<float>(dy));
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
    auto window_result = window::create("Touch Example - Touch the screen!", 1024, 768);
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
    
    // Print touch device information
    std::cout << "\n=== SDL++ Touch Example ===\n\n";
    std::cout << "Instructions:\n";
    std::cout << "- Touch the screen to draw trails\n";
    std::cout << "- Each finger gets a unique color\n";
    std::cout << "- Press 'C' to clear trails\n";
    std::cout << "- Press ESC to quit\n\n";
    
    // List touch devices
    auto devices = get_touch_devices();
    std::cout << "Found " << devices.size() << " touch device(s):\n";
    for (auto id : devices) {
        std::cout << "- Device " << id << ": " << get_touch_device_name(id);
        auto type = get_touch_device_type(id);
        switch (type) {
            case touch_device_type::direct:
                std::cout << " (Direct/Touchscreen)";
                break;
            case touch_device_type::indirect_absolute:
                std::cout << " (Indirect/Trackpad - Absolute)";
                break;
            case touch_device_type::indirect_relative:
                std::cout << " (Indirect/Trackpad - Relative)";
                break;
            default:
                std::cout << " (Unknown type)";
                break;
        }
        std::cout << "\n";
    }
    
    
    // Map to store finger trails
    std::map<finger_id, finger_trail> trails;
    
    // Window size for coordinate conversion
    int window_width = 0, window_height = 0;
    auto size_result = win.get_size();
    if (size_result) {
        window_width = size_result->width;
        window_height = size_result->height;
    }
    
    // Trail fade time (milliseconds)
    const Uint64 trail_fade_time = 2000;
    
    // Event loop
    event_queue events;
    bool running = true;
    frame_limiter limiter(60.0);
    
    while (running) {
        // Clear screen
        auto clear_color = ren.set_draw_color(color{20, 20, 30, 255});
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
                        else if (e.scan == scancode::c) {
                            // Clear trails
                            trails.clear();
                            std::cout << "Trails cleared\n";
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, touch_finger_event>) {
                    // Convert normalized coordinates to window coordinates
                    float wx = e.x * static_cast<float>(window_width);
                    float wy = e.y * static_cast<float>(window_height);
                    
                    if (e.is_down()) {
                        // New finger down - create new trail
                        finger_trail trail;
                        trail.trail_color = finger_id_to_color(e.fingerID);
                        trail.points.push_back({wx, wy, static_cast<Uint64>(sdlpp::timer::elapsed().count())});
                        trails[e.fingerID] = std::move(trail);
                        
                        std::cout << "Finger " << e.fingerID << " down at (" 
                                  << std::fixed << std::setprecision(2) 
                                  << wx << ", " << wy << ") pressure: " << e.pressure << "\n";
                    }
                    else if (e.is_motion()) {
                        // Finger moved - add to trail
                        auto it = trails.find(e.fingerID);
                        if (it != trails.end()) {
                            it->second.points.push_back({wx, wy, static_cast<Uint64>(sdlpp::timer::elapsed().count())});
                        }
                    }
                    else if (e.is_up()) {
                        // Finger up
                        std::cout << "Finger " << e.fingerID << " up\n";
                        // Keep the trail for fading
                    }
                }
                else if constexpr (std::is_same_v<T, window_event>) {
                    if (e.is_resized()) {
                        window_width = e.width();
                        window_height = e.height();
                    }
                }
            });
        }
        
        // Get current time for fade calculations
        Uint64 current_time = static_cast<Uint64>(sdlpp::timer::elapsed().count());
        
        // Draw trails
        for (auto it = trails.begin(); it != trails.end(); ) {
            const auto& trail = it->second;
            
            // Remove old trails
            if (!trail.points.empty() && 
                current_time - trail.points.back().timestamp > trail_fade_time) {
                it = trails.erase(it);
                continue;
            }
            
            // Draw trail points
            for (size_t i = 1; i < trail.points.size(); ++i) {
                const auto& p1 = trail.points[i-1];
                const auto& p2 = trail.points[i];
                
                // Calculate fade based on age
                Uint64 age = current_time - p2.timestamp;
                float fade = 1.0f - (static_cast<float>(age) / trail_fade_time);
                fade = std::max(0.0f, fade);
                
                // Set color with fade
                auto trail_color_faded = trail.trail_color;
                trail_color_faded.a = static_cast<uint8_t>(trail_color_faded.a * fade);
                auto color_result = ren.set_draw_color(trail_color_faded);
                (void)color_result;
                
                // Draw line segment
                auto line_result = ren.draw_line(
                    point_i{static_cast<int>(p1.x), static_cast<int>(p1.y)},
                    point_i{static_cast<int>(p2.x), static_cast<int>(p2.y)}
                );
                (void)line_result;
            }
            
            ++it;
        }
        
        // Draw current touch points
        for (auto id : devices) {
            touch_state state(id);
            auto fingers = state.get_fingers();
            
            for (const auto& finger : fingers) {
                float wx = finger.x * static_cast<float>(window_width);
                float wy = finger.y * static_cast<float>(window_height);
                
                // Draw touch point
                auto finger_color = finger_id_to_color(finger.id);
                auto color_result = ren.set_draw_color(finger_color);
                (void)color_result;
                
                // Draw circle with size based on pressure
                float radius = 10.0f + (finger.pressure * 20.0f);
                draw_filled_circle(ren, wx, wy, radius);
                
                // Draw white center dot
                auto white_result = ren.set_draw_color(color{255, 255, 255, 255});
                (void)white_result;
                draw_filled_circle(ren, wx, wy, 3.0f);
            }
        }
        
        // Draw info text area background
        auto info_bg = ren.set_draw_color(color{0, 0, 0, 180});
        (void)info_bg;
        auto info_rect = ren.fill_rect(rect_i{10, 10, 300, 100});
        (void)info_rect;
        
        // Present
        auto present_result = ren.present();
        (void)present_result;
        
        // Frame limiting
        limiter.wait_for_next_frame();
    }
    
    std::cout << "\nGoodbye!\n";
    return 0;
}