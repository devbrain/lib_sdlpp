#include <simplex/simplex.hh>
#include <sdlpp/app/entry_point.hh>
#include <sdlpp/system/random.hh>
#include <algorithm>
#include <iostream>

using namespace simplex::literals;
using namespace sdlpp;

class example : public simplex::application {
    public:
        example()
            : m_bat_pos(INIT_BAT_POS),
              rng(random::engine::from_time()) {
            const float alpha = 2 * 3.1415926f * rng.uniform_float();
            m_direction = {std::cos(alpha), std::sin(alpha)};
        }

    private:
        static constexpr auto INIT_BAT_POS = 20.0_dp;
        static constexpr auto BAT_WIDTH = 40.0_dp;

        static constexpr auto BAT_SPEED = 300_dp; // dp per second
        static constexpr auto BALL_RADIUS = 5_dp;

        void on_ready() override {
            m_ball.x = get_width() / 2.0;
            m_ball.y = get_height() / 2.0;
            m_ball.radius = BALL_RADIUS;

            application::on_ready();
        }

        void on_update([[maybe_unused]] float delta_time) override {
            const auto step = BAT_SPEED * delta_time;
            if (get_key(scancode::left)) {
                m_bat_pos = std::max(left(), m_bat_pos - step);
            }
            if (get_key(scancode::right)) {
                m_bat_pos = std::min(right() - BAT_WIDTH, m_bat_pos + step);
            }

            ///   simplex::point ball_speed = simplex::dp{delta_time * BAT_SPEED * };
            ///
            m_ball.x += m_direction.x*step;
            m_ball.y += m_direction.y*step;

            if (m_ball.x - BALL_RADIUS <= 0_dp) {
                m_direction.x *= -1.0f ;
            }
            if (m_ball.x + BALL_RADIUS >= right()) {
                m_direction.x *= -1.0f;
            }

            if (m_ball.y - BALL_RADIUS <= 0_dp) {
                m_direction.y *= -1.0f;
            }
            if (m_ball.y + BALL_RADIUS >= bottom()) {
                m_direction.y *= -1.0f;
            }

            if (get_mouse(mouse_button::left)) {
                auto [x, y] = get_mouse_pos();
                m_ball.x = std::max(left(), x - m_ball.radius);
                m_ball.x = std::min(right() - m_ball.radius, m_ball.x + m_ball.radius);

                m_ball.y = std::max(top(), y - m_ball.radius);
                m_ball.y = std::min(bottom() - m_ball.radius, m_ball.y + m_ball.radius);
            }



            clear(colors::dark_slate_blue);
            // Draw Boundary

            draw_border();

            draw_bat();
            draw_ball();
        }

        // ========================================================================================
        [[nodiscard]] static simplex::dp left() {
            return 10_dp;
        }

        [[nodiscard]] simplex::dp right() const {
            return get_width() - 10_dp;
        }

        [[nodiscard]] static simplex::dp top() {
            return 10_dp;
        }

        [[nodiscard]] simplex::dp bottom() const {
            return get_height() - 10_dp;
        }

        void draw_border() {
            const simplex::point left_top{left(), top()};
            const simplex::point right_top{right(), top()};
            const simplex::point left_bottom{left(), bottom()};
            const simplex::point right_bottom{right(), bottom()};
            draw_line(left_top, right_top, colors::yellow);
            draw_line(right_top, right_bottom, colors::yellow);
            draw_line(left_top, left_bottom, colors::yellow);
        }

        void draw_bat() {
            draw_rect_fill(simplex::rect{m_bat_pos, get_height() - 20_dp, BAT_WIDTH, 10_dp}, colors::green);
        }

        void draw_ball() {
            draw_circle(m_ball, colors::cyan);
        }

    private:
        simplex::dp m_bat_pos;
        simplex::circle m_ball;
        point_f m_direction;

    private:
        random::engine rng;
};

SDLPP_MAIN(example)
