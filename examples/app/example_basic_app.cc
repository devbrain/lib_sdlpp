//
// Example: Basic SDL++ Application
//

#include <sdlpp/app/entry_point.hh>
#include <sdlpp/app/game_application.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/utility/geometry.hh>
#include <iostream>

using namespace sdlpp;

// Simple application that draws a moving rectangle
class basic_app : public game_application {
    float rect_x_ = 100.0f;
    float rect_y_ = 100.0f;
    float rect_speed_ = 200.0f; // pixels per second
    float rect_dir_x_ = 1.0f;
    float rect_dir_y_ = 1.0f;
    bool paused_ = false;

protected:
    window_config get_window_config() override {
        return {"Basic App Example", 800, 600, window_flags::resizable, 60};
    }

    void on_ready() override {
        std::cout << "Application initialized!\n";
        std::cout << "Press ESC to quit\n";
        std::cout << "Press SPACE to pause/resume\n";
    }

    void on_update(float dt) override {
        if (paused_) return;

        // Update rectangle position
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
    }

    void on_render(renderer& r) override {
        // Clear screen
        [[maybe_unused]] auto color_res = r.set_draw_color(colors::dark_gray);
        [[maybe_unused]] auto clear_res = r.clear();

        // Draw rectangle
        [[maybe_unused]] auto rect_color_res = r.set_draw_color(colors::cyan);
        [[maybe_unused]] auto fill_res = r.fill_rect(rect_i{
            static_cast<int>(rect_x_),
            static_cast<int>(rect_y_),
            50, 50
        });

        r.present();
    }

    void handle_event(const event& e) override {
        if (e.type() == event_type::key_down) {
            const auto& key_event = e.key();
            switch (key_event.key) {
                case SDLK_ESCAPE:
                    quit();
                    break;

                case SDLK_SPACE:
                    paused_ = !paused_;
                    std::cout << (paused_ ? "Paused\n" : "Resumed\n");
                    break;

                default:
                    break;
            }
        }
    }

    void on_quit() noexcept override {
        std::cout << "Application shutting down...\n";
        std::cout << "Average FPS: " << fps() << "\n";
    }
};

// Entry point
SDLPP_MAIN(basic_app)
