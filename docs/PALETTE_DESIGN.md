# Palette Design Documentation

## Overview

The palette system in sdlpp provides a type-safe wrapper around SDL3's palette functionality with proper ownership semantics. The design distinguishes between owned palettes and non-owning references, similar to unique_ptr and raw pointers.

## Class Hierarchy

### `const_palette_ref`
- Non-owning const reference to a palette
- Read-only access to palette data
- Safe to use with palettes owned by surfaces or other objects
- Cannot modify the palette

### `palette_ref` (inherits from `const_palette_ref`)
- Non-owning mutable reference to a palette
- Full read/write access to palette data
- Can modify colors in the palette
- Use with caution as the palette lifetime is not managed

### `palette`
- Owning RAII wrapper for SDL_Palette
- Automatically destroys the palette when out of scope
- Full read/write access
- Can be implicitly converted to both `palette_ref` and `const_palette_ref`

## Usage Examples

### Creating and Using Owned Palettes

```cpp
// Create a 256-color palette
auto pal = palette::create(256);
if (pal) {
    // Set individual colors
    pal->set_color(0, colors::black);
    pal->set_color(255, colors::white);
    
    // Create grayscale palette
    auto gray_pal = palette::create_grayscale(8);  // 256 shades
    
    // Create color ramp
    auto ramp = palette::create_ramp(colors::red, colors::blue, 128);
}
```

### Working with Surface Palettes

```cpp
// Create indexed surface
auto surf = surface::create_rgb(320, 200, pixel_format_enum::INDEX8);

// Set a palette
auto pal = palette::create_grayscale(8);
surf->set_palette(*pal);  // Surface references but doesn't own the palette

// Get mutable reference from non-const surface
palette_ref mut_ref = surf->get_palette();
mut_ref.set_color(0, colors::red);  // Modifies the surface's palette

// Get const reference from const surface
const surface& const_surf = *surf;
const_palette_ref const_ref = const_surf.get_palette();
auto color = const_ref.get_color(0);  // Read-only access
// const_ref.set_color(0, colors::blue);  // Compile error!
```

## Design Rationale

### Why Three Classes?

1. **Ownership Safety**: `palette` manages lifetime, references don't
2. **Const Correctness**: `const_palette_ref` enforces read-only access
3. **Flexibility**: Can modify palettes owned by surfaces via `palette_ref`

### Key Design Decisions

1. **Inheritance**: `palette_ref` inherits from `const_palette_ref` to avoid code duplication
2. **Implicit Conversions**: `palette` can implicitly convert to references for convenience
3. **Surface Integration**: Surfaces return appropriate reference type based on constness
4. **SDL Compatibility**: Maintains zero-cost abstraction over SDL3 API

## Safety Considerations

1. **Reference Lifetime**: palette_ref and const_palette_ref do not manage lifetime
2. **Surface Palettes**: Modifying a surface's palette affects rendering immediately
3. **Thread Safety**: No additional thread safety beyond SDL3's guarantees

## Best Practices

1. Use `palette` when you need to own and manage a palette
2. Use `const_palette_ref` when you only need to read palette data
3. Use `palette_ref` carefully when you need to modify a palette you don't own
4. Always check validity with `is_valid()` before using references
5. Prefer factory methods (`create_grayscale`, `create_ramp`) for common patterns