# Geometry Refactoring Implementation Plan

## Overview

This document provides a detailed, phased implementation plan for refactoring SDL++'s geometry system based on the proposal in `geometry_refactoring_proposal.md`. Each phase is designed to be independently testable with clear success criteria.

## Implementation Phases

### Phase 1: Extract and Enhance Geometry Concepts (1-2 days)

**Goal**: Create a standalone geometry concepts header with zero dependencies.

**Tasks**:
1. Create `include/sdlpp/utility/geometry_concepts.hh` with:
   - Core concepts: `point_like`, `size_like`, `rect_like`, `line_like`
   - Utility functions: `get_x()`, `get_y()`, `get_width()`, `get_height()`, `get_area()`
   - SDL conversion helpers: `to_sdl_point()`, `to_sdl_rect()`, `to_sdl_fpoint()`, `to_sdl_frect()`
   
2. Add additional concepts for completeness:
   ```cpp
   template<typename T>
   concept circle_like = requires(T t) {
       typename T::value_type;
       { t.x } -> std::convertible_to<typename T::value_type>;
       { t.y } -> std::convertible_to<typename T::value_type>;
       { t.radius } -> std::convertible_to<typename T::value_type>;
   };
   
   template<typename T>
   concept polygon_like = requires(T t) {
       typename T::value_type;
       { t.size() } -> std::convertible_to<std::size_t>;
       { t[0] } -> point_like;
   };
   ```

3. Add arithmetic type traits:
   ```cpp
   template<typename T>
   concept arithmetic_point_like = point_like<T> && 
       std::is_arithmetic_v<typename T::value_type>;
   ```

**Test Criteria**:
- [ ] Header compiles independently without any SDL++ dependencies
- [ ] Concepts correctly identify conforming types
- [ ] Concepts correctly reject non-conforming types
- [ ] Utility functions work with test types
- [ ] No SDL headers are included

**Test Files**:
- `test/utility/test_geometry_concepts.cc`
- `test/utility/test_geometry_concepts_compile.cc`

---

### Phase 2: Create Concrete Geometry Types (1-2 days)

**Goal**: Implement built-in geometry types that satisfy the concepts.

**Tasks**:
1. Create `include/sdlpp/utility/geometry_types.hh` with:
   - Template types: `point<T>`, `size<T>`, `rect<T>`, `line<T>`, `circle<T>`
   - Common operations (arithmetic, comparison, hashing)
   - Type aliases: `point_i`, `point_f`, `size_i`, etc.
   
2. Add additional functionality:
   - Conversion constructors between numeric types
   - Stream operators for debugging
   - `std::hash` specializations
   - Deduction guides

3. Add geometric algorithms:
   ```cpp
   template<typename T>
   constexpr rect<T> rect<T>::intersect(const rect& other) const;
   
   template<typename T>
   constexpr rect<T> rect<T>::unite(const rect& other) const;
   
   template<typename T>
   constexpr bool rect<T>::contains(const rect& other) const;
   ```

**Test Criteria**:
- [ ] All types satisfy their corresponding concepts
- [ ] Arithmetic operations work correctly
- [ ] Conversion between types works
- [ ] Hash specializations work
- [ ] Static assertions pass

**Test Files**:
- `test/utility/test_geometry_types.cc`
- `test/utility/test_geometry_operations.cc`

---

### Phase 3: Create Conditional Inclusion System (0.5 days)

**Goal**: Implement the conditional compilation mechanism.

**Tasks**:
1. Create `include/sdlpp/utility/geometry.hh`:
   ```cpp
   #pragma once
   #include <sdlpp/utility/geometry_concepts.hh>
   
   #ifndef SDLPP_NO_BUILTIN_GEOMETRY
   #include <sdlpp/utility/geometry_types.hh>
   #endif
   
   // Common algorithms that work with any geometry
   #include <sdlpp/utility/geometry_algorithms.hh>
   ```

2. Create `include/sdlpp/utility/geometry_algorithms.hh` with:
   - Distance calculations
   - Intersection tests
   - Bounding box calculations
   - Interpolation functions

