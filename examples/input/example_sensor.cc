#include <sdlpp/core/core.hh>
#include <sdlpp/input/sensor.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/core/timer.hh>
#include <sdlpp/core/log.hh>
#include <sdlpp/utility/geometry.hh>

#include <iostream>
#include <iomanip>
#include <vector>
#include <deque>
#include <cmath>
#include <algorithm>

using namespace sdlpp;

// Data visualization
struct sensor_visualization {
    std::deque<float> x_history;
    std::deque<float> y_history;
    std::deque<float> z_history;
    size_t max_history = 200;
    
    void add_sample(float x, float y, float z) {
        x_history.push_back(x);
        y_history.push_back(y);
        z_history.push_back(z);
        
        while (x_history.size() > max_history) {
            x_history.pop_front();
            y_history.pop_front();
            z_history.pop_front();
        }
    }
    
    void clear() {
        x_history.clear();
        y_history.clear();
        z_history.clear();
    }
};

// Helper to draw graph
void draw_graph(renderer& ren, const std::deque<float>& data, 
                int x, int y, int width, int height, 
                float min_val, float max_val, const color& col) {
    if (data.empty()) return;
    
    // Draw background
    auto bg_result = ren.set_draw_color(color{40, 40, 40, 255});
    (void)bg_result;
    auto bg_rect = ren.fill_rect(rect_i{x, y, width, height});
    (void)bg_rect;
    
    // Draw axes
    auto axis_result = ren.set_draw_color(color{100, 100, 100, 255});
    (void)axis_result;
    
    // X axis
    auto x_axis = ren.draw_line(point_i{x, y + height/2}, 
                               point_i{x + width, y + height/2});
    (void)x_axis;
    
    // Y axis
    auto y_axis = ren.draw_line(point_i{x, y}, point_i{x, y + height});
    (void)y_axis;
    
    // Draw data
    auto data_result = ren.set_draw_color(col);
    (void)data_result;
    
    float x_step = static_cast<float>(width) / static_cast<float>(data.size() - 1);
    
    for (size_t i = 1; i < data.size(); ++i) {
        float v1 = data[i-1];
        float v2 = data[i];
        
        // Normalize to [0, 1]
        float n1 = (v1 - min_val) / (max_val - min_val);
        float n2 = (v2 - min_val) / (max_val - min_val);
        
        // Clamp
        n1 = std::max(0.0f, std::min(1.0f, n1));
        n2 = std::max(0.0f, std::min(1.0f, n2));
        
        // Convert to screen coordinates
        int x1 = x + static_cast<int>(static_cast<float>(i-1) * x_step);
        int y1 = y + height - static_cast<int>(n1 * static_cast<float>(height));
        int x2 = x + static_cast<int>(static_cast<float>(i) * x_step);
        int y2 = y + height - static_cast<int>(n2 * static_cast<float>(height));
        
        auto line_result = ren.draw_line(point_i{x1, y1}, point_i{x2, y2});
        (void)line_result;
    }
}

// Helper to visualize tilt
void draw_tilt_indicator(renderer& ren, float pitch, float roll, 
                        int cx, int cy, int radius) {
    // Draw circle
    auto circle_result = ren.set_draw_color(color{100, 100, 100, 255});
    (void)circle_result;
    
    const int segments = 32;
    for (int i = 0; i < segments; ++i) {
        float a1 = (static_cast<float>(i) * 2.0f * 3.14159f) / static_cast<float>(segments);
        float a2 = (static_cast<float>(i + 1) * 2.0f * 3.14159f) / static_cast<float>(segments);
        
        int x1 = cx + static_cast<int>(static_cast<float>(radius) * std::cos(a1));
        int y1 = cy + static_cast<int>(static_cast<float>(radius) * std::sin(a1));
        int x2 = cx + static_cast<int>(static_cast<float>(radius) * std::cos(a2));
        int y2 = cy + static_cast<int>(static_cast<float>(radius) * std::sin(a2));
        
        auto line_result = ren.draw_line(point_i{x1, y1}, point_i{x2, y2});
        (void)line_result;
    }
    
    // Draw crosshair
    auto cross_result = ren.draw_line(point_i{cx - radius, cy}, 
                                     point_i{cx + radius, cy});
    (void)cross_result;
    cross_result = ren.draw_line(point_i{cx, cy - radius}, 
                                point_i{cx, cy + radius});
    (void)cross_result;
    
    // Draw tilt indicator
    // Limit angles to reasonable range
    float max_angle = 45.0f * 3.14159f / 180.0f;
    pitch = std::max(-max_angle, std::min(max_angle, pitch));
    roll = std::max(-max_angle, std::min(max_angle, roll));
    
    // Calculate position
    int dx = static_cast<int>((roll / max_angle) * static_cast<float>(radius) * 0.8f);
    int dy = static_cast<int>((pitch / max_angle) * static_cast<float>(radius) * 0.8f);
    
    // Draw indicator
    auto indicator_result = ren.set_draw_color(color{255, 0, 0, 255});
    (void)indicator_result;
    
    // Draw filled circle
    const int indicator_size = 5;
    for (int y = -indicator_size; y <= indicator_size; ++y) {
        for (int x = -indicator_size; x <= indicator_size; ++x) {
            if (x*x + y*y <= indicator_size*indicator_size) {
                auto pt_result = ren.draw_point(static_cast<float>(cx + dx + x), 
                                               static_cast<float>(cy + dy + y));
                (void)pt_result;
            }
        }
    }
}

