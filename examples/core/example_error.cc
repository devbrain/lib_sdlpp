#include <sdlpp/core/error.hh>

#include <sdlpp/core/sdl.hh>

#include <iostream>
#include <thread>
#include <vector>

// Example function that simulates an SDL operation that might fail
bool simulate_sdl_operation(bool should_fail, const std::string& operation_name) {
    if (should_fail) {
        // Use the modern C++ error setting with variadic arguments
        return sdlpp::set_error("Failed to perform", operation_name, "- insufficient resources");
    }
    return true; // Success
}

// Example of using error handling in a class
class ResourceManager {
public:
    bool load_texture(const std::string& filename, int width, int height) {
        if (filename.empty()) {
            return sdlpp::set_invalid_param_error("filename");
        }
        
        if (width <= 0 || height <= 0) {
            return sdlpp::set_error("Invalid texture dimensions:", width, "x", height);
        }
        
        // Simulate memory allocation failure for large textures
        if (width * height > 4096 * 4096) {
            return sdlpp::set_out_of_memory_error();
        }
        
        std::cout << "Successfully loaded texture: " << filename << " (" << width << "x" << height << ")\n";
        return true;
    }
    
    bool load_unsupported_format(const std::string& format) {
        if (format != "PNG" && format != "JPG") {
            (void)sdlpp::set_error("Unsupported format:", format);
            return sdlpp::set_unsupported_error();
        }
        return true;
    }
};

// Example of error preservation with error_guard
void demonstrate_error_guard() {
    std::cout << "\n=== Error Guard Demo ===\n";
    
    // Set an initial error
    (void)sdlpp::set_error("Important error that should be preserved");
    std::cout << "Initial error: " << sdlpp::get_error() << "\n";
    
    {
        // Create a guard to preserve the error
        sdlpp::error_guard guard;
        std::cout << "Error after guard creation: " << sdlpp::get_error() << " (should be empty)\n";
        
        // Do some operations that might set errors
        simulate_sdl_operation(true, "temporary operation");
        std::cout << "Error during guard scope: " << sdlpp::get_error() << "\n";
        
    } // Guard restores original error
    
    std::cout << "Error after guard destruction: " << sdlpp::get_error() << " (should be restored)\n";
}

// Example of error scope for clean error state
void demonstrate_error_scope() {
    std::cout << "\n=== Error Scope Demo ===\n";
    
    (void)sdlpp::set_error("Error before scope");
    std::cout << "Error before scope: " << sdlpp::get_error() << "\n";
    
    {
        sdlpp::error_scope scope; // Clears on entry
        std::cout << "Error after scope creation: " << sdlpp::get_error() << " (should be empty)\n";
        
        (void)sdlpp::set_error("Error set within scope");
        std::cout << "Error within scope: " << sdlpp::get_error() << "\n";
        
    } // Clears on exit by default
    
    std::cout << "Error after scope exit: " << sdlpp::get_error() << " (should be empty)\n";
}

// Example of thread-safe error handling
void demonstrate_thread_safety() {
    std::cout << "\n=== Thread Safety Demo ===\n";
    
    std::vector<std::thread> threads;
    const int num_threads = 4;
    
    // Clear any existing error in main thread
    sdlpp::clear_error();
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([i]() {
            // Each thread sets its own error
            (void)sdlpp::set_error("Thread", i, "encountered an error with code", 100 + i);
            
            // Verify the error is thread-local
            std::string error = sdlpp::get_error();
            std::cout << "Thread " << i << " error: " << error << "\n";
            
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
            // Error should still be the same
            std::string error_after = sdlpp::get_error();
            if (error != error_after) {
                std::cout << "ERROR: Thread " << i << " error changed!\n";
            }
        });
    }
    
    // Set main thread error
    (void)sdlpp::set_error("Main thread error");
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Main thread error should be preserved
    std::cout << "Main thread error after joins: " << sdlpp::get_error() << "\n";
}

int main() {
    std::cout << "SDL++ Error Handling Example\n";
    std::cout << "===========================\n\n";
    
    // 1. Basic error handling
    std::cout << "=== Basic Error Handling ===\n";
    
    // Clear any existing errors
    sdlpp::clear_error();
    std::cout << "Initial error state: '" << sdlpp::get_error() << "' (should be empty)\n";
    
    // Set a simple error
    (void)sdlpp::set_error("Simple error message");
    std::cout << "After setting error: '" << sdlpp::get_error() << "'\n";
    
    // Clear the error
    sdlpp::clear_error();
    std::cout << "After clearing: '" << sdlpp::get_error() << "' (should be empty)\n";
    
    // 2. Type-safe error formatting
    std::cout << "\n=== Type-Safe Error Formatting ===\n";
    
    int width = 1920, height = 1080;
    float fps = 60.5f;
    bool vsync = true;
    
    (void)sdlpp::set_error("Failed to create window:", width, "x", height, "at", fps, "FPS, vsync:", vsync);
    std::cout << "Formatted error: " << sdlpp::get_error() << "\n";
    
    // 3. Simulate SDL operations
    std::cout << "\n=== Simulated SDL Operations ===\n";
    
    if (!simulate_sdl_operation(false, "load_texture")) {
        std::cout << "Operation failed: " << sdlpp::get_error() << "\n";
    } else {
        std::cout << "Operation succeeded\n";
    }
    
    if (!simulate_sdl_operation(true, "create_renderer")) {
        std::cout << "Operation failed: " << sdlpp::get_error() << "\n";
    }
    
    // 4. Resource manager example
    std::cout << "\n=== Resource Manager Example ===\n";
    
    ResourceManager manager;
    
    // Valid texture load
    if (!manager.load_texture("player.png", 64, 64)) {
        std::cout << "Failed to load texture: " << sdlpp::get_error() << "\n";
    }
    
    // Invalid parameters
    if (!manager.load_texture("", 64, 64)) {
        std::cout << "Failed to load texture: " << sdlpp::get_error() << "\n";
    }
    
    // Invalid dimensions
    if (!manager.load_texture("huge.png", -100, 200)) {
        std::cout << "Failed to load texture: " << sdlpp::get_error() << "\n";
    }
    
    // Out of memory
    if (!manager.load_texture("massive.png", 8192, 8192)) {
        std::cout << "Failed to load texture: " << sdlpp::get_error() << "\n";
    }
    
    // Unsupported format
    if (!manager.load_unsupported_format("BMP")) {
        std::cout << "Failed to load format: " << sdlpp::get_error() << "\n";
    }
    
    // 5. Error guard demonstration
    demonstrate_error_guard();
    
    // 6. Error scope demonstration
    demonstrate_error_scope();
    
    // 7. Thread safety demonstration
    demonstrate_thread_safety();
    
    // 8. Special error types
    std::cout << "\n=== Special Error Types ===\n";
    
    (void)sdlpp::set_out_of_memory_error();
    std::cout << "Out of memory error: " << sdlpp::get_error() << "\n";
    
    (void)sdlpp::set_unsupported_error();
    std::cout << "Unsupported error: " << sdlpp::get_error() << "\n";
    
    (void)sdlpp::set_invalid_param_error("test_parameter");
    std::cout << "Invalid parameter error: " << sdlpp::get_error() << "\n";
    
    // 9. Error return values
    std::cout << "\n=== Error Return Values ===\n";
    
    bool result = sdlpp::set_error("This returns false");
    std::cout << "set_error returned: " << std::boolalpha << result << " (should be false)\n";
    std::cout << "Error message: " << sdlpp::get_error() << "\n";
    
    std::cout << "\nError handling example completed!\n";
    
    return 0;
}