3. Add feature detection macros:
   ```cpp
   #ifdef SDLPP_NO_BUILTIN_GEOMETRY
   #define SDLPP_HAS_BUILTIN_GEOMETRY 0
   #else
   #define SDLPP_HAS_BUILTIN_GEOMETRY 1
   #endif
   ```

**Test Criteria**:
- [ ] Library compiles with `SDLPP_NO_BUILTIN_GEOMETRY` defined
- [ ] Library compiles without the macro (includes types)
- [ ] Feature detection macros work correctly
- [ ] No multiply defined symbols

**Test Files**:
- `test/utility/test_geometry_conditional.cc`
- Build system tests with different configurations

---

### Phase 4: Update Core Components - Window (1 day)

**Goal**: Update window class to use geometry concepts.

**Tasks**:
1. Update `include/sdlpp/video/window.hh`:
   - Change includes to use `geometry_concepts.hh` only
   - Convert methods to templates with concept constraints
   - Add tuple return overloads
   
2. Update specific methods:
   ```cpp
   template<size_like S>
   static expected<window, std::string> create(const std::string& title, const S& size);
   
   template<point_like P>
   expected<void, std::string> set_position(const P& pos);
   
   template<typename P = void>
   [[nodiscard]] auto get_position() const;
   ```

3. Update existing non-template overloads to call template versions

**Test Criteria**:
- [ ] Window class compiles without built-in geometry types
- [ ] All existing tests pass
- [ ] New template methods work with custom types
- [ ] Tuple returns work correctly
- [ ] No performance regression

**Test Files**:
- `test/video/test_window_geometry.cc`
- Update existing `test/video/test_window.cc`

---

### Phase 5: Update Core Components - Renderer (2 days)

**Goal**: Update renderer class to use geometry concepts.

**Tasks**:
1. Update `include/sdlpp/video/renderer.hh`:
   - Drawing methods: `draw_point()`, `draw_line()`, `draw_rect()`, `fill_rect()`
   - Viewport and clip rect methods
   - Render target methods
   
2. Add overloads for collections:
   ```cpp
   template<typename Container>
   requires std::ranges::range<Container> && 
            point_like<std::ranges::range_value_t<Container>>
   expected<void, std::string> draw_points(const Container& points);
   ```

3. Update texture rendering methods:
   ```cpp
   template<rect_like SrcRect, rect_like DstRect>
   expected<void, std::string> copy(texture& tex, 
                                   const SrcRect* src, 
                                   const DstRect* dst);
   ```

**Test Criteria**:
- [ ] Renderer compiles without built-in geometry
- [ ] All drawing methods work with custom types
- [ ] Performance is not impacted
- [ ] Existing code continues to work

**Test Files**:
- `test/video/test_renderer_geometry.cc`
- Performance benchmarks

---

### Phase 6: Update Other Video Components (2 days)

**Goal**: Update remaining video subsystem components.

**Tasks**:
1. Update `include/sdlpp/video/surface.hh`:
   - Blitting methods
   - Clipping methods
   - Fill methods
   
2. Update `include/sdlpp/video/texture.hh`:
   - Update methods with rect parameters
   
3. Update `include/sdlpp/video/display.hh`:
   - Display bounds methods
   - Display mode methods

**Test Criteria**:
- [ ] All video components compile without built-in geometry
- [ ] Existing tests pass
- [ ] New geometry-agnostic tests pass

**Test Files**:
- `test/video/test_surface_geometry.cc`
- `test/video/test_texture_geometry.cc`
- `test/video/test_display_geometry.cc`

---

### Phase 7: Update Input Components (1 day)

**Goal**: Update input subsystem to use geometry concepts.

**Tasks**:
1. Update mouse-related classes:
   - `include/sdlpp/input/mouse.hh`
   - Motion events with position
   - Cursor position methods
   
2. Update touch-related classes:
   - `include/sdlpp/input/touch.hh`
   - Touch point positions
   
3. Update gamepad/joystick classes:
   - Touchpad positions
   - Motion sensor data

**Test Criteria**:
- [ ] Input components compile without built-in geometry
- [ ] Event handling works with custom types
- [ ] No breaking changes to existing code

