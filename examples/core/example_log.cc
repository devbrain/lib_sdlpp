//
// Example: SDL++ Logging System Usage
//

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <sdlpp/core/sdl.hh>
#include "../include/sdlpp/core/log.hh"

using namespace sdlpp;

// Example 1: Basic logging
void example_basic_logging() {
    std::cout << "\n=== Basic Logging Example ===" << std::endl;
    
    // Simple logging with automatic source location
    SDLPP_LOG_APP("Application started");
    SDLPP_LOG_APP("Version:", 1, ".", 0, ".", 0);
    
    // Different priority levels
    SDLPP_LOG_APP_DEBUG("Debug information - may not appear by default");
    SDLPP_LOG_APP("Info level message - default visibility");
    SDLPP_LOG_APP_WARN("Warning:", "Low memory condition");
    SDLPP_LOG_APP_ERROR("Error:", "Failed to load resource");
    
    // Using variables in logs
    int user_count = 42;
    double fps = 60.5;
    bool vsync = true;
    
    SDLPP_LOG_APP("Users online:", user_count, "FPS:", fps, "VSync:", vsync);
}

// Example 2: Category-based logging
void example_categories() {
    std::cout << "\n=== Category-Based Logging Example ===" << std::endl;
    
    // Enable verbose logging for this example
    log_config::set_all_priorities(log_priority::verbose);
    
    // Different subsystem logs
    SDLPP_LOG_INFO(log_category::audio, "Initializing audio subsystem");
    SDLPP_LOG_DEBUG(log_category::audio, "Sample rate:", 48000, "Channels:", 2);
    
    SDLPP_LOG_INFO(log_category::video, "Setting video mode");
    SDLPP_LOG_DEBUG(log_category::video, "Resolution:", "1920x1080", "Fullscreen:", false);
    
    SDLPP_LOG_INFO(log_category::render, "Creating renderer");
    SDLPP_LOG_VERBOSE(log_category::render, "Backend: OpenGL");
    
    SDLPP_LOG_INFO(log_category::input, "Input devices detected:", 2);
    
    // Custom category
    const int GAME_LOGIC = static_cast<int>(log_category::custom);
    const int NETWORK = static_cast<int>(log_category::custom) + 1;
    
    SDLPP_LOG_INFO(GAME_LOGIC, "Game logic initialized");
    SDLPP_LOG_INFO(NETWORK, "Connected to server");
    
    // Reset priorities
    log_config::reset_priorities();
}

// Example 3: Priority filtering
void example_priority_filtering() {
    std::cout << "\n=== Priority Filtering Example ===" << std::endl;
    
    // Show current priority for application category
    auto current = log_config::get_priority(log_category::application);
    std::cout << "Current app priority: " << static_cast<int>(current) << std::endl;
    
    // Only show warnings and above
    log_config::set_priority(log_category::application, log_priority::warn);
    
    SDLPP_LOG_APP_DEBUG("Debug - won't show");
    SDLPP_LOG_APP("Info - won't show");
    SDLPP_LOG_APP_WARN("Warning - will show");
    SDLPP_LOG_APP_ERROR("Error - will show");
    
    // Set different priorities for different categories
    log_config::set_priority(log_category::audio, log_priority::debug);
    log_config::set_priority(log_category::video, log_priority::error);
    
    SDLPP_LOG_DEBUG(log_category::audio, "Audio debug - will show");
    SDLPP_LOG_DEBUG(log_category::video, "Video debug - won't show");
    SDLPP_LOG_ERROR(log_category::video, "Video error - will show");
    
    // Reset all priorities
    log_config::reset_priorities();
}

// Example 4: Custom output handling
void example_custom_output() {
    std::cout << "\n=== Custom Output Example ===" << std::endl;
    
    // Collect logs in memory
    std::vector<std::string> log_buffer;
    
    auto guard = log_config::scoped_output_function(
        [&log_buffer](int category, log_priority priority, const std::string& message) {
            std::ostringstream oss;
            
            // Custom formatting
            oss << "[" << std::chrono::system_clock::now().time_since_epoch().count() << "] ";
            
            // Add priority indicator
            switch (priority) {
                case log_priority::trace:    oss << "[TRACE] "; break;
                case log_priority::verbose:  oss << "[VERBOSE] "; break;
                case log_priority::debug:    oss << "[DEBUG] "; break;
                case log_priority::info:     oss << "[INFO] "; break;
                case log_priority::warn:     oss << "[WARN] "; break;
                case log_priority::error:    oss << "[ERROR] "; break;
                case log_priority::critical: oss << "[CRITICAL] "; break;
                default: break;
            }
            
            // Add category
            oss << "Cat" << category << ": ";
            
            // Add message
            oss << message;
            
            log_buffer.push_back(oss.str());
            
            // Also print to console
            std::cout << "Custom: " << oss.str() << std::endl;
        });
    
    // These will use custom output
    SDLPP_LOG_APP("Custom output test");
    SDLPP_LOG_APP_WARN("Warning with custom format");
    
    std::cout << "\nCollected " << log_buffer.size() << " log entries" << std::endl;
}

