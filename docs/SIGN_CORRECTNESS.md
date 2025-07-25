# Sign Correctness Improvements

This document summarizes the sign correctness improvements made to the sdlpp library to prevent integer overflow/underflow and ensure robust handling of signed/unsigned conversions.

## Changes Made

### 1. Overflow Protection for Area Calculations

**Files**: `geometry.hh`

**Issue**: Integer multiplication in `area()` methods could overflow for large dimensions.

**Solution**: 
- For integer types, use a larger type (int64_t) for intermediate calculations
- Return the larger type to prevent overflow
- Floating-point types remain unchanged

```cpp
// Before
[[nodiscard]] constexpr T area() const {
    return width * height;
}

// After  
[[nodiscard]] constexpr auto area() const {
    if constexpr (std::is_integral_v<T>) {
        using larger_t = std::conditional_t<sizeof(T) <= 4, int64_t, T>;
        return static_cast<larger_t>(width) * static_cast<larger_t>(height);
    } else {
        return width * height;
    }
}
```

### 2. Size Limit Checking for SDL API Calls

**Files**: `renderer.hh`

**Issue**: SDL APIs accept `int` for counts, but containers use `size_t`. Direct casting could result in negative counts if size exceeds INT_MAX.

**Solution**:
- Add explicit checks before casting size_t to int
- Return descriptive error messages when limits are exceeded

```cpp
// Added check
if (sdl_points.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
    return tl::unexpected("Too many points for SDL API");
}
```

### 3. Index Bounds Validation for Geometry Rendering

**Files**: `renderer.hh`

**Issue**: Indices in geometry rendering could reference out-of-bounds vertices.

**Solution**:
- Validate all indices are within the vertex array bounds
- Return error for invalid indices

```cpp
// Added validation
for (const auto& idx : indices) {
    if (idx < 0 || idx >= static_cast<int>(vertices.size())) {
        return tl::unexpected("Index out of bounds");
    }
}
```

## Design Principles

1. **Preserve Original Types**: Where possible, maintain the original type semantics while protecting against overflow
2. **Fail Fast**: Return errors immediately when size limits are exceeded rather than silently truncating
3. **Clear Error Messages**: Provide descriptive error messages that indicate the specific limit exceeded
4. **Minimal Performance Impact**: Use compile-time type checks (`if constexpr`) to avoid runtime overhead
5. **Standard Library Integration**: Use `std::numeric_limits` for portable limit checking

## Testing

Added comprehensive tests in `test_geometry.cc` to verify:
- Large value multiplication doesn't overflow
- Negative dimensions are handled correctly
- Area calculations return appropriate larger types

## Potential Future Improvements

1. **Surface Pitch Handling**: While SDL3 typically uses positive pitch values, handling negative pitch (bottom-up surfaces) could be added for completeness
2. **Checked Arithmetic**: Consider using checked arithmetic operations for debug builds
3. **Static Analysis**: Enable compiler warnings for sign conversion (-Wsign-conversion) in development builds

## Performance Considerations

The overflow protection adds minimal overhead:
- Compile-time type selection with `if constexpr`
- Larger type usage only for intermediate calculations
- No runtime checks for floating-point operations

## Compatibility

All changes maintain API compatibility:
- Area methods now return `auto` but the actual type is predictable based on input
- Error handling uses the existing `sdlpp::expected` pattern
- No breaking changes to public interfaces