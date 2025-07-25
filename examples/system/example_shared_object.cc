#include <iostream>
#include <iomanip>
#include <cmath>
#include <sdlpp/system/shared_object.hh>
#include <sdlpp/core/core.hh>

// Define a simple plugin API
struct PluginAPI : sdlpp::symbol_resolver<PluginAPI> {
    // Function types
    using get_version_func = const char*();
    using initialize_func = int(const char*);
    using process_func = double(double);
    using cleanup_func = void();
    
    // Function pointers
    get_version_func* get_version = nullptr;
    initialize_func* initialize = nullptr;
    process_func* process = nullptr;
    cleanup_func* cleanup = nullptr;
    
    // Define symbol bindings
    static constexpr auto symbols() {
        return std::make_tuple(
            symbol_binding{"plugin_get_version", &PluginAPI::get_version},
            symbol_binding{"plugin_initialize", &PluginAPI::initialize},
            symbol_binding{"plugin_process", &PluginAPI::process},
            symbol_binding{"plugin_cleanup", &PluginAPI::cleanup}
        );
    }
};

void basic_example() {
    std::cout << "\n=== Basic Shared Object Example ===\n";
    
    // Try to load the math library
    #ifdef _WIN32
        const char* math_lib = "msvcrt.dll";
    #elif defined(__APPLE__)
        const char* math_lib = "libm.dylib";
    #else
        const char* math_lib = "libm.so.6";
    #endif
    
    auto lib_result = sdlpp::shared_object::load(math_lib);
    if (!lib_result) {
        std::cout << "Failed to load math library: " << lib_result.error() << "\n";
        return;
    }
    
    auto& lib = lib_result.value();
    std::cout << "Successfully loaded: " << math_lib << "\n";
    
    // Get some math functions
    using sqrt_func = double(double);
    using sin_func = double(double);
    using cos_func = double(double);
    
    auto sqrt_result = lib.get_function<sqrt_func>("sqrt");
    auto sin_result = lib.get_function<sin_func>("sin");
    auto cos_result = lib.get_function<cos_func>("cos");
    
    if (sqrt_result && sin_result && cos_result) {
        auto sqrt_fn = sqrt_result.value();
        auto sin_fn = sin_result.value();
        auto cos_fn = cos_result.value();
        
        std::cout << "\nMath function tests:\n";
        std::cout << "  sqrt(16.0) = " << sqrt_fn(16.0) << "\n";
        std::cout << "  sin(0.0) = " << sin_fn(0.0) << "\n";
        std::cout << "  cos(0.0) = " << cos_fn(0.0) << "\n";
        
        // Calculate sin²(x) + cos²(x) = 1
        double x = 0.5;
        double sin_x = sin_fn(x);
        double cos_x = cos_fn(x);
        double result = sin_x * sin_x + cos_x * cos_x;
        std::cout << "  sin²(" << x << ") + cos²(" << x << ") = " << result << "\n";
    }
    
    // Check for symbol existence
    std::cout << "\nSymbol existence checks:\n";
    std::cout << "  Has 'sqrt': " << (lib.has_symbol("sqrt") ? "Yes" : "No") << "\n";
    std::cout << "  Has 'pow': " << (lib.has_symbol("pow") ? "Yes" : "No") << "\n";
    std::cout << "  Has 'fake_function': " << (lib.has_symbol("fake_function") ? "Yes" : "No") << "\n";
}

