//
// Example: Basic SDL++ Application
//

#include <sdlpp/app/app.hh>
#include <sdlpp/app/app_impl.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/utility/geometry.hh>
#include <iostream>

using namespace sdlpp;

// Simple application that draws a moving rectangle
class basic_app : public application {
    float rect_x_ = 100.0f;
    float rect_y_ = 100.0f;
    float rect_speed_ = 200.0f; // pixels per second
    float rect_dir_x_ = 1.0f;
    float rect_dir_y_ = 1.0f;
    
protected:
    bool on_init() override {
        std::cout << "Application initialized!\n";
        std::cout << "Press ESC to quit\n";
        std::cout << "Press SPACE to pause/resume\n";
        return true;
    }
    
    void on_frame() override {
        // Update rectangle position
        float dt = get_delta_time();
        rect_x_ += rect_speed_ * rect_dir_x_ * dt;
        rect_y_ += rect_speed_ * rect_dir_y_ * dt;
        
        // Bounce off walls
        auto window_size = get_window().get_size();
        if (window_size) {
            if (rect_x_ < 0 || rect_x_ > static_cast<float>(window_size->width - 50)) {
                rect_dir_x_ = -rect_dir_x_;
                rect_x_ = std::clamp(rect_x_, 0.0f, static_cast<float>(window_size->width - 50));
            }
            if (rect_y_ < 0 || rect_y_ > static_cast<float>(window_size->height - 50)) {
                rect_dir_y_ = -rect_dir_y_;
                rect_y_ = std::clamp(rect_y_, 0.0f, static_cast<float>(window_size->height - 50));
            }
        }
        
        // Render
        auto& renderer = get_renderer();
        
        // Clear screen
        if (auto res = renderer.set_draw_color(colors::dark_gray); !res) {
            on_error("Failed to set draw color: " + res.error());
        }
        if (auto res = renderer.clear(); !res) {
            on_error("Failed to clear: " + res.error());
        }
        
        // Draw rectangle
        if (auto res = renderer.set_draw_color(colors::cyan); !res) {
            on_error("Failed to set draw color: " + res.error());
        }
        if (auto res = renderer.fill_rect(rect_i{
            static_cast<int>(rect_x_), 
            static_cast<int>(rect_y_), 
            50, 50
        }); !res) {
            on_error("Failed to fill rect: " + res.error());
        }
        
        // Draw FPS counter
        if (auto res = renderer.set_draw_color(colors::white); !res) {
            on_error("Failed to set draw color: " + res.error());
        }
        // Note: In a real app, you'd use a text rendering library
        
        // Renderer::present() is called automatically by the base class
    }
    
    bool on_event(const sdlpp::event& e) override {
        // Handle keyboard events
        if (e.is<keyboard_event>()) {
            const auto* kb = e.as<keyboard_event>();
            if (!kb) return true;
            if (kb->type == event_type::key_down) {
                switch (kb->key) {
                    case keycodes::escape:
                        request_quit();
                        return false;
                        
                    case keycodes::space:
                        // Toggle pause
                        rect_speed_ = (rect_speed_ == 0) ? 200.0f : 0.0f;
                        break;
                        
                    default:
                        break;
                }
            }
        }
        
        return true;
    }
    
    void on_quit() override {
        std::cout << "Application shutting down...\n";
        std::cout << "Average FPS: " << get_fps() << "\n";
    }
};

// Entry point
SDLPP_MAIN(basic_app)