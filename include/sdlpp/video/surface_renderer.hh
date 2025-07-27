/**
 * @file surface_renderer.hh
 * @brief Surface-based software renderer with DDA drawing capabilities
 */

#pragma once

#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/video/blend_mode.hh>
#include <sdlpp/video/pixels.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/utility/geometry_concepts.hh>
#include <sdlpp/video/surface.hh>
#include <memory>
#include <optional>
#include <functional>
#include <cstdlib>
#include <algorithm>
#include <climits>
#include <vector>
#include <euler/dda/aa_line_iterator.hh>
#include <euler/dda/thick_line_iterator.hh>
#include <euler/dda/circle_iterator.hh>
#include <euler/dda/ellipse_iterator.hh>
#include <euler/dda/bezier_iterator.hh>
#include <euler/dda/batched_bezier_iterator.hh>
#include <euler/dda/bspline_iterator.hh>
#include <euler/dda/pixel_batch.hh>
#include <euler/angles/angle.hh>

namespace sdlpp {

/**
 * @brief Software renderer that operates directly on SDL surfaces
 * 
 * This class provides the same DDA drawing capabilities as the hardware renderer
 * but operates directly on surface memory for software rendering.
 */
class surface_renderer {
public:
    /**
     * @brief Construct renderer for existing surface
     * @param surface Surface to render to (not owned)
     */
    SDLPP_EXPORT explicit surface_renderer(const surface& surface);
    
    /**
     * @brief Construct renderer with new surface
     * @param width Surface width
     * @param height Surface height
     * @param format Pixel format
     */
    SDLPP_EXPORT surface_renderer(int width, int height, SDL_PixelFormat format);
    
    /**
     * @brief Destructor
     */
    SDLPP_EXPORT ~surface_renderer();
    
    // Move operations
    SDLPP_EXPORT surface_renderer(surface_renderer&& other) noexcept;
    SDLPP_EXPORT surface_renderer& operator=(surface_renderer&& other) noexcept;
    
    // Delete copy operations
    surface_renderer(const surface_renderer&) = delete;
    surface_renderer& operator=(const surface_renderer&) = delete;
    
    /**
     * @brief Get the underlying surface
     */
    SDL_Surface* get_surface() const { return surface_; }
    
    /**
     * @brief Check if renderer owns the surface
     */
    bool owns_surface() const { return owns_surface_; }
    
    // Basic operations
    
    /**
     * @brief Clear the entire surface with current draw color
     */
    SDLPP_EXPORT expected<void, std::string> clear();
    
    /**
     * @brief Set the current drawing color
     */
    SDLPP_EXPORT expected<void, std::string> set_draw_color(const color& c);
    
    /**
     * @brief Get the current drawing color
     */
    SDLPP_EXPORT expected<color, std::string> get_draw_color() const;
    
    /**
     * @brief Set the blend mode for drawing operations
     * @param mode Blend mode to use (defaults to none)
     */
    SDLPP_EXPORT expected<void, std::string> set_draw_blend_mode(blend_mode mode = blend_mode::none);
    
    /**
     * @brief Get the current blend mode
     */
    SDLPP_EXPORT expected<blend_mode, std::string> get_draw_blend_mode() const;
    
    /**
     * @brief Set the clipping rectangle
     */
    template<rect_like R>
    expected<void, std::string> set_clip_rect(const std::optional<R>& clip) {
        if (clip.has_value()) {
            clip_rect_ = rect<int>{
                static_cast<int>(get_x(*clip)),
                static_cast<int>(get_y(*clip)),
                static_cast<int>(get_width(*clip)),
                static_cast<int>(get_height(*clip))
            };
            
            // Ensure clip rect is within surface bounds
            rect<int> surface_bounds{0, 0, surface_->w, surface_->h};
            if (!clip_rect_to_clip(*clip_rect_)) {
                // Clip rect is completely outside surface
                clip_rect_ = rect<int>{0, 0, 0, 0};
            }
        } else {
            clip_rect_ = std::nullopt;
        }
        
        return {};
    }
    
    /**
     * @brief Get the current clipping rectangle
     */
    SDLPP_EXPORT expected<std::optional<rect<int>>, std::string> get_clip_rect() const;
    
    /**
     * @brief Reset clipping (disable clipping)
     */
    expected<void, std::string> reset_clip_rect() {
        clip_rect_ = std::nullopt;
        return {};
    }
    
    /**
     * @brief Set alpha modulation for the entire surface
     */
    SDLPP_EXPORT expected<void, std::string> set_alpha_mod(uint8_t alpha);
    
    /**
     * @brief Get alpha modulation value
     */
    SDLPP_EXPORT expected<uint8_t, std::string> get_alpha_mod() const;
    
    /**
     * @brief Set color modulation for the entire surface
     */
    SDLPP_EXPORT expected<void, std::string> set_color_mod(uint8_t r, uint8_t g, uint8_t b);
    
    /**
     * @brief Get color modulation values
     */
    SDLPP_EXPORT expected<std::tuple<uint8_t, uint8_t, uint8_t>, std::string> get_color_mod() const;
    
    /**
     * @brief Check if clipping is enabled
     */
    bool is_clip_enabled() const { return clip_rect_.has_value(); }
    
    // Basic drawing methods
    
