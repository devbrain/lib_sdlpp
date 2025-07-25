//
// Minimal debug test for failsafe backend
//

#include <doctest/doctest.h>
#include <sdlpp/core/failsafe_backend.hh>
#include <sdlpp/core/log.hh>
#include <failsafe/failsafe.hh>
#include <iostream>

TEST_CASE("failsafe_backend_debug") {
    // Set up log capture with debug output
    std::vector<std::pair<int, std::string>> captured;
    sdlpp::log_config::set_output_function(
        [&captured](int category, sdlpp::log_priority priority, const std::string& message) {
            std::cout << "CAPTURED: category=" << category 
                      << ", priority=" << static_cast<int>(priority)
                      << ", message='" << message << "'\n";
            captured.push_back({category, message});
        }
    );
    
    // Create backend without any formatting
    sdlpp::failsafe_backend::config cfg;
    cfg.show_timestamp = false;
    cfg.show_thread_id = false;
    cfg.show_file_line = false;
    auto backend = sdlpp::failsafe_backend::create(cfg);
    
    // Test 1: Category mapping
    std::cout << "\n=== Test 1: Category Mapping ===\n";
    std::cout << "application = " << static_cast<int>(sdlpp::log_category::application) << "\n";
    std::cout << "system = " << static_cast<int>(sdlpp::log_category::system) << "\n";
    std::cout << "test = " << static_cast<int>(sdlpp::log_category::test) << "\n";
    
    backend.map_category("network", sdlpp::log_category::system);
    backend.set_default_category(sdlpp::log_category::test);
    failsafe::logger::set_backend(backend.get_logger());
    
    captured.clear();
    LOG_INFO("network", "Network message");
    
    if (!captured.empty()) {
        std::cout << "Got " << captured.size() << " messages\n";
        std::cout << "First message category: " << captured[0].first << "\n";
        std::cout << "First message content: '" << captured[0].second << "'\n";
    }
    
    // Test 2: Variadic logging
    std::cout << "\n=== Test 2: Variadic Logging ===\n";
    captured.clear();
    int count = 42;
    double value = 3.14;
    LOG_INFO("test", "Count: ", count, ", Value: ", value);
    
    if (!captured.empty()) {
        std::cout << "Message: '" << captured[0].second << "'\n";
        auto pos = captured[0].second.find("Count: 42");
        std::cout << "Find 'Count: 42' result: " << pos << " (npos=" << std::string::npos << ")\n";
    }
}