#include <failsafe/logger/backend/sdl_backend.hh>

#include <algorithm>
#include <cmath>

#include <simplex/simplex.hh>
#include <simplex/dp.hh>

namespace simplex {
    namespace {
        template<typename Draw>
        sdlpp::expected<void, std::string> with_draw_color(sdlpp::renderer& renderer, const sdlpp::color& c, Draw&& draw) {
            auto previous = renderer.get_draw_color();
            if (!previous) {
                return sdlpp::make_unexpected(previous.error());
            }

            auto set_result = renderer.set_draw_color(c);
            if (!set_result) {
                return set_result;
            }

            auto draw_result = draw();
            auto restore_result = renderer.set_draw_color(*previous);
            if (!draw_result) {
                return draw_result;
            }
            return restore_result;
        }
    }

    struct application::impl {

    };

    application::application()
        : m_pimpl(std::make_unique<impl>()){
    }

    application::~application() = default;

    sdlpp::window_config application::get_window_config() {
        return {
            "Bane",
            800,
            600,
            sdlpp::window_flags::resizable | sdlpp::window_flags::high_pixel_density,
            60
        };
    }

    void application::on_ready() {
        failsafe::logger::set_backend(failsafe::logger::backends::make_sdl_backend());

        set_scale(get_window().display_scale());
    }

    void application::on_update([[maybe_unused]] float dt) {
        game_application::on_update(dt);
    }

    void application::on_render(sdlpp::renderer& r) {
        r.present();
    }

    void application::handle_event([[maybe_unused]] const sdlpp::event& e) {
    }

    void application::on_quit() noexcept {
        LOG_INFO("Shutting down, average FPS: ", fps());
    }

    sdlpp::expected<void, std::string> application::set_draw_color(const sdlpp::color& c) {
        return get_renderer().set_draw_color(c);
    }

    sdlpp::expected<sdlpp::color, std::string> application::get_draw_color() const {
        return get_renderer().get_draw_color();
    }

    sdlpp::expected<void, std::string> application::set_draw_blend_mode(sdlpp::blend_mode mode) {
        return get_renderer().set_draw_blend_mode(mode);
    }

    sdlpp::expected<sdlpp::blend_mode, std::string> application::get_draw_blend_mode() const {
        return get_renderer().get_draw_blend_mode();
    }

    sdlpp::expected<void, std::string> application::draw_point(const dp& x, const dp& y) {
        return get_renderer().draw_point(x.px(), y.px());
    }

