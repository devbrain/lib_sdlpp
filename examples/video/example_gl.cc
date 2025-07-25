//
// Created by igor on 7/14/25.
//
// Example: OpenGL context creation and rendering

#include <iostream>
#include <sdlpp/core/core.hh>
#include <sdlpp/video/gl.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/events/events.hh>
#include <chrono>
#include <thread>
#include <GL/gl.h>

// Simple OpenGL 3.3 Core Profile example
const char* vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

void main() {
    gl_Position = vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

const char* fragment_shader_source = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";

int main() {
    using namespace sdlpp;
    
    std::cout << "SDL OpenGL Example\n";
    std::cout << "==================\n\n";
    
    // Initialize SDL
    try {
        sdlpp::init init(sdlpp::init_flags::video);
        
        if (!init.is_initialized()) {
            std::cerr << "Failed to initialize SDL\n";
            return 1;
        }
    
    // Configure OpenGL attributes before creating window
    std::cout << "Configuring OpenGL 3.3 Core Profile...\n";
    auto gl_config = gl::attribute_config::core_profile(3, 3);
    gl_config.stencil_size = 8;
    gl_config.multisamplesamples = 4;
    
    if (!gl_config.apply()) {
        std::cerr << "Failed to set OpenGL attributes\n";
    }
    
    // Create window with OpenGL flag
    std::cout << "Creating window...\n";
    auto window_result = sdlpp::window::create(
        "SDL++ OpenGL Example",
        800, 600,
        sdlpp::window_flags::opengl | sdlpp::window_flags::resizable
    );
    
    if (!window_result) {
        std::cerr << "Failed to create window: " << window_result.error() << "\n";
        return 1;
    }
    
    auto& win = *window_result;
    
    // Create OpenGL context
    std::cout << "Creating OpenGL context...\n";
    auto context_result = gl_context::create(win);
    
    if (!context_result) {
        std::cerr << "Failed to create OpenGL context: " << context_result.error() << "\n";
        return 1;
    }
    
    auto context = std::move(context_result.value());
    
    // Make context current
    if (!context.make_current(win)) {
        std::cerr << "Failed to make context current\n";
        return 1;
    }
    
    // Load OpenGL functions (would use GLAD or similar in real app)
    std::cout << "\nOpenGL Context Info:\n";
    std::cout << "  Vendor:   " << glGetString(GL_VENDOR) << "\n";
    std::cout << "  Renderer: " << glGetString(GL_RENDERER) << "\n";
    std::cout << "  Version:  " << glGetString(GL_VERSION) << "\n";
    std::cout << "  GLSL:     " << glGetString(GL_SHADING_LANGUAGE_VERSION) << "\n";
    
    // Query actual attributes
    std::cout << "\nActual OpenGL attributes:\n";
    auto print_attr = [](const char* name, gl_attr attr) {
        auto value = gl::get_attribute(attr);
        if (value) {
            std::cout << "  " << name << ": " << *value << "\n";
        }
    };
    
    print_attr("Red bits", gl_attr::red_size);
    print_attr("Green bits", gl_attr::green_size);
    print_attr("Blue bits", gl_attr::blue_size);
    print_attr("Alpha bits", gl_attr::alpha_size);
    print_attr("Depth bits", gl_attr::depth_size);
    print_attr("Stencil bits", gl_attr::stencil_size);
    print_attr("Multisample samples", gl_attr::multisamplesamples);
    print_attr("Context major version", gl_attr::context_major_version);
    print_attr("Context minor version", gl_attr::context_minor_version);
    
    // Set vsync
    std::cout << "\nSetting vsync...\n";
    if (gl::set_swap_interval(1)) {
        std::cout << "  VSync enabled\n";
    } else {
        std::cout << "  VSync not available\n";
    }
    
    // Check extensions
    std::cout << "\nChecking some common extensions:\n";
    const char* extensions[] = {
        "GL_ARB_vertex_array_object",
        "GL_ARB_framebuffer_object",
        "GL_ARB_texture_compression",
        "GL_EXT_texture_filter_anisotropic"
    };
    
    for (const char* ext : extensions) {
        bool supported = gl::extension_supported(ext);
        std::cout << "  " << ext << ": " << (supported ? "Yes" : "No") << "\n";
    }
    
    // Simple render loop
    std::cout << "\nStarting render loop (close window to exit)...\n";
    
    bool running = true;
    auto& event_queue = sdlpp::get_event_queue();
    
    while (running) {
        // Handle events
        while (auto event_opt = event_queue.poll()) {
            auto& event = *event_opt;
            if (event.type() == sdlpp::event_type::quit) {
                running = false;
            }
        }
        
        // Clear the screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Swap buffers
        if (!gl::swap_window(win)) {
            std::cerr << "Failed to swap buffers\n";
            break;
        }
        
        // Small delay to not max out CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    std::cout << "\nCleaning up...\n";
    
    // Context and window are automatically destroyed when they go out of scope
    
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "Done!\n";
    return 0;
}
