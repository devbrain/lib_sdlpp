//
// Created by Igor on 7/26/25.
//

#include <sdlpp/video/renderer.hh>
#include <algorithm>
#include <cmath>

namespace sdlpp {

// Helper function to set pixel with alpha blending
static void set_pixel_alpha(SDL_Renderer* renderer, int x, int y, const color& base_color, float alpha) {
    // Save current blend mode and color
    SDL_BlendMode old_mode;
    SDL_GetRenderDrawBlendMode(renderer, &old_mode);
    
    Uint8 r, g, b, a;
    SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a);
    
    // Set alpha blending mode
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    
    // Set color with modulated alpha
    color pixel_color = base_color;
    pixel_color.a = static_cast<uint8_t>(base_color.a * alpha);
    SDL_SetRenderDrawColor(renderer, pixel_color.r, pixel_color.g, pixel_color.b, pixel_color.a);
    
    // Draw pixel
    SDL_RenderPoint(renderer, static_cast<float>(x), static_cast<float>(y));
    
    // Restore previous state
    SDL_SetRenderDrawBlendMode(renderer, old_mode);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
}

// Helper to process a batch of pixels efficiently
template<typename PixelType>
void process_pixel_batch(SDL_Renderer* renderer, const euler::dda::pixel_batch<PixelType>& batch) {
    // Allocate temporary buffer for SDL points
    std::vector<SDL_FPoint> points;
    points.reserve(batch.count);
    
    for (size_t i = 0; i < batch.count; ++i) {
        points.push_back({
            static_cast<float>(batch.pixels[i].pos.x),
            static_cast<float>(batch.pixels[i].pos.y)
        });
    }
    
    // Draw all points in batch at once
    SDL_RenderPoints(renderer, points.data(), static_cast<int>(points.size()));
}

// Helper to process antialiased pixel batch
static void process_aa_pixel_batch(SDL_Renderer* renderer, 
                                  const euler::dda::pixel_batch<euler::dda::aa_pixel<float>>& batch,
                                  const color& base_color) {
    // For antialiased pixels, we need to draw each with its coverage value
    for (size_t i = 0; i < batch.count; ++i) {
        const auto& pixel = batch.pixels[i];
        set_pixel_alpha(renderer, 
                       static_cast<int>(pixel.pos.x), 
                       static_cast<int>(pixel.pos.y), 
                       base_color, 
                       pixel.coverage);
    }
}

expected<void, std::string> renderer::draw_line_aa(float x1, float y1, float x2, float y2) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    // Get current draw color
    color draw_color = get_draw_color().value_or(color{255, 255, 255});
    
    // Use euler's antialiased line iterator with batch writer
    using namespace euler::dda;
    
    batch_writer<aa_pixel<float>> writer([this, draw_color](const pixel_batch<aa_pixel<float>>& batch) {
        process_aa_pixel_batch(ptr.get(), batch, draw_color);
    });
    
    auto line = make_aa_line_iterator(euler::point2<float>{x1, y1}, euler::point2<float>{x2, y2});
    while (line != aa_line_iterator<float>::end()) {
        writer.write(*line);
        ++line;
    }
    
    return {};
}

expected<void, std::string> renderer::draw_line_thick(float x1, float y1, float x2, float y2, float width) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    if (width <= 0) {
        return make_unexpected("Line width must be positive");
    }
    
    // Use euler's thick line iterator with manual batching
    using namespace euler::dda;
    auto line = make_thick_line_iterator(euler::point2<float>{x1, y1}, euler::point2<float>{x2, y2}, width);
    
    // Use batch writer for efficient rendering
    batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
        process_pixel_batch(ptr.get(), batch);
    });
    
    // Process all pixels through batch writer
    for (; line != decltype(line)::end(); ++line) {
        writer.write(*line);
    }
    
    return {};
}

expected<void, std::string> renderer::draw_circle(int x, int y, int radius) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    if (radius < 0) {
        return make_unexpected("Circle radius must be non-negative");
    }
    
    if (radius == 0) {
        // Just draw center point
        return draw_point(x, y);
    }
    
    // Use euler's circle iterator with batch writer for better performance
    using namespace euler::dda;
    
    batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
        process_pixel_batch(ptr.get(), batch);
    });
    
    for (auto pixel : circle_pixels(euler::point2<int>{x, y}, radius)) {
        writer.write(pixel);
    }
    
    return {};
}

expected<void, std::string> renderer::fill_circle(int x, int y, int radius) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    if (radius < 0) {
        return make_unexpected("Circle radius must be non-negative");
    }
    
    if (radius == 0) {
        // Just draw center point
        return draw_point(x, y);
    }
    
    // Use euler's filled circle iterator
    using namespace euler::dda;
    auto filled = make_filled_circle_iterator(euler::point2<int>{x, y}, radius);
    
    // Draw horizontal lines for each span
    for (; filled != decltype(filled)::end(); ++filled) {
        auto span = *filled;
        // Draw horizontal line from span.x_start to span.x_end at y = span.y
        if (!SDL_RenderLine(ptr.get(), 
                           static_cast<float>(span.x_start), 
                           static_cast<float>(span.y), 
                           static_cast<float>(span.x_end), 
                           static_cast<float>(span.y))) {
            return make_unexpected(get_error());
        }
    }
    
    return {};
}

