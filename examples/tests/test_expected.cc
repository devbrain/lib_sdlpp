//
// Test program for expected wrapper
//

#include <iostream>
#include <sdlpp/detail/expected.hh>

// Test function that returns expected
sdlpp::expected<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return sdlpp::make_unexpected("Division by zero");
    }
    return a / b;
}

// Test function that returns void expected
sdlpp::result do_something(bool should_fail) {
    if (should_fail) {
        return sdlpp::make_unexpected("Operation failed");
    }
    return {};  // Success
}

int main() {
    std::cout << "Testing sdlpp::expected wrapper\n";
    std::cout << "Using implementation: " << sdlpp::expected_implementation() << "\n";
    std::cout << "Using std::expected: " << (sdlpp::using_std_expected ? "yes" : "no") << "\n\n";
    
    // Test successful operation
    auto result1 = divide(10, 2);
    if (result1) {
        std::cout << "10 / 2 = " << *result1 << "\n";
    } else {
        std::cout << "Error: " << result1.error() << "\n";
    }
    
    // Test error case
    auto result2 = divide(10, 0);
    if (result2) {
        std::cout << "10 / 0 = " << *result2 << "\n";
    } else {
        std::cout << "Error: " << result2.error() << "\n";
    }
    
    // Test void expected (success)
    auto result3 = do_something(false);
    if (result3) {
        std::cout << "Operation succeeded\n";
    } else {
        std::cout << "Error: " << result3.error() << "\n";
    }
    
    // Test void expected (failure)
    auto result4 = do_something(true);
    if (result4) {
        std::cout << "Operation succeeded\n";
    } else {
        std::cout << "Error: " << result4.error() << "\n";
    }
    
    // Test error_type
    sdlpp::error_type err = "Custom error message";
    std::cout << "\nError type test: " << err << "\n";
    
    return 0;
}