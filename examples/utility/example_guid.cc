#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <unordered_map>
#include <sdlpp/core/core.hh>
#include <sdlpp/utility/guid.hh>

void demonstrate_guid_construction() {
    std::cout << "=== GUID Construction ===\n\n";
    
    // Default construction creates zero GUID
    sdlpp::guid zero_guid;
    std::cout << "Zero GUID: " << zero_guid << "\n";
    std::cout << "Is zero: " << std::boolalpha << zero_guid.is_zero() << "\n";
    std::cout << "Is valid: " << zero_guid.is_valid() << "\n\n";
    
    // Construction from raw data
    std::array<Uint8, 16> data = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
        0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
    };
    sdlpp::guid data_guid(data);
    std::cout << "GUID from data: " << data_guid << "\n";
    std::cout << "Is valid: " << data_guid.is_valid() << "\n\n";
    
    // Construction from string
    std::string guid_string = "deadbeefcafebabe1234567890abcdef";
    auto string_guid = sdlpp::guid::from_string(guid_string);
    if (string_guid) {
        std::cout << "GUID from string: " << *string_guid << "\n";
        std::cout << "Original string: " << guid_string << "\n";
        std::cout << "Converted back: " << string_guid->to_string() << "\n";
    }
    
    // Invalid string examples
    std::cout << "\nInvalid string tests:\n";
    auto too_short = sdlpp::guid::from_string("deadbeef");
    std::cout << "Too short string valid: " << too_short.has_value() << "\n";
    
    auto invalid_chars = sdlpp::guid::from_string("xxxx0000111122223333444455556666");
    std::cout << "Invalid chars valid: " << invalid_chars.has_value() << "\n";
}

void demonstrate_guid_comparison() {
    std::cout << "\n=== GUID Comparison ===\n\n";
    
    std::array<Uint8, 16> data1 = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::array<Uint8, 16> data2 = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    std::array<Uint8, 16> data3 = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
    
    sdlpp::guid g1(data1);
    sdlpp::guid g2(data2);
    sdlpp::guid g3(data3);
    sdlpp::guid g4(data1);  // Same as g1
    
    std::cout << "g1: " << g1 << "\n";
    std::cout << "g2: " << g2 << "\n";
    std::cout << "g3: " << g3 << "\n";
    std::cout << "g4: " << g4 << "\n\n";
    
    // Equality
    std::cout << "g1 == g4: " << (g1 == g4) << "\n";
    std::cout << "g1 == g2: " << (g1 == g2) << "\n";
    
    // Ordering (using spaceship operator)
    std::cout << "g1 < g2: " << (g1 < g2) << "\n";
    std::cout << "g1 < g3: " << (g1 < g3) << "\n";
    std::cout << "g2 > g1: " << (g2 > g1) << "\n";
    
    // Three-way comparison results
    auto cmp1 = g1 <=> g2;
    auto cmp2 = g1 <=> g4;
    
    std::cout << "\nThree-way comparison:\n";
    std::cout << "g1 <=> g2 is less: " << std::is_lt(cmp1) << "\n";
    std::cout << "g1 <=> g4 is equal: " << std::is_eq(cmp2) << "\n";
}

void demonstrate_guid_containers() {
    std::cout << "\n=== GUID in Containers ===\n\n";
    
    // GUIDs can be used as keys in ordered containers
    std::map<sdlpp::guid, std::string> guid_names;
    
    auto g1 = sdlpp::guid::from_string("11111111111111111111111111111111").value();
    auto g2 = sdlpp::guid::from_string("22222222222222222222222222222222").value();
    auto g3 = sdlpp::guid::from_string("33333333333333333333333333333333").value();
    
    guid_names[g1] = "First Controller";
    guid_names[g2] = "Second Controller";
    guid_names[g3] = "Third Controller";
    
    std::cout << "Ordered map of GUIDs:\n";
    for (const auto& [guid, name] : guid_names) {
        std::cout << "  " << guid << " -> " << name << "\n";
    }
    
    // GUIDs can also be used in unordered containers
    std::unordered_map<sdlpp::guid, int> guid_scores;
    guid_scores[g1] = 100;
    guid_scores[g2] = 200;
    guid_scores[g3] = 150;
    
    std::cout << "\nUnordered map of GUIDs:\n";
    for (const auto& [guid, score] : guid_scores) {
        std::cout << "  " << guid << " -> " << score << " points\n";
    }
    
    // Demonstrate hash function
    std::hash<sdlpp::guid> hasher;
    std::cout << "\nHash values:\n";
    std::cout << "  Hash of g1: " << std::hex << hasher(g1) << std::dec << "\n";
    std::cout << "  Hash of g2: " << std::hex << hasher(g2) << std::dec << "\n";
    std::cout << "  Hash of zero: " << std::hex << hasher(sdlpp::guid::zero()) << std::dec << "\n";
}

