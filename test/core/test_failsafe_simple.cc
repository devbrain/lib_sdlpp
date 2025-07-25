//
// Simple test for failsafe backend
//

#include <sdlpp/core/core.hh>
#include <sdlpp/core/failsafe_backend.hh>
#include <sdlpp/core/log.hh>
#include <failsafe/failsafe.hh>
#include <iostream>
#include <vector>

int main() {
    // Initialize SDL
    sdlpp::init sdl_init(sdlpp::init_flags::video);
    
    // Set up log capture
    std::vector<std::pair<int, std::string>> captured;
    sdlpp::log_config::set_output_function(
        [&captured](int category, sdlpp::log_priority priority, const std::string& message) {
            std::cout << "CAPTURED: cat=" << category 
                      << ", pri=" << static_cast<int>(priority)
                      << ", msg='" << message << "'\n";
            captured.push_back({category, message});
        }
    );
    
    // Create backend
    sdlpp::failsafe_backend::config cfg;
    cfg.show_timestamp = false;
    cfg.show_thread_id = false;
    cfg.show_file_line = false;
    auto backend = sdlpp::failsafe_backend::create(cfg);
    
    std::cout << "SDL log categories:\n";
    std::cout << "  application = " << static_cast<int>(sdlpp::log_category::application) << "\n";
    std::cout << "  system = " << static_cast<int>(sdlpp::log_category::system) << "\n";
    std::cout << "  test = " << static_cast<int>(sdlpp::log_category::test) << "\n";
    
    // Test category mapping
    backend.map_category("network", sdlpp::log_category::system);
    backend.map_category("database", sdlpp::log_category::application);
    backend.set_default_category(sdlpp::log_category::test);
    
    failsafe::logger::set_backend(backend.get_logger());
    
    std::cout << "\n=== Testing category mapping ===\n";
    captured.clear();
    LOG_INFO("network", "Network message");
    LOG_INFO("database", "Database message");
    LOG_INFO("unmapped", "Unmapped message");
    
    std::cout << "\nCaptured " << captured.size() << " messages\n";
    for (size_t i = 0; i < captured.size(); ++i) {
        std::cout << "  [" << i << "] cat=" << captured[i].first 
                  << ", msg='" << captured[i].second << "'\n";
    }
    
    // Test variadic
    std::cout << "\n=== Testing variadic logging ===\n";
    captured.clear();
    int count = 42;
    double value = 3.14;
    LOG_INFO("test", "Count: ", count, ", Value: ", value);
    
    if (!captured.empty()) {
        std::cout << "Message: '" << captured[0].second << "'\n";
        auto pos = captured[0].second.find("Count: 42");
        std::cout << "Find 'Count: 42' at position: " << pos 
                  << " (npos=" << std::string::npos << ")\n";
    }
    
    return 0;
}