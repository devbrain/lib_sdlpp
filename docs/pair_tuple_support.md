# Pair and Tuple Support in String Utilities

The `sdlpp::detail::append_to_stream` and `sdlpp::detail::build_message` functions now support `std::pair` and `std::tuple` types, making it easier to format complex data structures for logging and error messages.

## Basic Usage

### Pairs

```cpp
#include <sdlpp/log.hh>
#include <utility>

// Basic pair
auto coords = std::make_pair(10.5, 20.3);
SDLPP_LOG_APP("Mouse position:", coords);
// Output: Mouse position: (10.5, 20.3)

// Nested pairs
auto nested = std::make_pair(1, std::make_pair(2.5, "nested"));
SDLPP_LOG_APP("Nested data:", nested);
// Output: Nested data: (1, (2.5, nested))

// Pairs with formatters
auto data = std::make_pair(sdlpp::detail::hex(0xDEADBEEF), sdlpp::detail::bin(0b10101010));
SDLPP_LOG_APP("Formatted:", data);
// Output: Formatted: (0xdeadbeef, 0b10101010)
```

### Tuples

```cpp
#include <tuple>

// Empty tuple
auto empty = std::make_tuple();
SDLPP_LOG_APP("Empty:", empty);
// Output: Empty: ()

// Single element
auto single = std::make_tuple(42);
SDLPP_LOG_APP("Single:", single);
// Output: Single: (42)

// Multiple elements
auto rgb = std::make_tuple(255, 128, 64);
SDLPP_LOG_APP("Color RGB:", rgb);
// Output: Color RGB: (255, 128, 64)

// Complex tuple
auto complex = std::make_tuple(
    std::filesystem::path("/home/user"),
    std::optional<int>(42),
    100ms,
    true
);
SDLPP_LOG_APP("Complex:", complex);
// Output: Complex: (/home/user, 42, 100ms, true)
```

## Advanced Usage

### Mixed Combinations

```cpp
// Tuple containing pair
auto data = std::make_tuple(
    1, 
    std::make_pair(2.5, "pair in tuple"), 
    true
);
SDLPP_LOG_APP("Mixed:", data);
// Output: Mixed: (1, (2.5, pair in tuple), true)

// Pair containing tuple
auto pt = std::make_pair(std::make_tuple(1, 2, 3), "description");
SDLPP_LOG_APP("Pair with tuple:", pt);
// Output: Pair with tuple: ((1, 2, 3), description)
```

### With Optional and Variant

```cpp
// Optional pair
std::optional<std::pair<int, std::string>> opt = std::make_pair(42, "test");
SDLPP_LOG_APP("Optional:", opt);
// Output: Optional: (42, test)

// Empty optional pair
std::optional<std::pair<int, std::string>> empty;
SDLPP_LOG_APP("Empty optional:", empty);
// Output: Empty optional: nullopt

// Variant with pair
using var_t = std::variant<int, std::pair<double, bool>>;
var_t v = std::make_pair(3.14, false);
SDLPP_LOG_APP("Variant:", v);
// Output: Variant: (3.14, false)
```

### In Error Messages

```cpp
#include <sdlpp/error.hh>

// Using pairs for coordinate errors
auto pos = std::make_pair(1920, 1080);
sdlpp::set_error("Invalid window position:", pos, "exceeds display bounds");

// Using tuples for multi-value errors
auto config = std::make_tuple(800, 600, 32, 60);
sdlpp::set_error("Unsupported display mode:", config);
```

## Implementation Details

The pair and tuple support is implemented using C++20 concepts and `std::apply` for tuples:

- Pairs are detected using a concept that checks for `first` and `second` members
- Tuples are detected using `std::tuple_size`
- Elements are recursively formatted using `append_to_stream`
- The output format uses parentheses with comma separation: `(element1, element2, ...)`

This feature integrates seamlessly with all existing formatters and special type handling, including:
- `std::optional`
- `std::variant`
- `std::filesystem::path`
- `std::chrono` types
- Custom formatters (hex, oct, bin, uppercase, lowercase)