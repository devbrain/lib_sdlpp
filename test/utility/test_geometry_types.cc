//
// Test file for geometry types
//

#include <sdlpp/utility/geometry_types.hh>
#include <cassert>
#include <sstream>
#include <unordered_map>
#include <cmath>

using namespace sdlpp;

void test_point() {
    // Construction
    point_i p1;
    assert(p1.x == 0 && p1.y == 0);
    
    point_i p2{10, 20};
    assert(p2.x == 10 && p2.y == 20);
    
    // Arithmetic
    auto p3 = p2 + point_i{5, 3};
    assert(p3.x == 15 && p3.y == 23);
    
    auto p4 = p2 - point_i{3, 7};
    assert(p4.x == 7 && p4.y == 13);
    
    auto p5 = p2 * 2;
    assert(p5.x == 20 && p5.y == 40);
    
    auto p6 = 3 * p2;
    assert(p6.x == 30 && p6.y == 60);
    
    auto p7 = point_i{20, 40} / 2;
    assert(p7.x == 10 && p7.y == 20);
    
    // In-place operations
    point_i p8{5, 5};
    p8 += point_i{3, 2};
    assert(p8.x == 8 && p8.y == 7);
    
    p8 -= point_i{1, 2};
    assert(p8.x == 7 && p8.y == 5);
    
    p8 *= 2;
    assert(p8.x == 14 && p8.y == 10);
    
    p8 /= 2;
    assert(p8.x == 7 && p8.y == 5);
    
    // Negation
    auto p9 = -p2;
    assert(p9.x == -10 && p9.y == -20);
    
    // Comparison
    assert(p2 == point_i(10, 20));
    assert(p2 != point_i(10, 21));
    assert(point_i(1, 2) < point_i(1, 3));
    assert(point_i(2, 1) > point_i(1, 3));
    
    // Utility methods
    point_f pf{3.0f, 4.0f};
    assert(pf.length_squared() == 25.0f);
    assert(pf.length() == 5.0f);
    
    assert(point_i(1, 2).dot(point_i(3, 4)) == 11);
    assert(point_i(2, 3).cross(point_i(4, 5)) == -2);
    
    auto pn = point_f{3.0f, 4.0f}.normalized();
    assert(std::abs(pn.x - 0.6f) < 0.001f);
    assert(std::abs(pn.y - 0.8f) < 0.001f);
    
    // Stream output
    std::stringstream ss;
    ss << p2;
    assert(ss.str() == "(10, 20)");
    
    // Conversion
    point_f pf2{point_i{10, 20}};
    assert(pf2.x == 10.0f && pf2.y == 20.0f);
}

void test_size() {
    // Construction
    size_i s1;
    assert(s1.width == 0 && s1.height == 0);
    
    size_i s2{800, 600};
    assert(s2.width == 800 && s2.height == 600);
    
    // Area and empty
    assert(s2.area() == 480000);
    assert(!s2.empty());
    assert(size_i(0, 100).empty());
    assert(size_i(100, 0).empty());
    assert(size_i(-5, 10).empty());
    
    // Arithmetic
    auto s3 = s2 + size_i{100, 50};
    assert(s3.width == 900 && s3.height == 650);
    
    auto s4 = s2 - size_i{100, 100};
    assert(s4.width == 700 && s4.height == 500);
    
    auto s5 = s2 * 2;
    assert(s5.width == 1600 && s5.height == 1200);
    
    auto s6 = s2 / 2;
    assert(s6.width == 400 && s6.height == 300);
    
    // Comparison
    assert(s2 == size_i(800, 600));
    assert(s2 != size_i(800, 601));
    
    // Aspect ratio
    assert(size_f(16.0f, 9.0f).aspect_ratio() == 16.0f / 9.0f);
    
    // Fit within
    auto fitted = size_f{1920.0f, 1080.0f}.fit_within(size_f{800.0f, 600.0f});
    assert(std::abs(fitted.width - 800.0f) < 0.1f);
    assert(std::abs(fitted.height - 450.0f) < 0.1f);
    
    // Stream output
    std::stringstream ss;
    ss << s2;
    assert(ss.str() == "800x600");
}

