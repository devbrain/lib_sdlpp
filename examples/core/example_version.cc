//
// Example: SDL++ Version System Usage
//

#include <iostream>
#include <iomanip>
#include <sdlpp/core/sdl.hh>
#include "sdlpp/core/version.hh"

using namespace sdlpp;
using namespace sdlpp::literals;

// Example 1: Basic version information
void example_basic_version() {
    std::cout << "\n=== Basic Version Information ===" << std::endl;
    
    // Compile-time version (from headers)
    auto compile_ver = version_info::compile_time;
    std::cout << "Compiled against SDL: " << compile_ver << std::endl;
    std::cout << "  Major: " << compile_ver.major() << std::endl;
    std::cout << "  Minor: " << compile_ver.minor() << std::endl;
    std::cout << "  Patch: " << compile_ver.patch() << std::endl;
    std::cout << "  Numeric: " << compile_ver.to_number() << std::endl;
    
    // Runtime version (from linked library)
    auto runtime_ver = version_info::runtime();
    std::cout << "\nRuntime SDL version: " << runtime_ver << std::endl;
    std::cout << "  Numeric: " << runtime_ver.to_number() << std::endl;
    
    // Revision string
    std::cout << "\nSDL Revision: " << version_info::revision() << std::endl;
}

// Example 2: Version comparison
void example_version_comparison() {
    std::cout << "\n=== Version Comparison ===" << std::endl;
    
    auto compile_ver = version_info::compile_time;
    auto runtime_ver = version_info::runtime();
    
    std::cout << "Compile version: " << compile_ver << std::endl;
    std::cout << "Runtime version: " << runtime_ver << std::endl;
    
    if (version_info::versions_match()) {
        std::cout << "✓ Versions match exactly!" << std::endl;
    } else if (runtime_ver > compile_ver) {
        std::cout << "✓ Runtime is newer than compile-time (backward compatible)" << std::endl;
    } else if (runtime_ver < compile_ver) {
        std::cout << "⚠ Warning: Runtime is older than compile-time!" << std::endl;
        std::cout << "  Some features may not be available." << std::endl;
    }
    
    // Check minimum version requirements
    std::cout << "\nVersion checks:" << std::endl;
    std::cout << "  SDL 3.0.0 or later: " 
              << (runtime_ver.at_least(3, 0, 0) ? "Yes" : "No") << std::endl;
    std::cout << "  SDL 3.2.0 or later: " 
              << (runtime_ver.at_least(3, 2, 0) ? "Yes" : "No") << std::endl;
    std::cout << "  SDL 4.0.0 or later: " 
              << (runtime_ver.at_least(4, 0, 0) ? "Yes" : "No") << std::endl;
}

// Example 3: Feature detection
void example_feature_detection() {
    std::cout << "\n=== Feature Detection ===" << std::endl;
    
    std::cout << "Compile-time features:" << std::endl;
    std::cout << "  Properties API: " 
              << (version_info::features::has_properties ? "Available" : "Not available") << std::endl;
    std::cout << "  GPU API: " 
              << (version_info::features::has_gpu ? "Available" : "Not available") << std::endl;
    
    std::cout << "\nRuntime features:" << std::endl;
    
    // Check specific features at runtime
    struct Feature {
        const char* name;
        int major, minor, micro;
    };
    
    Feature features[] = {
        {"Properties API", 3, 2, 0},
        {"GPU API", 3, 2, 0},
        {"Hypothetical Future Feature", 3, 5, 0}
    };
    
    for (const auto& feature : features) {
        bool available = version_info::features::available_at_runtime(
            feature.major, feature.minor, feature.micro);
        std::cout << "  " << feature.name << " (SDL " 
                  << feature.major << "." << feature.minor << "." << feature.micro 
                  << "+): " << (available ? "Available" : "Not available") << std::endl;
    }
}