    /**
     * @brief Draw a single point
     */
    template<point_like P>
    expected<void, std::string> draw_point(const P& p) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        int x = static_cast<int>(get_x(p));
        int y = static_cast<int>(get_y(p));
        
        if (!clip_point(x, y)) {
            return {};
        }
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        put_pixel(x, y, mapped_color_);
        return {};
    }
    
    /**
     * @brief Draw a line
     */
    template<point_like P1, point_like P2>
    expected<void, std::string> draw_line(const P1& start, const P2& end) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        int x0 = static_cast<int>(get_x(start));
        int y0 = static_cast<int>(get_y(start));
        int x1 = static_cast<int>(get_x(end));
        int y1 = static_cast<int>(get_y(end));
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Bresenham's line algorithm
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = (x0 < x1) ? 1 : -1;
        int sy = (y0 < y1) ? 1 : -1;
        int err = dx - dy;
        
        while (true) {
            if (clip_point(x0, y0)) {
                put_pixel(x0, y0, mapped_color_);
            }
            
            if (x0 == x1 && y0 == y1) break;
            
            int e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
        
        return {};
    }
    
    /**
     * @brief Draw a rectangle outline
     */
    template<rect_like R>
    expected<void, std::string> draw_rect(const R& rect) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        int x = static_cast<int>(get_x(rect));
        int y = static_cast<int>(get_y(rect));
        int w = static_cast<int>(get_width(rect));
        int h = static_cast<int>(get_height(rect));
        
        if (w <= 0 || h <= 0) {
            return {};
        }
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Draw four lines
        // Top
        for (int i = x; i < x + w; ++i) {
            if (clip_point(i, y)) {
                put_pixel(i, y, mapped_color_);
            }
        }
        
        // Bottom
        for (int i = x; i < x + w; ++i) {
            if (clip_point(i, y + h - 1)) {
                put_pixel(i, y + h - 1, mapped_color_);
            }
        }
        
        // Left (skip corners to avoid overdraw)
        for (int i = y + 1; i < y + h - 1; ++i) {
            if (clip_point(x, i)) {
                put_pixel(x, i, mapped_color_);
            }
        }
        
        // Right (skip corners to avoid overdraw)
        for (int i = y + 1; i < y + h - 1; ++i) {
            if (clip_point(x + w - 1, i)) {
                put_pixel(x + w - 1, i, mapped_color_);
            }
        }
        