void test_rect() {
    // Construction
    rect_i r1;
    assert(r1.x == 0 && r1.y == 0 && r1.w == 0 && r1.h == 0);
    
    rect_i r2{10, 20, 100, 200};
    assert(r2.x == 10 && r2.y == 20 && r2.w == 100 && r2.h == 200);
    
    rect_i r3{point_i{10, 20}, size_i{100, 200}};
    assert(r3 == r2);
    
    // Properties
    assert(r2.area() == 20000);
    assert(!r2.empty());
    assert(r2.position() == point_i(10, 20));
    assert(r2.dimensions() == size_i(100, 200));
    
    // Edges
    assert(r2.left() == 10);
    assert(r2.top() == 20);
    assert(r2.right() == 110);
    assert(r2.bottom() == 220);
    
    // Corners
    assert(r2.top_left() == point_i(10, 20));
    assert(r2.top_right() == point_i(110, 20));
    assert(r2.bottom_left() == point_i(10, 220));
    assert(r2.bottom_right() == point_i(110, 220));
    assert(r2.center() == point_i(60, 120));
    
    // Contains point
    assert(r2.contains(point_i(50, 100)));
    assert(!r2.contains(point_i(5, 100)));
    assert(!r2.contains(point_i(150, 100)));
    
    // Contains rect
    assert(r2.contains(rect_i(20, 30, 50, 50)));
    assert(!r2.contains(rect_i(20, 30, 100, 50)));
    
    // Intersects
    assert(r2.intersects(rect_i(50, 100, 100, 100)));
    assert(!r2.intersects(rect_i(200, 300, 100, 100)));
    
    // Intersection
    auto isect = r2.intersection(rect_i(50, 100, 100, 100));
    assert(isect == rect_i(50, 100, 60, 100));
    
    auto no_isect = r2.intersection(rect_i(200, 300, 100, 100));
    assert(no_isect.empty());
    
    // Union
    auto united = r2.unite(rect_i(50, 100, 100, 100));
    assert(united == rect_i(10, 20, 140, 200));
    
    // Move
    auto moved = r2.moved_by(point_i(5, 10));
    assert(moved == rect_i(15, 30, 100, 200));
    
    rect_i r4 = r2;
    r4.move_by(point_i(5, 10));
    assert(r4 == moved);
    
    // Inflate
    auto inflated = r2.inflated(10, 20);
    assert(inflated == rect_i(0, 0, 120, 240));
    
    rect_i r5 = r2;
    r5.inflate(10, 20);
    assert(r5 == inflated);
    
    // Stream output
    std::stringstream ss;
    ss << r2;
    assert(ss.str() == "[10, 20, 100, 200]");
}

void test_line() {
    // Construction
    line_i l1;
    assert(l1.x1 == 0 && l1.y1 == 0 && l1.x2 == 0 && l1.y2 == 0);
    
    line_i l2{10, 20, 30, 40};
    assert(l2.x1 == 10 && l2.y1 == 20 && l2.x2 == 30 && l2.y2 == 40);
    
    line_i l3{point_i{10, 20}, point_i{30, 40}};
    assert(l3 == l2);
    
    // Properties
    assert(l2.start() == point_i(10, 20));
    assert(l2.end() == point_i(30, 40));
    assert(l2.vector() == point_i(20, 20));
    assert(l2.midpoint() == point_i(20, 30));
    
    line_f lf{0.0f, 0.0f, 3.0f, 4.0f};
    assert(lf.length() == 5.0f);
    assert(lf.length_squared() == 25.0f);
    
    // Stream output
    std::stringstream ss;
    ss << l2;
    assert(ss.str() == "(10, 20) -> (30, 40)");
}