// Example 4: Version literals and construction
void example_version_construction() {
    std::cout << "\n=== Version Construction ===" << std::endl;
    
    // Different ways to create versions
    version v1(3, 2, 1);
    version v2(3002001);  // Numeric form
    auto v3 = 321_v;      // User-defined literal
    
    std::cout << "v1 (component): " << v1 << std::endl;
    std::cout << "v2 (numeric): " << v2 << std::endl;
    std::cout << "v3 (literal): " << v3 << std::endl;
    
    // All should be equal
    std::cout << "All equal? " << ((v1 == v2 && v2 == v3) ? "Yes" : "No") << std::endl;
    
    // Structured bindings
    auto [major, minor, micro] = v1;
    std::cout << "\nStructured binding: " << major << "." << minor << "." << micro << std::endl;
}

// Example 5: Compatibility report
void example_compatibility_report() {
    std::cout << "\n=== Compatibility Report ===" << std::endl;
    std::cout << version_compat::report() << std::endl;
}

// Example 6: Version-based code paths
void example_conditional_features() {
    std::cout << "\n=== Conditional Feature Usage ===" << std::endl;
    
    // Compile-time feature selection
    if constexpr (version_info::features::has_properties) {
        std::cout << "Properties API is available at compile time." << std::endl;
        std::cout << "Can use SDL_CreateProperties, etc." << std::endl;
    } else {
        std::cout << "Properties API not available at compile time." << std::endl;
        std::cout << "Need alternative implementation." << std::endl;
    }
    
    // Runtime feature selection
    if (version_info::features::available_at_runtime(3, 2, 0)) {
        std::cout << "\nSDL 3.2.0+ features available at runtime." << std::endl;
        // Safe to use 3.2.0+ features
    } else {
        std::cout << "\nSDL 3.2.0+ features NOT available at runtime." << std::endl;
        // Need fallback code
    }
}

// Example 7: Version requirements
void example_version_requirements() {
    std::cout << "\n=== Version Requirements ===" << std::endl;
    
    // This would cause compile error if SDL < 3.0.0
    version_compat::require_compile_time<3, 0, 0>();
    std::cout << "✓ SDL 3.0.0 requirement satisfied at compile time" << std::endl;
    
    // Runtime checks
    const version required(3, 2, 0);
    auto current = version_info::runtime();
    
    std::cout << "\nChecking runtime requirement: SDL " << required << std::endl;
    std::cout << "Current runtime version: " << current << std::endl;
    
    if (current >= required) {
        std::cout << "✓ Requirement satisfied" << std::endl;
    } else {
        std::cout << "✗ Requirement NOT satisfied" << std::endl;
        std::cout << "  Please update SDL to at least " << required << std::endl;
    }
}

// Example 8: Custom version handling
class ApplicationVersion {
    version sdl_min;
    version sdl_max;
    
public:
    ApplicationVersion(version min_ver, version max_ver = version(99, 99, 99))
        : sdl_min(min_ver), sdl_max(max_ver) {}
    
    bool is_compatible() const {
        auto current = version_info::runtime();
        return current >= sdl_min && current <= sdl_max;
    }
    
    std::string compatibility_message() const {
        auto current = version_info::runtime();
        std::ostringstream oss;
        
        oss << "Application requires SDL " << sdl_min;
        if (sdl_max.major() < 99) {
            oss << " to " << sdl_max;
        }
        oss << "\n";
        
        oss << "Current SDL version: " << current << " - ";
        if (is_compatible()) {
            oss << "Compatible ✓";
        } else if (current < sdl_min) {
            oss << "Too old ✗";
        } else {
            oss << "Too new ✗";
        }
        
        return oss.str();
    }
};

void example_custom_version_handling() {
    std::cout << "\n=== Custom Version Handling ===" << std::endl;
    
    // Application that requires SDL 3.2.x
    ApplicationVersion app_req(version(3, 2, 0), version(3, 2, 999));
    std::cout << app_req.compatibility_message() << std::endl;
    
    // Application that requires at least SDL 3.0.0
    ApplicationVersion app_min(300_v);
    std::cout << "\n" << app_min.compatibility_message() << std::endl;
}

int main() {
    std::cout << "=== SDL++ Version System Examples ===" << std::endl;
    
    // Run examples
    example_basic_version();
    example_version_comparison();
    example_feature_detection();
    example_version_construction();
    example_compatibility_report();
    example_conditional_features();
    example_version_requirements();
    example_custom_version_handling();
    
    std::cout << "\n✅ All version examples completed!" << std::endl;
    
    return 0;
}