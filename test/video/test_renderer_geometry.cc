//
// Test renderer geometry concept updates
//

#include <doctest/doctest.h>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/window.hh>

// Custom types for testing
namespace test_types {
    struct CustomPoint {
        using value_type = float;
        float x, y;
    };
    
    struct CustomRect {
        using value_type = int;
        int x, y, w, h;
    };
    
    struct CustomTriangle {
        using value_type = float;
        CustomPoint a, b, c;
    };
    
    // Helper functions for triangle_like concept
    constexpr const CustomPoint& get_a(const CustomTriangle& t) { return t.a; }
    constexpr const CustomPoint& get_b(const CustomTriangle& t) { return t.b; }
    constexpr const CustomPoint& get_c(const CustomTriangle& t) { return t.c; }
}

// Test with built-in geometry types
TEST_CASE("renderer accepts built-in geometry types") {
    // Create window and renderer
    auto window_result = sdlpp::window::create("Test", 800, 600);
    REQUIRE(window_result.has_value());
    
    auto renderer_result = sdlpp::renderer::create(*window_result);
    REQUIRE(renderer_result.has_value());
    
    auto& renderer = *renderer_result;
    
    // Test draw_point with built-in types
    {
        CHECK(renderer.draw_point(100, 200).has_value());
        CHECK(renderer.draw_point(sdlpp::point_i{100, 200}).has_value());
        CHECK(renderer.draw_point(100.5f, 200.5f).has_value());
        CHECK(renderer.draw_point(sdlpp::point_f{100.5f, 200.5f}).has_value());
    }
    
    // Test draw_rect with built-in types
    {
        CHECK(renderer.draw_rect(sdlpp::rect_i{10, 20, 100, 200}).has_value());
        CHECK(renderer.draw_rect(sdlpp::rect_f{10.5f, 20.5f, 100.5f, 200.5f}).has_value());
    }
    
    // Test viewport/clip with built-in types
    {
        CHECK(renderer.set_viewport(std::optional<sdlpp::rect_i>{sdlpp::rect_i{0, 0, 400, 300}}).has_value());
        auto viewport = renderer.get_viewport();
        CHECK(viewport.has_value());
        
        CHECK(renderer.set_clip_rect(std::optional<sdlpp::rect_i>{sdlpp::rect_i{10, 10, 380, 280}}).has_value());
        auto clip = renderer.get_clip_rect();
        CHECK(clip.has_value());
    }
    
    // Test get_scale and get_output_size
    {
        auto scale = renderer.get_scale();
        CHECK(scale.has_value());
        
        auto output_size = renderer.get_output_size();
        CHECK(output_size.has_value());
        
        auto current_size = renderer.get_current_output_size();
        CHECK(current_size.has_value());
    }
}

// Test with custom geometry types
TEST_CASE("renderer accepts custom geometry types") {
    using namespace test_types;
    
    // Verify concepts
    static_assert(sdlpp::point_like<CustomPoint>);
    static_assert(sdlpp::rect_like<CustomRect>);
    static_assert(sdlpp::triangle_like<CustomTriangle>);
    
    // Create window and renderer
    auto window_result = sdlpp::window::create("Test", 800, 600);
    REQUIRE(window_result.has_value());
    
    auto renderer_result = sdlpp::renderer::create(*window_result);
    REQUIRE(renderer_result.has_value());
    
    auto& renderer = *renderer_result;
    
    // Test draw_point with custom point
    {
        CHECK(renderer.draw_point(CustomPoint{100.5f, 200.5f}).has_value());
    }
    
    // Test draw_rect with custom rect
    {
        CHECK(renderer.draw_rect(CustomRect{10, 20, 100, 200}).has_value());
    }
    
    // Test draw_points with container of custom points
    {
        std::vector<CustomPoint> points = {
            {10.0f, 10.0f},
            {20.0f, 20.0f},
            {30.0f, 30.0f}
        };
        CHECK(renderer.draw_points(points).has_value());
    }
    
    // Test draw_lines with container of custom points
    {
        std::array<CustomPoint, 4> line_points = {{
            {50.0f, 50.0f},
            {100.0f, 50.0f},
            {100.0f, 100.0f},
            {50.0f, 100.0f}
        }};
        CHECK(renderer.draw_lines(line_points).has_value());
    }
    
    // Test draw_rects with container of custom rects
    {
        std::vector<CustomRect> rects = {
            {10, 10, 50, 50},
            {70, 70, 50, 50},
            {130, 130, 50, 50}
        };
        CHECK(renderer.draw_rects(rects).has_value());
    }
    
    // Test fill_rect and fill_rects
    {
        CHECK(renderer.fill_rect(CustomRect{200, 200, 100, 100}).has_value());
        
        std::vector<CustomRect> fill_rects = {
            {310, 310, 30, 30},
            {350, 350, 30, 30}
        };
        CHECK(renderer.fill_rects(fill_rects).has_value());
    }
    
    // Test viewport and clipping with custom rect
    {
        CHECK(renderer.set_viewport(std::optional<CustomRect>{CustomRect{0, 0, 400, 300}}).has_value());
        auto viewport = renderer.get_viewport<CustomRect>();
        CHECK(viewport.has_value());
        if (viewport) {
            CHECK(viewport->w == 400);
            CHECK(viewport->h == 300);
        }
        
        CHECK(renderer.set_clip_rect(std::optional<CustomRect>{CustomRect{10, 10, 380, 280}}).has_value());
        auto clip = renderer.get_clip_rect<CustomRect>();
        CHECK(clip.has_value());
        if (clip && *clip) {
            CHECK((*clip)->w == 380);
            CHECK((*clip)->h == 280);
        }
    }
    
    // Test get_scale with custom point type
    {
        auto scale = renderer.get_scale<CustomPoint>();
        CHECK(scale.has_value());
    }
    
    // Test render_triangle with custom triangle
    {
        CustomTriangle tri = {
            {100.0f, 100.0f},
            {150.0f, 100.0f},
            {125.0f, 150.0f}
        };
        CHECK(renderer.render_triangle(tri, sdlpp::color{255, 0, 0, 255}).has_value());
    }
    
    // Test make_vertex with custom point types
    {
        CustomPoint pos{100.0f, 200.0f};
        CustomPoint tex_coord{0.5f, 0.5f};
        auto vertex = sdlpp::renderer::make_vertex(pos, sdlpp::color{255, 255, 255, 255}, tex_coord);
        CHECK(vertex.position.x == 100.0f);
        CHECK(vertex.position.y == 200.0f);
        CHECK(vertex.tex_coord.x == 0.5f);
        CHECK(vertex.tex_coord.y == 0.5f);
    }
}

// Test that defaults work when types aren't specified
TEST_CASE("renderer geometry methods work with default types") {
    auto window_result = sdlpp::window::create("Test", 800, 600);
    REQUIRE(window_result.has_value());
    
    auto renderer_result = sdlpp::renderer::create(*window_result);
    REQUIRE(renderer_result.has_value());
    
    auto& renderer = *renderer_result;
    
    // These should use the default built-in types
    auto viewport = renderer.get_viewport();
    CHECK(viewport.has_value());
    
    auto clip = renderer.get_clip_rect();
    CHECK(clip.has_value());
    
    auto scale = renderer.get_scale();
    CHECK(scale.has_value());
    
    auto size = renderer.get_output_size();
    CHECK(size.has_value());
    
    auto current = renderer.get_current_output_size();
    CHECK(current.has_value());
}