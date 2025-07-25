#include <sdlpp/core/core.hh>
#include <sdlpp/input/pen.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <../include/sdlpp/core/timer.hh>
#include <../include/sdlpp/core/log.hh>

#include <iostream>
#include <iomanip>
#include <vector>
#include <deque>
#include <cmath>
#include <algorithm>

using namespace sdlpp;

// Stroke point
struct stroke_point {
    float x, y;
    float pressure;
    bool eraser;
    color point_color;
};

// Drawing stroke
struct stroke {
    std::vector<stroke_point> points;
    float max_width = 20.0f;
};

// Helper to interpolate between colors
color lerp_color(const color& a, const color& b, float t) {
    return color{
        static_cast<uint8_t>(a.r + (b.r - a.r) * t),
        static_cast<uint8_t>(a.g + (b.g - a.g) * t),
        static_cast<uint8_t>(a.b + (b.b - a.b) * t),
        static_cast<uint8_t>(a.a + (b.a - a.a) * t)
    };
}

// Helper to draw a line with pressure
void draw_pressure_line(renderer& ren, float x1, float y1, float x2, float y2, 
                       float pressure1, float pressure2, float max_width, [[maybe_unused]] const color& col) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distance = std::sqrt(dx*dx + dy*dy);
    
    if (distance < 0.1f) return;
    
    // Number of segments based on distance
    int segments = static_cast<int>(distance / 2.0f) + 1;
    
    for (int i = 0; i < segments; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(segments);
        float x = x1 + dx * t;
        float y = y1 + dy * t;
        float pressure = pressure1 + (pressure2 - pressure1) * t;
        float width = pressure * max_width;
        
        // Draw circle for this segment
        int w = static_cast<int>(width);
        for (int dy_offset = -w; dy_offset <= w; dy_offset++) {
            for (int dx_offset = -w; dx_offset <= w; dx_offset++) {
                if (dx_offset*dx_offset + dy_offset*dy_offset <= w*w) {
                    auto result = ren.draw_point(x + static_cast<float>(dx_offset), y + static_cast<float>(dy_offset));
                    (void)result;
                }
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
    auto window_result = window::create("Pen Example - Draw with your stylus!", 1024, 768);
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
    
    // Set blend mode for drawing
    auto blend_result = ren.set_draw_blend_mode(blend_mode::blend);
    (void)blend_result;
    
    // Print pen information
    std::cout << "\n=== SDL++ Pen Example ===\n\n";
    std::cout << "Instructions:\n";
    std::cout << "- Draw with your stylus/pen\n";
    std::cout << "- Pressure affects line width\n";
    std::cout << "- Tilt affects color (if supported)\n";
    std::cout << "- Use eraser to erase (if supported)\n";
    std::cout << "- Press 'C' to clear canvas\n";
    std::cout << "- Press '1-5' to select colors\n";
    std::cout << "- Press ESC to quit\n\n";
    
    std::cout << "Note: Pen information is available through events.\n";
    std::cout << "Connect a graphics tablet or stylus to start drawing.\n";
    
    // Drawing state
    std::vector<stroke> strokes;
    stroke* current_stroke = nullptr;
    stroke_point last_point;
    bool pen_was_down = false;
    
    // Color palette
    std::vector<color> palette = {
        {0, 0, 0, 255},        // Black
        {255, 0, 0, 255},      // Red
        {0, 255, 0, 255},      // Green
        {0, 0, 255, 255},      // Blue
        {255, 255, 0, 255}     // Yellow
    };
    size_t current_color_index = 0;
    
    // Canvas background
    color canvas_color{255, 255, 255, 255};
    
    // Info display
    struct pen_info {
        float x = 0.0f, y = 0.0f;
        float pressure = 0.0f;
        float xtilt = 0.0f, ytilt = 0.0f;
        bool eraser = false;
        bool down = false;
    } last_pen_info;
    bool show_info = true;
    
    // Event loop
    event_queue events;
    bool running = true;
    frame_limiter limiter(60.0);
    
    while (running) {
        // Clear screen with canvas color
        auto clear_color = ren.set_draw_color(canvas_color);
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
                            // Clear canvas
                            strokes.clear();
                            current_stroke = nullptr;
                            std::cout << "Canvas cleared\n";
                        }
                        else if (e.scan >= scancode::num_1 && e.scan <= scancode::num_5) {
                            // Select color
                            int index = static_cast<int>(e.scan) - static_cast<int>(scancode::num_1);
                            if (index >= 0 && index < static_cast<int>(palette.size())) {
                                current_color_index = static_cast<size_t>(index);
                                std::cout << "Color " << (index + 1) << " selected\n";
                            }
                        }
                        else if (e.scan == scancode::i) {
                            show_info = !show_info;
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, pen_proximity_event>) {
                    if (e.is_in()) {
                        std::cout << "Pen " << e.which << " entered proximity\n";
                    } else {
                        std::cout << "Pen " << e.which << " left proximity\n";
                        current_stroke = nullptr;
                    }
                }
                else if constexpr (std::is_same_v<T, pen_touch_event>) {
                    // Update pen info
                    last_pen_info.x = e.x;
                    last_pen_info.y = e.y;
                    last_pen_info.eraser = e.eraser;
                    last_pen_info.down = e.down;
                    
                    if (e.is_down()) {
                        // Start new stroke
                        strokes.emplace_back();
                        current_stroke = &strokes.back();
                        
                        stroke_point pt;
                        pt.x = e.x;
                        pt.y = e.y;
                        pt.pressure = last_pen_info.pressure; // Use last known pressure
                        pt.eraser = e.eraser;
                        pt.point_color = e.eraser ? canvas_color : palette[current_color_index];
                        
                        current_stroke->points.push_back(pt);
                        last_point = pt;
                        pen_was_down = true;
                    } else {
                        // End stroke
                        current_stroke = nullptr;
                        pen_was_down = false;
                    }
                }
                else if constexpr (std::is_same_v<T, pen_motion_event>) {
                    // Update pen position
                    last_pen_info.x = e.x;
                    last_pen_info.y = e.y;
                    last_pen_info.eraser = has_flag(static_cast<pen_input_flags>(e.pen_state), pen_input_flags::eraser_tip);
                    last_pen_info.down = has_flag(static_cast<pen_input_flags>(e.pen_state), pen_input_flags::down);
                    
                    if (current_stroke && pen_was_down) {
                        stroke_point pt;
                        pt.x = e.x;
                        pt.y = e.y;
                        pt.pressure = last_pen_info.pressure;
                        pt.eraser = last_pen_info.eraser;
                        
                        // Use tilt to modify color if available
                        color base_color = pt.eraser ? canvas_color : palette[current_color_index];
                        if (!pt.eraser && last_pen_info.xtilt != 0.0f) {
                            float tilt_factor = std::abs(last_pen_info.xtilt) / 90.0f;
                            pt.point_color = lerp_color(base_color, color{128, 128, 128, 255}, tilt_factor * 0.5f);
                        } else {
                            pt.point_color = base_color;
                        }
                        
                        current_stroke->points.push_back(pt);
                        last_point = pt;
                    }
                }
                else if constexpr (std::is_same_v<T, pen_axis_event>) {
                    // Update axis values
                    switch (static_cast<pen_axis>(e.axis)) {
                        case pen_axis::pressure:
                            last_pen_info.pressure = e.value;
                            break;
                        case pen_axis::xtilt:
                            last_pen_info.xtilt = e.value;
                            break;
                        case pen_axis::ytilt:
                            last_pen_info.ytilt = e.value;
                            break;
                        default:
                            break;
                    }
                }
                else if constexpr (std::is_same_v<T, pen_button_event>) {
                    std::cout << "Pen button " << static_cast<int>(e.button) 
                              << (e.is_pressed() ? " pressed" : " released") << "\n";
                }
            });
        }
        
        // Draw all strokes
        for (const auto& stroke : strokes) {
            if (stroke.points.size() < 2) continue;
            
            for (size_t i = 1; i < stroke.points.size(); ++i) {
                const auto& p1 = stroke.points[i-1];
                const auto& p2 = stroke.points[i];
                
                // Set color
                auto color_result = ren.set_draw_color(p2.point_color);
                (void)color_result;
                
                // Draw pressure-sensitive line
                draw_pressure_line(ren, p1.x, p1.y, p2.x, p2.y, 
                                  p1.pressure, p2.pressure, stroke.max_width, p2.point_color);
            }
        }
        
        // Draw info overlay
        if (show_info) {
            // Info background
            auto info_bg = ren.set_draw_color(color{0, 0, 0, 180});
            (void)info_bg;
            auto info_rect = ren.fill_rect(rect{10, 10, 250, 150});
            (void)info_rect;
            
            // Draw pressure bar
            auto pressure_bg = ren.set_draw_color(color{100, 100, 100, 255});
            (void)pressure_bg;
            auto pressure_bg_rect = ren.fill_rect(rect{20, 50, 200, 20});
            (void)pressure_bg_rect;
            
            auto pressure_fg = ren.set_draw_color(color{255, 255, 0, 255});
            (void)pressure_fg;
            int pressure_width = static_cast<int>(last_pen_info.pressure * 200);
            auto pressure_rect = ren.fill_rect(rect{20, 50, pressure_width, 20});
            (void)pressure_rect;
            
            // Draw tilt indicator
            // Tilt visualization center
            int tilt_cx = 120;
            int tilt_cy = 110;
            int tilt_radius = 30;
            
            // Draw circle
            auto circle_color = ren.set_draw_color(color{100, 100, 100, 255});
            (void)circle_color;
            for (int i = 0; i < 360; i += 10) {
                float angle = static_cast<float>(i) * 3.14159f / 180.0f;
                int x = tilt_cx + static_cast<int>(static_cast<float>(tilt_radius) * std::cos(angle));
                int y = tilt_cy + static_cast<int>(static_cast<float>(tilt_radius) * std::sin(angle));
                auto pt_result = ren.draw_point(static_cast<float>(x), static_cast<float>(y));
                (void)pt_result;
            }
            
            // Draw tilt indicator
            float xtilt = last_pen_info.xtilt;
            float ytilt = last_pen_info.ytilt;
            int tx = tilt_cx + static_cast<int>((xtilt / 90.0f) * static_cast<float>(tilt_radius));
            int ty = tilt_cy + static_cast<int>((ytilt / 90.0f) * static_cast<float>(tilt_radius));
            
            auto tilt_color = ren.set_draw_color(color{255, 0, 0, 255});
            (void)tilt_color;
            auto line_result = ren.draw_line(point{tilt_cx, tilt_cy}, point{tx, ty});
            (void)line_result;
            
            // Draw tilt position
            for (int dy = -2; dy <= 2; dy++) {
                for (int dx = -2; dx <= 2; dx++) {
                    auto pt_result = ren.draw_point(static_cast<float>(tx + dx), 
                                                   static_cast<float>(ty + dy));
                    (void)pt_result;
                }
            }
            
            // Draw eraser indicator
            if (last_pen_info.eraser) {
                auto eraser_color = ren.set_draw_color(color{255, 0, 0, 255});
                (void)eraser_color;
                auto eraser_rect = ren.fill_rect(rect{200, 20, 40, 20});
                (void)eraser_rect;
            }
        }
        
        // Draw color palette
        for (size_t i = 0; i < palette.size(); ++i) {
            int x = 10 + static_cast<int>(i * 40);
            int y = 720;
            
            // Draw color swatch
            auto swatch_color = ren.set_draw_color(palette[i]);
            (void)swatch_color;
            auto swatch_rect = ren.fill_rect(rect{x, y, 30, 30});
            (void)swatch_rect;
            
            // Draw selection indicator
            if (i == current_color_index) {
                auto select_color = ren.set_draw_color(color{255, 255, 255, 255});
                (void)select_color;
                auto select_rect = ren.draw_rect(rect{x-2, y-2, 34, 34});
                (void)select_rect;
            }
        }
        
        // Present
        auto present_result = ren.present();
        (void)present_result;
        
        // Frame limiting
        limiter.wait_for_next_frame();
    }
    
    std::cout << "\nGoodbye!\n";
    return 0;
}