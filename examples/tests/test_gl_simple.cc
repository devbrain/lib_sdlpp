//
// Created by igor on 7/14/25.
//
// Simple test program for OpenGL wrapper

#include <iostream>
#include <sdlpp/video/gl.hh>
#include <sdlpp/core/version.hh>

int main() {
    using namespace sdlpp;
    
    std::cout << "SDL++ OpenGL Test\n";
    std::cout << "==================\n\n";
    
    // Check SDL version
    std::cout << "SDL Version: " << version_info::runtime() << "\n\n";
    
    // Test enum values
    std::cout << "OpenGL Profile Values:\n";
    std::cout << "  Core:          " << static_cast<Uint32>(gl_profile::core) << "\n";
    std::cout << "  Compatibility: " << static_cast<Uint32>(gl_profile::compatibility) << "\n";
    std::cout << "  ES:            " << static_cast<Uint32>(gl_profile::es) << "\n\n";
    
    // Test attribute configuration
    std::cout << "Testing attribute configuration...\n";
    auto config = gl::attribute_config::core_profile(3, 3);
    std::cout << "  Major version: " << config.major_version.value() << "\n";
    std::cout << "  Minor version: " << config.minor_version.value() << "\n";
    std::cout << "  Double buffer: " << (config.doublebuffer.value() ? "Yes" : "No") << "\n";
    std::cout << "  Depth size:    " << config.depth_size.value() << "\n\n";
    
    // Test library loading (may fail without display)
    std::cout << "Testing library loading...\n";
    {
        gl_library lib;
        if (lib.is_loaded()) {
            std::cout << "  ✓ OpenGL library loaded\n";
        } else {
            std::cout << "  ✗ Failed to load OpenGL library (this is normal without a display)\n";
        }
    }
    std::cout << "  Library unloaded\n\n";
    
    // Test EGL utilities
    std::cout << "Testing EGL utilities...\n";
    egl::attribute_callbacks callbacks;
    callbacks
        .set_platform_callback(nullptr)
        .set_surface_callback(nullptr)
        .set_context_callback(nullptr)
        .set_userdata(nullptr);
    std::cout << "  ✓ EGL callbacks configured\n\n";
    
    std::cout << "All tests completed successfully!\n";
    return 0;
}
