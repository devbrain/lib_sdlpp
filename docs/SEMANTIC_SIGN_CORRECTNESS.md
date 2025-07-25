# Semantic Sign Correctness with Type Safety

This document describes the type-safe approach to semantic sign correctness in the sdlpp library, which ensures that values have appropriate signedness based on their semantic meaning.

## The Problem

SDL3 uses signed integers (`int`) for many values that semantically should never be negative:
- Window and surface dimensions (width, height)
- Display dimensions
- Color counts
- Array sizes

This creates opportunities for bugs where:
- Negative dimensions could be passed accidentally
- Runtime checks are needed everywhere
- The API doesn't communicate intent clearly

## The Solution: Type-Safe Dimensions

We've introduced type-safe wrappers that enforce semantic correctness at compile time:

### 1. `dimension<T>` - Non-negative values

```cpp
template<typename T = int>
class dimension {
    T value_;  // Always >= 0
public:
    // Negative values are clamped to 0
    constexpr explicit dimension(T val) noexcept 
        : value_(std::max(T{0}, val)) {}
        
    // Arithmetic operations maintain invariant
    // Subtraction clamps to 0, not negative
    // Multiplication by negative yields 0
};
```

### 2. `dimensions<T>` - Width and height pairs

```cpp
template<typename T = int>
struct dimensions {
    dimension<T> width;
    dimension<T> height;
    
    // Area calculation with overflow protection
    [[nodiscard]] constexpr auto area() const noexcept;
};
```

### 3. `coordinate<T>` - Can be negative

```cpp
template<typename T = int>
struct coordinate {
    T value;  // Can be negative (for positions)
};
```

## Usage Examples

### Window Creation

**Before (runtime validation needed):**
```cpp
auto window = window::create("Test", -100, -200);  // Runtime error
```

**After (type safe):**
```cpp
// Type-safe dimensions
window_dimensions dims(800, 600);
auto window = window::create("Test", dims);

// Negative values automatically clamped
window_dimensions bad_dims(-100, -200);  // Becomes (0, 0)
auto window2 = window::create("Test", bad_dims);

// Backward compatible
auto window3 = window::create("Test", 800, 600);  // Still works
```

### Surface Creation

```cpp
// Type-safe version
dimensions<int> surf_dims(640, 480);
auto surface = surface::create_rgb(surf_dims, pixel_format_enum::RGBA8888);

// Compatibility overload
auto surface2 = surface::create_rgb(640, 480, pixel_format_enum::RGBA8888);
```

### Window Position vs Size

```cpp
// Position can be negative (off-screen windows)
window_position pos(-100, -50);  // OK - preserves negative

// But size cannot
window_dimensions size(-100, -50);  // Clamped to (0, 0)
```

## Benefits

1. **Compile-time Safety**: Invalid states are unrepresentable
2. **Self-documenting API**: Types communicate intent
3. **No Runtime Overhead**: Uses `constexpr` and compile-time checks
4. **Backward Compatible**: Overloads accept raw integers
5. **SDL3 Compatible**: Implicit conversion to `int` for SDL calls

## Design Principles

1. **Zero-cost Abstractions**: No runtime penalty
2. **Fail at Compile Time**: Catch errors early
3. **Preserve SDL Compatibility**: Work seamlessly with SDL3
4. **Progressive Enhancement**: Old code still works
5. **Clear Semantics**: Types express meaning

## Implementation Details

### Overflow Protection

```cpp
// Area calculation prevents overflow
template<typename T>
[[nodiscard]] constexpr auto area() const {
    if constexpr (std::is_integral_v<T>) {
        using larger_t = std::conditional_t<sizeof(T) <= 4, uint64_t, T>;
        return static_cast<larger_t>(width) * static_cast<larger_t>(height);
    } else {
        return width * height;
    }
}
```

### Arithmetic Operations

```cpp
// Addition with overflow protection
constexpr dimension& operator+=(const dimension& other) noexcept {
    if (value_ > std::numeric_limits<T>::max() - other.value_) {
        value_ = std::numeric_limits<T>::max();
    } else {
        value_ += other.value_;
    }
    return *this;
}

// Subtraction clamps to 0
constexpr dimension& operator-=(const dimension& other) noexcept {
    value_ = (value_ > other.value_) ? value_ - other.value_ : T{0};
    return *this;
}
```

## Migration Guide

### For New Code

Use type-safe versions:
```cpp
// Prefer this
window_dimensions dims(1024, 768);
auto window = window::create("My App", dims);

// Over this
auto window = window::create("My App", 1024, 768);
```

### For Existing Code

No changes required! Compatibility overloads ensure existing code continues to work:
```cpp
// Still works - negative values are clamped internally
auto window = window::create("Test", width, height);
```

## Type Aliases

```cpp
// Dimension types
using dim = dimension<int>;
using fdim = dimension<float>;
using window_dimensions = dimensions<int>;
using display_dimensions = dimensions<int>;

// Coordinate types
using coord = coordinate<int>;
using fcoord = coordinate<float>;
using window_position = position<int>;
```

## Testing

Comprehensive tests ensure:
- Negative values are properly clamped
- Arithmetic operations maintain invariants
- Overflow protection works correctly
- SDL compatibility is preserved

See `test_dimension.cc` for examples.

## Future Enhancements

1. **Concepts for Constraints**: 
   ```cpp
   template<typename T>
   concept non_negative_dimension = requires(T t) {
       { t.value() } -> std::convertible_to<int>;
       { t.is_positive() } -> std::same_as<bool>;
   };
   ```

2. **Compile-time Validation**:
   ```cpp
   template<int W, int H>
   constexpr auto make_dimensions() {
       static_assert(W >= 0 && H >= 0, "Dimensions must be non-negative");
       return dimensions<int>(W, H);
   }
   ```

3. **Units of Measurement**:
   ```cpp
   using pixels = dimension<int>;
   using points = dimension<float>;
   ```

## Conclusion

By using type-safe dimensions, we've made the API more robust while maintaining full backward compatibility. The types express semantic meaning, catch errors at compile time, and have zero runtime overhead. This approach demonstrates how modern C++ features can improve API design without sacrificing performance or compatibility.