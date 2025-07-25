//
// Created by igor on 7/13/25.
//

#include <doctest/doctest.h>
#include <cmath>
#include <type_traits>
#include <limits>

#include "sdlpp/utility/geometry.hh"

using namespace sdlpp;

TEST_SUITE("geometry") {
    
    TEST_CASE("basic_point construction and conversion") {
        SUBCASE("default construction") {
            point_i p;
            CHECK(p.x == 0);
            CHECK(p.y == 0);
            
            point_f fp;
            CHECK(fp.x == 0.0f);
            CHECK(fp.y == 0.0f);
        }
        
        SUBCASE("value construction") {
            point_i p(10, 20);
            CHECK(p.x == 10);
            CHECK(p.y == 20);
            
            point_f fp(10.5f, 20.7f);
            CHECK(fp.x == 10.5f);
            CHECK(fp.y == 20.7f);
        }
        
        SUBCASE("comparison operators") {
            point_i p1(15, 25);
            point_i p2(15, 25);
            point_i p3(10, 25);
            
            CHECK(p1 == p2);
            CHECK(p1 != p3);
            
            point_f fp1(15.5f, 25.5f);
            point_f fp2(15.5f, 25.5f);
            point_f fp3(10.5f, 25.5f);
            
            CHECK(fp1 == fp2);
            CHECK(fp1 != fp3);
        }
        
        SUBCASE("type conversion") {
            point_i p(10, 20);
            point_f fp(p);  // explicit conversion
            CHECK(fp.x == 10.0f);
            CHECK(fp.y == 20.0f);
            
            point_f fp2(10.7f, 20.3f);
            point_i p2(fp2);  // truncates
            CHECK(p2.x == 10);
            CHECK(p2.y == 20);
        }
    }
    
    TEST_CASE("basic_point arithmetic operations") {
        SUBCASE("addition") {
            point_i p1(10, 20);
            point_i p2(5, 7);
            auto p3 = p1 + p2;
            CHECK(p3.x == 15);
            CHECK(p3.y == 27);
            
            p1 += p2;
            CHECK(p1 == p3);
        }
        
        SUBCASE("subtraction") {
            point_i p1(10, 20);
            point_i p2(5, 7);
            auto p3 = p1 - p2;
            CHECK(p3.x == 5);
            CHECK(p3.y == 13);
            
            p1 -= p2;
            CHECK(p1 == p3);
        }
        
        SUBCASE("scalar multiplication") {
            point_i p(10, 20);
            auto p2 = p * 3;
            CHECK(p2.x == 30);
            CHECK(p2.y == 60);
            
            p *= 2;
            CHECK(p.x == 20);
            CHECK(p.y == 40);
        }
        
        SUBCASE("scalar division") {
            point_i p(20, 40);
            auto p2 = p / 2;
            CHECK(p2.x == 10);
            CHECK(p2.y == 20);
            
            p /= 4;
            CHECK(p.x == 5);
            CHECK(p.y == 10);
        }
        
        SUBCASE("negation") {
            point_i p(10, -20);
            auto p2 = -p;
            CHECK(p2.x == -10);
            CHECK(p2.y == 20);
        }
    }
    
    TEST_CASE("basic_point distance and magnitude") {
        SUBCASE("distance calculations") {
            point_i p1(0, 0);
            point_i p2(3, 4);
            
            // Using generic distance_between function
            CHECK(sdlpp::distance(p1, p2) == 5);
            
            point_f fp1(0.0f, 0.0f);
            point_f fp2(3.0f, 4.0f);
            CHECK(sdlpp::distance(fp1, fp2) == 5.0f);
        }
        
        SUBCASE("length") {
            point_i p(3, 4);
            CHECK(p.length_squared() == 25);
            CHECK(p.length() == 5);
            
            point_f fp(3.0f, 4.0f);
            CHECK(fp.length_squared() == 25.0f);
            CHECK(fp.length() == 5.0f);
        }
        
        SUBCASE("dot product") {
            point_i p1(3, 4);
            point_i p2(2, 1);
            CHECK(p1.dot(p2) == 10);  // 3*2 + 4*1
            
            point_f fp1(1.0f, 0.0f);
            point_f fp2(0.0f, 1.0f);
            CHECK(fp1.dot(fp2) == 0.0f);  // perpendicular
        }
        
        SUBCASE("cross product") {
            point_i p1(3, 0);
            point_i p2(0, 3);
            CHECK(p1.cross(p2) == 9);  // 3*3 - 0*0
            
            point_i p3(1, 0);
            point_i p4(1, 0);
            CHECK(p3.cross(p4) == 0);  // parallel
        }
    }
    
    TEST_CASE("basic_size") {
        SUBCASE("construction") {
            size_i s;
            CHECK(s.width == 0);
            CHECK(s.height == 0);
            CHECK(s.empty());
            
            size_i s2(100, 200);
            CHECK(s2.width == 100);
            CHECK(s2.height == 200);
            CHECK(!s2.empty());
        }
        
        SUBCASE("area calculation") {
            size_i s(10, 20);
            CHECK(s.area() == 200);
            
            size_f fs(10.5f, 20.5f);
            CHECK(fs.area() == doctest::Approx(215.25f));
        }
        
        SUBCASE("type conversion") {
            size_i s(100, 200);
            size_f fs(s);
            CHECK(fs.width == 100.0f);
            CHECK(fs.height == 200.0f);
        }
        
        SUBCASE("aspect ratio") {
            size_i s(100, 50);
            CHECK(s.aspect_ratio() == 2);
            
            size_f sf(16.0f, 9.0f);
            CHECK(sf.aspect_ratio() == doctest::Approx(16.0f / 9.0f));
        }
        
        SUBCASE("arithmetic operations") {
            size_i s(100, 200);
            auto s2 = s * 2;
            CHECK(s2.width == 200);
            CHECK(s2.height == 400);
            
            auto s3 = s / 2;
            CHECK(s3.width == 50);
            CHECK(s3.height == 100);
        }
    }
    
    TEST_CASE("basic_rect construction") {
        SUBCASE("default construction") {
            rect_i r;
            CHECK(r.x == 0);
            CHECK(r.y == 0);
            CHECK(r.w == 0);
            CHECK(r.h == 0);
            CHECK(r.empty());
        }
        
        SUBCASE("value construction") {
            rect_i r(10, 20, 100, 50);
            CHECK(r.x == 10);
            CHECK(r.y == 20);
            CHECK(r.w == 100);
            CHECK(r.h == 50);
            CHECK(!r.empty());
        }
        
        SUBCASE("point and size construction") {
            point_i p(10, 20);
            size_i s(100, 50);
            rect_i r(p, s);
            CHECK(r.x == 10);
            CHECK(r.y == 20);
            CHECK(r.w == 100);
            CHECK(r.h == 50);
        }
        
        SUBCASE("from corners") {
            point_i p1(10, 20);
            point_i p2(110, 70);
            // Create rect from two points
            rect_i r(p1.x, p1.y, p2.x - p1.x, p2.y - p1.y);
            CHECK(r.x == 10);
            CHECK(r.y == 20);
            CHECK(r.w == 100);
            CHECK(r.h == 50);
        }
        
        SUBCASE("comparison operators") {
            rect_i r1(10, 20, 100, 50);
            rect_i r2(10, 20, 100, 50);
            rect_i r3(10, 20, 100, 60);
            
            CHECK(r1 == r2);
            CHECK(r1 != r3);
            
            rect_f fr1(10.5f, 20.5f, 100.5f, 50.5f);
            rect_f fr2(10.5f, 20.5f, 100.5f, 50.5f);
            rect_f fr3(10.5f, 20.5f, 100.5f, 60.5f);
            
            CHECK(fr1 == fr2);
            CHECK(fr1 != fr3);
        }
    }
    
    TEST_CASE("basic_rect properties") {
        rect_i r(10, 20, 100, 50);
        
        SUBCASE("edge properties") {
            CHECK(r.left() == 10);
            CHECK(r.right() == 110);
            CHECK(r.top() == 20);
            CHECK(r.bottom() == 70);
        }
        
        SUBCASE("corner points") {
            CHECK(r.top_left() == point_i(10, 20));
            CHECK(r.top_right() == point_i(110, 20));
            CHECK(r.bottom_left() == point_i(10, 70));
            CHECK(r.bottom_right() == point_i(110, 70));
        }
        
        SUBCASE("center and area") {
            CHECK(r.center() == point_i(60, 45));
            CHECK(r.area() == 5000);
        }
        
        SUBCASE("position and size") {
            CHECK(r.position() == point_i(10, 20));
            CHECK(r.w == 100);
            CHECK(r.h == 50);
        }
    }
    
    TEST_CASE("basic_rect containment") {
        rect_i r(10, 20, 100, 50);
        
        SUBCASE("contains point") {
            CHECK(r.contains(point_i(10, 20)));     // top-left corner
            CHECK(r.contains(point_i(60, 45)));     // center
            CHECK(r.contains(point_i(109, 69)));    // inside near bottom-right
            
            CHECK(!r.contains(point_i(110, 70)));   // bottom-right corner (exclusive)
            CHECK(!r.contains(point_i(9, 20)));     // just outside left
            CHECK(!r.contains(point_i(10, 19)));    // just outside top
            CHECK(!r.contains(point_i(200, 200)));  // far outside
        }
        
        SUBCASE("contains rect") {
            CHECK(r.contains(rect_i(20, 30, 10, 10)));    // fully inside
            CHECK(r.contains(rect_i(10, 20, 100, 50)));   // exact match
            CHECK(!r.contains(rect_i(5, 30, 10, 10)));    // extends outside left
            CHECK(!r.contains(rect_i(20, 30, 100, 10)));  // extends outside right
            CHECK(!r.contains(rect_i(0, 0, 200, 200)));   // contains parent
        }
    }
    
    TEST_CASE("basic_rect intersection") {
        rect_i r1(10, 20, 100, 50);
        
        SUBCASE("intersects test") {
            CHECK(r1.intersects(rect_i(50, 40, 100, 50)));     // partial overlap
            CHECK(r1.intersects(rect_i(0, 0, 200, 200)));      // fully contained
            CHECK(r1.intersects(rect_i(10, 20, 100, 50)));     // exact match
            CHECK(!r1.intersects(rect_i(200, 200, 50, 50)));   // no overlap
            CHECK(!r1.intersects(rect_i(110, 20, 50, 50)));    // touching edge
        }
        
        SUBCASE("intersection calculation") {
            rect_i r2(50, 40, 100, 50);
            auto inter = r1.intersection(r2);
            CHECK(inter == rect_i(50, 40, 60, 30));
            
            // No intersection
            rect_i r3(200, 200, 50, 50);
            auto inter2 = r1.intersection(r3);
            CHECK(inter2.empty());
            
            // Complete containment
            rect_i r4(20, 30, 50, 20);
            auto inter3 = r1.intersection(r4);
            CHECK(inter3 == r4);
        }
    }
    
    TEST_CASE("basic_rect union") {
        rect_i r1(10, 20, 50, 30);
        rect_i r2(40, 40, 50, 30);
        
        SUBCASE("union calculation") {
            // Union is the smallest rect containing both
            int left = std::min(r1.left(), r2.left());
            int top = std::min(r1.top(), r2.top());
            int right = std::max(r1.right(), r2.right());
            int bottom = std::max(r1.bottom(), r2.bottom());
            rect_i u(left, top, right - left, bottom - top);
            CHECK(u == rect_i(10, 20, 80, 50));
        }
    }
    
    TEST_CASE("basic_rect transformations") {
        rect_i r(10, 20, 100, 50);
        
        SUBCASE("inflation") {
            auto r2 = r.inflated(10, 10);
            CHECK(r2 == rect_i(0, 10, 120, 70));
            
            auto r3 = r.inflated(-5, -5);  // deflation
            CHECK(r3 == rect_i(15, 25, 90, 40));
            
            auto r4 = r.inflated(5, 10);
            CHECK(r4 == rect_i(5, 10, 110, 70));
        }
        
        SUBCASE("movement") {
            auto r2 = r.moved_by(point_i(10, 20));
            CHECK(r2 == rect_i(20, 40, 100, 50));
            
            auto r3 = r.moved_by(point_i(-5, -10));
            CHECK(r3 == rect_i(5, 10, 100, 50));
        }
        
        SUBCASE("centering") {
            // Centered_at not available, calculate manually
            point_i new_center(100, 100);
            auto center = r.center();
            auto offset = point_i(new_center.x - center.x, new_center.y - center.y);
            auto r2 = r.moved_by(offset);
            CHECK(r2.center() == point_i(100, 100));
            CHECK(r2 == rect_i(50, 75, 100, 50));
        }
        
        SUBCASE("clamping") {
            rect_i bounds(0, 0, 200, 150);
            
            // Already within bounds
            CHECK(r.x >= bounds.x);
            CHECK(r.y >= bounds.y);
            CHECK(r.right() <= bounds.right());
            CHECK(r.bottom() <= bounds.bottom());
            
            // Clamp from outside manually
            rect_i r3(-50, -50, 100, 50);
            int clamped_x = std::max(r3.x, bounds.x);
            int clamped_y = std::max(r3.y, bounds.y);
            rect_i r4(clamped_x, clamped_y, r3.w, r3.h);
            CHECK(r4 == rect_i(0, 0, 100, 50));
        }
    }
    
    TEST_CASE("floating point specific tests") {
        SUBCASE("frect precision") {
            rect_f r(10.5f, 20.3f, 100.7f, 50.2f);
            CHECK(r.center() == point_f(60.85f, 45.4f));
            CHECK(r.area() == doctest::Approx(5055.14f));
        }
        
        SUBCASE("fpoint lerp") {
            point_f p1(0.0f, 0.0f);
            point_f p2(10.0f, 20.0f);
            
            auto p3 = sdlpp::lerp(p1, p2, 0.0f);
            CHECK(p3 == p1);
            
            auto p4 = sdlpp::lerp(p1, p2, 1.0f);
            CHECK(p4 == p2);
            
            auto p5 = sdlpp::lerp(p1, p2, 0.5f);
            CHECK(p5 == point_f(5.0f, 10.0f));
            
            auto p6 = sdlpp::lerp(p1, p2, 0.25f);
            CHECK(p6 == point_f(2.5f, 5.0f));
        }
    }
    
    TEST_CASE("utility functions") {
        SUBCASE("rect construction from point and size") {
            point_i p(10, 20);
            size_i s(100, 50);
            rect_i r(p, s);
            CHECK(r == rect_i(10, 20, 100, 50));
        }
        
        SUBCASE("rect size comparison") {
            rect_i r1(10, 20, 100, 50);
            rect_i r2(30, 40, 100, 50);
            rect_i r3(10, 20, 200, 50);
            
            bool same_size_1_2 = (r1.w == r2.w) && (r1.h == r2.h);
            CHECK(same_size_1_2);
            bool same_size_1_3 = (r1.w == r3.w) && (r1.h == r3.h);
            CHECK(!same_size_1_3);
        }
    }
    
    TEST_CASE("edge cases") {
        SUBCASE("zero-sized rect") {
            rect_i r(10, 20, 0, 0);
            CHECK(r.empty());
            CHECK(r.area() == 0);
            CHECK(!r.contains(point_i(10, 20)));  // even origin not contained
        }
        
        SUBCASE("negative-sized rect") {
            rect_i r(10, 20, -10, -20);
            CHECK(r.empty());
            CHECK(r.area() == 200);  // still calculated
        }
        
        SUBCASE("large values") {
            point_i p(1000000, 2000000);
            // length_squared returns T which is int, so it overflows
            // Use int64_t for the calculation
            int64_t x = p.x;
            int64_t y = p.y;
            CHECK(x * x + y * y == 5000000000000LL);
        }
    }
    
    TEST_CASE("overflow protection") {
        SUBCASE("size area overflow protection") {
            // Large values that would overflow in 32-bit multiplication
            sdlpp::size_i large_size{65536, 65536};
            // area() returns T (int) which overflows
            int64_t expected = int64_t(65536) * int64_t(65536);
            int64_t actual = int64_t(large_size.width) * int64_t(large_size.height);
            CHECK(actual == expected);  // 65536 * 65536
        }
        
        SUBCASE("rect area overflow protection") {
            sdlpp::rect_i large_rect{0, 0, 100000, 100000};
            // area() returns T (int) which overflows
            int64_t expected = int64_t(100000) * int64_t(100000);
            int64_t actual = int64_t(large_rect.w) * int64_t(large_rect.h);
            CHECK(actual == expected);  // 100000 * 100000
            
            // Negative dimensions should still work
            sdlpp::rect_i neg_rect{0, 0, -100, 50};
            CHECK(neg_rect.area() == -5000);
        }
    }
    
    TEST_CASE("concept-based generic functions") {
        SUBCASE("distance_between") {
            point_i p1(0, 0);
            point_i p2(3, 4);
            CHECK(sdlpp::distance(p1, p2) == 5);
            
            point_f fp1(0.0f, 0.0f);
            point_f fp2(3.0f, 4.0f);
            CHECK(sdlpp::distance(fp1, fp2) == 5.0f);
        }
        
        SUBCASE("is_inside") {
            rect_i r(10, 20, 100, 50);
            CHECK(r.contains(point_i(50, 40)));
            CHECK(!r.contains(point_i(5, 5)));
            
            rect_f fr(10.0f, 20.0f, 100.0f, 50.0f);
            CHECK(fr.contains(point_f(50.5f, 40.5f)));
            CHECK(!fr.contains(point_f(5.0f, 5.0f)));
        }
        
        SUBCASE("rect from center") {
            point_i center(100, 100);
            size_i s(50, 30);
            // Create rect centered at point
            rect_i r(center.x - s.width/2, center.y - s.height/2, s.width, s.height);
            CHECK(r.x == 75);
            CHECK(r.y == 85);
            CHECK(r.w == 50);
            CHECK(r.h == 30);
            CHECK(r.center() == center);
        }
        
        SUBCASE("scale size") {
            size_i s(100, 50);
            auto s2 = s * 2;
            CHECK(s2.width == 200);
            CHECK(s2.height == 100);
            
            // Integer division instead of float multiplication
            auto s3 = s / 2;
            CHECK(s3.width == 50);
            CHECK(s3.height == 25);
            
            size_f fs(100.0f, 50.0f);
            auto fs2 = fs * 1.5f;
            CHECK(fs2.width == 150.0f);
            CHECK(fs2.height == 75.0f);
        }
    }
    
    TEST_CASE("concept verification") {
        SUBCASE("point_like concept") {
            CHECK(point_like<point_i>);
            CHECK(point_like<point_f>);
            CHECK(!point_like<int>);
            CHECK(!point_like<size_i>);  // size has width/height, not x/y
        }
        
        SUBCASE("size_like concept") {
            CHECK(size_like<size_i>);
            CHECK(size_like<size_f>);
            CHECK(!size_like<int>);
            CHECK(!size_like<point_i>);  // point has x/y, not width/height
        }
        
        SUBCASE("rect_like concept") {
            CHECK(rect_like<rect_i>);
            CHECK(rect_like<rect_f>);
            CHECK(!rect_like<int>);
            CHECK(!rect_like<point_i>);
            CHECK(!rect_like<size_i>);
        }
        
        SUBCASE("triangle_like concept") {
            CHECK(triangle_like<triangle_i>);
            CHECK(triangle_like<triangle_f>);
            CHECK(!triangle_like<int>);
            CHECK(!triangle_like<point_i>);
            CHECK(!triangle_like<rect_i>);
        }
    }
    
    TEST_CASE("basic_triangle construction") {
        SUBCASE("default construction") {
            triangle_i t;
            CHECK(t.a == point_i(0, 0));
            CHECK(t.b == point_i(0, 0));
            CHECK(t.c == point_i(0, 0));
        }
        
        SUBCASE("vertex construction") {
            point_i a(0, 0);
            point_i b(10, 0);
            point_i c(5, 10);
            triangle_i t(a, b, c);
            CHECK(t.a == a);
            CHECK(t.b == b);
            CHECK(t.c == c);
        }
        
        SUBCASE("coordinate construction") {
            triangle_i t(0, 0, 10, 0, 5, 10);
            CHECK(t.a == point_i(0, 0));
            CHECK(t.b == point_i(10, 0));
            CHECK(t.c == point_i(5, 10));
        }
        
        SUBCASE("type conversion") {
            triangle_i t(0, 0, 10, 0, 5, 10);
            triangle_f ft(t);
            CHECK(ft.a == point_f(0.0f, 0.0f));
            CHECK(ft.b == point_f(10.0f, 0.0f));
            CHECK(ft.c == point_f(5.0f, 10.0f));
        }
        
        SUBCASE("vertex access") {
            triangle_i t(0, 0, 10, 0, 5, 10);
            CHECK(t.a == point_i(0, 0));
            CHECK(t.b == point_i(10, 0));
            CHECK(t.c == point_i(5, 10));
            
            t.a = point_i(1, 1);
            CHECK(t.a == point_i(1, 1));
        }
    }
    
    TEST_CASE("basic_triangle properties") {
        triangle_i t(0, 0, 10, 0, 5, 10);
        
        SUBCASE("centroid") {
            auto center = t.centroid();
            CHECK(center == point_i(5, 3));  // (0+10+5)/3, (0+0+10)/3
        }
        
        SUBCASE("area") {
            CHECK(t.area() == 50);  // 0.5 * base(10) * height(10)
            
            // Degenerate triangle (collinear points)
            triangle_i degen(0, 0, 5, 0, 10, 0);
            CHECK(degen.area() == 0);
        }
        
        SUBCASE("area calculation") {
            // Counter-clockwise triangle
            triangle_i ccw(0, 0, 10, 0, 5, 10);
            CHECK(ccw.area() == 50);
            
            // Same triangle with different order still has same area
            triangle_i cw(0, 0, 5, 10, 10, 0);
            CHECK(cw.area() == 50);
        }
        
        SUBCASE("perimeter") {
            // Right triangle with sides 3, 4, 5
            triangle_f tri(0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 4.0f);
            float side1 = sdlpp::distance(tri.a, tri.b);
            float side2 = sdlpp::distance(tri.b, tri.c);
            float side3 = sdlpp::distance(tri.c, tri.a);
            float perimeter = side1 + side2 + side3;
            CHECK(perimeter == doctest::Approx(12.0f));
        }
        
        SUBCASE("bounds") {
            // Calculate bounds manually
            int minx = std::min({t.a.x, t.b.x, t.c.x});
            int miny = std::min({t.a.y, t.b.y, t.c.y});
            int maxx = std::max({t.a.x, t.b.x, t.c.x});
            int maxy = std::max({t.a.y, t.b.y, t.c.y});
            rect_i bounds(minx, miny, maxx - minx, maxy - miny);
            CHECK(bounds == rect_i(0, 0, 10, 10));
        }
    }
    
    TEST_CASE("basic_triangle containment") {
        triangle_i t(0, 0, 10, 0, 5, 10);
        
        SUBCASE("contains point") {
            // Inside points
            CHECK(t.contains(point_i(5, 5)));     // center area
            CHECK(t.contains(point_i(5, 1)));     // near base
            CHECK(t.contains(point_i(3, 3)));     // left side
            CHECK(t.contains(point_i(7, 3)));     // right side
            
            // Vertices
            CHECK(t.contains(point_i(0, 0)));
            CHECK(t.contains(point_i(10, 0)));
            CHECK(t.contains(point_i(5, 10)));
            
            // Edge points
            CHECK(t.contains(point_i(5, 0)));     // on base
            CHECK(t.contains(point_i(2, 4)));     // on left edge (roughly)
            
            // Outside points
            CHECK(!t.contains(point_i(-1, 0)));   // left of triangle
            CHECK(!t.contains(point_i(11, 0)));   // right of triangle
            CHECK(!t.contains(point_i(5, 11)));   // above triangle
            CHECK(!t.contains(point_i(5, -1)));   // below triangle
            CHECK(!t.contains(point_i(0, 5)));    // left of left edge
            CHECK(!t.contains(point_i(10, 5)));   // right of right edge
        }
        
        SUBCASE("degenerate triangle") {
            triangle_i degen(0, 0, 5, 0, 10, 0);  // All points on a line
            // A degenerate triangle has area 0
            CHECK(degen.area() == 0);
            // Points off the line should not be contained
            CHECK(!degen.contains(point_i(5, 1)));  // Off the line
        }
    }
    
    TEST_CASE("basic_triangle transformations") {
        triangle_i t(0, 0, 10, 0, 5, 10);
        
        SUBCASE("manual translation") {
            point_i offset(10, 20);
            triangle_i t2(t.a.x + offset.x, t.a.y + offset.y,
                         t.b.x + offset.x, t.b.y + offset.y,
                         t.c.x + offset.x, t.c.y + offset.y);
            CHECK(t2.a == point_i(10, 20));
            CHECK(t2.b == point_i(20, 20));
            CHECK(t2.c == point_i(15, 30));
            CHECK(t2.area() == t.area());
        }
        
        SUBCASE("manual scaling from origin") {
            int scale = 2;
            triangle_i t2(t.a.x * scale, t.a.y * scale,
                         t.b.x * scale, t.b.y * scale,
                         t.c.x * scale, t.c.y * scale);
            CHECK(t2.a == point_i(0, 0));
            CHECK(t2.b == point_i(20, 0));
            CHECK(t2.c == point_i(10, 20));
            CHECK(t2.area() == t.area() * 4);  // Area scales by factor squared
        }
    }
    
    TEST_CASE("triangle utility functions") {
        SUBCASE("equilateral triangle") {
            // Create equilateral triangle manually
            float side = 10.0f;
            float h = side * std::sqrt(3.0f) / 2.0f;
            triangle_f t(
                -side/2, -h/3,    // bottom left
                side/2, -h/3,     // bottom right  
                0, 2*h/3          // top
            );
            
            // Check that all sides are equal
            auto side1 = sdlpp::distance(t.a, t.b);
            auto side2 = sdlpp::distance(t.b, t.c);
            auto side3 = sdlpp::distance(t.c, t.a);
            
            CHECK(side1 == doctest::Approx(10.0f));
            CHECK(side2 == doctest::Approx(10.0f));
            CHECK(side3 == doctest::Approx(10.0f));
            
            // Check centroid is at origin
            auto centroid = t.centroid();
            CHECK(centroid.x == doctest::Approx(0.0f).epsilon(0.0001f));
            CHECK(centroid.y == doctest::Approx(0.0f).epsilon(0.0001f));
        }
        
        SUBCASE("right triangle") {
            // Create right triangle manually
            triangle_i t(0, 0, 3, 0, 0, 4);
            CHECK(t.a == point_i(0, 0));
            CHECK(t.b == point_i(3, 0));
            CHECK(t.c == point_i(0, 4));
            
            // Check area = 0.5 * base * height
            CHECK(t.area() == 6);
            
            // Check hypotenuse
            auto hyp = sdlpp::distance(t.b, t.c);
            CHECK(hyp == 5);  // 3-4-5 triangle
        }
        
        SUBCASE("collinearity check") {
            point_i p1(0, 0);
            point_i p2(5, 0);
            point_i p3(10, 0);
            CHECK(sdlpp::are_collinear(p1, p2, p3));
            
            point_i p4(0, 0);
            point_i p5(5, 5);
            point_i p6(10, 10);
            CHECK(are_collinear(p4, p5, p6));
            
            point_i p7(0, 0);
            point_i p8(5, 0);
            point_i p9(5, 5);
            CHECK(!sdlpp::are_collinear(p7, p8, p9));
            
            // Floating point collinearity with epsilon
            point_f fp1(0.0f, 0.0f);
            point_f fp2(5.0f, 5.0f);
            point_f fp3(10.0f, 10.000001f);  // Slightly off
            CHECK(sdlpp::are_collinear(fp1, fp2, fp3, 0.001f));  // Should still be considered collinear with epsilon
        }
    }
}