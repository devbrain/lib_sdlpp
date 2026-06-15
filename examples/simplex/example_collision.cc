#include <simplex/simplex.hh>
#include <simplex/collide/shapes.hh>
#include <simplex/collide/bridge.hh>
#include <sdlpp/app/entry_point.hh>
#include <optional>

using namespace simplex::literals;
using namespace sdlpp;
namespace collide = simplex::collide;

// Scratchpad for the collision engine. Collision runs in world space
// (simplex::collide); dp shapes are only used to draw. The view{} below is an
// identity transform, so world and display coincide for now — swap in a real
// camera (origin / pixels_per_unit / y_up) later without touching the queries.

class example : public simplex::application {
    private:
        static constexpr simplex::rect box{200_dp, 200_dp, 50_dp, 50_dp};

        simplex::view world_view{};

        std::optional <simplex::point> line_ends[2] = {};

        void on_update([[maybe_unused]] float delta_time) override {


            if (get_mouse(mouse_button::left).pressed) {
                const auto mousep = get_mouse_pos();
                if (!line_ends[0]) {
                    line_ends[0] = mousep;
                } else if (!line_ends[1]) {
                    line_ends[1] = mousep;
                } else {                       // both set → start a new pair
                    line_ends[0] = mousep;
                    line_ends[1] = std::nullopt;
                }
            }
            if (get_key(scancode::escape).pressed) {
                if (line_ends[1]) {
                    line_ends[1] = std::nullopt;
                } else if (line_ends[0]) {
                    line_ends[0] = std::nullopt;
                }
            }
            clear();

            draw_field(box);
        }

        void draw_field(const simplex::rect& r) {
            const auto x1 = r.position().x;
            const auto x2 = x1 + r.w - 1_dp;

            const auto y1 = r.position().y;
            const auto y2 = y1 + r.h - 1_dp;

            constexpr auto gap = 5_dp;
            const auto w = get_width();
            const auto h = get_height();

            draw_line_dotted({x1, 0_dp}, {x1, h}, gap, colors::yellow);
            draw_line_dotted({x2, 0_dp}, {x2, h}, gap, colors::yellow);
            draw_line_dotted({0_dp, y1}, {w, y1}, gap, colors::yellow);
            draw_line_dotted({0_dp, y2}, {w, y2}, gap, colors::yellow);

            draw_rect(r, colors::blue);

            if (line_ends[0]) {
                draw_cross(*line_ends[0], colors::aquamarine, 3_dp);
                const auto end_point = !line_ends[1] ? get_mouse_pos() : *line_ends[1];

                collide::segment seg  = simplex::to_world({*line_ends[0], end_point}, world_view);
                collide::aabb    rect = simplex::to_world(box, world_view);

                auto ir = collide::intersect_times(rect, seg);
                if (collide::segment_intersects(ir)) {
                    auto p1 = simplex::to_design(seg.point_in_time(ir.entry), world_view);
                    auto p2 = simplex::to_design(seg.point_in_time(ir.exit), world_view);

                    draw_line(*line_ends[0], p1,  colors::aquamarine);
                    draw_line_thick( p1, p2,  4, colors::yellow);
                    draw_line( p2, end_point, colors::aquamarine);

                } else {
                    draw_line(*line_ends[0], end_point,  colors::aquamarine);
                }

                draw_cross(end_point, colors::aquamarine, 3_dp);
            }
        }

        void draw_cross(const simplex::point& center, const color& c, const simplex::dp& size) {
            static constexpr float sqrt2 = 1.41421356237f;
            const auto x1 = -sqrt2 * size +  center.x;
            const auto x2 =  sqrt2 * size +  center.x;
            const auto y1 = -sqrt2 * size +  center.y;
            const auto y2 =  sqrt2 * size +  center.y;

            draw_line({x1, y1}, {x2, y2}, c);
            draw_line({x1, y2}, {x2, y1}, c);
        }
};

SDLPP_MAIN(example)