expected<void, std::string> renderer::draw_ellipse(int x, int y, int rx, int ry) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    if (rx < 0 || ry < 0) {
        return make_unexpected("Ellipse radii must be non-negative");
    }
    
    if (rx == 0 && ry == 0) {
        // Just draw center point
        return draw_point(x, y);
    }
    
    // Use euler's ellipse iterator with batch writer for better performance
    using namespace euler::dda;
    
    batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
        process_pixel_batch(ptr.get(), batch);
    });
    
    auto ellipse = make_ellipse_iterator(
        euler::point2<float>{static_cast<float>(x), static_cast<float>(y)}, 
        static_cast<float>(rx), 
        static_cast<float>(ry)
    );
    
    for (; ellipse != decltype(ellipse)::end(); ++ellipse) {
        pixel<int> int_pixel;
        int_pixel.pos.x = static_cast<int>(std::round((*ellipse).pos.x));
        int_pixel.pos.y = static_cast<int>(std::round((*ellipse).pos.y));
        writer.write(int_pixel);
    }
    
    return {};
}

expected<void, std::string> renderer::fill_ellipse(int x, int y, int rx, int ry) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    if (rx < 0 || ry < 0) {
        return make_unexpected("Ellipse radii must be non-negative");
    }
    
    if (rx == 0 && ry == 0) {
        // Just draw center point
        return draw_point(x, y);
    }
    
    // Use euler's filled ellipse iterator
    using namespace euler::dda;
    auto filled = make_filled_ellipse_iterator(
        euler::point2<float>{static_cast<float>(x), static_cast<float>(y)}, 
        static_cast<float>(rx), 
        static_cast<float>(ry)
    );
    
    // Draw horizontal lines for each span
    for (; filled != decltype(filled)::end(); ++filled) {
        auto span = *filled;
        // Convert float coordinates to int and draw horizontal line
        int y_int = static_cast<int>(std::round(span.y));
        int x_start = static_cast<int>(std::round(span.x_start));
        int x_end = static_cast<int>(std::round(span.x_end));
        
        if (!SDL_RenderLine(ptr.get(), 
                           static_cast<float>(x_start), 
                           static_cast<float>(y_int), 
                           static_cast<float>(x_end), 
                           static_cast<float>(y_int))) {
            return make_unexpected(get_error());
        }
    }
    
    return {};
}

expected<void, std::string> renderer::draw_ellipse_arc(int x, int y, int rx, int ry, float start_angle, float end_angle) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    if (rx < 0 || ry < 0) {
        return make_unexpected("Ellipse radii must be non-negative");
    }
    
    if (rx == 0 && ry == 0) {
        // Just draw center point
        return draw_point(x, y);
    }
    
    // Use euler's ellipse arc iterator with batch writer for better performance
    using namespace euler::dda;
    
    batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
        process_pixel_batch(ptr.get(), batch);
    });
    
    // Convert angles to euler radian type
    auto arc = make_ellipse_arc_iterator(
        euler::point2<float>{static_cast<float>(x), static_cast<float>(y)}, 
        static_cast<float>(rx), 
        static_cast<float>(ry), 
        euler::radian<float>(start_angle), 
        euler::radian<float>(end_angle)
    );
    
    for (; arc != decltype(arc)::end(); ++arc) {
        pixel<int> int_pixel;
        int_pixel.pos.x = static_cast<int>(std::round((*arc).pos.x));
        int_pixel.pos.y = static_cast<int>(std::round((*arc).pos.y));
        writer.write(int_pixel);
    }
    
    return {};
}

expected<void, std::string> renderer::draw_bezier_quad(float x0, float y0, float x1, float y1, float x2, float y2) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    // Use euler's quadratic bezier iterator with batch writer
    using namespace euler::dda;
    
    // Use batch writer for efficient rendering
    batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
        process_pixel_batch(ptr.get(), batch);
    });
    
    // Create quadratic bezier iterator
    auto bezier = make_quadratic_bezier(
        euler::point2<float>{x0, y0},
        euler::point2<float>{x1, y1},
        euler::point2<float>{x2, y2}
    );
    
    // Process all pixels through batch writer
    for (; bezier != decltype(bezier)::end(); ++bezier) {
        pixel<int> int_pixel;
        int_pixel.pos.x = static_cast<int>(std::round((*bezier).pos.x));
        int_pixel.pos.y = static_cast<int>(std::round((*bezier).pos.y));
        writer.write(int_pixel);
    }
    
    return {};
}

expected<void, std::string> renderer::draw_bezier_cubic(float x0, float y0, float x1, float y1, 
                                                       float x2, float y2, float x3, float y3) {
    if (!ptr) {
        return make_unexpected("Invalid renderer");
    }
    
    // Use euler's cubic bezier iterator with batch writer
    using namespace euler::dda;
    
    // Use batch writer for efficient rendering
    batch_writer<pixel<int>> writer([this](const pixel_batch<pixel<int>>& batch) {
        process_pixel_batch(ptr.get(), batch);
    });
    
    // Create cubic bezier iterator
    auto bezier = make_cubic_bezier(
        euler::point2<float>{x0, y0},
        euler::point2<float>{x1, y1},
        euler::point2<float>{x2, y2},
        euler::point2<float>{x3, y3}
    );
    
    // Process all pixels through batch writer
    for (; bezier != decltype(bezier)::end(); ++bezier) {
        pixel<int> int_pixel;
        int_pixel.pos.x = static_cast<int>(std::round((*bezier).pos.x));
        int_pixel.pos.y = static_cast<int>(std::round((*bezier).pos.y));
        writer.write(int_pixel);
    }
    
    return {};
}

// Explicit template instantiations for process_pixel_batch
template void process_pixel_batch<euler::dda::pixel<int>>(SDL_Renderer*, const euler::dda::pixel_batch<euler::dda::pixel<int>>&);
template void process_pixel_batch<euler::dda::aa_pixel<float>>(SDL_Renderer*, const euler::dda::pixel_batch<euler::dda::aa_pixel<float>>&);

} // namespace sdlpp