        return {};
    }
    
    /**
     * @brief Fill a rectangle
     */
    template<rect_like R>
    expected<void, std::string> fill_rect(const R& r) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        rect<int> rect_to_fill{
            static_cast<int>(get_x(r)),
            static_cast<int>(get_y(r)),
            static_cast<int>(get_width(r)),
            static_cast<int>(get_height(r))
        };
        
        if (rect_to_fill.w <= 0 || rect_to_fill.h <= 0) {
            return {};
        }
        
        // Apply clipping
        if (clip_rect_.has_value() && !clip_rect_to_clip(rect_to_fill)) {
            return {}; // Completely clipped
        }
        
        // Use SDL's efficient fill function
        SDL_Rect sdl_rect{rect_to_fill.x, rect_to_fill.y, rect_to_fill.w, rect_to_fill.h};
        if (SDL_FillSurfaceRect(surface_, &sdl_rect, mapped_color_) != 0) {
            return make_unexpected(std::string(SDL_GetError()));
        }
        
        return {};
    }
    
    // Phase 2: DDA Line Drawing Methods
    
    /**
     * @brief Draw an antialiased line using Wu's algorithm
     * @param start Start point
     * @param end End point
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P1, point_like P2>
    expected<void, std::string> draw_line_aa(const P1& start, const P2& end) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        // Convert to euler points
        euler::point2f p0{static_cast<float>(get_x(start)), static_cast<float>(get_y(start))};
        euler::point2f p1{static_cast<float>(get_x(end)), static_cast<float>(get_y(end))};
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use euler's antialiased line iterator with batch writer for better performance
        euler::dda::batch_writer<euler::dda::aa_pixel<float>> writer([this](const euler::dda::pixel_batch<euler::dda::aa_pixel<float>>& batch) {
            for (size_t i = 0; i < batch.count; ++i) {
                const auto& pixel = batch.pixels[i];
                int x = static_cast<int>(pixel.pos.x);
                int y = static_cast<int>(pixel.pos.y);
                
                if (clip_point(x, y)) {
                    blend_pixel(x, y, mapped_color_, pixel.coverage);
                }
            }
        });
        
        auto line = euler::dda::make_aa_line_iterator(p0, p1);
        while (line != euler::dda::aa_line_iterator<float>::end()) {
            writer.write(*line);
            ++line;
        }
        
        return {};
    }
    
    /**
     * @brief Draw a thick line with specified width
     * @param start Start point
     * @param end End point
     * @param width Line width in pixels
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P1, point_like P2>
    expected<void, std::string> draw_line_thick(const P1& start, const P2& end, float width) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        if (width <= 0) {
            return make_unexpected("Line width must be positive");
        }
        
        // Convert to euler points
        euler::point2f p0{static_cast<float>(get_x(start)), static_cast<float>(get_y(start))};
        euler::point2f p1{static_cast<float>(get_x(end)), static_cast<float>(get_y(end))};
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use euler's thick line iterator with batch writer for better performance
        euler::dda::batch_writer<euler::dda::pixel<int>> writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
            process_pixel_batch(batch);
        });
        
        auto line = euler::dda::make_thick_line_iterator(p0, p1, width);
        while (line != euler::dda::thick_line_iterator<float>::end()) {
            auto pixel = *line;
            euler::dda::pixel<int> int_pixel;
            int_pixel.pos.x = static_cast<int>(pixel.pos.x);
            int_pixel.pos.y = static_cast<int>(pixel.pos.y);
            writer.write(int_pixel);
            ++line;
        }
        
        return {};
    }
    
    // Phase 3: Circle and Ellipse Drawing
    
    /**
     * @brief Draw a circle outline
     * @param center Center point
     * @param radius Circle radius
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P>
    expected<void, std::string> draw_circle(const P& center, int radius) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        if (radius <= 0) {
            return make_unexpected("Circle radius must be positive");
        }
        
        // Convert to euler point
        euler::point2f c{static_cast<float>(get_x(center)), static_cast<float>(get_y(center))};
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use euler's circle iterator with batch writer for better performance
        euler::dda::batch_writer<euler::dda::pixel<int>> writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
            process_pixel_batch(batch);
        });
        
        auto circle = euler::dda::make_circle_iterator(c, static_cast<float>(radius));
        while (circle != euler::dda::circle_iterator<float>::end()) {
            auto pixel = *circle;
            euler::dda::pixel<int> int_pixel;
            int_pixel.pos.x = static_cast<int>(pixel.pos.x);
            int_pixel.pos.y = static_cast<int>(pixel.pos.y);
            writer.write(int_pixel);
            ++circle;
        }
        
        return {};
    }
    
    /**
     * @brief Fill a circle
     * @param center Center point
     * @param radius Circle radius
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P>
    expected<void, std::string> fill_circle(const P& center, int radius) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        if (radius <= 0) {
            return make_unexpected("Circle radius must be positive");
        }
        
        // Convert to euler point
        euler::point2f c{static_cast<float>(get_x(center)), static_cast<float>(get_y(center))};
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use euler's filled circle iterator
        auto filled = euler::dda::make_filled_circle_iterator(c, static_cast<float>(radius));
        while (filled != euler::dda::filled_circle_iterator<float>::end()) {
            auto span = *filled;
            int y = static_cast<int>(span.y);
            int x_start = static_cast<int>(span.x_start);
            int x_end = static_cast<int>(span.x_end);
            
            // Draw horizontal span with clipping
            if (y >= 0 && y < surface_->h) {
                if (clip_rect_) {
                    x_start = std::max(x_start, clip_rect_->x);
                    x_end = std::min(x_end, clip_rect_->x + clip_rect_->w - 1);
                }
                
                for (int x = x_start; x <= x_end; ++x) {
                    if (x >= 0 && x < surface_->w) {
                        put_pixel(x, y, mapped_color_);
                    }
                }
            }
            ++filled;
        }
        
        return {};
    }
    
    /**
     * @brief Draw an ellipse outline
     * @param center Center point
     * @param rx Horizontal radius
     * @param ry Vertical radius
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P>
    expected<void, std::string> draw_ellipse(const P& center, int rx, int ry) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        if (rx <= 0 || ry <= 0) {
            return make_unexpected("Ellipse radii must be positive");
        }
        
        // Convert to euler point
        euler::point2f c{static_cast<float>(get_x(center)), static_cast<float>(get_y(center))};
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use euler's ellipse iterator with batch writer for better performance
        euler::dda::batch_writer<euler::dda::pixel<int>> writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
            process_pixel_batch(batch);
        });
        
        auto ellipse = euler::dda::make_ellipse_iterator(c, static_cast<float>(rx), static_cast<float>(ry));
        while (ellipse != euler::dda::ellipse_iterator<float>::end()) {
            auto pixel = *ellipse;
            euler::dda::pixel<int> int_pixel;
            int_pixel.pos.x = static_cast<int>(pixel.pos.x);
            int_pixel.pos.y = static_cast<int>(pixel.pos.y);
            writer.write(int_pixel);
            ++ellipse;
        }
        
        return {};
    }
    
    /**
     * @brief Fill an ellipse
     * @param center Center point
     * @param rx Horizontal radius
     * @param ry Vertical radius
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P>
    expected<void, std::string> fill_ellipse(const P& center, int rx, int ry) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        if (rx <= 0 || ry <= 0) {
            return make_unexpected("Ellipse radii must be positive");
        }
        
        // Convert to euler point
        euler::point2f c{static_cast<float>(get_x(center)), static_cast<float>(get_y(center))};
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use euler's filled ellipse iterator
        auto filled = euler::dda::make_filled_ellipse_iterator(c, static_cast<float>(rx), static_cast<float>(ry));
        while (filled != euler::dda::filled_ellipse_iterator<float>::end()) {
            auto span = *filled;
            int y = static_cast<int>(span.y);
            int x_start = static_cast<int>(span.x_start);
            int x_end = static_cast<int>(span.x_end);
            
            // Draw horizontal span with clipping
            if (y >= 0 && y < surface_->h) {
                if (clip_rect_) {
                    x_start = std::max(x_start, clip_rect_->x);
                    x_end = std::min(x_end, clip_rect_->x + clip_rect_->w - 1);
                }
                
                for (int x = x_start; x <= x_end; ++x) {
                    if (x >= 0 && x < surface_->w) {
                        put_pixel(x, y, mapped_color_);
                    }
                }
            }
            ++filled;
        }
        
        return {};
    }
    
    /**
     * @brief Draw an ellipse arc
     * @param center Center point
     * @param rx Horizontal radius
     * @param ry Vertical radius
     * @param start_angle Start angle in radians
     * @param end_angle End angle in radians
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P>
    expected<void, std::string> draw_ellipse_arc(const P& center, int rx, int ry, float start_angle, float end_angle) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        if (rx <= 0 || ry <= 0) {
            return make_unexpected("Ellipse radii must be positive");
        }
        
        // Convert to euler point
        euler::point2f c{static_cast<float>(get_x(center)), static_cast<float>(get_y(center))};
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use euler's ellipse arc iterator with batch writer
        euler::dda::batch_writer<euler::dda::pixel<int>> writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
            process_pixel_batch(batch);
        });
        
        auto arc = euler::dda::make_ellipse_arc_iterator(c, static_cast<float>(rx), static_cast<float>(ry), 
                                                          euler::radian<float>(start_angle), euler::radian<float>(end_angle));
        while (arc != euler::dda::ellipse_iterator<float>::end()) {
            auto pixel = *arc;
            euler::dda::pixel<int> int_pixel;
            int_pixel.pos.x = static_cast<int>(pixel.pos.x);
            int_pixel.pos.y = static_cast<int>(pixel.pos.y);
            writer.write(int_pixel);
            ++arc;
        }
        
        return {};
    }
    
    /**
     * @brief Draw an ellipse arc using euler angles
     * @param center Ellipse center point
     * @param rx Horizontal radius
     * @param ry Vertical radius
     * @param start_angle Start angle
     * @param end_angle End angle
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P>
    expected<void, std::string> draw_ellipse_arc(const P& center, int rx, int ry, 
                                                euler::radian<float> start_angle, 
                                                euler::radian<float> end_angle) {
        return draw_ellipse_arc(center, rx, ry, start_angle.value(), end_angle.value());
    }
    
    // Phase 4: Bezier and Spline Curves
    
    /**
     * @brief Draw a quadratic Bezier curve
     * @param p0 Start point
     * @param p1 Control point
     * @param p2 End point
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P1, point_like P2, point_like P3>
    expected<void, std::string> draw_bezier_quad(const P1& p0, const P2& p1, const P3& p2) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        // Convert to euler points
        std::vector<euler::point2f> control_points{
            euler::point2f{static_cast<float>(get_x(p0)), static_cast<float>(get_y(p0))},
            euler::point2f{static_cast<float>(get_x(p1)), static_cast<float>(get_y(p1))},
            euler::point2f{static_cast<float>(get_x(p2)), static_cast<float>(get_y(p2))}
        };
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use bezier iterator with batch writer
        euler::dda::batch_writer<euler::dda::pixel<int>> writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
            process_pixel_batch(batch);
        });
        
        auto bezier = euler::dda::make_bezier(control_points, 0.5f);
        while (bezier != euler::dda::bezier_iterator<float>::end()) {
            auto pixel = *bezier;
            euler::dda::pixel<int> int_pixel;
            int_pixel.pos.x = static_cast<int>(pixel.pos.x);
            int_pixel.pos.y = static_cast<int>(pixel.pos.y);
            writer.write(int_pixel);
            ++bezier;
        }
        
        return {};
    }
    
    /**
     * @brief Draw a cubic Bezier curve using batched processing
     * @param p0 Start point
     * @param p1 First control point
     * @param p2 Second control point
     * @param p3 End point
     * @return Expected<void> - empty on success, error message on failure
     */
    template<point_like P1, point_like P2, point_like P3, point_like P4>
    expected<void, std::string> draw_bezier_cubic(const P1& p0, const P2& p1, const P3& p2, const P4& p3) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        // Convert to euler points
        euler::point2f ep0{static_cast<float>(get_x(p0)), static_cast<float>(get_y(p0))};
        euler::point2f ep1{static_cast<float>(get_x(p1)), static_cast<float>(get_y(p1))};
        euler::point2f ep2{static_cast<float>(get_x(p2)), static_cast<float>(get_y(p2))};
        euler::point2f ep3{static_cast<float>(get_x(p3)), static_cast<float>(get_y(p3))};
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use batched cubic bezier for better performance
        auto batched = euler::dda::make_batched_cubic_bezier(ep0, ep1, ep2, ep3, 0.5f);
        
        // Process pixel batches
        batched.process_all([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
            process_pixel_batch(batch);
        });
        
        return {};
    }
    
    /**
     * @brief Draw a B-spline curve
     * @param control_points Container of control points
     * @param degree Spline degree (default 3 for cubic)
     * @return Expected<void> - empty on success, error message on failure
     */
    template<typename Container>
    requires requires(Container c) {
        typename Container::value_type;
        requires point_like<typename Container::value_type>;
        { std::begin(c) };
        { std::end(c) };
    }
    expected<void, std::string> draw_bspline(const Container& control_points, int degree = 3) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        // Convert to euler points
        std::vector<euler::point2f> euler_points;
        euler_points.reserve(static_cast<size_t>(std::distance(std::begin(control_points), std::end(control_points))));
        
        for (const auto& p : control_points) {
            euler_points.push_back({static_cast<float>(get_x(p)), static_cast<float>(get_y(p))});
        }
        
        if (euler_points.size() < static_cast<size_t>(degree + 1)) {
            return make_unexpected("Not enough control points for specified degree");
        }
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use B-spline iterator with batch writer
        euler::dda::batch_writer<euler::dda::pixel<int>> writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
            process_pixel_batch(batch);
        });
        
        auto spline = euler::dda::make_bspline(euler_points, degree, 0.5f);
        while (spline != euler::dda::bspline_iterator<float>::end()) {
            auto pixel = *spline;
            euler::dda::pixel<int> int_pixel;
            int_pixel.pos.x = static_cast<int>(pixel.pos.x);
            int_pixel.pos.y = static_cast<int>(pixel.pos.y);
            writer.write(int_pixel);
            ++spline;
        }
        
        return {};
    }
    
    /**
     * @brief Draw a Catmull-Rom spline (interpolating spline)
     * @param points Container of points to interpolate through
     * @param tension Tension parameter (default 0.5)
     * @return Expected<void> - empty on success, error message on failure
     */
    template<typename Container>
    requires requires(Container c) {
        typename Container::value_type;
        requires point_like<typename Container::value_type>;
        { std::begin(c) };
        { std::end(c) };
    }
    expected<void, std::string> draw_catmull_rom(const Container& points, float tension = 0.5f) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        // Convert to euler points
        std::vector<euler::point2f> euler_points;
        euler_points.reserve(static_cast<size_t>(std::distance(std::begin(points), std::end(points))));
        
        for (const auto& p : points) {
            euler_points.push_back({static_cast<float>(get_x(p)), static_cast<float>(get_y(p))});
        }
        
        if (euler_points.size() < 2) {
            return make_unexpected("Need at least 2 points for Catmull-Rom spline");
        }
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Use Catmull-Rom iterator with batch writer
        euler::dda::batch_writer<euler::dda::pixel<int>> writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
            process_pixel_batch(batch);
        });
        
        auto catmull = euler::dda::make_catmull_rom(euler_points, tension);
        while (catmull != euler::dda::catmull_rom_iterator<float>::end()) {
            auto pixel = *catmull;
            euler::dda::pixel<int> int_pixel;
            int_pixel.pos.x = static_cast<int>(pixel.pos.x);
            int_pixel.pos.y = static_cast<int>(pixel.pos.y);
            writer.write(int_pixel);
            ++catmull;
        }
        
        return {};
    }
    
    // Phase 5: Polygon and Advanced Features
    
    /**
     * @brief Draw a polygon outline
     * @param vertices Container of vertices
     * @param close Whether to close the polygon (connect last to first vertex)
     * @return Expected<void> - empty on success, error message on failure
     */
    template<typename Container>
    requires requires(Container c) {
        typename Container::value_type;
        requires point_like<typename Container::value_type>;
        { std::begin(c) };
        { std::end(c) };
    }
    expected<void, std::string> draw_polygon(const Container& vertices, bool close = true) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        auto count = std::distance(std::begin(vertices), std::end(vertices));
        if (count < 2) {
            return make_unexpected("Polygon needs at least 2 vertices");
        }
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Draw edges
        auto it = std::begin(vertices);
        auto prev = it;
        ++it;
        
        while (it != std::end(vertices)) {
            // Draw line from prev to current
            euler::point2<int> p0{static_cast<int>(get_x(*prev)), static_cast<int>(get_y(*prev))};
            euler::point2<int> p1{static_cast<int>(get_x(*it)), static_cast<int>(get_y(*it))};
            
            euler::dda::batch_writer<euler::dda::pixel<int>> line_writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
                process_pixel_batch(batch);
            });
            
            auto line = euler::dda::make_line_iterator(euler::point2<float>{static_cast<float>(p0.x), static_cast<float>(p0.y)},
                                                       euler::point2<float>{static_cast<float>(p1.x), static_cast<float>(p1.y)});
            while (line != euler::dda::line_iterator<float>::end()) {
                auto pixel = *line;
                euler::dda::pixel<int> int_pixel;
                int_pixel.pos.x = static_cast<int>(pixel.pos.x);
                int_pixel.pos.y = static_cast<int>(pixel.pos.y);
                line_writer.write(int_pixel);
                ++line;
            }
            
            prev = it;
            ++it;
        }
        
        // Close polygon if requested
        if (close && count > 2) {
            auto first = std::begin(vertices);
            euler::point2<int> p0{static_cast<int>(get_x(*prev)), static_cast<int>(get_y(*prev))};
            euler::point2<int> p1{static_cast<int>(get_x(*first)), static_cast<int>(get_y(*first))};
            
            euler::dda::batch_writer<euler::dda::pixel<int>> line_writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
                process_pixel_batch(batch);
            });
            
            auto line = euler::dda::make_line_iterator(euler::point2<float>{static_cast<float>(p0.x), static_cast<float>(p0.y)},
                                                       euler::point2<float>{static_cast<float>(p1.x), static_cast<float>(p1.y)});
            while (line != euler::dda::line_iterator<float>::end()) {
                auto pixel = *line;
                euler::dda::pixel<int> int_pixel;
                int_pixel.pos.x = static_cast<int>(pixel.pos.x);
                int_pixel.pos.y = static_cast<int>(pixel.pos.y);
                line_writer.write(int_pixel);
                ++line;
            }
        }
        
        return {};
    }
    
    /**
     * @brief Fill a polygon using scanline algorithm
     * @param vertices Container of vertices
     * @return Expected<void> - empty on success, error message on failure
     */
    template<typename Container>
    requires requires(Container c) {
        typename Container::value_type;
        requires point_like<typename Container::value_type>;
        { std::begin(c) };
        { std::end(c) };
    }
    expected<void, std::string> fill_polygon(const Container& vertices) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        auto count = std::distance(std::begin(vertices), std::end(vertices));
        if (count < 3) {
            return make_unexpected("Polygon needs at least 3 vertices to fill");
        }
        
        // Convert vertices to vector for easier access
        std::vector<std::pair<int, int>> points;
        points.reserve(static_cast<size_t>(count));
        
        int min_y = INT_MAX, max_y = INT_MIN;
        for (const auto& v : vertices) {
            int x = static_cast<int>(get_x(v));
            int y = static_cast<int>(get_y(v));
            points.push_back({x, y});
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Scanline fill algorithm
        for (int y = min_y; y <= max_y; ++y) {
            std::vector<int> intersections;
            
            // Find all edge intersections with current scanline
            for (size_t i = 0; i < points.size(); ++i) {
                size_t j = (i + 1) % points.size();
                int y0 = points[i].second;
                int y1 = points[j].second;
                
                // Check if edge crosses scanline
                if ((y0 <= y && y1 > y) || (y1 <= y && y0 > y)) {
                    int x0 = points[i].first;
                    int x1 = points[j].first;
                    
                    // Calculate intersection x coordinate
                    float t = static_cast<float>(y - y0) / (y1 - y0);
                    int x = static_cast<int>(x0 + t * (x1 - x0));
                    intersections.push_back(x);
                }
            }
            
            // Sort intersections
            std::sort(intersections.begin(), intersections.end());
            
            // Fill between pairs of intersections
            for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
                int x_start = intersections[i];
                int x_end = intersections[i + 1];
                
                // Apply clipping
                if (clip_rect_) {
                    x_start = std::max(x_start, clip_rect_->x);
                    x_end = std::min(x_end, clip_rect_->x + clip_rect_->w - 1);
                }
                
                // Draw horizontal span
                if (y >= 0 && y < surface_->h) {
                    for (int x = x_start; x <= x_end; ++x) {
                        if (x >= 0 && x < surface_->w) {
                            put_pixel(x, y, mapped_color_);
                        }
                    }
                }
            }
        }
        
        return {};
    }
    
    /**
     * @brief Draw an antialiased polygon outline
     * @param vertices Container of vertices
     * @param close Whether to close the polygon
     * @return Expected<void> - empty on success, error message on failure
     */
    template<typename Container>
    requires requires(Container c) {
        typename Container::value_type;
        requires point_like<typename Container::value_type>;
        { std::begin(c) };
        { std::end(c) };
    }
    expected<void, std::string> draw_polygon_aa(const Container& vertices, bool close = true) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        auto count = std::distance(std::begin(vertices), std::end(vertices));
        if (count < 2) {
            return make_unexpected("Polygon needs at least 2 vertices");
        }
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Draw antialiased edges
        auto it = std::begin(vertices);
        auto prev = it;
        ++it;
        
        while (it != std::end(vertices)) {
            // Draw antialiased line from prev to current
            euler::point2f p0{static_cast<float>(get_x(*prev)), static_cast<float>(get_y(*prev))};
            euler::point2f p1{static_cast<float>(get_x(*it)), static_cast<float>(get_y(*it))};
            
            euler::dda::batch_writer<euler::dda::aa_pixel<float>> line_writer([this](const euler::dda::pixel_batch<euler::dda::aa_pixel<float>>& batch) {
                for (size_t i = 0; i < batch.count; ++i) {
                    const auto& pixel = batch.pixels[i];
                    int x = static_cast<int>(pixel.pos.x);
                    int y = static_cast<int>(pixel.pos.y);
                    
                    if (clip_point(x, y)) {
                        blend_pixel(x, y, mapped_color_, pixel.coverage);
                    }
                }
            });
            
            auto line = euler::dda::make_aa_line_iterator(p0, p1);
            while (line != euler::dda::aa_line_iterator<float>::end()) {
                line_writer.write(*line);
                ++line;
            }
            
            prev = it;
            ++it;
        }
        
        // Close polygon if requested
        if (close && count > 2) {
            auto first = std::begin(vertices);
            euler::point2f p0{static_cast<float>(get_x(*prev)), static_cast<float>(get_y(*prev))};
            euler::point2f p1{static_cast<float>(get_x(*first)), static_cast<float>(get_y(*first))};
            
            euler::dda::batch_writer<euler::dda::aa_pixel<float>> line_writer([this](const euler::dda::pixel_batch<euler::dda::aa_pixel<float>>& batch) {
                for (size_t i = 0; i < batch.count; ++i) {
                    const auto& pixel = batch.pixels[i];
                    int x = static_cast<int>(pixel.pos.x);
                    int y = static_cast<int>(pixel.pos.y);
                    
                    if (clip_point(x, y)) {
                        blend_pixel(x, y, mapped_color_, pixel.coverage);
                    }
                }
            });
            
            auto line = euler::dda::make_aa_line_iterator(p0, p1);
            while (line != euler::dda::aa_line_iterator<float>::end()) {
                line_writer.write(*line);
                ++line;
            }
        }
        
        return {};
    }
    
    /**
     * @brief Draw a parametric curve using a function
     * @param curve Function that takes parameter t and returns a point
     * @param t_start Start parameter value
     * @param t_end End parameter value
     * @param steps Number of steps to evaluate
     * @return Expected<void> - empty on success, error message on failure
     */
    template<typename CurveFunc>
    requires requires(CurveFunc f, float t) {
        { f(t) } -> point_like;
    }
    expected<void, std::string> draw_curve(CurveFunc&& curve, float t_start = 0.0f, float t_end = 1.0f, int steps = 100) {
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        if (steps <= 1) {
            return make_unexpected("Need at least 2 steps for curve");
        }
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        float dt = (t_end - t_start) / (steps - 1);
        
        // Evaluate first point
        auto prev_point = curve(t_start);
        
        // Draw line segments between evaluated points
        for (int i = 1; i < steps; ++i) {
            float t = t_start + i * dt;
            auto curr_point = curve(t);
            
            // Draw line from prev to current
            euler::point2<int> p0{static_cast<int>(get_x(prev_point)), static_cast<int>(get_y(prev_point))};
            euler::point2<int> p1{static_cast<int>(get_x(curr_point)), static_cast<int>(get_y(curr_point))};
            
            euler::dda::batch_writer<euler::dda::pixel<int>> line_writer([this](const euler::dda::pixel_batch<euler::dda::pixel<int>>& batch) {
                process_pixel_batch(batch);
            });
            
            auto line = euler::dda::make_line_iterator(euler::point2<float>{static_cast<float>(p0.x), static_cast<float>(p0.y)},
                                                       euler::point2<float>{static_cast<float>(p1.x), static_cast<float>(p1.y)});
            while (line != euler::dda::line_iterator<float>::end()) {
                auto pixel = *line;
                euler::dda::pixel<int> int_pixel;
                int_pixel.pos.x = static_cast<int>(pixel.pos.x);
                int_pixel.pos.y = static_cast<int>(pixel.pos.y);
                line_writer.write(int_pixel);
                ++line;
            }
            
            prev_point = curr_point;
        }
        
        return {};
    }

    /**
     * @brief Blend two surfaces together with specified blend mode
     * @param src Source surface to blend
     * @param src_rect Source rectangle (nullopt for entire surface)
     * @param dst_pos Destination position
     * @param mode Blend mode to use
     */
    template<rect_like R = rect<int>>
    expected<void, std::string> blend_surface(
        const surface_renderer& src,
        const std::optional<R>& src_rect,
        const point<int>& dst_pos,
        blend_mode mode = blend_mode::blend) {
        
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        // Get actual source rectangle
        rect<int> src_bounds = src_rect.has_value() ? 
            rect<int>{
                static_cast<int>(get_x(*src_rect)),
                static_cast<int>(get_y(*src_rect)),
                static_cast<int>(get_width(*src_rect)),
                static_cast<int>(get_height(*src_rect))
            } : rect<int>{0, 0, src.surface_->w, src.surface_->h};
        
        // Calculate destination rectangle
        rect<int> dst_bounds{dst_pos.x, dst_pos.y, src_bounds.w, src_bounds.h};
        
        // Clip to destination surface
        if (!clip_rect_to_clip(dst_bounds)) {
            return {}; // Nothing to draw
        }
        
        // Adjust source rectangle based on clipping
        src_bounds.x += (dst_bounds.x - dst_pos.x);
        src_bounds.y += (dst_bounds.y - dst_pos.y);
        src_bounds.w = dst_bounds.w;
        src_bounds.h = dst_bounds.h;
        
        // Lock both surfaces
        surface_lock dst_lock(surface_);
        surface_lock src_lock(const_cast<SDL_Surface*>(src.surface_));
        
        if (!dst_lock.is_locked() || !src_lock.is_locked()) {
            return make_unexpected("Failed to lock surfaces");
        }
        
        // Store current blend mode and restore it later
        auto old_mode = blend_mode_;
        blend_mode_ = mode;
        
        // Perform blending using optimized get_pixel method
        for (int y = 0; y < src_bounds.h; ++y) {
            for (int x = 0; x < src_bounds.w; ++x) {
                int src_x = src_bounds.x + x;
                int src_y = src_bounds.y + y;
                int dst_x = dst_bounds.x + x;
                int dst_y = dst_bounds.y + y;
                
                // Use the optimized get_pixel method from the source surface_renderer
                uint32_t src_pixel = src.get_pixel(src_x, src_y);
                
                // Apply blending based on mode
                apply_blend_mode(dst_x, dst_y, src_pixel);
            }
        }
        
        blend_mode_ = old_mode;
        return {};
    }
    
    /**
     * @brief Fill a rectangle with gradient
     * @param rect Rectangle to fill
     * @param c1 Top-left color
     * @param c2 Top-right color
     * @param c3 Bottom-right color
     * @param c4 Bottom-left color
     */
    template<rect_like R>
    expected<void, std::string> fill_rect_gradient(
        const R& rect,
        const color& c1, const color& c2,
        const color& c3, const color& c4) {
        
        if (!surface_) {
            return make_unexpected("Invalid surface");
        }
        
        sdlpp::rect<int> r{
            static_cast<int>(get_x(rect)),
            static_cast<int>(get_y(rect)),
            static_cast<int>(get_width(rect)),
            static_cast<int>(get_height(rect))
        };
        
        if (!clip_rect_to_clip(r)) {
            return {};
        }
        
        surface_lock lock(surface_);
        if (!lock.is_locked()) {
            return make_unexpected("Failed to lock surface");
        }
        
        // Bilinear interpolation for gradient
        for (int y = r.y; y < r.y + r.h; ++y) {
            float ty = static_cast<float>(y - r.y) / static_cast<float>(r.h - 1);
            
            for (int x = r.x; x < r.x + r.w; ++x) {
                float tx = static_cast<float>(x - r.x) / static_cast<float>(r.w - 1);
                
                // Interpolate colors
                uint8_t r_top = static_cast<uint8_t>(c1.r * (1.0f - tx) + c2.r * tx);
                uint8_t g_top = static_cast<uint8_t>(c1.g * (1.0f - tx) + c2.g * tx);
                uint8_t b_top = static_cast<uint8_t>(c1.b * (1.0f - tx) + c2.b * tx);
                uint8_t a_top = static_cast<uint8_t>(c1.a * (1.0f - tx) + c2.a * tx);
                
                uint8_t r_bot = static_cast<uint8_t>(c4.r * (1.0f - tx) + c3.r * tx);
                uint8_t g_bot = static_cast<uint8_t>(c4.g * (1.0f - tx) + c3.g * tx);
                uint8_t b_bot = static_cast<uint8_t>(c4.b * (1.0f - tx) + c3.b * tx);
                uint8_t a_bot = static_cast<uint8_t>(c4.a * (1.0f - tx) + c3.a * tx);
                
                uint8_t r_final = static_cast<uint8_t>(r_top * (1.0f - ty) + r_bot * ty);
                uint8_t g_final = static_cast<uint8_t>(g_top * (1.0f - ty) + g_bot * ty);
                uint8_t b_final = static_cast<uint8_t>(b_top * (1.0f - ty) + b_bot * ty);
                uint8_t a_final = static_cast<uint8_t>(a_top * (1.0f - ty) + a_bot * ty);
                
                auto details = SDL_GetPixelFormatDetails(surface_->format);
                uint32_t pixel = SDL_MapRGBA(details, nullptr, r_final, g_final, b_final, a_final);
                
                if (blend_mode_ == blend_mode::none || a_final == 255) {
                    put_pixel(x, y, pixel);
                } else {
                    blend_pixel(x, y, pixel, a_final / 255.0f);
                }
            }
        }
        
        return {};
    }
    
    // Surface lock RAII helper
    class SDLPP_EXPORT surface_lock {
        SDL_Surface* surface_;
        bool locked_;
    public:
        explicit surface_lock(SDL_Surface* s);
        ~surface_lock();
        
        surface_lock(const surface_lock&) = delete;
        surface_lock& operator=(const surface_lock&) = delete;
        surface_lock(surface_lock&& other) noexcept;
        surface_lock& operator=(surface_lock&& other) noexcept;
        
        void* pixels() const { return surface_->pixels; }
        int pitch() const { return surface_->pitch; }
        bool is_locked() const { return locked_; }
    };

