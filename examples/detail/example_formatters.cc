#include <failsafe/detail/string_utils.hh>
#include <sdlpp/core/error.hh>
#include <iostream>
#include <iomanip>
#include <cstdint>

int main() {
    std::cout << "SDL++ String Formatters Example\n";
    std::cout << "===============================\n\n";
    
    // Basic formatters
    std::cout << "=== Basic Formatters ===\n";
    std::cout << failsafe::detail::build_message("Uppercase:", 
                                              failsafe::detail::uppercase("hello world")) << "\n";
    std::cout << failsafe::detail::build_message("Lowercase:", 
                                              failsafe::detail::lowercase("HELLO WORLD")) << "\n";
    
    // Hexadecimal formatting
    std::cout << "\n=== Hexadecimal Formatting ===\n";
    std::cout << failsafe::detail::build_message("Default hex:", 
                                              failsafe::detail::hex(255)) << "\n";
    std::cout << failsafe::detail::build_message("Uppercase hex:", 
                                              failsafe::detail::hex(255, 0, true, true)) << "\n";
    std::cout << failsafe::detail::build_message("Padded hex:", 
                                              failsafe::detail::hex(15, 4)) << "\n";
    std::cout << failsafe::detail::build_message("No prefix:", 
                                              failsafe::detail::hex(0xABCD, 0, false)) << "\n";
    
    // Memory addresses
    std::cout << "\n=== Memory Addresses ===\n";
    int value = 42;
    int* ptr = &value;
    std::cout << failsafe::detail::build_message("Pointer:", 
                                              failsafe::detail::hex(ptr)) << "\n";
    int* null_ptr = nullptr;
    std::cout << failsafe::detail::build_message("Null pointer:", 
                                              failsafe::detail::hex(null_ptr)) << "\n";
    
    // Octal formatting
    std::cout << "\n=== Octal Formatting ===\n";
    std::cout << failsafe::detail::build_message("File permissions:", 
                                              failsafe::detail::oct(0755)) << "\n";
    std::cout << failsafe::detail::build_message("Octal value:", 
                                              failsafe::detail::oct(64)) << "\n";
    std::cout << failsafe::detail::build_message("No prefix:", 
                                              failsafe::detail::oct(8, 0, false)) << "\n";
    
    // Binary formatting
    std::cout << "\n=== Binary Formatting ===\n";
    std::cout << failsafe::detail::build_message("Binary:", 
                                              failsafe::detail::bin(42)) << "\n";
    std::cout << failsafe::detail::build_message("8-bit padded:", 
                                              failsafe::detail::bin(42, 8)) << "\n";
    std::cout << failsafe::detail::build_message("Grouped by 4:", 
                                              failsafe::detail::bin(0xFF, 0, true, 4)) << "\n";
    std::cout << failsafe::detail::build_message("Grouped by 2:", 
                                              failsafe::detail::bin(0b10101010, 0, true, 2)) << "\n";
    
    // Combining formatters
    std::cout << "\n=== Combining Formatters ===\n";
    std::cout << failsafe::detail::build_message("Uppercase hex:", 
                                              failsafe::detail::uppercase(failsafe::detail::hex(0xabcd))) << "\n";
    
    // Real-world example
    std::cout << "\n=== Real-World Example ===\n";
    uint32_t error_code = 0x80001234;
    uint8_t status_flags = 0b10110101;
    uint16_t permissions = 0644;
    
    std::cout << failsafe::detail::build_message(
        "Error", failsafe::detail::uppercase(failsafe::detail::hex(error_code, 8)),
        "status:", failsafe::detail::bin(status_flags, 0, true, 4),
        "perms:", failsafe::detail::oct(permissions)
    ) << "\n";
    
    // Using with SDL++ error system
    std::cout << "\n=== Integration with Error System ===\n";
    (void)sdlpp::set_error("Failed with code", 
                           failsafe::detail::hex(0xDEAD, 4, true, true),
                           "at address", 
                           failsafe::detail::hex(ptr));
    std::cout << "Error: " << sdlpp::get_error() << "\n";
    
    // Edge cases
    std::cout << "\n=== Edge Cases ===\n";
    std::cout << failsafe::detail::build_message("Zero hex:", 
                                              failsafe::detail::hex(0)) << "\n";
    std::cout << failsafe::detail::build_message("Zero oct:", 
                                              failsafe::detail::oct(0)) << "\n";
    std::cout << failsafe::detail::build_message("Zero bin:", 
                                              failsafe::detail::bin(0)) << "\n";
    std::cout << failsafe::detail::build_message("Negative as hex:", 
                                              failsafe::detail::hex(-1, 8)) << "\n";
    
    // Custom formatting demonstration
    std::cout << "\n=== Custom Formatting ===\n";
    
    // Simulate a register dump
    std::cout << "Register dump:\n";
    for (int i = 0; i < 4; ++i) {
        uint32_t reg_value = 0x1000 + static_cast<uint32_t>(i) * 0x1111;
        std::cout << "  R" << i << ": " 
                  << failsafe::detail::build_message(
                      failsafe::detail::hex(reg_value, 8, true, true),
                      "(" + failsafe::detail::build_message(failsafe::detail::bin(reg_value, 16, false, 8)) + ")")
                  << "\n";
    }
    
    return 0;
}