**Test Files**:
- `test/input/test_mouse_geometry.cc`
- `test/input/test_touch_geometry.cc`

---

### Phase 8: Integration Testing (2 days)

**Goal**: Verify the system works with external geometry libraries.

**Tasks**:
1. Create integration tests with:
   - GLM (glm::vec2, glm::ivec2)
   - Eigen (with adapters)
   - Custom simple geometry types
   
2. Create example programs:
   - `examples/geometry/example_geometry_glm.cc`
   - `examples/geometry/example_geometry_eigen.cc`
   - `examples/geometry/example_geometry_custom.cc`
   - `examples/geometry/example_geometry_none.cc`

3. Create compile-time tests:
   - Verify library compiles with various configurations
   - Check for any unintended dependencies

**Test Criteria**:
- [ ] GLM integration works seamlessly
- [ ] Eigen integration works with adapters
- [ ] Custom types work correctly
- [ ] Library works with NO geometry types
- [ ] Examples compile and run

**Test Files**:
- `test/integration/test_geometry_glm.cc`
- `test/integration/test_geometry_eigen.cc`
- `test/integration/test_geometry_custom.cc`

---

### Phase 9: Documentation and Migration (1 day)

**Goal**: Document the new system and provide migration guides.

**Tasks**:
1. Update API documentation:
   - Document all concepts
   - Document type requirements
   - Provide examples for each component
   
2. Create migration guide:
   - Step-by-step instructions
   - Common pitfalls
   - Performance considerations
   
3. Update README and examples:
   - Show basic usage
   - Show advanced usage
   - Show integration examples

**Deliverables**:
- [ ] Updated Doxygen comments
- [ ] Migration guide document
- [ ] Updated README
- [ ] Tutorial examples

---

### Phase 10: Performance Validation (1 day)

**Goal**: Ensure zero-overhead abstraction promise is kept.

**Tasks**:
1. Create benchmarks:
   - Direct SDL calls vs concept-based calls
   - Built-in types vs external types
   - Template instantiation overhead
   
2. Profile compilation:
   - Measure compile time impact
   - Measure binary size impact
   - Check for code bloat

3. Optimize if needed:
   - Add explicit instantiations
   - Use `if constexpr` for optimization
   - Consider `extern template`

**Test Criteria**:
- [ ] No runtime overhead vs direct SDL calls
- [ ] Compile time increase < 10%
- [ ] Binary size increase < 5%
- [ ] All optimizations documented

**Test Files**:
- `benchmark/bench_geometry_overhead.cc`
- `benchmark/bench_compile_time.sh`

---

## Risk Mitigation

### Technical Risks

1. **Compile Time Impact**
   - Mitigation: Use explicit instantiation for common types
   - Fallback: Provide non-template overloads for built-in types

2. **Error Message Quality**
   - Mitigation: Add static_assert with clear messages
   - Fallback: Provide diagnostic helpers

3. **Breaking Changes**
   - Mitigation: Keep old signatures as delegates to new ones
   - Fallback: Provide compatibility header

### Schedule Risks

1. **Unforeseen Dependencies**
   - Mitigation: Start with core components first
   - Fallback: Implement in stages, release incrementally

2. **Testing Complexity**
   - Mitigation: Automate testing with multiple configurations
   - Fallback: Focus on most common use cases

## Success Metrics

1. **Functionality**
   - All existing tests pass
   - New geometry-agnostic tests pass
   - Integration tests with 3+ libraries pass

2. **Performance**
   - Zero runtime overhead
   - < 10% compile time increase
   - < 5% binary size increase

3. **Usability**
   - Clean error messages
   - Clear documentation
   - Working examples

4. **Adoption**
   - Successfully used with GLM
   - Successfully used with custom types
   - Successfully used with NO types

## Timeline Summary

- Phase 1-3: 3-4 days (Core geometry system)
- Phase 4-7: 6 days (Component updates)
- Phase 8-10: 4 days (Testing and documentation)

**Total: 13-14 days**

## Next Steps

1. Review and approve implementation plan
2. Set up CI builds with different configurations
3. Begin Phase 1 implementation
4. Create tracking issues for each phase