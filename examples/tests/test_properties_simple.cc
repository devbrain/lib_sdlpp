//
// Simple properties test to check SDL availability
//

#include <iostream>
#include <sdlpp/core/core.hh>
#include <sdlpp/config/properties.hh>
#include <sdlpp/core/version.hh>
#include <sdlpp/core/error.hh>

int main() {
    // Check SDL version
    auto version = sdlpp::version_info::runtime();
    std::cout << "SDL Version: " << version.major() << "." << version.minor() << "." << version.patch() << std::endl;
    
    // Check if properties API is available
    if (!sdlpp::version_info::features::has_properties) {
        std::cerr << "Properties API requires SDL 3.2.0 or later" << std::endl;
        return 1;
    }
    
    try {
        // Initialize SDL (minimal)
        sdlpp::init init(sdlpp::init_flags::none);
        
        if (!init.is_initialized()) {
            std::cerr << "Failed to initialize SDL" << std::endl;
            return 1;
        }
        
        // Create properties
        std::cout << "\nCreating properties..." << std::endl;
        auto props_result = sdlpp::properties::create();
        if (!props_result) {
            std::cerr << "Failed to create properties: " << props_result.error() << std::endl;
            return 1;
        }
        
        auto& props = *props_result;
    
        std::cout << "Properties created successfully" << std::endl;
        
        // Try to set a string property
        bool result = props.set("test", "value");
        if (!result) {
            std::cerr << "Failed to set string property" << std::endl;
        } else {
            std::cout << "String property set successfully" << std::endl;
            
            auto value = props.get_string("test", "default");
            std::cout << "Retrieved value: " << value << std::endl;
        }
        
        // Test other property types
        result = props.set("number", 42);
        std::cout << "Set number property: " << (result ? "success" : "failed") << std::endl;
        
        result = props.set("float", 3.14f);
        std::cout << "Set float property: " << (result ? "success" : "failed") << std::endl;
        
        result = props.set("bool", true);
        std::cout << "Set boolean property: " << (result ? "success" : "failed") << std::endl;
        
        // Check if property exists
        bool has_test = props.has("test");
        std::cout << "Has 'test' property: " << (has_test ? "yes" : "no") << std::endl;
        
        // Get property type
        auto type = props.get_type("test");
        std::cout << "Property 'test' type: " << static_cast<int>(type) << std::endl;
        
        // Test getting global properties
        std::cout << "\nTesting global properties:" << std::endl;
        auto global_props = sdlpp::properties::get_global();
        std::cout << "Got global properties reference" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}