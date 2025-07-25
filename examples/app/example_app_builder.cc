//
// Example: SDL++ Application Builder
//

#include <sdlpp/app/app_builder.hh>
#include <sdlpp/video/color.hh>
#include <iostream>
#include <vector>
#include <random>

using namespace sdlpp;

int main(int argc, char* argv[]) {
    // Application state
    bool running = true;
    int click_count = 0;
    float color_cycle = 0.0f;
    
    // Build and run the application
    return app_builder()
        .with_window("Builder Pattern Demo", 800, 600)
        .with_renderer()
        .with_window_flags(window_flags::resizable)
        .on_init([&](int, char*[]) {
            std::cout << "Application Builder Demo\n";
            std::cout << "Click anywhere to increment counter\n";
            std::cout << "Press SPACE to reset\n";
            std::cout << "Press ESC to quit\n";
            return true;
        })
        .on_frame([&]() -> bool {
            // The builder pattern doesn't provide direct access to window/renderer
            // This example shows the minimal callback-based approach
            
            // Update color cycle
            color_cycle += 0.01f;
            if (color_cycle > 1.0f) color_cycle -= 1.0f;
            
            return running;
        })
        .on_event([&](const event& e) -> bool {
            if (e.is<keyboard_event>()) {
                const auto* kb = e.as<keyboard_event>();
                if (!kb) return true;
                if (kb->type == event_type::key_down) {
                    switch (kb->key) {
                        case keycodes::escape:
                            running = false;
                            return false;
                            
                        case keycodes::space:
                            click_count = 0;
                            std::cout << "Counter reset!\n";
                            break;
                    }
                }
            }
            else if (e.is<mouse_button_event>()) {
                const auto* mb = e.as<mouse_button_event>();
                if (!mb) return true;
                if (mb->type == event_type::mouse_button_down && 
                    mb->button == static_cast<Uint8>(mouse_button::left)) {
                    click_count++;
                    std::cout << "Click #" << click_count << " at (" 
                              << mb->x << ", " << mb->y << ")\n";
                }
            }
            return true;
        })
        .on_quit([&]() {
            std::cout << "Application quit\n";
            std::cout << "Total clicks: " << click_count << "\n";
        })
        .on_error([](const std::string& err) {
            std::cerr << "Application error: " << err << "\n";
        })
        .run(argc, argv);
}