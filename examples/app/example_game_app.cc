//
// Example: Fixed Timestep Game Application
//

#include <sdlpp/app/game_app.hh>
#include <sdlpp/app/app_impl.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/utility/geometry.hh>
#include <iostream>
#include <vector>
#include <cmath>

using namespace sdlpp;

// Simple physics-based game with bouncing balls
class physics_game : public game_application {
    struct ball {
        float x, y;           // Position
        float vx, vy;         // Velocity
        float radius;
        color col;
        
        // For interpolation
        float prev_x, prev_y;
    };
    
    std::vector<ball> balls_;
    float gravity_ = 500.0f;
    float damping_ = 0.99f;
    bool show_stats_ = true;
    
protected:
    bool on_init() override {
        std::cout << "Fixed Timestep Physics Demo\n";
        std::cout << "Press SPACE to spawn balls\n";
        std::cout << "Press R to reset\n";
        std::cout << "Press S to toggle stats\n";
        std::cout << "Press ESC to quit\n";
        
        // Spawn initial balls
        spawn_ball(400, 100);
        spawn_ball(350, 150);
        spawn_ball(450, 150);
        
        return true;
    }
    
    void fixed_update(float dt) override {
        // This is called at exactly 60 Hz (by default)
        // Perfect for physics simulation
        
        for (auto& b : balls_) {
            // Save previous position for interpolation
            b.prev_x = b.x;
            b.prev_y = b.y;
            
            // Apply gravity
            b.vy += gravity_ * dt;
            
            // Update position
            b.x += b.vx * dt;
            b.y += b.vy * dt;
            
            // Apply damping
            b.vx *= damping_;
            b.vy *= damping_;
            
            // Collision with walls
            if (b.x - b.radius < 0) {
                b.x = b.radius;
                b.vx = std::abs(b.vx) * 0.8f;
            }
            if (b.x + b.radius > 800) {
                b.x = 800 - b.radius;
                b.vx = -std::abs(b.vx) * 0.8f;
            }
            if (b.y - b.radius < 0) {
                b.y = b.radius;
                b.vy = std::abs(b.vy) * 0.8f;
            }
            if (b.y + b.radius > 600) {
                b.y = 600 - b.radius;
                b.vy = -std::abs(b.vy) * 0.9f; // Less energy loss on ground
                
                // Stop tiny bounces
                if (std::abs(b.vy) < 50.0f) {
                    b.vy = 0;
                }
            }
        }
        
        // Ball-to-ball collisions (simple)
        for (size_t i = 0; i < balls_.size(); ++i) {
            for (size_t j = i + 1; j < balls_.size(); ++j) {
                auto& b1 = balls_[i];
                auto& b2 = balls_[j];
                
                float dx = b2.x - b1.x;
                float dy = b2.y - b1.y;
                float dist_sq = dx * dx + dy * dy;
                float min_dist = b1.radius + b2.radius;
                
                if (dist_sq < min_dist * min_dist) {
                    // Collision! Simple response
                    float dist = std::sqrt(dist_sq);
                    float nx = dx / dist;
                    float ny = dy / dist;
                    
                    // Separate balls
                    float overlap = min_dist - dist;
                    b1.x -= nx * overlap * 0.5f;
                    b1.y -= ny * overlap * 0.5f;
                    b2.x += nx * overlap * 0.5f;
                    b2.y += ny * overlap * 0.5f;
                    
                    // Exchange velocities (simplified)
                    float v1_dot = b1.vx * nx + b1.vy * ny;
                    float v2_dot = b2.vx * nx + b2.vy * ny;
                    
                    b1.vx += (v2_dot - v1_dot) * nx;
                    b1.vy += (v2_dot - v1_dot) * ny;
                    b2.vx += (v1_dot - v2_dot) * nx;
                    b2.vy += (v1_dot - v2_dot) * ny;
                }
            }
        }
    }
    
    void render(float alpha) override {
        auto& r = get_renderer();
        
        // Clear screen
        if (auto res = r.set_draw_color(colors::black); !res) {
            return;
        }
        if (auto res = r.clear(); !res) {
            return;
        }
        
        // Draw balls with interpolation
        for (const auto& b : balls_) {
            // Interpolate position for smooth rendering
            float render_x = b.prev_x + (b.x - b.prev_x) * alpha;
            float render_y = b.prev_y + (b.y - b.prev_y) * alpha;
            
            // Draw shadow
            [[maybe_unused]] auto shadow_res = r.set_draw_color({0, 0, 0, 64});
            [[maybe_unused]] auto shadow_rect_res = r.fill_rect(rect_i{
                static_cast<int>(render_x - b.radius),
                590,
                static_cast<int>(b.radius * 2),
                10
            });
            
            // Draw ball (approximated with rectangle for simplicity)
            [[maybe_unused]] auto ball_color_res = r.set_draw_color(b.col);
            [[maybe_unused]] auto ball_rect_res = r.fill_rect(rect_i{
                static_cast<int>(render_x - b.radius),
                static_cast<int>(render_y - b.radius),
                static_cast<int>(b.radius * 2),
                static_cast<int>(b.radius * 2)
            });
        }
        
        // Draw stats
        if (show_stats_) {
            [[maybe_unused]] auto text_color_res = r.set_draw_color(colors::white);
            // In real app, render text showing:
            // - FPS: get_fps()
            // - Fixed updates/frame: get_fixed_updates_per_frame()
            // - Interpolation: get_interpolation_alpha()
            // - Ball count: balls_.size()
        }
    }
    
    bool on_event(const sdlpp::event& e) override {
        if (e.is<keyboard_event>()) {
            const auto* kb = e.as<keyboard_event>();
            if (!kb) return true;
            if (kb->type == event_type::key_down) {
                switch (kb->key) {
                    case keycodes::escape:
                        request_quit();
                        break;
                        
                    case keycodes::space:
                        spawn_ball(400, 100);
                        break;
                        
                    case keycodes::r:
                        balls_.clear();
                        break;
                        
                    case keycodes::s:
                        show_stats_ = !show_stats_;
                        break;
                }
            }
        }
        else if (e.is<mouse_button_event>()) {
            const auto* mb = e.as<mouse_button_event>();
                if (!mb) return true;
            if (mb->type == event_type::mouse_button_down) {
                spawn_ball(static_cast<float>(mb->x), static_cast<float>(mb->y));
            }
        }
        
        return true;
    }
    
private:
    void spawn_ball(float x, float y) {
        static const color ball_colors[] = {
            colors::red, colors::green, colors::blue,
            colors::yellow, colors::cyan, colors::magenta,
            colors::red, colors::green, colors::blue
        };
        
        balls_.push_back({
            x, y,
            static_cast<float>(rand() % 200 - 100) * 0.5f,  // Random velocity
            -static_cast<float>(rand() % 200 + 100) * 0.5f,
            10.0f + static_cast<float>(rand() % 20),         // Random radius
            ball_colors[rand() % 9],
            x, y                           // Previous position
        });
    }
};

// Entry point
SDLPP_MAIN(physics_game)