    sdlpp::expected<void, std::string> application::draw_point(const dp& x, const dp& y, const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_point(x, y);
        });
    }

    sdlpp::expected<void, std::string> application::draw_point(const point& p) {
        return draw_point(p.x, p.y);
    }

    sdlpp::expected<void, std::string> application::draw_point(const point& p, const sdlpp::color& c) {
        return draw_point(p.x, p.y, c);
    }

    sdlpp::expected<void, std::string> application::draw_line(const dp& x1, const dp& y1, const dp& x2, const dp& y2) {
        return get_renderer().draw_line(x1.px(), y1.px(), x2.px(), y2.px());
    }

    sdlpp::expected<void, std::string> application::draw_line(const dp& x1, const dp& y1, const dp& x2, const dp& y2,
        const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_line(x1, y1, x2, y2);
        });
    }

    sdlpp::expected<void, std::string> application::draw_line(const point& p1, const point& p2) {
        return draw_line(p1.x, p1.y, p2.x, p2.y);
    }

    sdlpp::expected<void, std::string> application::draw_line(const point& p1, const point& p2,
        const sdlpp::color& c) {
        return draw_line(p1.x, p1.y, p2.x, p2.y, c);
    }

    sdlpp::expected<void, std::string> application::draw_line(const line& l) {
        return draw_line(l.x1, l.y1, l.x2, l.y2);
    }

    sdlpp::expected<void, std::string> application::draw_line(const line& l, const sdlpp::color& c) {
        return draw_line(l.x1, l.y1, l.x2, l.y2, c);
    }

    sdlpp::expected<void, std::string> application::draw_rect(const dp& x1, const dp& y1, const dp& x2, const dp& y2) {
        const SDL_FRect fr {
            x1.px(), y1.px(), x2.px() - x1.px(), y2.px() - y1.px()
        };
        return get_renderer().draw_rect(fr);
    }

    sdlpp::expected<void, std::string> application::draw_rect(const dp& x1, const dp& y1, const dp& x2, const dp& y2,
        const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_rect(x1, y1, x2, y2);
        });
    }

    sdlpp::expected<void, std::string> application::draw_rect(const rect& r) {
        return draw_rect(r.x, r.y, r.right(), r.bottom());
    }

    sdlpp::expected<void, std::string> application::draw_rect(const rect& r, const sdlpp::color& c) {
        return draw_rect(r.x, r.y, r.right(), r.bottom(), c);
    }

    sdlpp::expected<void, std::string> application::draw_rect_fill(const dp& x1, const dp& y1, const dp& x2,
        const dp& y2) {
        const SDL_FRect fr {
            x1.px(), y1.px(), x2.px() - x1.px(), y2.px() - y1.px()
        };
        return get_renderer().fill_rect(fr);
    }

    sdlpp::expected<void, std::string> application::draw_rect_fill(const dp& x1, const dp& y1, const dp& x2,
        const dp& y2, const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_rect_fill(x1, y1, x2, y2);
        });
    }

    sdlpp::expected<void, std::string> application::draw_rect_fill(const rect& r) {
        return draw_rect_fill(r.x, r.y, r.right(), r.bottom());
    }

    sdlpp::expected<void, std::string> application::draw_rect_fill(const rect& r, const sdlpp::color& c) {
        return draw_rect_fill(r.x, r.y, r.right(), r.bottom(), c);
    }

    sdlpp::expected<void, std::string> application::
    draw_line_aa(const dp& x1, const dp& y1, const dp& x2, const dp& y2) {
         return get_renderer().draw_line_aa(x1.px(), y1.px(), x2.px(), y2.px());
    }

    sdlpp::expected<void, std::string> application::draw_line_aa(const dp& x1, const dp& y1, const dp& x2,
        const dp& y2, const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_line_aa(x1, y1, x2, y2);
        });
    }

    sdlpp::expected<void, std::string> application::draw_line_aa(const point& p1, const point& p2) {
        return draw_line_aa(p1.x, p1.y, p2.x, p2.y);
    }

    sdlpp::expected<void, std::string> application::draw_line_aa(const point& p1, const point& p2,
        const sdlpp::color& c) {
        return draw_line_aa(p1.x, p1.y, p2.x, p2.y, c);
    }

    sdlpp::expected<void, std::string> application::draw_line_aa(const line& l) {
        return draw_line_aa(l.x1, l.y1, l.x2, l.y2);
    }

    sdlpp::expected<void, std::string> application::draw_line_aa(const line& l, const sdlpp::color& c) {
        return draw_line_aa(l.x1, l.y1, l.x2, l.y2, c);
    }

    sdlpp::expected<void, std::string> application::draw_line_thick(const dp& x1, const dp& y1, const dp& x2,
        const dp& y2, float w) {
        // w is a design-pixel thickness; scale it the same way the endpoints
        // are scaled so the stroke keeps its weight on HiDPI displays.
        return get_renderer().draw_line_thick(x1.px(), y1.px(), x2.px(), y2.px(), w * scale());
    }

    sdlpp::expected<void, std::string> application::draw_line_thick(const dp& x1, const dp& y1, const dp& x2,
        const dp& y2, float w, const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_line_thick(x1, y1, x2, y2, w);
        });
    }

    sdlpp::expected<void, std::string> application::draw_line_thick(const point& p1, const point& p2, float w) {
        return draw_line_thick(p1.x, p1.y, p2.x, p2.y, w);
    }

    sdlpp::expected<void, std::string> application::draw_line_thick(const point& p1, const point& p2, float w,
        const sdlpp::color& c) {
        return draw_line_thick(p1.x, p1.y, p2.x, p2.y, w, c);
    }

    sdlpp::expected<void, std::string> application::draw_line_thick(const line& l, float w) {
        return draw_line_thick(l.x1, l.y1, l.x2, l.y2, w);
    }

    sdlpp::expected<void, std::string> application::draw_line_thick(const line& l, float w, const sdlpp::color& c) {
        return draw_line_thick(l.x1, l.y1, l.x2, l.y2, w, c);
    }

    sdlpp::expected<void, std::string> application::draw_line_dashed(const point& p1, const point& p2, dp dash,
        dp gap) {
        // Walk the segment in design space and emit a solid draw_line for each
        // `dash` run; draw_line handles the dp -> physical-pixel scaling.
        const float x1 = p1.x.design();
        const float y1 = p1.y.design();
        const float dx = p2.x.design() - x1;
        const float dy = p2.y.design() - y1;
        const float length = std::sqrt(dx * dx + dy * dy);
        const float period = dash.design() + gap.design();

        if (length <= 0.0f || period <= 0.0f) {
            return draw_line(p1, p2);
        }

        const float ux = dx / length;
        const float uy = dy / length;
        const float run = dash.design();

        sdlpp::expected<void, std::string> result{};
        for (float s = 0.0f; s < length; s += period) {
            const float e = std::min(s + run, length);
            result = draw_line(point{dp{x1 + ux * s}, dp{y1 + uy * s}},
                               point{dp{x1 + ux * e}, dp{y1 + uy * e}});
            if (!result) {
                return result;
            }
        }
        return result;
    }

    sdlpp::expected<void, std::string> application::draw_line_dashed(const point& p1, const point& p2, dp dash,
        dp gap, const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_line_dashed(p1, p2, dash, gap);
        });
    }

    sdlpp::expected<void, std::string> application::draw_line_dashed(const line& l, dp dash, dp gap) {
        return draw_line_dashed(l.start(), l.end(), dash, gap);
    }

    sdlpp::expected<void, std::string> application::draw_line_dashed(const line& l, dp dash, dp gap,
        const sdlpp::color& c) {
        return draw_line_dashed(l.start(), l.end(), dash, gap, c);
    }

    sdlpp::expected<void, std::string> application::draw_line_dotted(const point& p1, const point& p2, dp spacing) {
        const float x1 = p1.x.design();
        const float y1 = p1.y.design();
        const float dx = p2.x.design() - x1;
        const float dy = p2.y.design() - y1;
        const float length = std::sqrt(dx * dx + dy * dy);
        const float step = spacing.design();

        if (length <= 0.0f || step <= 0.0f) {
            return draw_point(p1);
        }

        const float ux = dx / length;
        const float uy = dy / length;

        sdlpp::expected<void, std::string> result{};
        for (float s = 0.0f; s <= length; s += step) {
            result = draw_point(point{dp{x1 + ux * s}, dp{y1 + uy * s}});
            if (!result) {
                return result;
            }
        }
        return result;
    }

    sdlpp::expected<void, std::string> application::draw_line_dotted(const point& p1, const point& p2, dp spacing,
        const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_line_dotted(p1, p2, spacing);
        });
    }

    sdlpp::expected<void, std::string> application::draw_line_dotted(const line& l, dp spacing) {
        return draw_line_dotted(l.start(), l.end(), spacing);
    }

    sdlpp::expected<void, std::string> application::draw_line_dotted(const line& l, dp spacing,
        const sdlpp::color& c) {
        return draw_line_dotted(l.start(), l.end(), spacing, c);
    }

    sdlpp::expected<void, std::string> application::draw_circle(const dp& x1, const dp& y1, const dp& radius) {
        return get_renderer().draw_circle(x1.pxi(), y1.pxi(), radius.pxi());
    }

    sdlpp::expected<void, std::string> application::draw_circle(const dp& x1, const dp& y1, const dp& radius,
        const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_circle(x1, y1, radius);
        });
    }

    sdlpp::expected<void, std::string> application::draw_circle(const circle& c) {
        return draw_circle(c.x, c.y, c.radius);
    }

    sdlpp::expected<void, std::string> application::draw_circle(const circle& shape, const sdlpp::color& c) {
        return draw_circle(shape.x, shape.y, shape.radius, c);
    }

    sdlpp::expected<void, std::string> application::draw_circle_fill(const dp& x1, const dp& y1, const dp& radius) {
        return get_renderer().fill_circle(x1.pxi(), y1.pxi(), radius.pxi());
    }

    sdlpp::expected<void, std::string> application::draw_circle_fill(const dp& x1, const dp& y1, const dp& radius,
        const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_circle_fill(x1, y1, radius);
        });
    }

    sdlpp::expected<void, std::string> application::draw_circle_fill(const circle& c) {
        return draw_circle_fill(c.x, c.y, c.radius);
    }

    sdlpp::expected<void, std::string> application::draw_circle_fill(const circle& shape, const sdlpp::color& c) {
        return draw_circle_fill(shape.x, shape.y, shape.radius, c);
    }

    sdlpp::expected<void, std::string> application::clear(const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return get_renderer().clear();
        });
    }

    sdlpp::expected<void, std::string> application::draw_arrow(const point& from, const point& to, dp head_size, float head_angle, float thickness) {
        return get_renderer().draw_arrow(from.x.px(), from.y.px(), to.x.px(), to.y.px(), head_size.px(), head_angle, thickness * scale());
    }

    sdlpp::expected<void, std::string> application::draw_arrow(const point& from, const point& to, dp head_size, float head_angle, const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_arrow(from, to, head_size, head_angle);
        });
    }

    sdlpp::expected<void, std::string> application::draw_arrow(const point& from, const point& to, dp head_size, float head_angle, float thickness, const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_arrow(from, to, head_size, head_angle, thickness);
        });
    }

    sdlpp::expected<void, std::string> application::draw_arrow(const line& l, dp head_size, float head_angle, float thickness) {
        return draw_arrow(l.start(), l.end(), head_size, head_angle, thickness);
    }

    sdlpp::expected<void, std::string> application::draw_arrow(const line& l, dp head_size, float head_angle, const sdlpp::color& c) {
        return draw_arrow(l.start(), l.end(), head_size, head_angle, c);
    }

    sdlpp::expected<void, std::string> application::draw_arrow(const line& l, dp head_size, float head_angle, float thickness, const sdlpp::color& c) {
        return draw_arrow(l.start(), l.end(), head_size, head_angle, thickness, c);
    }

    sdlpp::expected<void, std::string> application::draw_cross(const point& center, dp size, float thickness) {
        return get_renderer().draw_cross(center.x.px(), center.y.px(), size.px(), thickness * scale());
    }

    sdlpp::expected<void, std::string> application::draw_cross(const point& center, dp size, const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_cross(center, size);
        });
    }

    sdlpp::expected<void, std::string> application::draw_cross(const point& center, dp size, float thickness, const sdlpp::color& c) {
        return with_draw_color(get_renderer(), c, [&]() {
            return draw_cross(center, size, thickness);
        });
    }

    dp application::get_width() const {
        const auto w = get_window().get_dimensions()->width;
        return dp{static_cast <float>(w.value())};
    }

    dp application::get_height() const {
        const auto h = get_window().get_dimensions()->height;
        return dp{static_cast <float>(h.value())};
    }

    dp application::get_mouse_x() const noexcept {
        return dp{static_cast<float>(game_application::get_mouse_x())};
    }

    dp application::get_mouse_y() const noexcept {
        return dp{static_cast<float>(game_application::get_mouse_y())};
    }

    point application::get_mouse_pos() const noexcept {
        return {get_mouse_x(), get_mouse_y()};
    }

    point application::get_window_mouse() const noexcept {
        return get_mouse_pos();
    }

    void application::on_window_display_scale_changed(float scale) {
        LOG_INFO("Setting scale to ", scale);
        set_scale(scale);
    }
} // namespace simplex
