//
// Example: Function-based Callbacks Application
//

#include <sdlpp/app/app_builder.hh>
#include <sdlpp/video/color.hh>
#include <iostream>

using namespace sdlpp;

int main(int argc, char* argv[]) {
    // Simple state
    float angle = 0.0f;
    int frame_count = 0;
    
    // Run app with callbacks
    return run(argc, argv, {
        .init = [](int, char*[]) {
            std::cout << "Simple spinning square demo\n";
            std::cout << "Press ESC to quit\n";
            
            // Note: Window and renderer are created automatically
            // by the callback_application wrapper
            return true;
        },
        
        .iterate = [&]() {
            // This is called every frame
            angle += 2.0f; // Rotate 2 degrees per frame
            if (angle >= 360.0f) angle -= 360.0f;
            
            frame_count++;
            
            // Note: In this simple callback mode, we don't have
            // direct access to the renderer, so this example
            // just demonstrates the callback structure
            
            return true; // Continue running
        },
        
        .event = [&](const event& e) {
            if (e.is<keyboard_event>()) {
                const auto* kb = e.as<keyboard_event>();
                if (!kb) return true;
                if (kb->type == event_type::key_down && 
                    kb->key == keycodes::escape) {
                    return false; // Quit
                }
            }
            return true; // Continue
        },
        
        .quit = [&]() {
            std::cout << "Shutting down after " << frame_count << " frames\n";
            std::cout << "Final angle: " << angle << " degrees\n";
        },
        
        .error = [](const std::string& err) {
            std::cerr << "Application error: " << err << "\n";
        },
        
        .parse_args = {},
        .get_delta_time = {}
    });
}