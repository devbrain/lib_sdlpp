#pragma once

#include <memory>

#include <simplex/detail/export.hh>
#include <simplex/dp.hh>
#include <simplex/geometry.hh>
#include <sdlpp/app/game_application.hh>
#include <sdlpp/video/line_style.hh>

#include <failsafe/logger.hh>


namespace simplex {
    class sprite_atlas;
    class animated_sprite;

    class SIMPLEX_EXPORT application : public sdlpp::game_application {
        public:
            application();

            application(const application&) = delete;
            application& operator = (const application&) = delete;

            ~application();
        protected:
            sdlpp::window_config get_window_config() override;

            void on_ready() override;

            void on_update(float dt) override;

            void on_render(sdlpp::renderer& r) override;

            void handle_event(const sdlpp::event& e) override;

            void on_quit() noexcept override;
        protected:
            sdlpp::expected <void, std::string> set_draw_color(const sdlpp::color& c);
            [[nodiscard]] sdlpp::expected <sdlpp::color, std::string> get_draw_color() const;

            sdlpp::expected <void, std::string> set_draw_blend_mode(sdlpp::blend_mode mode = sdlpp::blend_mode::none);
            [[nodiscard]] sdlpp::expected <sdlpp::blend_mode, std::string> get_draw_blend_mode() const;

            sdlpp::expected <void, std::string> set_line_style(const sdlpp::line_style& style);
            [[nodiscard]] sdlpp::expected <sdlpp::line_style, std::string> get_line_style() const;

