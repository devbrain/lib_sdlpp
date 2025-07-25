#include <sdlpp/core/log.hh>
#include <sdlpp/core/error.hh>


#include <iostream>
#include <filesystem>
#include <chrono>
#include <optional>
#include <variant>
#include <thread>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

int main() {
    std::cout << "SDL++ Enhanced String Utilities Example\n";
    std::cout << "======================================\n\n";
    
    // Example 1: Logging with filesystem paths
    std::cout << "1. Logging with filesystem paths:\n";
    {
        fs::path config_path = "/etc/myapp/config.json";
        fs::path log_path = "/var/log/myapp.log";
        
        // Using static logger methods with custom category
        sdlpp::logger::info(sdlpp::log_category::custom, std::source_location::current(),
            "Loading config from:", config_path);
        sdlpp::logger::info(sdlpp::log_category::custom, std::source_location::current(),
            "Logging to:", log_path);
        
        // Direct use of build_message
        std::string msg = failsafe::detail::build_message(
            "Paths:", config_path, "and", log_path, "are configured"
        );
        std::cout << "Built message: " << msg << "\n";
    }
    
    // Example 2: Timing information with chrono
    std::cout << "\n2. Timing information:\n";
    {
        auto start = std::chrono::steady_clock::now();
        
        // Simulate some work
        std::this_thread::sleep_for(100ms);
        
        auto end = std::chrono::steady_clock::now();
        auto duration = end - start;
        auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
        
        // Using app_info shortcuts for application logging
        sdlpp::logger::app_info(std::source_location::current(),
            "Operation took:", duration_ms);
        
        // Mix different duration types
        sdlpp::logger::app_debug(std::source_location::current(),
            "Timings:", 500ns, 250us, 100ms, 5s, 2min);
        
        // Log current time
        auto now = std::chrono::system_clock::now();
        sdlpp::logger::app_info(std::source_location::current(),
            "Current time:", now);
    }
    
    // Example 3: Error messages with optional values
    std::cout << "\n3. Error handling with optional values:\n";
    {
        std::optional<int> port;
        std::optional<std::string> hostname = "localhost";
        
        if (!port.has_value()) {
            (void)sdlpp::set_error("Failed to connect to", hostname, "port:", port);
            std::cout << "Error: " << sdlpp::get_error() << "\n";
        }
        
        port = 8080;
        (void)sdlpp::set_error("Connected to", hostname, "port:", port);
        std::cout << "Info: " << sdlpp::get_error() << "\n";
    }
    
    // Example 4: Complex state with variants
    std::cout << "\n4. State logging with variants:\n";
    {
        using State = std::variant<std::monostate, std::string, int, fs::path>;
        
        State state;
        // Using log_category enum
        sdlpp::logger::info(sdlpp::log_category::application, std::source_location::current(),
            "Initial state:", state);
        
        state = std::string("loading");
        sdlpp::logger::info(sdlpp::log_category::application, std::source_location::current(),
            "String state:", state);
        
        state = 42;
        sdlpp::logger::info(sdlpp::log_category::application, std::source_location::current(),
            "Numeric state:", state);
        
        state = fs::path("/tmp/state.dat");
        sdlpp::logger::info(sdlpp::log_category::application, std::source_location::current(),
            "Path state:", state);
    }
    
    // Example 5: Mixed complex types
    std::cout << "\n5. Complex type combinations:\n";
    {
        fs::path data_dir = "/home/user/data";
        std::optional<fs::path> backup_dir;
        std::variant<int, std::string> status = "ready";
        auto uptime = 3h + 25min + 30s;
        
        // Complex logging with multiple arguments
        sdlpp::logger::app_info(std::source_location::current(),
            "System status:",
            "data_dir:", data_dir,
            "backup:", backup_dir,
            "status:", status,
            "uptime:", uptime
        );
        
        backup_dir = "/mnt/backup";
        status = 200;
        sdlpp::logger::app_info(std::source_location::current(),
            "Updated - backup:", backup_dir, "status:", status);
    }
    
    // Example 6: Direct string building
    std::cout << "\n6. Direct string building:\n";
    {
        using namespace failsafe::detail;
        
        // Build complex messages
        auto msg1 = build_message(
            "Server", "started", "at", std::chrono::system_clock::now(),
            "on", "port", 8080, "with", "SSL:", true
        );
        std::cout << "Message 1: " << msg1 << "\n";
        
        // Build path list
        std::vector<fs::path> paths = {
            "/usr/bin",
            "/usr/local/bin",
            "/home/user/.local/bin"
        };
        
        std::ostringstream oss;
        oss << "PATH:";
        for (const auto& p : paths) {
            append_to_stream(oss, " ");
            append_to_stream(oss, p);
        }
        std::cout << "Message 2: " << oss.str() << "\n";
        
        // Optional chain
        std::optional<std::optional<int>> nested = std::optional<int>(42);
        auto msg3 = build_message("Nested optional:", nested);
        std::cout << "Message 3: " << msg3 << "\n";
    }
    
    std::cout << "\nString utilities example completed!\n";
    
    return 0;
}