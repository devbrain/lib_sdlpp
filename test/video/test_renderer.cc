//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <vector>

#include "sdlpp/video/renderer.hh"
#include "sdlpp/video/texture.hh"
#include "sdlpp/video/surface.hh"

using namespace sdlpp;

TEST_SUITE("renderer and texture") {
    
    // Note: These tests require SDL video subsystem to be initialized
    // and a window to be created for the renderer
    
    TEST_CASE("renderer construction") {
        SUBCASE("default construction") {
            renderer r;
            CHECK(!r.is_valid());
            CHECK(!r);
        }
        
        SUBCASE("move semantics") {
            renderer r1;
            // Would need a window to create a valid renderer
            
            renderer r2(std::move(r1));
            CHECK(!r1.is_valid());
        }
    }
    
    TEST_CASE("blend mode and scale mode enums") {
        SUBCASE("blend mode values") {
            // Just verify the enums compile and have expected values
            CHECK(static_cast<int>(blend_mode::none) == SDL_BLENDMODE_NONE);
            CHECK(static_cast<int>(blend_mode::blend) == SDL_BLENDMODE_BLEND);
            CHECK(static_cast<int>(blend_mode::add) == SDL_BLENDMODE_ADD);
            CHECK(static_cast<int>(blend_mode::mod) == SDL_BLENDMODE_MOD);
            CHECK(static_cast<int>(blend_mode::mul) == SDL_BLENDMODE_MUL);
        }
        
        SUBCASE("scale mode values") {
            CHECK(static_cast<int>(scale_mode::nearest) == SDL_SCALEMODE_NEAREST);
            CHECK(static_cast<int>(scale_mode::linear) == SDL_SCALEMODE_LINEAR);
        }
        
        SUBCASE("flip mode bitwise operations") {
            auto both = flip_mode::horizontal | flip_mode::vertical;
            CHECK(static_cast<uint32_t>(both) == 
                  (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));
            
            flip_mode mode = flip_mode::horizontal;
            mode |= flip_mode::vertical;
            CHECK(static_cast<uint32_t>(mode) == 
                  (SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL));
        }
        
        SUBCASE("renderer driver names") {
            // Just check that driver names are defined
            CHECK(renderer_driver::software != nullptr);
            CHECK(renderer_driver::opengl != nullptr);
        }
    }
    
    TEST_CASE("renderer drawing operations without window") {
        // Create a software renderer on a surface
        auto surf_result = surface::create_rgb(320, 240, pixel_format_enum::RGBA8888);
        if (!surf_result) {
            MESSAGE("Cannot create surface for software renderer");
            return;
        }
        
        auto rend_result = renderer::create_software(surf_result->get());
        if (!rend_result) {
            MESSAGE("Cannot create software renderer: " << rend_result.error());
            return;
        }
        
        auto& rend = *rend_result;
        
        SUBCASE("draw color") {
            auto set_result = rend.set_draw_color(colors::red);
            CHECK(set_result.has_value());
            
            auto get_result = rend.get_draw_color();
            CHECK(get_result.has_value());
            if (get_result) {
                CHECK(*get_result == colors::red);
            }
        }
        
        SUBCASE("clear and present") {
            auto clear_result = rend.clear();
            CHECK(clear_result.has_value());
            
            auto present_result = rend.present();
            CHECK(present_result.has_value());
        }
        
        SUBCASE("draw primitives") {
            auto color_result = rend.set_draw_color(colors::white);
            CHECK(color_result.has_value());
            
            // Draw point
            auto point_result = rend.draw_point(10, 10);
            CHECK(point_result.has_value());
            
            // Draw point with point object
            point p{20, 20};
            auto point2_result = rend.draw_point(p);
            CHECK(point2_result.has_value());
            
            // Draw line
            auto line_result = rend.draw_line(0, 0, 100, 100);
            CHECK(line_result.has_value());
            
            // Draw rectangle
            rect r{50, 50, 100, 80};
            auto rect_result = rend.draw_rect(r);
            CHECK(rect_result.has_value());
            
            // Fill rectangle
            auto fill_result = rend.fill_rect(r);
            CHECK(fill_result.has_value());
        }
        
        SUBCASE("draw multiple primitives") {
            std::vector<point_i> points = {
                {10, 10}, {20, 20}, {30, 30}, {40, 40}
            };
            
            auto points_result = rend.draw_points(points);
            CHECK(points_result.has_value());
            
            auto lines_result = rend.draw_lines(points);
            CHECK(lines_result.has_value());
            
            std::vector<rect_i> rects = {
                {10, 10, 20, 20},
                {40, 40, 30, 30},
                {80, 80, 40, 40}
            };
            
            auto rects_result = rend.draw_rects(rects);
            CHECK(rects_result.has_value());
            
            auto fill_rects_result = rend.fill_rects(rects);
            CHECK(fill_rects_result.has_value());
        }
        
        SUBCASE("blend mode") {
            auto set_result = rend.set_draw_blend_mode(blend_mode::blend);
            CHECK(set_result.has_value());
            
            auto get_result = rend.get_draw_blend_mode();
            CHECK(get_result.has_value());
            if (get_result) {
                CHECK(*get_result == blend_mode::blend);
            }
        }
        
        SUBCASE("viewport and clipping") {
            rect_i viewport{10, 10, 100, 100};
            auto set_vp = rend.set_viewport(std::make_optional(viewport));
            CHECK(set_vp.has_value());
            
            auto get_vp = rend.get_viewport();
            CHECK(get_vp.has_value());
            if (get_vp) {
                CHECK(*get_vp == viewport);
            }
            
            // Reset viewport
            auto reset_vp = rend.set_viewport(std::optional<rect_i>{});
            CHECK(reset_vp.has_value());
            
            // Clipping
            rect_i clip{20, 20, 80, 80};
            auto set_clip = rend.set_clip_rect(std::make_optional(clip));
            CHECK(set_clip.has_value());
            
            CHECK(rend.is_clip_enabled());
            
            auto get_clip = rend.get_clip_rect();
            CHECK(get_clip.has_value());
            if (get_clip && *get_clip) {
                CHECK(**get_clip == clip);
            }
        }
        
        SUBCASE("scale") {
            auto set_scale = rend.set_scale(2.0f, 2.0f);
            CHECK(set_scale.has_value());
            
            auto get_scale = rend.get_scale();
            CHECK(get_scale.has_value());
            if (get_scale) {
                CHECK(get_scale->x == 2.0f);
                CHECK(get_scale->y == 2.0f);
            }
        }
        
        SUBCASE("output size") {
            auto size = rend.get_output_size();
            CHECK(size.has_value());
            if (size) {
                CHECK(size->width == 320);
                CHECK(size->height == 240);
            }
            
            auto current_size = rend.get_current_output_size();
            CHECK(current_size.has_value());
        }
    }
    
    TEST_CASE("texture operations") {
        // Create surface and renderer for texture tests
        auto surf_result = surface::create_rgb(320, 240, pixel_format_enum::RGBA8888);
        if (!surf_result) return;
        
        auto rend_result = renderer::create_software(surf_result->get());
        if (!rend_result) return;
        
        auto& rend = *rend_result;
        
        SUBCASE("texture creation") {
            auto tex_result = texture::create(
                rend, 
                pixel_format_enum::RGBA8888,
                texture_access::static_access,
                64, 64
            );
            
            CHECK(tex_result.has_value());
            if (tex_result) {
                auto& tex = *tex_result;
                CHECK(tex.is_valid());
                
                auto size = tex.get_size();
                CHECK(size.has_value());
                if (size) {
                    CHECK(size->width == 64);
                    CHECK(size->height == 64);
                }
            }
        }
        
        SUBCASE("texture from surface") {
            // Create a small surface
            auto tex_surf = surface::create_rgb(32, 32, pixel_format_enum::RGBA8888);
            if (!tex_surf) return;
            
            // Fill with color
            auto fill_result = tex_surf->fill(colors::blue);
            CHECK(fill_result.has_value());
            
            auto tex_result = texture::create(rend, *tex_surf);
            CHECK(tex_result.has_value());
            if (tex_result) {
                CHECK(tex_result->is_valid());
                
                auto size = tex_result->get_size();
                CHECK(size.has_value());
                if (size) {
                    CHECK(size->width == 32);
                    CHECK(size->height == 32);
                }
            }
        }
        
        SUBCASE("texture modulation") {
            auto tex_result = texture::create(
                rend, 
                pixel_format_enum::RGBA8888,
                texture_access::static_access,
                32, 32
            );
            
            if (!tex_result) return;
            auto& tex = *tex_result;
            
            // Color modulation
            auto set_color = tex.set_color_mod(color{128, 255, 128});
            CHECK(set_color.has_value());
            
            auto get_color = tex.get_color_mod();
            CHECK(get_color.has_value());
            if (get_color) {
                CHECK(get_color->r == 128);
                CHECK(get_color->g == 255);
                CHECK(get_color->b == 128);
            }
            
            // Alpha modulation
            auto set_alpha = tex.set_alpha_mod(128);
            CHECK(set_alpha.has_value());
            
            auto get_alpha = tex.get_alpha_mod();
            CHECK(get_alpha.has_value());
            if (get_alpha) {
                CHECK(*get_alpha == 128);
            }
            
            // Blend mode
            auto set_blend = tex.set_blend_mode(blend_mode::blend);
            CHECK(set_blend.has_value());
            
            auto get_blend = tex.get_blend_mode();
            CHECK(get_blend.has_value());
            if (get_blend) {
                CHECK(*get_blend == blend_mode::blend);
            }
            
            // Scale mode
            auto set_scale = tex.set_scale_mode(scale_mode::linear);
            CHECK(set_scale.has_value());
            
            auto get_scale = tex.get_scale_mode();
            CHECK(get_scale.has_value());
            if (get_scale) {
                CHECK(*get_scale == scale_mode::linear);
            }
        }
        
        SUBCASE("streaming texture") {
            auto tex_result = texture::create(
                rend, 
                pixel_format_enum::RGBA8888,
                texture_access::streaming,
                16, 16
            );
            
            if (!tex_result) return;
            auto& tex = *tex_result;
            
            // Lock texture
            {
                texture::lock_guard lock(tex, std::optional<rect_i>{});
                CHECK(lock.is_locked());
                CHECK(lock.pixels != nullptr);
                CHECK(lock.pitch > 0);
                
                // Could write to pixels here
            }
            // Automatically unlocked
            
            // Update texture
            std::vector<uint32_t> pixels(16 * 16, 0xFF0000FF); // Red pixels
            auto update = tex.update(std::optional<rect_i>{}, pixels.data(), 16 * sizeof(uint32_t));
            CHECK(update.has_value());
        }
        
        SUBCASE("render texture") {
            auto tex_result = texture::create(
                rend, 
                pixel_format_enum::RGBA8888,
                texture_access::static_access,
                32, 32
            );
            
            if (!tex_result) return;
            auto& tex = *tex_result;
            
            // Basic copy
            auto copy = rend.copy(tex, std::optional<rect_i>{}, std::optional<rect_i>{});
            CHECK(copy.has_value());
            
            // Copy with source and destination
            rect_i src{0, 0, 16, 16};
            rect_i dst{100, 100, 32, 32};
            auto copy2 = rend.copy(tex, std::make_optional(src), std::make_optional(dst));
            CHECK(copy2.has_value());
            
            // Copy with rotation and flip
            auto copy_ex = rend.copy_ex(tex, std::optional<rect_i>{}, std::make_optional(dst), 45.0, std::optional<point_i>{}, flip_mode::horizontal);
            CHECK(copy_ex.has_value());
            
            // Floating point destination
            rect_f fdst{50.5f, 50.5f, 64.0f, 64.0f};
            auto copy_f = rend.copy(tex, std::optional<rect_f>{}, std::optional<rect_f>{fdst});
            CHECK(copy_f.has_value());
        }
        
        SUBCASE("render target texture") {
            auto target_tex = texture::create(
                rend,
                pixel_format_enum::RGBA8888,
                texture_access::target,
                128, 128
            );
            
            if (!target_tex) return;
            
            // Set as render target
            auto set_target = rend.set_target(*target_tex);
            CHECK(set_target.has_value());
            
            // Draw to texture
            auto color_res = rend.set_draw_color(colors::green);
            CHECK(color_res.has_value());
            auto clear_res = rend.clear();
            CHECK(clear_res.has_value());
            auto fill_res = rend.fill_rect(rect{10, 10, 50, 50});
            CHECK(fill_res.has_value());
            
            // Reset to default target
            auto reset_target = rend.set_target(texture());
            CHECK(reset_target.has_value());
            
            // Now the texture contains our drawing
            auto copy = rend.copy(*target_tex, std::optional<rect_i>{}, std::optional<rect_i>{});
            CHECK(copy.has_value());
        }
    }
    
    TEST_CASE("error handling") {
        renderer invalid_rend;
        texture invalid_tex;
        
        SUBCASE("invalid renderer operations") {
            auto clear = invalid_rend.clear();
            CHECK(!clear.has_value());
            
            auto color = invalid_rend.set_draw_color(colors::red);
            CHECK(!color.has_value());
            
            auto point = invalid_rend.draw_point(10, 10);
            CHECK(!point.has_value());
        }
        
        SUBCASE("invalid texture operations") {
            auto size = invalid_tex.get_size();
            CHECK(!size.has_value());
            
            auto blend = invalid_tex.set_blend_mode(blend_mode::blend);
            CHECK(!blend.has_value());
        }
        
        SUBCASE("invalid cross operations") {
            // Valid renderer but invalid texture
            auto surf = surface::create_rgb(100, 100, pixel_format_enum::RGBA8888);
            if (!surf) return;
            
            auto rend = renderer::create_software(surf->get());
            if (!rend) return;
            
            auto copy = rend->copy(invalid_tex, std::optional<rect_i>{}, std::optional<rect_i>{});
            CHECK(!copy.has_value());
            CHECK(copy.error() == "Invalid texture");
        }
    }
    
    TEST_CASE("geometry rendering") {
        // Create surface and renderer for geometry tests
        auto surf_result = surface::create_rgb(320, 240, pixel_format_enum::RGBA8888);
        if (!surf_result) return;
        
        auto rend_result = renderer::create_software(surf_result->get());
        if (!rend_result) return;
        
        auto& rend = *rend_result;
        
        SUBCASE("render single triangle") {
            // Create vertices for a simple triangle
            SDL_Vertex v0 = renderer::make_vertex(point_f{10.0f, 10.0f}, colors::red);
            SDL_Vertex v1 = renderer::make_vertex(point_f{50.0f, 10.0f}, colors::green);
            SDL_Vertex v2 = renderer::make_vertex(point_f{30.0f, 50.0f}, colors::blue);
            
            auto result = rend.render_triangle(v0, v1, v2);
            CHECK(result.has_value());
        }
        
        SUBCASE("render triangle from geometry types") {
            triangle_f tri{{10.0f, 10.0f}, {50.0f, 10.0f}, {30.0f, 50.0f}};
            auto result = rend.render_triangle(tri, colors::white);
            CHECK(result.has_value());
        }
        
        SUBCASE("render textured triangle") {
            // Create a small texture
            auto tex_surf = surface::create_rgb(32, 32, pixel_format_enum::RGBA8888);
            if (!tex_surf) return;
            
            auto fill_result = tex_surf->fill(colors::blue);
            CHECK(fill_result.has_value());
            
            auto tex_result = texture::create(rend, *tex_surf);
            if (!tex_result) return;
            
            triangle_f tri{{10.0f, 10.0f}, {50.0f, 10.0f}, {30.0f, 50.0f}};
            triangle_f tex_coords{{0.0f, 0.0f}, {1.0f, 0.0f}, {0.5f, 1.0f}};
            
            auto result = rend.render_textured_triangle(
                tex_result->get(), tri, colors::white, tex_coords);
            CHECK(result.has_value());
        }
        
        SUBCASE("render multiple triangles") {
            // Create vertices for two triangles forming a quad
            std::vector<SDL_Vertex> vertices = {
                renderer::make_vertex(point_f{10.0f, 10.0f}, colors::red, point_f{0.0f, 0.0f}),
                renderer::make_vertex(point_f{50.0f, 10.0f}, colors::green, point_f{1.0f, 0.0f}),
                renderer::make_vertex(point_f{50.0f, 50.0f}, colors::blue, point_f{1.0f, 1.0f}),
                renderer::make_vertex(point_f{10.0f, 50.0f}, colors::white, point_f{0.0f, 1.0f})
            };
            
            std::vector<int> indices = {
                0, 1, 2,  // First triangle
                0, 2, 3   // Second triangle
            };
            
            auto result = rend.render_geometry(vertices, indices);
            CHECK(result.has_value());
        }
        
        SUBCASE("error cases") {
            // Empty vertices
            std::vector<SDL_Vertex> empty_verts;
            std::vector<int> indices = {0, 1, 2};
            auto result1 = rend.render_geometry(empty_verts, indices);
            CHECK(result1.has_value()); // Should succeed but render nothing
            
            // Empty indices
            std::vector<SDL_Vertex> vertices = {
                renderer::make_vertex(point_f{0, 0}, colors::white),
                renderer::make_vertex(point_f{10, 0}, colors::white),
                renderer::make_vertex(point_f{5, 10}, colors::white)
            };
            std::vector<int> empty_indices;
            auto result2 = rend.render_geometry(vertices, empty_indices);
            CHECK(result2.has_value()); // Should succeed but render nothing
            
            // Invalid index count (not multiple of 3)
            std::vector<int> bad_indices = {0, 1, 2, 3, 4}; // 5 indices
            auto result3 = rend.render_geometry(vertices, bad_indices);
            CHECK(!result3.has_value());
            CHECK(result3.error() == "Index count must be multiple of 3 for triangles");
        }
        
        SUBCASE("vertex creation helper") {
            // Test vertex creation with different parameters
            auto v1 = renderer::make_vertex(point_f{10.5f, 20.5f}, colors::red);
            CHECK(v1.position.x == 10.5f);
            CHECK(v1.position.y == 20.5f);
            CHECK(v1.color.r == 1.0f);
            CHECK(v1.color.g == 0.0f);
            CHECK(v1.color.b == 0.0f);
            CHECK(v1.color.a == 1.0f);
            CHECK(v1.tex_coord.x == 0.0f);
            CHECK(v1.tex_coord.y == 0.0f);
            
            // With texture coordinates
            auto v2 = renderer::make_vertex(point_f{30.0f, 40.0f}, colors::green, point_f{0.5f, 0.5f});
            CHECK(v2.tex_coord.x == 0.5f);
            CHECK(v2.tex_coord.y == 0.5f);
            
            // Test color conversion (0-255 to 0.0-1.0)
            color semi_transparent{128, 64, 32, 128};
            auto v3 = renderer::make_vertex(point_f{0, 0}, semi_transparent);
            CHECK(v3.color.r == doctest::Approx(128.0f / 255.0f));
            CHECK(v3.color.g == doctest::Approx(64.0f / 255.0f));
            CHECK(v3.color.b == doctest::Approx(32.0f / 255.0f));
            CHECK(v3.color.a == doctest::Approx(128.0f / 255.0f));
        }
    }
}