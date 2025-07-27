/**
 * @file renderer_dda.inl
 * @brief Template implementations for DDA-based renderer methods
 */

#pragma once

namespace sdlpp {

template<typename Container>
requires requires(Container c) {
    typename Container::value_type;
    requires point_like<typename Container::value_type>;
    { std::begin(c) };
    { std::end(c) };
}
expected<void, std::string> renderer::draw_bspline(const Container& control_points, int degree) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    // Convert container to vector of euler points
    std::vector<euler::point2<float>> euler_points;
    euler_points.reserve(static_cast<size_t>(std::distance(std::begin(control_points), std::end(control_points))));
    
    for (const auto& p : control_points) {
        euler_points.push_back({static_cast<float>(get_x(p)), static_cast<float>(get_y(p))});
    }
    
    if (euler_points.size() < static_cast<size_t>(degree + 1)) {
        return make_unexpected("Not enough control points for specified degree");
    }
    
    // Use euler's B-spline iterator with manual batching
    using namespace euler::dda;
    
    // Use batch writer for efficient rendering
    batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
        // Direct SDL rendering since we're in header
        std::vector<SDL_FPoint> render_points;
        render_points.reserve(batch.count);
        
        for (size_t i = 0; i < batch.count; ++i) {
            render_points.push_back({
                static_cast<float>(batch.pixels[i].pos.x),
                static_cast<float>(batch.pixels[i].pos.y)
            });
        }
        
        SDL_RenderPoints(ptr.get(), render_points.data(), static_cast<int>(render_points.size()));
    });
    
    // Create B-spline iterator
    auto spline = make_bspline(euler_points, degree);
    
    // Process all pixels through batch writer
    for (; spline != decltype(spline)::end(); ++spline) {
        writer.write(*spline);
    }
    
    return {};
}

template<typename Container>
requires requires(Container c) {
    typename Container::value_type;
    requires point_like<typename Container::value_type>;
    { std::begin(c) };
    { std::end(c) };
}
expected<void, std::string> renderer::draw_catmull_rom(const Container& points, float tension) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    // Convert container to vector of euler points
    std::vector<euler::point2<float>> euler_points;
    euler_points.reserve(static_cast<size_t>(std::distance(std::begin(points), std::end(points))));
    
    for (const auto& p : points) {
        euler_points.push_back({static_cast<float>(get_x(p)), static_cast<float>(get_y(p))});
    }
    
    if (euler_points.size() < 2) {
        return make_unexpected("Need at least 2 points for Catmull-Rom spline");
    }
    
    // Use euler's Catmull-Rom iterator with manual batching
    using namespace euler::dda;
    
    // Use batch writer for efficient rendering
    batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
        // Direct SDL rendering since we're in header
        std::vector<SDL_FPoint> render_points;
        render_points.reserve(batch.count);
        
        for (size_t i = 0; i < batch.count; ++i) {
            render_points.push_back({
                static_cast<float>(batch.pixels[i].pos.x),
                static_cast<float>(batch.pixels[i].pos.y)
            });
        }
        
        SDL_RenderPoints(ptr.get(), render_points.data(), static_cast<int>(render_points.size()));
    });
    
    // Create Catmull-Rom spline iterator
    auto spline = make_catmull_rom(euler_points, tension);
    
    // Process all pixels through batch writer
    for (; spline != decltype(spline)::end(); ++spline) {
        writer.write(*spline);
    }
    
    return {};
}

template<typename CurveFunc>
requires requires(CurveFunc f, float t) {
    { f(t) } -> point_like;
}
expected<void, std::string> renderer::draw_curve(CurveFunc&& curve, 
                                               float t_start, 
                                               float t_end, 
                                               int steps) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    if (steps <= 0) {
        return make_unexpected("Steps must be positive");
    }
    
    if (t_start >= t_end) {
        return make_unexpected("t_start must be less than t_end");
    }
    
    // Use manual batching for parametric curves
    using namespace euler::dda;
    
    // Use batch writer for efficient rendering
    batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
        // Direct SDL rendering since we're in header
        std::vector<SDL_FPoint> render_points;
        render_points.reserve(batch.count);
        
        for (size_t i = 0; i < batch.count; ++i) {
            render_points.push_back({
                static_cast<float>(batch.pixels[i].pos.x),
                static_cast<float>(batch.pixels[i].pos.y)
            });
        }
        
        SDL_RenderPoints(ptr.get(), render_points.data(), static_cast<int>(render_points.size()));
    });
    
    // Evaluate curve at discrete points
    float dt = (t_end - t_start) / static_cast<float>(steps);
    auto last_point = curve(t_start);
    euler::point2<int> last_pixel{
        static_cast<int>(std::round(get_x(last_point))),
        static_cast<int>(std::round(get_y(last_point)))
    };
    
    writer.write({last_pixel});
    
    for (int i = 1; i <= steps; ++i) {
        float t = t_start + static_cast<float>(i) * dt;
        auto point = curve(t);
        euler::point2<int> pixel{
            static_cast<int>(std::round(get_x(point))),
            static_cast<int>(std::round(get_y(point)))
        };
        
        // Fill gaps with line if necessary
        if (euler::distance_squared(pixel, last_pixel) > 1) {
            // Use line iterator to fill gaps
            auto line = make_line_iterator(last_pixel, pixel);
            for (; line != decltype(line)::end(); ++line) {
                if ((*line).pos != last_pixel) { // Skip duplicate
                    writer.write(*line);
                }
            }
        } else if (pixel != last_pixel) {
            writer.write({pixel});
        }
        
        last_pixel = pixel;
    }
    
    return {};
}

} // namespace sdlpp