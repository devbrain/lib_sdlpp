//
// Test program to demonstrate version checking
//

#include <iostream>
#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/version.hh>

// Uncomment to simulate older SDL headers (this would fail compilation)
// #undef SDL_MAJOR_VERSION
// #undef SDL_MINOR_VERSION  
// #undef SDL_MICRO_VERSION
// #define SDL_MAJOR_VERSION 3
// #define SDL_MINOR_VERSION 1
// #define SDL_MICRO_VERSION 0

// This will cause compile error if properties.hh is included with old SDL
#include <sdlpp/config/properties.hh>

int main() {
    using namespace sdlpp;
    
    std::cout << "SDL Version Check Test" << std::endl;
    std::cout << "=====================" << std::endl;
    
    std::cout << "\nCompile-time SDL: " << version_info::compile_time << std::endl;
    std::cout << "Runtime SDL: " << version_info::runtime() << std::endl;
    
    // Check if properties are available
    std::cout << "\nProperties API available at compile-time: " 
              << (version_info::features::has_properties ? "Yes" : "No") << std::endl;
    std::cout << "Properties API available at runtime: " 
              << (version_info::features::available_at_runtime(3, 2, 0) ? "Yes" : "No") << std::endl;
    
    // Try to create properties
    std::cout << "\nAttempting to create properties..." << std::endl;
    auto props = properties::create();
    if (props) {
        std::cout << "✓ Properties created successfully!" << std::endl;
        
        // Test basic functionality
        if (props->set("test", "value")) {
            std::cout << "✓ Property set successfully" << std::endl;
            std::cout << "  Value: " << props->get_string("test") << std::endl;
        }
    } else {
        std::cout << "✗ Failed to create properties: " << props.error() << std::endl;
    }
    
    return 0;
}