void test_circle() {
    // Construction
    circle_i c1;
    assert(c1.x == 0 && c1.y == 0 && c1.radius == 0);
    
    circle_i c2{100, 200, 50};
    assert(c2.x == 100 && c2.y == 200 && c2.radius == 50);
    
    circle_i c3{point_i{100, 200}, 50};
    assert(c3 == c2);
    
    // Properties
    assert(c2.center() == point_i(100, 200));
    
    circle_f cf{0.0f, 0.0f, 10.0f};
    assert(std::abs(cf.area() - 314.159f) < 1.0f);
    assert(std::abs(cf.circumference() - 62.831f) < 0.1f);
    
    // Contains
    assert(c2.contains(point_i(100, 200)));
    assert(c2.contains(point_i(120, 200)));
    assert(!c2.contains(point_i(200, 200)));
    
    // Intersects
    assert(c2.intersects(circle_i(120, 200, 40)));
    assert(!c2.intersects(circle_i(300, 200, 40)));
    
    // Bounding rect
    auto bounds = c2.bounding_rect();
    assert(bounds == rect_i(50, 150, 100, 100));
    
    // Stream output
    std::stringstream ss;
    ss << c2;
    assert(ss.str() == "Circle(100, 200, r=50)");
}

void test_triangle() {
    // Construction
    triangle_i t1;
    
    triangle_i t2{point_i{0, 0}, point_i{10, 0}, point_i{5, 10}};
    triangle_i t3{0, 0, 10, 0, 5, 10};
    assert(t2 == t3);
    
    // Properties
    assert(t2.area() == 50);
    assert(t2.centroid() == point_i(5, 3));  // (0+10+5)/3, (0+0+10)/3 = 5, 3
    
    // Contains
    assert(t2.contains(point_i(5, 5)));
    assert(!t2.contains(point_i(20, 5)));
    
    // Bounding rect
    auto bounds = t2.bounding_rect();
    assert(bounds == rect_i(0, 0, 10, 10));
    
    // Stream output
    std::stringstream ss;
    ss << t2;
    assert(ss.str() == "Triangle((0, 0), (10, 0), (5, 10))");
}

void test_hashing() {
    // Test that our types can be used in unordered containers
    std::unordered_map<point_i, int> point_map;
    point_map[point_i(10, 20)] = 42;
    assert(point_map[point_i(10, 20)] == 42);
    
    std::unordered_map<size_i, int> size_map;
    size_map[size_i(800, 600)] = 123;
    assert(size_map[size_i(800, 600)] == 123);
    
    std::unordered_map<rect_i, int> rect_map;
    rect_map[rect_i(0, 0, 100, 100)] = 456;
    assert(rect_map[rect_i(0, 0, 100, 100)] == 456);
    
    std::unordered_map<line_i, int> line_map;
    line_map[line_i(0, 0, 10, 10)] = 789;
    assert(line_map[line_i(0, 0, 10, 10)] == 789);
    
    std::unordered_map<circle_i, int> circle_map;
    circle_map[circle_i(50, 50, 25)] = 321;
    assert(circle_map[circle_i(50, 50, 25)] == 321);
}

void test_deduction_guides() {
    // Test that deduction guides work
    auto p = point{10, 20};  // Should deduce point<int>
    static_assert(std::is_same_v<decltype(p), point<int>>);
    
    auto s = size{100.0f, 200.0f};  // Should deduce size<float>
    static_assert(std::is_same_v<decltype(s), size<float>>);
    
    auto r = rect{1.0, 2.0, 3.0, 4.0};  // Should deduce rect<double>
    static_assert(std::is_same_v<decltype(r), rect<double>>);
    
    auto l = line{1, 2, 3, 4};  // Should deduce line<int>
    static_assert(std::is_same_v<decltype(l), line<int>>);
    
    auto c = circle{50.0f, 50.0f, 25.0f};  // Should deduce circle<float>
    static_assert(std::is_same_v<decltype(c), circle<float>>);
}

int main() {
    test_point();
    test_size();
    test_rect();
    test_line();
    test_circle();
    test_triangle();
    test_hashing();
    test_deduction_guides();
    
    return 0;
}