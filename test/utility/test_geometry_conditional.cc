//
// Test conditional inclusion of geometry types
//

// Test 1: With built-in geometry types (default)
#include <sdlpp/utility/geometry.hh>
#include <cassert>

void test_with_builtin_types() {
    // Verify feature macro
    static_assert(SDLPP_HAS_BUILTIN_GEOMETRY == 1);
    
    // Built-in types should be available
    sdlpp::point p{10, 20};
    assert(p.x == 10 && p.y == 20);
    
    sdlpp::size s{100, 200};
    assert(s.width == 100 && s.height == 200);
    
    sdlpp::rect r{0, 0, 50, 50};
    assert(r.w == 50 && r.h == 50);
    
    // Concepts should work
    static_assert(sdlpp::point_like<sdlpp::point_i>);
    static_assert(sdlpp::size_like<sdlpp::size_i>);
    static_assert(sdlpp::rect_like<sdlpp::rect_i>);
    
    // Algorithms should work
    sdlpp::point p2{30, 40};
    auto dist = sdlpp::distance(p, p2);
    assert(dist > 28.0 && dist < 29.0);  // Should be ~28.28
}

// Now test without built-in types
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
#undef SDLPP_HAS_BUILTIN_GEOMETRY
#endif

// Can't directly test SDLPP_NO_BUILTIN_GEOMETRY in same file,
// so we'll test that the concepts work with custom types

struct CustomPoint {
    using value_type = float;
    float x, y;
};

struct CustomSize {
    using value_type = float;
    float width, height;
};

struct CustomRect {
    using value_type = float;
    float x, y, w, h;
};

void test_with_custom_types() {
    // Custom types should satisfy concepts
    static_assert(sdlpp::point_like<CustomPoint>);
    static_assert(sdlpp::size_like<CustomSize>);
    static_assert(sdlpp::rect_like<CustomRect>);
    
    // Algorithms should work with custom types
    CustomPoint p1{10.0f, 20.0f};
    CustomPoint p2{30.0f, 40.0f};
    auto dist = sdlpp::distance(p1, p2);
    assert(dist > 28.0f && dist < 29.0f);
    
    // Utility functions should work
    assert(sdlpp::get_x(p1) == 10.0f);
    assert(sdlpp::get_y(p1) == 20.0f);
    
    CustomSize sz{100.0f, 200.0f};
    assert(sdlpp::get_width(sz) == 100.0f);
    assert(sdlpp::get_height(sz) == 200.0f);
    assert(sdlpp::get_area(sz) == 20000.0f);
    
    CustomRect rect{0.0f, 0.0f, 50.0f, 50.0f};
    assert(sdlpp::get_area(rect) == 2500.0f);
    assert(!sdlpp::is_empty(rect));
    
    // Contains test
    assert(sdlpp::contains(rect, CustomPoint{25.0f, 25.0f}));
    assert(!sdlpp::contains(rect, CustomPoint{60.0f, 25.0f}));
    
    // Intersects test
    CustomRect rect2{25.0f, 25.0f, 50.0f, 50.0f};
    assert(sdlpp::intersects(rect, rect2));
    
    // Algorithms
    auto mid = sdlpp::lerp(p1, p2, 0.5);
    assert(mid.x == 20.0f && mid.y == 30.0f);
    
    auto rotated = sdlpp::rotate(CustomPoint{1.0f, 0.0f}, 3.14159f / 2);
    assert(std::abs(rotated.x) < 0.001f && std::abs(rotated.y - 1.0f) < 0.001f);
}

int main() {
    test_with_builtin_types();
    test_with_custom_types();
    return 0;
}