private:
    // Surface management
    SDL_Surface* surface_;
    bool owns_surface_;
    
    // Fast pixel access function pointers
    void (*put_pixel_fast_)(void* pixels, int pitch, int x, int y, uint32_t pixel) = nullptr;
    uint32_t (*get_pixel_fast_)(const void* pixels, int pitch, int x, int y) = nullptr;
    
    // Current drawing state
    color draw_color_;
    blend_mode blend_mode_;
    std::optional<rect<int>> clip_rect_;
    
    // Mapped color for current format
    uint32_t mapped_color_;
    
    // Helper to update mapped color
    void update_mapped_color();
    
    // Direct pixel setting (assumes surface is locked)
    // These are internal implementation details that use the surface's fast pixel functions
    SDLPP_EXPORT void put_pixel(int x, int y, uint32_t pixel);
    SDLPP_EXPORT uint32_t get_pixel(int x, int y) const;
    
    // Blend pixel with alpha (for antialiasing)
    SDLPP_EXPORT void blend_pixel(int x, int y, uint32_t pixel, float alpha);
    
    // Clipping helpers
    inline bool clip_point(int x, int y) const {
        if (!clip_rect_) return true;
        return x >= clip_rect_->x && x < clip_rect_->x + clip_rect_->w &&
               y >= clip_rect_->y && y < clip_rect_->y + clip_rect_->h;
    }
    
    bool clip_line(float& x0, float& y0, float& x1, float& y1) const;
    SDLPP_EXPORT bool clip_rect_to_clip(rect<int>& r) const;
    
    // Blending helper - applies current blend mode
    SDLPP_EXPORT void apply_blend_mode(int x, int y, uint32_t src_pixel);
    
    // Batch processing helper
    template<typename PixelType>
    void process_pixel_batch(const euler::dda::pixel_batch<PixelType>& batch) {
        for (size_t i = 0; i < batch.count; ++i) {
            int x = static_cast<int>(batch.pixels[i].pos.x);
            int y = static_cast<int>(batch.pixels[i].pos.y);
            
            if (clip_point(x, y)) {
                put_pixel(x, y, mapped_color_);
            }
        }
    }
};

} // namespace sdlpp