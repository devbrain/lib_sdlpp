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
        static constexpr simplex::circle static_circle{{450_dp, 225_dp}, 30_dp};

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

            draw_field();
        }

        void draw_field() {
            const auto x1 = box.position().x;
            const auto x2 = x1 + box.w - 1_dp;

            const auto y1 = box.position().y;
            const auto y2 = y1 + box.h - 1_dp;

            constexpr auto gap = 5_dp;
            const auto w = get_width();
            const auto h = get_height();

            draw_line_dotted({x1, 0_dp}, {x1, h}, gap, colors::yellow);
            draw_line_dotted({x2, 0_dp}, {x2, h}, gap, colors::yellow);
            draw_line_dotted({0_dp, y1}, {w, y1}, gap, colors::yellow);
            draw_line_dotted({0_dp, y2}, {w, y2}, gap, colors::yellow);

            draw_rect(box, colors::blue);
            draw_circle(static_circle, colors::blue);

            if (line_ends[0]) {
                draw_cross(*line_ends[0], 8_dp, colors::aquamarine);
                const auto end_point = !line_ends[1] ? get_mouse_pos() : *line_ends[1];

                draw_line(*line_ends[0], end_point, colors::aquamarine);

                collide::segment seg = simplex::to_world({*line_ends[0], end_point}, world_view);

                constexpr float normal_len = 15.0f;
                auto get_design_normal = [&](const collide::vec& n) {
                    float scale_factor = normal_len / world_view.pixels_per_unit;
                    return simplex::to_design_vector(n, world_view) * scale_factor;
                };

                // AABB Intersection
                collide::aabb rect = simplex::to_world(box, world_view);
                auto ir_box = collide::intersect_param(rect, seg);
                if (ir_box && ir_box->segment_overlaps()) {
                    auto p1 = simplex::to_design(seg.point_in_time(ir_box->entry_param), world_view);
                    auto p2 = simplex::to_design(seg.point_in_time(ir_box->exit_param), world_view);

                    draw_line_thick(p1, p2, 4, colors::yellow);

                    set_line_style(line_style::dashed(4.0f, 2.0f));
                    draw_arrow(p1, p1 + get_design_normal(ir_box->entry_normal), 6_dp, 30.0f, 2.0f, colors::red);
                    set_line_style(line_style::solid());
                    draw_arrow(p2, p2 + get_design_normal(ir_box->exit_normal), 6_dp, 30.0f, 2.0f, colors::green);
                }

                // Circle Intersection
                collide::circle circ = simplex::to_world(static_circle, world_view);
                auto ir_circ = collide::intersect_param(circ, seg);
                if (ir_circ && ir_circ->segment_overlaps()) {
                    auto p1 = simplex::to_design(seg.point_in_time(ir_circ->entry_param), world_view);
                    auto p2 = simplex::to_design(seg.point_in_time(ir_circ->exit_param), world_view);

                    draw_line_thick(p1, p2, 4, colors::yellow);

                    set_line_style(line_style::dashed(4.0f, 2.0f));
                    draw_arrow(p1, p1 + get_design_normal(ir_circ->entry_normal), 6_dp, 30.0f, 2.0f, colors::red);
                    set_line_style(line_style::solid());
                    draw_arrow(p2, p2 + get_design_normal(ir_circ->exit_normal), 6_dp, 30.0f, 2.0f, colors::green);
                }

                draw_cross(end_point, 8_dp, colors::aquamarine);
            }
        }
};

SDLPP_MAIN(example)