int main() {
    // Initialize SDL with sensor support
    auto init = sdlpp::init(init_flags::video | init_flags::events | init_flags::sensor);
    if (!init) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to initialize SDL");
        return 1;
    }
    
    // Create window
    auto window_result = window::create("Sensor Example - Accelerometer & Gyroscope", 
                                       1024, 768);
    if (!window_result) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to create window: ", window_result.error());
        return 1;
    }
    auto& win = window_result.value();
    
    // Create renderer
    auto renderer_result = renderer::create(win);
    if (!renderer_result) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to create renderer: ", renderer_result.error());
        return 1;
    }
    auto& ren = renderer_result.value();
    
    // Print sensor information
    std::cout << "\n=== SDL++ Sensor Example ===\n\n";
    
    // Enumerate sensors
    auto sensors = get_sensors();
    std::cout << "Found " << sensors.size() << " sensor(s):\n\n";
    
    for (auto id : sensors) {
        std::cout << "Sensor ID " << id << ":\n";
        std::cout << "  Name: " << get_sensor_name_for_id(id) << "\n";
        
        auto type = get_sensor_type_for_id(id);
        std::cout << "  Type: ";
        switch (type) {
            case sensor_type::accel:
                std::cout << "Accelerometer";
                break;
            case sensor_type::gyro:
                std::cout << "Gyroscope";
                break;
            case sensor_type::accel_l:
                std::cout << "Left Accelerometer";
                break;
            case sensor_type::gyro_l:
                std::cout << "Left Gyroscope";
                break;
            case sensor_type::accel_r:
                std::cout << "Right Accelerometer";
                break;
            case sensor_type::gyro_r:
                std::cout << "Right Gyroscope";
                break;
            case sensor_type::unknown:
                std::cout << "Unknown";
                break;
            default:
                std::cout << "Invalid";
                break;
        }
        std::cout << "\n";
        
        auto non_portable = get_sensor_non_portable_type_for_id(id);
        if (non_portable != -1) {
            std::cout << "  Non-portable type: " << non_portable << "\n";
        }
        std::cout << "\n";
    }
    
    // Open sensors using manager
    sensor_manager manager;
    int opened = static_cast<int>(manager.open_all());
    std::cout << "Opened " << opened << " sensor(s)\n\n";
    
    if (opened == 0) {
        std::cout << "No sensors available. The example will display a demo UI.\n";
        std::cout << "To test with real sensors:\n";
        std::cout << "- On mobile devices, sensors should be available by default\n";
        std::cout << "- On desktop, some game controllers have accelerometer/gyroscope\n";
    }
    
    std::cout << "\nControls:\n";
    std::cout << "- Press 'R' to reset graphs\n";
    std::cout << "- Press '1' to toggle accelerometer display\n";
    std::cout << "- Press '2' to toggle gyroscope display\n";
    std::cout << "- Press ESC to quit\n\n";
    
    // Find sensors
    sensor* accel = manager.find_by_type(sensor_type::accel);
    sensor* gyro = manager.find_by_type(sensor_type::gyro);
    
    // Visualization data
    sensor_visualization accel_viz;
    sensor_visualization gyro_viz;
    
    // Display settings
    bool show_accel = true;
    bool show_gyro = true;
    
    // Event loop
    event_queue events;
    bool running = true;
    frame_limiter limiter(60.0);
    
    while (running) {
        // Clear screen
        auto clear_color = ren.set_draw_color(color{30, 30, 30, 255});
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
                        else if (e.scan == scancode::r) {
                            accel_viz.clear();
                            gyro_viz.clear();
                            std::cout << "Graphs reset\n";
                        }
                        else if (e.scan == scancode::num_1) {
                            show_accel = !show_accel;
                            std::cout << "Accelerometer display: " 
                                     << (show_accel ? "ON" : "OFF") << "\n";
                        }
                        else if (e.scan == scancode::num_2) {
                            show_gyro = !show_gyro;
                            std::cout << "Gyroscope display: " 
                                     << (show_gyro ? "ON" : "OFF") << "\n";
                        }
                    }
                }
                else if constexpr (std::is_same_v<T, sensor_event>) {
                    // Handle sensor events if we want real-time updates
                    std::cout << "Sensor event from sensor " << e.which << "\n";
                }
            });
        }
        
        // Update sensor data
        update_sensors();
        
        // Read and visualize accelerometer data
        if (accel && show_accel) {
            auto data = accel->get_data_3();
            if (data) {
                accelerometer_data accel_data(data.value());
                accel_viz.add_sample(accel_data.x(), accel_data.y(), accel_data.z());
                
                // Draw title
                auto title_color = ren.set_draw_color(color{255, 255, 255, 255});
                (void)title_color;
                
                // Draw graphs
                draw_graph(ren, accel_viz.x_history, 50, 50, 400, 150, 
                          -20.0f, 20.0f, color{255, 0, 0, 255});
                draw_graph(ren, accel_viz.y_history, 50, 220, 400, 150, 
                          -20.0f, 20.0f, color{0, 255, 0, 255});
                draw_graph(ren, accel_viz.z_history, 50, 390, 400, 150, 
                          -20.0f, 20.0f, color{0, 0, 255, 255});
                
                // Draw labels
                auto label_bg = ren.set_draw_color(color{0, 0, 0, 200});
                (void)label_bg;
                auto x_label_bg = ren.fill_rect(rect_i{50, 30, 100, 20});
                (void)x_label_bg;
                auto y_label_bg = ren.fill_rect(rect_i{50, 200, 100, 20});
                (void)y_label_bg;
                auto z_label_bg = ren.fill_rect(rect_i{50, 370, 100, 20});
                (void)z_label_bg;
                
                // Draw current values
                auto value_bg = ren.fill_rect(rect_i{470, 50, 200, 150});
                (void)value_bg;
                
                // Draw magnitude indicator
                [[maybe_unused]] float mag = accel_data.magnitude();
                [[maybe_unused]] bool at_rest = accel_data.is_at_rest(1.0f);
                
                auto mag_bg = ren.fill_rect(rect_i{470, 220, 200, 80});
                (void)mag_bg;
                
                // Draw device orientation visualization
                // Use accelerometer data to estimate tilt
                float pitch = std::atan2(accel_data.x(), 
                                        std::sqrt(accel_data.y() * accel_data.y() + 
                                                 accel_data.z() * accel_data.z()));
                float roll = std::atan2(accel_data.y(), accel_data.z());
                
                draw_tilt_indicator(ren, pitch, roll, 570, 420, 80);
            }
        }
        
        // Read and visualize gyroscope data  
        if (gyro && show_gyro) {
            auto data = gyro->get_data_3();
            if (data) {
                gyroscope_data gyro_data(data.value());
                gyro_viz.add_sample(gyro_data.pitch(), gyro_data.yaw(), gyro_data.roll());
                
                // Draw graphs (offset to the right)
                draw_graph(ren, gyro_viz.x_history, 550, 50, 400, 150, 
                          -3.0f, 3.0f, color{255, 128, 0, 255});
                draw_graph(ren, gyro_viz.y_history, 550, 220, 400, 150, 
                          -3.0f, 3.0f, color{128, 255, 0, 255});
                draw_graph(ren, gyro_viz.z_history, 550, 390, 400, 150, 
                          -3.0f, 3.0f, color{128, 0, 255, 255});
                
                // Draw current rotation status
                [[maybe_unused]] bool stationary = gyro_data.is_stationary(0.1f);
                auto status_bg = ren.set_draw_color(color{0, 0, 0, 200});
                (void)status_bg;
                auto status_rect = ren.fill_rect(rect_i{700, 560, 200, 40});
                (void)status_rect;
            }
        }
        
        // Draw info panel
        auto info_bg = ren.set_draw_color(color{0, 0, 0, 200});
        (void)info_bg;
        auto info_rect = ren.fill_rect(rect_i{10, 680, 1004, 78});
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