// Example 5: Complex data logging
void example_complex_data() {
    std::cout << "\n=== Complex Data Logging Example ===" << std::endl;
    
    // Logging with multiple values
    std::string player_name = "Hero";
    int level = 42;
    float x = 100.5f, y = 0.0f, z = -50.0f;
    
    SDLPP_LOG_APP("Player:", player_name, "Level:", level, "Position: {", x, ",", y, ",", z, "}");
    SDLPP_LOG_APP("Position update: {", x, ",", y, ",", z, "} -> {", 110.0f, ",", 0.0f, ",", -45.0f, "}");
    
    // Collections
    std::vector<int> items = {101, 102, 103};
    SDLPP_LOG_APP("Inventory size:", items.size(), "First item:", items.front());
    
    // Pointers
    std::string* name_ptr = &player_name;
    std::string* null_ptr = nullptr;
    
    SDLPP_LOG_APP("Name pointer:", name_ptr, "Null pointer:", null_ptr);
    
    // Using different types
    double pi = 3.14159265359;
    bool is_active = true;
    char grade = 'A';
    
    SDLPP_LOG_APP("Pi:", pi, "Active:", is_active, "Grade:", grade);
}

// Example 6: Performance logging
void example_performance_logging() {
    std::cout << "\n=== Performance Logging Example ===" << std::endl;
    
    // Simple performance timer
    class Timer {
        std::chrono::high_resolution_clock::time_point start;
        std::string name;
        
    public:
        explicit Timer(const std::string& n) : name(n) {
            start = std::chrono::high_resolution_clock::now();
            SDLPP_LOG_DEBUG(log_category::application, "Timer", name, "started");
        }
        
        ~Timer() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            SDLPP_LOG_DEBUG(log_category::application, "Timer", name, "took", 
                          duration.count(), "µs");
        }
    };
    
    log_config::set_priority(log_category::application, log_priority::debug);
    
    {
        Timer t("Overall operation");
        
        {
            Timer t2("Step 1");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        
        {
            Timer t2("Step 2");
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    }
    
    log_config::reset_priorities();
}

// Example 7: Error handling with logging
void example_error_handling() {
    std::cout << "\n=== Error Handling Example ===" << std::endl;
    
    auto load_resource = [](const std::string& path) -> bool {
        SDLPP_LOG_DEBUG(log_category::application, "Attempting to load:", path);
        
        if (path.empty()) {
            SDLPP_LOG_ERROR(log_category::application, "Invalid path: empty string");
            return false;
        }
        
        if (path.find(".txt") == std::string::npos) {
            SDLPP_LOG_WARN(log_category::application, 
                          "Unsupported file type:", path,
                          "Expected: .txt");
            return false;
        }
        
        // Simulate loading
        SDLPP_LOG_INFO(log_category::application, "Successfully loaded:", path);
        return true;
    };
    
    load_resource("data.txt");
    load_resource("image.png");
    load_resource("");
}

// Example 8: Thread-safe logging
void example_thread_safety() {
    std::cout << "\n=== Thread Safety Example ===" << std::endl;
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([i]() {
            for (int j = 0; j < 3; ++j) {
                SDLPP_LOG_INFO(log_category::application,
                             "Thread", i, "iteration", j);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    SDLPP_LOG_APP("All threads completed");
}

int main() {
    // Note: SDL_Init is not required for logging to work
    
    std::cout << "=== SDL++ Logging System Examples ===" << std::endl;
    
    // Configure initial logging
    std::cout << "\nSetting up default SDL log output..." << std::endl;
    
    // Run examples
    example_basic_logging();
    example_categories();
    example_priority_filtering();
    example_custom_output();
    example_complex_data();
    example_performance_logging();
    example_error_handling();
    example_thread_safety();
    
    std::cout << "\n✅ All logging examples completed!" << std::endl;
    
    return 0;
}