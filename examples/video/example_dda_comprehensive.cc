//
// Comprehensive DDA Example
// Demonstrates all DDA drawing capabilities using SDL++ renderer
//

#include <iostream>
#include <vector>
#include <cmath>
#include <sstream>
#include <chrono>
#include <thread>
#include "sdlpp/core/core.hh"
#include "sdlpp/video/window.hh"
#include "sdlpp/video/renderer.hh"
#include "sdlpp/video/color.hh"
#include "sdlpp/utility/geometry.hh"
#include "sdlpp/core/error.hh"
#include "sdlpp/events/events.hh"
#include "sdlpp/core/time.hh"
#include "sdlpp/video/blend_mode.hh"
#include "sdlpp/events/keyboard_codes.hh"

using namespace sdlpp;

int main() {
    try {
        // Initialize SDL
        sdlpp::init init(sdlpp::init_flags::video);
        
        if (!init.is_initialized()) {
            std::cerr << "Failed to initialize SDL" << std::endl;
            return 1;
        }

        // Create window
        auto window_result = sdlpp::window::create(
            "SDL++ Comprehensive DDA Example",
            1024, 768,
            sdlpp::window_flags::resizable
        );
        
        if (!window_result) {
            std::cerr << "Failed to create window: " << window_result.error() << std::endl;
            return 1;
        }
        
        auto& window = *window_result;

        // Create renderer
        auto renderer_result = window.create_renderer();
        
        if (!renderer_result) {
            std::cerr << "Failed to create renderer: " << renderer_result.error() << std::endl;
            return 1;
        }
        
        auto& renderer = *renderer_result;

        // Main loop
        bool running = true;
        int demo_index = 0;
        const int num_demos = 8;
        auto& event_queue = sdlpp::get_event_queue();
        
        while (running) {
            // Handle events
            while (auto event_opt = event_queue.poll()) {
                auto& e = *event_opt;
                if (e.type() == sdlpp::event_type::quit) {
                    running = false;
                } else if (e.type() == sdlpp::event_type::key_down) {
                    switch (e.key().key) {
                        case sdlpp::keycodes::escape:
                            running = false;
                            break;
                        case sdlpp::keycodes::space:
                        case sdlpp::keycodes::right:
                            demo_index = (demo_index + 1) % num_demos;
                            break;
                        case sdlpp::keycodes::left:
                            demo_index = (demo_index - 1 + num_demos) % num_demos;
                            break;
                        default:
                            break;
                    }
                }
            }

            // Clear screen
            [[maybe_unused]] auto res1 = renderer.set_draw_color(color{20, 20, 30, 255});
            [[maybe_unused]] auto res2 = renderer.clear();

            // Draw title
            [[maybe_unused]] auto res3 = renderer.set_draw_color(color{200, 200, 200, 255});
            std::stringstream title;
            title << "Demo " << (demo_index + 1) << "/" << num_demos << " - ";
            
            // Run current demo
            switch (demo_index) {
                case 0: {
                    // Antialiased lines demo
                    title << "Antialiased Lines (Press SPACE/Arrow keys to navigate)";
                    
                    [[maybe_unused]] auto res4 = renderer.set_draw_color(color{255, 100, 100, 255});
                    
                    // Draw star pattern with antialiased lines
                    int centerX = 512;
                    int centerY = 384;
                    int radius = 200;
                    
                    for (int i = 0; i < 12; ++i) {
                        float angle1 = static_cast<float>(i * M_PI / 6.0);
                        float angle2 = static_cast<float>((i + 5) * M_PI / 6.0);
                        
                        float x1 = static_cast<float>(centerX) + static_cast<float>(radius) * std::cos(angle1);
                        float y1 = static_cast<float>(centerY) + static_cast<float>(radius) * std::sin(angle1);
                        float x2 = static_cast<float>(centerX) + static_cast<float>(radius) * std::cos(angle2);
                        float y2 = static_cast<float>(centerY) + static_cast<float>(radius) * std::sin(angle2);
                        
                        [[maybe_unused]] auto res5 = renderer.draw_line_aa(x1, y1, x2, y2);
                    }
                    
                    // Draw comparison with regular lines
                    [[maybe_unused]] auto res6 = renderer.set_draw_color(color{100, 100, 255, 255});
                    for (int i = 0; i < 12; ++i) {
                        float angle1 = static_cast<float>(i * M_PI / 6.0 + M_PI / 12.0);
                        float angle2 = static_cast<float>((i + 5) * M_PI / 6.0 + M_PI / 12.0);
                        
                        float x1 = static_cast<float>(centerX) + static_cast<float>(radius) * 0.7f * std::cos(angle1);
                        float y1 = static_cast<float>(centerY) + static_cast<float>(radius) * 0.7f * std::sin(angle1);
                        float x2 = static_cast<float>(centerX) + static_cast<float>(radius) * 0.7f * std::cos(angle2);
                        float y2 = static_cast<float>(centerY) + static_cast<float>(radius) * 0.7f * std::sin(angle2);
                        
                        [[maybe_unused]] auto res7 = renderer.draw_line(x1, y1, x2, y2);
                    }
                    break;
                }
                
                case 1: {
                    // Thick lines demo
                    title << "Thick Lines with Variable Width";
                    
                    for (int i = 0; i < 10; ++i) {
                        float width = 1.0f + static_cast<float>(i) * 2.0f;
                        int y = 100 + i * 60;
                        
                        // Color gradient
                        uint8_t r = static_cast<uint8_t>(255 - i * 20);
                        uint8_t g = static_cast<uint8_t>(50 + i * 20);
                        uint8_t b = 100;
                        [[maybe_unused]] auto res8 = renderer.set_draw_color(color{r, g, b, 255});
                        
                        [[maybe_unused]] auto res9 = renderer.draw_line_thick(100.0f, static_cast<float>(y), 
                                               900.0f, static_cast<float>(y), width);
                    }
                    break;
                }
                
                case 2: {
                    // Circles demo
                    title << "Circles and Filled Circles";
                    
                    // Draw concentric circles
                    [[maybe_unused]] auto res10 = renderer.set_draw_color(color{100, 200, 255, 255});
                    for (int r = 20; r <= 200; r += 20) {
                        [[maybe_unused]] auto res11 = renderer.draw_circle(300, 384, r);
                    }
                    
                    // Draw filled circles with transparency
                    for (int i = 0; i < 5; ++i) {
                        int x = 600 + i * 80;
                        int y = 384;
                        int r = 30 + i * 10;
                        
                        uint8_t alpha = static_cast<uint8_t>(255 - i * 40);
                        [[maybe_unused]] auto res12 = renderer.set_draw_color(color{255, 150, 50, alpha});
                        // Note: SDL++ renderer doesn't expose blend mode API directly
                        [[maybe_unused]] auto res13 = renderer.fill_circle(x, y, r);
                    }
                    break;
                }
                
                case 3: {
                    // Ellipses demo
                    title << "Ellipses and Filled Ellipses";
                    
                    // Draw rotating ellipses
                    [[maybe_unused]] auto res14 = renderer.set_draw_color(color{200, 100, 255, 255});
                    for (int i = 0; i < 8; ++i) {
                        float angle = static_cast<float>(i * M_PI / 4.0);
                        int rx = static_cast<int>(150 + 50 * std::cos(angle));
                        int ry = static_cast<int>(150 + 50 * std::sin(angle));
                        
                        [[maybe_unused]] auto res15 = renderer.draw_ellipse(300, 384, rx, ry);
                    }
                    
                    // Draw filled ellipses
                    [[maybe_unused]] auto res16 = renderer.set_draw_color(color{100, 255, 150, 200});
                    [[maybe_unused]] auto res17 = renderer.fill_ellipse(700, 300, 120, 80);
                    [[maybe_unused]] auto res18 = renderer.fill_ellipse(700, 450, 80, 120);
                    break;
                }
                
                case 4: {
                    // Ellipse arcs demo
                    title << "Ellipse Arcs";
                    
                    // Draw pie chart using ellipse arcs
                    int centerX = 512;
                    int centerY = 384;
                    int rx = 150;
                    int ry = 150;
                    
                    float angles[] = {0.0f, 0.3f, 0.7f, 1.2f, 1.8f, static_cast<float>(2.0 * M_PI)};
                    color colors[] = {
                        {255, 100, 100, 255},
                        {100, 255, 100, 255},
                        {100, 100, 255, 255},
                        {255, 255, 100, 255},
                        {255, 100, 255, 255}
                    };
                    
                    for (int i = 0; i < 5; ++i) {
                        [[maybe_unused]] auto res19 = renderer.set_draw_color(colors[i]);
                        
                        // Draw arc outline
                        [[maybe_unused]] auto res20 = renderer.draw_ellipse_arc(centerX, centerY, rx, ry, 
                                                angles[i], angles[i + 1]);
                        
                        // Draw radial lines
                        float x1 = static_cast<float>(centerX) + static_cast<float>(rx) * std::cos(angles[i]);
                        float y1 = static_cast<float>(centerY) + static_cast<float>(ry) * std::sin(angles[i]);
                        [[maybe_unused]] auto res21 = renderer.draw_line(static_cast<float>(centerX), static_cast<float>(centerY), x1, y1);
                    }
                    
                    // Draw the last radial line
                    float x1 = static_cast<float>(centerX) + static_cast<float>(rx) * std::cos(angles[5]);
                    float y1 = static_cast<float>(centerY) + static_cast<float>(ry) * std::sin(angles[5]);
                    [[maybe_unused]] auto res22 = renderer.draw_line(static_cast<float>(centerX), static_cast<float>(centerY), x1, y1);
                    break;
                }
                
                case 5: {
                    // Quadratic Bezier curves demo
                    title << "Quadratic Bezier Curves";
                    
                    [[maybe_unused]] auto res23 = renderer.set_draw_color(color{255, 200, 100, 255});
                    
                    // Interactive-looking curves
                    struct QuadBezier {
                        float x0, y0, x1, y1, x2, y2;
                    };
                    
                    QuadBezier curves[] = {
                        {100, 400, 300, 100, 500, 400},
                        {500, 400, 700, 700, 900, 400},
                        {200, 200, 500, 300, 800, 200},
                        {200, 600, 500, 500, 800, 600}
                    };
                    
                    for (const auto& c : curves) {
                        // Draw curve
                        [[maybe_unused]] auto res24 = renderer.draw_bezier_quad(c.x0, c.y0, c.x1, c.y1, c.x2, c.y2);
                        
                        // Draw control points and lines
                        [[maybe_unused]] auto res25 = renderer.set_draw_color(color{100, 100, 100, 255});
                        [[maybe_unused]] auto res26 = renderer.draw_line(c.x0, c.y0, c.x1, c.y1);
                        [[maybe_unused]] auto res27 = renderer.draw_line(c.x1, c.y1, c.x2, c.y2);
                        
                        [[maybe_unused]] auto res28 = renderer.set_draw_color(color{255, 100, 100, 255});
                        [[maybe_unused]] auto res29 = renderer.fill_circle(static_cast<int>(c.x0), static_cast<int>(c.y0), 5);
                        [[maybe_unused]] auto res30 = renderer.fill_circle(static_cast<int>(c.x2), static_cast<int>(c.y2), 5);
                        
                        [[maybe_unused]] auto res31 = renderer.set_draw_color(color{100, 255, 100, 255});
                        [[maybe_unused]] auto res32 = renderer.fill_circle(static_cast<int>(c.x1), static_cast<int>(c.y1), 5);
                        
                        [[maybe_unused]] auto res33 = renderer.set_draw_color(color{255, 200, 100, 255});
                    }
                    break;
                }
                
                case 6: {
                    // Cubic Bezier curves demo
                    title << "Cubic Bezier Curves";
                    
                    [[maybe_unused]] auto res34 = renderer.set_draw_color(color{100, 200, 255, 255});
                    
                    // Draw some smooth curves
                    struct CubicBezier {
                        float x0, y0, x1, y1, x2, y2, x3, y3;
                    };
                    
                    CubicBezier curves[] = {
                        {100, 384, 200, 100, 400, 668, 500, 384},
                        {500, 384, 600, 100, 800, 668, 900, 384},
                        {100, 200, 300, 400, 700, 400, 900, 200},
                        {100, 568, 300, 368, 700, 368, 900, 568}
                    };
                    
                    for (const auto& c : curves) {
                        // Draw curve
                        [[maybe_unused]] auto res35 = renderer.draw_bezier_cubic(c.x0, c.y0, c.x1, c.y1, 
                                                 c.x2, c.y2, c.x3, c.y3);
                        
                        // Draw control structure
                        [[maybe_unused]] auto res36 = renderer.set_draw_color(color{100, 100, 100, 255});
                        [[maybe_unused]] auto res37 = renderer.draw_line(c.x0, c.y0, c.x1, c.y1);
                        [[maybe_unused]] auto res38 = renderer.draw_line(c.x2, c.y2, c.x3, c.y3);
                        
                        [[maybe_unused]] auto res39 = renderer.set_draw_color(color{255, 100, 100, 255});
                        [[maybe_unused]] auto res40 = renderer.fill_circle(static_cast<int>(c.x0), static_cast<int>(c.y0), 5);
                        [[maybe_unused]] auto res41 = renderer.fill_circle(static_cast<int>(c.x3), static_cast<int>(c.y3), 5);
                        
                        [[maybe_unused]] auto res42 = renderer.set_draw_color(color{100, 255, 100, 255});
                        [[maybe_unused]] auto res43 = renderer.fill_circle(static_cast<int>(c.x1), static_cast<int>(c.y1), 5);
                        [[maybe_unused]] auto res44 = renderer.fill_circle(static_cast<int>(c.x2), static_cast<int>(c.y2), 5);
                        
                        [[maybe_unused]] auto res45 = renderer.set_draw_color(color{100, 200, 255, 255});
                    }
                    break;
                }
                
                case 7: {
                    // Combined demo - animated spiral
                    title << "Combined Demo - Animated Patterns";
                    
                    static float animation_time = 0.0f;
                    animation_time += 0.02f;
                    
                    // Animated spiral using lines
                    [[maybe_unused]] auto res46 = renderer.set_draw_color(color{255, 150, 100, 255});
                    float prev_x = 512.0f;
                    float prev_y = 384.0f;
                    
                    for (float t = 0; t < static_cast<float>(4.0 * M_PI); t += 0.1f) {
                        float r = 20.0f + t * 15.0f;
                        float x = 512.0f + r * std::cos(t + animation_time);
                        float y = 384.0f + r * std::sin(t + animation_time);
                        
                        if (t > 0) {
                            [[maybe_unused]] auto res47 = renderer.draw_line_aa(prev_x, prev_y, x, y);
                        }
                        
                        prev_x = x;
                        prev_y = y;
                    }
                    
                    // Rotating circles
                    for (int i = 0; i < 6; ++i) {
                        float angle = animation_time + static_cast<float>(i * M_PI / 3.0);
                        int x = static_cast<int>(512 + 200 * std::cos(angle));
                        int y = static_cast<int>(384 + 200 * std::sin(angle));
                        
                        auto r = static_cast<uint8_t>(128 + 127 * std::sin(angle));
                        auto g = static_cast<uint8_t>(128 + 127 * std::sin(angle + 2.0f));
                        auto b = static_cast<uint8_t>(128 + 127 * std::sin(angle + 4.0f));
                        
                        [[maybe_unused]] auto res48 = renderer.set_draw_color(color{r, g, b, 200});
                        [[maybe_unused]] auto res49 = renderer.fill_circle(x, y, 40);
                        
                        [[maybe_unused]] auto res50 = renderer.set_draw_color(color{r, g, b, 255});
                        [[maybe_unused]] auto res51 = renderer.draw_circle(x, y, 40);
                    }
                    
                    // Bezier wave
                    [[maybe_unused]] auto res52 = renderer.set_draw_color(color{100, 255, 200, 255});
                    for (int i = 0; i < 3; ++i) {
                        float offset = static_cast<float>(i) * 300.0f;
                        float y_base = 600.0f;
                        float wave = 50.0f * std::sin(animation_time + static_cast<float>(i));
                        
                        [[maybe_unused]] auto res53 = renderer.draw_bezier_cubic(
                            100.0f + offset, y_base,
                            200.0f + offset, y_base - 100.0f + wave,
                            300.0f + offset, y_base + 100.0f - wave,
                            400.0f + offset, y_base
                        );
                    }
                    break;
                }
                
                default:
                    break;
            }

            // Draw help text
            [[maybe_unused]] auto res54 = renderer.set_draw_color(color{200, 200, 200, 255});
            // Note: In a real application, you would use a text rendering library
            // For this example, we'll just show the title
            std::cout << "\r" << title.str() << std::flush;
            
            // Present
            [[maybe_unused]] auto res55 = renderer.present();
            
            // Cap frame rate
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}