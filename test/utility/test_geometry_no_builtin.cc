//
// Test that library works without built-in geometry types
//

#define SDLPP_NO_BUILTIN_GEOMETRY
#include <sdlpp/utility/geometry.hh>
#include <cassert>

// Verify feature detection
static_assert(SDLPP_HAS_BUILTIN_GEOMETRY == 0);

// Define our own geometry types
namespace my_geom {
    struct vec2 {
        using value_type = float;
        float x, y;
        
        vec2() = default;
        vec2(float x_, float y_) : x(x_), y(y_) {}
    };
    
    struct dimensions {
        using value_type = int;
        int width, height;
        
        dimensions() = default;
        dimensions(int w, int h) : width(w), height(h) {}
    };
    
    struct bounds {
        using value_type = double;
        double x, y, w, h;
        
        bounds() = default;
        bounds(double x_, double y_, double w_, double h_) 
            : x(x_), y(y_), w(w_), h(h_) {}
    };
}

// Verify our types satisfy the concepts
static_assert(sdlpp::point_like<my_geom::vec2>);
static_assert(sdlpp::size_like<my_geom::dimensions>);
static_assert(sdlpp::rect_like<my_geom::bounds>);

void test_concepts_only() {
    // The built-in types should NOT be available
    // This would cause compilation error if uncommented:
    // sdlpp::point p{10, 20};  // ERROR!
    
    // But we can use our own types
    my_geom::vec2 p1{10.0f, 20.0f};
    my_geom::vec2 p2{30.0f, 40.0f};
    
    // Utility functions work
    assert(sdlpp::get_x(p1) == 10.0f);
    assert(sdlpp::get_y(p1) == 20.0f);
    
    // Algorithms work
    auto dist = sdlpp::distance(p1, p2);
    assert(dist > 28.0f && dist < 29.0f);
    
    // Size operations
    my_geom::dimensions sz{800, 600};
    assert(sdlpp::get_width(sz) == 800);
    assert(sdlpp::get_height(sz) == 600);
    assert(sdlpp::get_area(sz) == 480000);
    assert(!sdlpp::is_empty(sz));
    
    // Rectangle operations
    my_geom::bounds rect{0.0, 0.0, 100.0, 100.0};
    assert(sdlpp::get_area(rect) == 10000.0);
    
    my_geom::vec2 test_point{50.0f, 50.0f};
    assert(sdlpp::contains(rect, test_point));
    
    my_geom::bounds rect2{50.0, 50.0, 100.0, 100.0};
    assert(sdlpp::intersects(rect, rect2));
    
    // Advanced algorithms
    auto mid = sdlpp::lerp(p1, p2, 0.5);
    assert(mid.x == 20.0f && mid.y == 30.0f);
    
    auto scaled = sdlpp::scale_from_center(rect, 0.5, 0.5);
    assert(scaled.x == 25.0 && scaled.y == 25.0);
    assert(scaled.w == 50.0 && scaled.h == 50.0);
}

int main() {
    test_concepts_only();
    return 0;
}