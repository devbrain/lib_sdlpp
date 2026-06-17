#include <simplex/simplex.hh>
#include <simplex/sprite_atlas.hh>
#include <simplex/sprite.hh>
#include <sdlpp/app/entry_point.hh>
#include <sdlpp/image/image.hh>
#include <random>
#include <cmath>

using namespace simplex::literals;

namespace simplex {
    class wgt_animation_demo : public simplex::application {
    private:
        std::optional<simplex::sprite_atlas> m_atlas;
        std::optional<sdlpp::texture> m_background;

        struct sprite_data {
            simplex::animated_sprite sprite;
            float dx{4.0f};
            float dy{4.0f};
            simplex::size size{};
        };
        std::vector<sprite_data> m_sprites;
        simplex::animated_sprite m_scripted_sprite;

        void on_ready() override {
            simplex::application::on_ready();

            auto& r = get_renderer();

            // Load WGT sprite file (.spr) and generate frames/bitmasks
            auto atlas_res = simplex::sprite_atlas::load_wgt_spr(r, "/home/igor/proj/wgt/wgttut4/ANIM.SPR", true);
            if (!atlas_res) {
                LOG_ERROR("Failed to load WGT sprites: ", atlas_res.error());
                return;
            }
            m_atlas = std::move(*atlas_res);
            LOG_INFO("Successfully loaded WGT sprite atlas with ", m_atlas->frame_count(), " frames.");

            // Load background PCX using onyx_image
            auto bg_res = sdlpp::image::load_texture(r, "/home/igor/proj/wgt/wgttut4/LUNAR.PCX");
            if (!bg_res) {
                LOG_ERROR("Failed to load background PCX: ", bg_res.error());
                return;
            }
            m_background = std::move(*bg_res);

            // Setup run animation sequence (frames 0 to 29)
            simplex::animation_sequence run_seq;
            run_seq.frames.reserve(30);
            for (std::size_t i = 0; i < 30; ++i) {
                run_seq.frames.push_back(i);
            }
            run_seq.fps = 15.0f; // 15 frames per second
            run_seq.loop = true;

            // Initialize 10 sprites
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis_x(0.0f, 320.0f - 42.0f);
            std::uniform_real_distribution<float> dis_y(0.0f, 200.0f - 26.0f);
            std::uniform_real_distribution<float> dis_dir(-1.0f, 1.0f);

            m_sprites.reserve(10);
            for (int i = 0; i < 10; ++i) {
                sprite_data s;
                s.sprite.add_animation("run", run_seq);
                s.sprite.play("run");
                
                float sx = dis_x(gen);
                float sy = dis_y(gen);
                s.sprite.position = {simplex::dp{sx}, simplex::dp{sy}};
                
                s.dx = dis_dir(gen) > 0.0f ? 4.0f : -4.0f;
                s.dy = dis_dir(gen) > 0.0f ? 4.0f : -4.0f;
                s.size = m_atlas->get_frame_size(0);

                m_sprites.push_back(std::move(s));
            }

            // Initialize scripted sprite starting at (60, 50)
            m_scripted_sprite.add_animation("run", run_seq);
            m_scripted_sprite.play("run");
            m_scripted_sprite.position = {60_dp, 50_dp};
        }

        void on_update(float dt) override {
            simplex::application::on_update(dt);

            // Speed multiplier: original WGT program ran at speed = 4 per loop.
            // Scale by target 60fps to make movements independent of refresh rate.
            float speed_multiplier = dt * 60.0f;

            for (auto& s : m_sprites) {
                s.sprite.update(dt);

                s.sprite.position.x += simplex::dp{s.dx * speed_multiplier};
                s.sprite.position.y += simplex::dp{s.dy * speed_multiplier};

                // Bounce off virtual screen boundaries (320x200 space)
                float x_val = s.sprite.position.x.design();
                float y_val = s.sprite.position.y.design();
                float w_val = s.size.width.design();
                float h_val = s.size.height.design();

                if (x_val < 0.0f) {
                    s.sprite.position.x = 0_dp;
                    s.dx = -s.dx;
                } else if (x_val > 320.0f - w_val) {
                    s.sprite.position.x = simplex::dp{320.0f - w_val};
                    s.dx = -s.dx;
                }

                if (y_val < 0.0f) {
                    s.sprite.position.y = 0_dp;
                    s.dy = -s.dy;
                } else if (y_val > 200.0f - h_val) {
                    s.sprite.position.y = simplex::dp{200.0f - h_val};
                    s.dy = -s.dy;
                }
            }

            // Update the automated scripted sprite and queue path if not moving
            m_scripted_sprite.update(dt);
            if (!m_scripted_sprite.is_moving()) {
                auto ease_in_out = [](float t) {
                    return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
                };
                m_scripted_sprite.queue_movement({.target_offset = {200_dp, 0_dp}, .duration = 2.0f, .easing_fn = ease_in_out});
                m_scripted_sprite.queue_movement({.target_offset = {0_dp, 100_dp}, .duration = 1.0f, .easing_fn = ease_in_out});
                m_scripted_sprite.queue_movement({.target_offset = {-200_dp, 0_dp}, .duration = 2.0f, .easing_fn = ease_in_out});
                m_scripted_sprite.queue_movement({.target_offset = {0_dp, -100_dp}, .duration = 1.0f, .easing_fn = ease_in_out});
            }
        }

        void on_render(sdlpp::renderer& r) override {
            r.clear();

            // Draw background stretched to fit the virtual layout coordinates (320x200)
            if (m_background) {
                std::optional<sdlpp::rect<float>> src_opt = std::nullopt;
                std::optional<sdlpp::rect<float>> dst_opt = sdlpp::rect<float>{0.0f, 0.0f, 320.0f * scale(), 200.0f * scale()};
                r.copy(*m_background, src_opt, dst_opt);
            }

            // Draw all sprites via the atlas
            if (m_atlas) {
                for (const auto& s : m_sprites) {
                    draw_sprite(*m_atlas, s.sprite);
                }
                // Draw the automated scripted sprite
                draw_sprite(*m_atlas, m_scripted_sprite);
            }

            simplex::application::on_render(r);
        }

        sdlpp::window_config get_window_config() override {
            auto cfg = simplex::application::get_window_config();
            cfg.title = "WGT Animation Port to Simplex";
            cfg.width = 960;  // 3x integer scaling width
            cfg.height = 600; // 3x integer scaling height
            return cfg;
        }
    };
} // namespace simplex

SDLPP_MAIN(simplex::wgt_animation_demo)
