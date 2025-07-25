//
// Example: Scene-based Application
//

#include <sdlpp/app/scene_app.hh>
#include <sdlpp/app/app_impl.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/video/blend_mode.hh>
#include <sdlpp/utility/geometry.hh>
#include <iostream>

using namespace sdlpp;
using sdlpp::colors::blue;
using sdlpp::colors::white;
using sdlpp::colors::yellow;
using sdlpp::colors::black;
using sdlpp::colors::green;
using sdlpp::colors::light_gray;
using sdlpp::colors::dark_gray;
using sdlpp::keycodes::up;
using sdlpp::keycodes::down;
using sdlpp::keycodes::left;
using sdlpp::keycodes::right;
using sdlpp::keycodes::return_key;
using sdlpp::keycodes::space;
using sdlpp::keycodes::escape;

// Forward declarations
class main_menu_scene;
class game_scene;
class pause_menu_scene;

// Main menu scene
class main_menu_scene : public scene {
    int selected_option_ = 0;
    static constexpr int num_options = 3;
    const char* options_[num_options] = {"Start Game", "Options", "Quit"};
    
public:
    void on_enter() override {
        std::cout << "Entered main menu\n";
    }
    
    void render(renderer& r) override {
        if (auto res = r.set_draw_color(blue); !res) {}
        if (auto res = r.clear(); !res) {}
        
        // Draw title
        if (auto res = r.set_draw_color(white); !res) {}
        // In real app: render "DEMO GAME" text
        
        // Draw menu options
        for (int i = 0; i < num_options; ++i) {
            if (i == selected_option_) {
                if (auto res = r.set_draw_color(yellow); !res) {}
                if (auto res = r.fill_rect(rect_i{200, 200 + i * 50, 400, 40}); !res) {}
                if (auto res = r.set_draw_color(black); !res) {}
            } else {
                if (auto res = r.set_draw_color(white); !res) {}
            }
            // In real app: render options_[i] text
        }
    }
    
    bool handle_event(const event& e) override {
        if (e.is<keyboard_event>()) {
            const auto* kb = e.as<keyboard_event>();
            if (!kb) return true;
            if (kb->type == event_type::key_down) {
                switch (kb->key) {
                    case up:
                        selected_option_ = (selected_option_ - 1 + num_options) % num_options;
                        break;
                        
                    case down:
                        selected_option_ = (selected_option_ + 1) % num_options;
                        break;
                        
                    case return_key:
                    case space:
                        switch (selected_option_) {
                            case 0: // Start Game
                                app().push_scene<game_scene>();
                                break;
                            case 1: // Options
                                std::cout << "Options not implemented\n";
                                break;
                            case 2: // Quit
                                app().request_quit();
                                break;
                        }
                        break;
                        
                    case escape:
                        app().request_quit();
                        break;
                }
            }
        }
        return true;
    }
};

// Game scene
class game_scene : public scene {
    float player_x_ = 400.0f;
    float player_y_ = 300.0f;
    float player_speed_ = 300.0f;
    
    // Simple input state
    bool move_left_ = false;
    bool move_right_ = false;
    bool move_up_ = false;
    bool move_down_ = false;
    
public:
    void on_enter() override {
        std::cout << "Game started!\n";
    }
    
    void update(float dt) override {
        // Move player
        if (move_left_) player_x_ -= player_speed_ * dt;
        if (move_right_) player_x_ += player_speed_ * dt;
        if (move_up_) player_y_ -= player_speed_ * dt;
        if (move_down_) player_y_ += player_speed_ * dt;
        
        // Keep player on screen
        player_x_ = std::clamp(player_x_, 0.0f, 800.0f - 50.0f);
        player_y_ = std::clamp(player_y_, 0.0f, 600.0f - 50.0f);
    }
    
    void render(renderer& r) override {
        if (auto res = r.set_draw_color(green); !res) {}
        if (auto res = r.clear(); !res) {}
        
        // Draw player
        if (auto res = r.set_draw_color(white); !res) {}
        if (auto res = r.fill_rect(rect_i{
            static_cast<int>(player_x_),
            static_cast<int>(player_y_),
            50, 50
        }); !res) {}
        
        // Draw instructions
        if (auto res = r.set_draw_color(light_gray); !res) {}
        // In real app: render "Arrow keys to move, ESC for pause" text
    }
    
    bool handle_event(const event& e) override {
        if (e.is<keyboard_event>()) {
            const auto* kb = e.as<keyboard_event>();
            if (!kb) return true;
            bool pressed = (kb->type == event_type::key_down);
            
            switch (kb->key) {
                case left:
                    move_left_ = pressed;
                    break;
                case right:
                    move_right_ = pressed;
                    break;
                case keycodes::up:
                    move_up_ = pressed;
                    break;
                case keycodes::down:
                    move_down_ = pressed;
                    break;
                    
                case keycodes::escape:
                    if (pressed) {
                        app().push_scene<pause_menu_scene>();
                    }
                    break;
            }
        }
        return true;
    }
};

// Pause menu scene (transparent overlay)
class pause_menu_scene : public scene {
    int selected_option_ = 0;
    static constexpr int num_options = 2;
    const char* options_[num_options] = {"Resume", "Main Menu"};
    
public:
    bool is_transparent() const override { 
        return true; // Render game scene underneath
    }
    
    void on_enter() override {
        std::cout << "Game paused\n";
    }
    
    void render(renderer& r) override {
        // Draw semi-transparent overlay
        if (auto res = r.set_draw_color({0, 0, 0, 128}); !res) {}
        if (auto res = r.set_draw_blend_mode(blend_mode::blend); !res) {}
        auto viewport = r.get_viewport();
        if (viewport) {
            if (auto res = r.fill_rect(*viewport); !res) {}
        }
        if (auto res = r.set_draw_blend_mode(blend_mode::none); !res) {}
        
        // Draw pause menu box
        if (auto res = r.set_draw_color(dark_gray); !res) {}
        if (auto res = r.fill_rect(rect_i{250, 200, 300, 200}); !res) {}
        
        // Draw options
        for (int i = 0; i < num_options; ++i) {
            if (i == selected_option_) {
                if (auto res = r.set_draw_color(yellow); !res) {}
                if (auto res = r.fill_rect(rect_i{270, 250 + i * 50, 260, 40}); !res) {}
                if (auto res = r.set_draw_color(black); !res) {}
            } else {
                if (auto res = r.set_draw_color(white); !res) {}
            }
            // In real app: render options_[i] text
        }
    }
    
    bool handle_event(const event& e) override {
        if (e.is<keyboard_event>()) {
            const auto* kb = e.as<keyboard_event>();
            if (!kb) return true;
            if (kb->type == event_type::key_down) {
                switch (kb->key) {
                    case up:
                        selected_option_ = (selected_option_ - 1 + num_options) % num_options;
                        break;
                        
                    case down:
                        selected_option_ = (selected_option_ + 1) % num_options;
                        break;
                        
                    case return_key:
                    case space:
                        switch (selected_option_) {
                            case 0: // Resume
                                app().pop_scene();
                                break;
                            case 1: // Main Menu
                                app().clear_scenes();
                                app().push_scene<main_menu_scene>();
                                break;
                        }
                        break;
                        
                    case escape:
                        app().pop_scene(); // Resume
                        break;
                }
            }
        }
        return true;
    }
};

// Application class
class scene_demo_app : public scene_application {
protected:
    bool on_init() override {
        std::cout << "Scene-based Application Demo\n";
        
        // Start with main menu
        push_scene<main_menu_scene>();
        
        return true;
    }
};

// Entry point
SDLPP_MAIN(scene_demo_app)