void symbol_resolver_example() {
    std::cout << "\n=== Symbol Resolver Example ===\n";
    
    // Define a math API using symbol resolver
    struct MathAPI : sdlpp::symbol_resolver<MathAPI> {
        using sqrt_func = double(double);
        using pow_func = double(double, double);
        using log_func = double(double);
        using exp_func = double(double);
        
        sqrt_func* sqrt = nullptr;
        pow_func* pow = nullptr;
        log_func* log = nullptr;
        exp_func* exp = nullptr;
        
        static constexpr auto symbols() {
            return std::make_tuple(
                symbol_binding{"sqrt", &MathAPI::sqrt},
                symbol_binding{"pow", &MathAPI::pow},
                symbol_binding{"log", &MathAPI::log},
                symbol_binding{"exp", &MathAPI::exp}
            );
        }
    };
    
    #ifdef _WIN32
        const char* math_lib = "msvcrt.dll";
    #elif defined(__APPLE__)
        const char* math_lib = "libm.dylib";
    #else
        const char* math_lib = "libm.so.6";
    #endif
    
    auto lib_result = sdlpp::shared_object::load(math_lib);
    if (!lib_result) {
        std::cout << "Failed to load math library\n";
        return;
    }
    
    MathAPI math;
    auto load_result = math.load_from(lib_result.value());
    
    if (!load_result) {
        std::cout << "Failed to load symbols: " << load_result.error() << "\n";
        return;
    }
    
    std::cout << "Successfully loaded all math symbols\n\n";
    
    // Use the loaded functions
    std::cout << "Math calculations:\n";
    std::cout << "  sqrt(25) = " << math.sqrt(25.0) << "\n";
    std::cout << "  pow(2, 8) = " << math.pow(2.0, 8.0) << "\n";
    std::cout << "  log(e) = " << math.log(std::exp(1.0)) << "\n";
    std::cout << "  exp(1) = " << math.exp(1.0) << "\n";
    
    // Verify: e^(ln(x)) = x
    double x = 42.0;
    double result = math.exp(math.log(x));
    std::cout << "  exp(log(" << x << ")) = " << result << "\n";
}

void error_handling_example() {
    std::cout << "\n=== Error Handling Example ===\n";
    
    // Try to load a non-existent library
    auto result = sdlpp::shared_object::load("this_library_does_not_exist.so");
    if (!result) {
        std::cout << "Expected error: " << result.error() << "\n";
    }
    
    // Try to get a symbol from an invalid object
    sdlpp::shared_object invalid_obj;
    using func_type = void();
    auto func_result = invalid_obj.get_function<func_type>("some_function");
    if (!func_result) {
        std::cout << "Expected error: " << func_result.error() << "\n";
    }
    
    // Load a valid library but request invalid symbol
    #ifdef _WIN32
        const char* lib_name = "kernel32.dll";
    #else
        const char* lib_name = "libc.so.6";
    #endif
    
    auto lib_result = sdlpp::shared_object::load(lib_name);
    if (lib_result) {
        auto& lib = lib_result.value();
        auto bad_symbol = lib.get_function<func_type>("this_symbol_does_not_exist");
        if (!bad_symbol) {
            std::cout << "Expected error: " << bad_symbol.error() << "\n";
        }
    }
}

void plugin_simulation() {
    std::cout << "\n=== Plugin System Simulation ===\n";
    std::cout << "Note: This is a simulation of how a plugin system would work\n";
    std::cout << "In a real system, you would load actual plugin libraries\n\n";
    
    // Simulate plugin discovery
    std::vector<std::string> plugin_paths = {
        "plugin_filter.so",
        "plugin_effects.so",
        "plugin_analysis.so"
    };
    
    for (const auto& path : plugin_paths) {
        std::cout << "Attempting to load plugin: " << path << "\n";
        
        // In a real system, this would load actual plugins
        auto plugin_result = sdlpp::shared_object::load(path);
        if (!plugin_result) {
            std::cout << "  Skipping (not found): " << plugin_result.error() << "\n";
            continue;
        }
        
        // Would load plugin API here
        PluginAPI api;
        auto load_result = api.load_from(plugin_result.value());
        if (!load_result) {
            std::cout << "  Failed to load API: " << load_result.error() << "\n";
            continue;
        }
        
        // Would use plugin here
        if (api.get_version) {
            std::cout << "  Plugin version: " << api.get_version() << "\n";
        }
        
        if (api.initialize) {
            int result = api.initialize("config.json");
            std::cout << "  Initialization: " << (result == 0 ? "Success" : "Failed") << "\n";
        }
        
        if (api.process) {
            double value = api.process(1.0);
            std::cout << "  Process result: " << value << "\n";
        }
        
        if (api.cleanup) {
            api.cleanup();
            std::cout << "  Cleanup complete\n";
        }
    }
}

int main() {
    try {
        // SDL is required for shared object loading
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "SDL Shared Object Example\n";
        std::cout << "=========================\n";
        
        basic_example();
        symbol_resolver_example();
        error_handling_example();
        plugin_simulation();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}