void demonstrate_guid_info() {
    std::cout << "\n=== GUID Information ===\n\n";
    
    // Example joystick GUID patterns (these are just examples)
    std::vector<std::string> example_guids = {
        "030000005e040000ea02000000000000",  // Example Xbox controller
        "030000004c050000c405000000000000",  // Example PlayStation controller
        "00000000000000000000000000000000",  // Zero GUID
    };
    
    for (const auto& guid_str : example_guids) {
        auto guid = sdlpp::guid::from_string(guid_str);
        if (guid) {
            std::cout << "GUID: " << *guid << "\n";
            
            auto info = sdlpp::get_joystick_guid_info(*guid);
            std::cout << "  Vendor ID: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                      << info.vendor << std::dec << "\n";
            std::cout << "  Product ID: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                      << info.product << std::dec << "\n";
            std::cout << "  Version: " << info.version << "\n";
            std::cout << "  CRC16: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                      << info.crc16 << std::dec << "\n";
            std::cout << "  Is valid: " << std::boolalpha << info.is_valid() << "\n\n";
        }
    }
}

void demonstrate_practical_usage() {
    std::cout << "\n=== Practical GUID Usage ===\n\n";
    
    // Simulate a gamepad configuration system
    struct GamepadConfig {
        std::string name;
        float deadzone;
        bool inverted_y;
    };
    
    // Store configurations by GUID
    std::map<sdlpp::guid, GamepadConfig> configs;
    
    // Add some example configurations
    auto xbox_guid = sdlpp::guid::from_string("030000005e040000ea02000000000000").value();
    auto ps_guid = sdlpp::guid::from_string("030000004c050000c405000000000000").value();
    
    configs[xbox_guid] = {"Xbox Controller", 0.15f, false};
    configs[ps_guid] = {"PlayStation Controller", 0.10f, true};
    
    // Simulate looking up configuration
    std::cout << "Gamepad configurations:\n";
    for (const auto& [guid, config] : configs) {
        std::cout << "GUID: " << guid << "\n";
        std::cout << "  Name: " << config.name << "\n";
        std::cout << "  Deadzone: " << config.deadzone << "\n";
        std::cout << "  Inverted Y: " << std::boolalpha << config.inverted_y << "\n\n";
    }
    
    // Check if a specific GUID has configuration
    auto test_guid = xbox_guid;
    if (configs.find(test_guid) != configs.end()) {
        std::cout << "Found configuration for " << test_guid << "\n";
        std::cout << "Using settings: " << configs[test_guid].name << "\n";
    }
}

int main() {
    try {
        // Initialize SDL
        sdlpp::init sdl_init(sdlpp::init_flags::joystick | sdlpp::init_flags::gamepad);
        
        std::cout << "SDL++ GUID Example\n";
        std::cout << "==================\n\n";
        
        demonstrate_guid_construction();
        demonstrate_guid_comparison();
        demonstrate_guid_containers();
        demonstrate_guid_info();
        demonstrate_practical_usage();
        
        std::cout << "\n=== Summary ===\n";
        std::cout << "GUIDs are used to uniquely identify input devices across sessions.\n";
        std::cout << "They can be converted to/from strings for storage and display.\n";
        std::cout << "The spaceship operator (<=>) provides complete ordering support.\n";
        std::cout << "GUIDs work seamlessly with both ordered and unordered containers.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}