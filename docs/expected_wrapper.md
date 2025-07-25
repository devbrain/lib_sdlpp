# Expected Wrapper Implementation

## Overview
The SDL++ library now uses a wrapper around the `expected` type to provide future-proof error handling. This wrapper automatically selects between `std::expected` (when available in C++23) and `tl::expected` (as a fallback).

## Implementation Details

### Header Location
- **File**: `include/sdlpp/detail/expected.hh`
- **Purpose**: Provides a unified interface for expected types

### Type Aliases
```cpp
namespace sdlpp {
    // Main expected type
    template<typename T, typename E>
    using expected = /* std::expected or tl::expected */;
    
    // Error type used throughout the library
    using error_type = std::string;
    
    // Common result type for void operations
    using result = expected<void, error_type>;
    
    // Create unexpected values
    template<typename E>
    constexpr auto make_unexpected(E&& e);
}
```

### Usage Guidelines

1. **Include the wrapper header**:
   ```cpp
   #include <sdlpp/detail/expected.hh>
   ```

2. **Use `expected` without namespace prefix** (when inside `sdlpp` namespace):
   ```cpp
   expected<int, std::string> divide(int a, int b) {
       if (b == 0) {
           return make_unexpected("Division by zero");
       }
       return a / b;
   }
   ```

3. **Use `sdlpp::expected` from outside the namespace**:
   ```cpp
   sdlpp::expected<window, std::string> create_window();
   ```

4. **For void returns, use `result`**:
   ```cpp
   result do_something() {
       if (error_condition) {
           return make_unexpected("Error occurred");
       }
       return {};  // Success
   }
   ```

### Detection of Implementation

You can check which implementation is being used:

```cpp
if constexpr (sdlpp::using_std_expected) {
    // Using std::expected (C++23)
} else {
    // Using tl::expected (fallback)
}

// Or at runtime:
std::cout << "Using: " << sdlpp::expected_implementation() << "\n";
```

## Migration from Direct tl::expected Usage

The following replacements were made throughout the codebase:

| Old Code | New Code |
|----------|----------|
| `#include <tl/expected.hpp>` | `#include <sdlpp/detail/expected.hh>` |
| `tl::expected<T, E>` | `expected<T, E>` (in sdlpp namespace) |
| `tl::make_unexpected(e)` | `make_unexpected(e)` |
| `return tl::unexpected(e)` | `return make_unexpected(e)` |

## Benefits

1. **Future-Proof**: Automatically uses `std::expected` when available in C++23
2. **Consistent API**: Same interface regardless of underlying implementation
3. **Type Safety**: Strong typing with common error type
4. **No Direct Dependencies**: User code doesn't need to know about `tl::expected`

## Compatibility

- **C++20**: Uses `tl::expected` as fallback
- **C++23**: Uses `std::expected` when available
- **No Breaking Changes**: API remains the same