            sdlpp::expected <void, std::string> draw_point(const dp& x, const dp& y);
            sdlpp::expected <void, std::string> draw_point(const dp& x, const dp& y, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_point(const point& p);
            sdlpp::expected <void, std::string> draw_point(const point& p, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line(const dp& x1, const dp& y1, const dp& x2, const dp& y2);
            sdlpp::expected <void, std::string> draw_line(const dp& x1, const dp& y1, const dp& x2, const dp& y2, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line(const point& p1, const point& p2);
            sdlpp::expected <void, std::string> draw_line(const point& p1, const point& p2, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line(const line& l);
            sdlpp::expected <void, std::string> draw_line(const line& l, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_rect(const dp& x1, const dp& y1, const dp& x2, const dp& y2);
            sdlpp::expected <void, std::string> draw_rect(const dp& x1, const dp& y1, const dp& x2, const dp& y2, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_rect(const rect& r);
            sdlpp::expected <void, std::string> draw_rect(const rect& r, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_rect_fill(const dp& x1, const dp& y1, const dp& x2, const dp& y2);
            sdlpp::expected <void, std::string> draw_rect_fill(const dp& x1, const dp& y1, const dp& x2, const dp& y2, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_rect_fill(const rect& r);
            sdlpp::expected <void, std::string> draw_rect_fill(const rect& r, const sdlpp::color& c);


            sdlpp::expected <void, std::string> draw_line_aa(const dp& x1, const dp& y1, const dp& x2, const dp& y2);
            sdlpp::expected <void, std::string> draw_line_aa(const dp& x1, const dp& y1, const dp& x2, const dp& y2, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line_aa(const point& p1, const point& p2);
            sdlpp::expected <void, std::string> draw_line_aa(const point& p1, const point& p2, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line_aa(const line& l);
            sdlpp::expected <void, std::string> draw_line_aa(const line& l, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line_thick(const dp& x1, const dp& y1, const dp& x2, const dp& y2, float w);
            sdlpp::expected <void, std::string> draw_line_thick(const dp& x1, const dp& y1, const dp& x2, const dp& y2, float w, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line_thick(const point& p1, const point& p2, float w);
            sdlpp::expected <void, std::string> draw_line_thick(const point& p1, const point& p2, float w, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line_thick(const line& l, float w);
            sdlpp::expected <void, std::string> draw_line_thick(const line& l, float w, const sdlpp::color& c);

            // Dashed line: solid runs of `dash` design pixels separated by `gap`
            // design pixels. dash/gap are dp lengths, scaled like the endpoints
            // so the pattern keeps its proportions on HiDPI displays. A
            // non-positive period (dash + gap) falls back to a solid line.
            sdlpp::expected <void, std::string> draw_line_dashed(const point& p1, const point& p2, dp dash, dp gap);
            sdlpp::expected <void, std::string> draw_line_dashed(const point& p1, const point& p2, dp dash, dp gap, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line_dashed(const line& l, dp dash, dp gap);
            sdlpp::expected <void, std::string> draw_line_dashed(const line& l, dp dash, dp gap, const sdlpp::color& c);

            // Dotted line: single points spaced `spacing` design pixels apart
            // (inclusive of both endpoints). A non-positive spacing falls back
            // to a single point at the start.
            sdlpp::expected <void, std::string> draw_line_dotted(const point& p1, const point& p2, dp spacing);
            sdlpp::expected <void, std::string> draw_line_dotted(const point& p1, const point& p2, dp spacing, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_line_dotted(const line& l, dp spacing);
            sdlpp::expected <void, std::string> draw_line_dotted(const line& l, dp spacing, const sdlpp::color& c);

            sdlpp::expected<void, std::string>  draw_circle(const dp& x1, const dp& y1, const dp& radius);
            sdlpp::expected<void, std::string>  draw_circle(const dp& x1, const dp& y1, const dp& radius, const sdlpp::color& c);
            sdlpp::expected<void, std::string>  draw_circle(const circle& c);
            sdlpp::expected<void, std::string>  draw_circle(const circle& shape, const sdlpp::color& c);
            sdlpp::expected<void, std::string>  draw_circle_fill(const dp& x1, const dp& y1, const dp& radius);
            sdlpp::expected<void, std::string>  draw_circle_fill(const dp& x1, const dp& y1, const dp& radius, const sdlpp::color& c);
            sdlpp::expected<void, std::string>  draw_circle_fill(const circle& c);
            sdlpp::expected<void, std::string>  draw_circle_fill(const circle& shape, const sdlpp::color& c);

            sdlpp::expected <void, std::string> draw_arrow(const point& from, const point& to, dp head_size = 8_dp, float head_angle = 30.0f, float thickness = 1.0f);
            sdlpp::expected <void, std::string> draw_arrow(const point& from, const point& to, dp head_size, float head_angle, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_arrow(const point& from, const point& to, dp head_size, float head_angle, float thickness, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_arrow(const line& l, dp head_size = 8_dp, float head_angle = 30.0f, float thickness = 1.0f);
            sdlpp::expected <void, std::string> draw_arrow(const line& l, dp head_size, float head_angle, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_arrow(const line& l, dp head_size, float head_angle, float thickness, const sdlpp::color& c);

            sdlpp::expected <void, std::string> draw_cross(const point& center, dp size = 5_dp, float thickness = 1.0f);
            sdlpp::expected <void, std::string> draw_cross(const point& center, dp size, const sdlpp::color& c);
            sdlpp::expected <void, std::string> draw_cross(const point& center, dp size, float thickness, const sdlpp::color& c);

            sdlpp::expected<void, std::string> draw_sprite(const sprite_atlas& atlas, const animated_sprite& sprite);
            sdlpp::expected<void, std::string> draw_sprite(
                const sprite_atlas& atlas,
                std::size_t frame_index,
                const point& p,
                bool flip_h = false,
                bool flip_v = false,
                bool flip_d = false
            );

            sdlpp::expected<void, std::string> clear(const sdlpp::color& c = sdlpp::colors::black);

            [[nodiscard]] dp get_width() const;
            [[nodiscard]] dp get_height() const;

            [[nodiscard]] dp get_mouse_x() const noexcept;
            [[nodiscard]] dp get_mouse_y() const noexcept;
            [[nodiscard]] point get_mouse_pos() const noexcept;
            [[nodiscard]] point get_window_mouse() const noexcept;



        private:
            void on_window_display_scale_changed(float scale) override;
        private:
            struct impl;
            std::unique_ptr<impl> m_pimpl;
    };

    /**
     * @brief Centralized free function for drawing a sprite using a renderer and scale factor.
     */
    SIMPLEX_EXPORT sdlpp::expected<void, std::string> draw_sprite(
        sdlpp::renderer& r,
        const sprite_atlas& atlas,
        const animated_sprite& sprite,
        float scale = 1.0f
    );

    /**
     * @brief Centralized free function for drawing an atlas frame at a position with optional flips.
     */
    SIMPLEX_EXPORT sdlpp::expected<void, std::string> draw_sprite(
        sdlpp::renderer& r,
        const sprite_atlas& atlas,
        std::size_t frame_index,
        const point& p,
        float scale = 1.0f,
        bool flip_h = false,
        bool flip_v = false,
        bool flip_d = false
    );
} // namespace simplex
