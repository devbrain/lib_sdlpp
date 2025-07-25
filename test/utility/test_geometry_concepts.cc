//
// Test file for geometry concepts
// This verifies the concepts work correctly and the header has zero dependencies
//

#include <sdlpp/utility/geometry_concepts.hh>
#include <cassert>
#include <vector>

// Test types that should satisfy the concepts
struct TestPoint {
    using value_type = int;
    int x, y;
};

struct TestSize {
    using value_type = float;
    float width, height;
};

struct TestRect {
    using value_type = double;
    double x, y, w, h;
};

struct TestRectAlt {
    using value_type = int;
    int left, top, width, height;
};

struct TestLine {
    using value_type = float;
    float x1, y1, x2, y2;
};

struct TestCircle {
    using value_type = double;
    double x, y, radius;
};

struct TestTriangle {
    TestPoint a, b, c;
};

struct TestPolygon {
    using value_type = int;
    std::vector<TestPoint> points;
    
    TestPolygon() : points{{0, 0}, {1, 0}, {1, 1}} {} // Initialize with some points
    
    std::size_t size() const { return points.size(); }
    TestPoint operator[](std::size_t i) const { return points[i]; }
};

// Test types that should NOT satisfy the concepts
struct NotAPoint {
    int a, b;  // Wrong member names
};

struct PointWithoutValueType {
    int x, y;  // Missing value_type
};

// Compile-time tests
static_assert(sdlpp::point_like<TestPoint>);
static_assert(sdlpp::size_like<TestSize>);
static_assert(sdlpp::rect_like<TestRect>);
static_assert(sdlpp::rect_like_alt<TestRectAlt>);
static_assert(sdlpp::rectangle_like<TestRect>);
static_assert(sdlpp::rectangle_like<TestRectAlt>);
static_assert(sdlpp::line_like<TestLine>);
static_assert(sdlpp::circle_like<TestCircle>);
static_assert(sdlpp::triangle_like<TestTriangle>);
static_assert(sdlpp::polygon_like<TestPolygon>);

static_assert(sdlpp::arithmetic_point_like<TestPoint>);
static_assert(sdlpp::arithmetic_size_like<TestSize>);
static_assert(sdlpp::arithmetic_rect_like<TestRect>);

// Negative tests
static_assert(!sdlpp::point_like<NotAPoint>);
static_assert(!sdlpp::point_like<PointWithoutValueType>);
static_assert(!sdlpp::size_like<TestPoint>);
static_assert(!sdlpp::rect_like<TestSize>);

// Test utility functions
void test_utility_functions() {
    // Test get functions
    TestPoint p{10, 20};
    assert(sdlpp::get_x(p) == 10);
    assert(sdlpp::get_y(p) == 20);
    
    TestSize s{100.0f, 200.0f};
    assert(sdlpp::get_width(s) == 100.0f);
    assert(sdlpp::get_height(s) == 200.0f);
    assert(sdlpp::get_area(s) == 20000.0f);
    
    TestRect r{1.0, 2.0, 3.0, 4.0};
    assert(sdlpp::get_area(r) == 12.0);
    
    TestRectAlt ra{5, 6, 7, 8};
    assert(sdlpp::get_area(ra) == 56);
    
    // Test empty checks
    TestSize empty_size{0.0f, 100.0f};
    assert(sdlpp::is_empty(empty_size));
    
    TestRect empty_rect{0.0, 0.0, -1.0, 10.0};
    assert(sdlpp::is_empty(empty_rect));
    
    // Test contains
    TestPoint p2{5, 5};
    TestRect r2{0.0, 0.0, 10.0, 10.0};
    assert(sdlpp::contains(r2, p2));
    
    TestPoint p3{15, 5};
    assert(!sdlpp::contains(r2, p3));
    
    // Test intersects
    TestRect r3{5.0, 5.0, 10.0, 10.0};
    assert(sdlpp::intersects(r2, r3));
    
    TestRect r4{20.0, 20.0, 10.0, 10.0};
    assert(!sdlpp::intersects(r2, r4));
    
    // Test mixed format intersects
    TestRectAlt ra2{3, 3, 8, 8};
    assert(sdlpp::intersects(r2, ra2));
    assert(sdlpp::intersects(ra2, r2));
}

// Test type extraction
static_assert(std::is_same_v<sdlpp::geometry_value_type_t<TestPoint>, int>);
static_assert(std::is_same_v<sdlpp::geometry_value_type_t<TestSize>, float>);
static_assert(std::is_same_v<sdlpp::geometry_value_type_t<TestRect>, double>);

// Test with external library types (simulate GLM)
namespace glm {
    template<typename T>
    struct tvec2 {
        using value_type = T;
        T x, y;
        tvec2(T x_, T y_) : x(x_), y(y_) {}
    };
    
    using vec2 = tvec2<float>;
    using ivec2 = tvec2<int>;
}

static_assert(sdlpp::point_like<glm::vec2>);
static_assert(sdlpp::point_like<glm::ivec2>);
static_assert(sdlpp::arithmetic_point_like<glm::vec2>);

int main() {
    test_utility_functions();
    
    // Test with GLM-like types
    glm::vec2 glm_point{1.5f, 2.5f};
    assert(sdlpp::get_x(glm_point) == 1.5f);
    assert(sdlpp::get_y(glm_point) == 2.5f);
    
    return 0;
}