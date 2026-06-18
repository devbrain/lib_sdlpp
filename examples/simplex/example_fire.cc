#include <simplex/simplex.hh>
#include <sdlpp/video/texture.hh>
#include <sdlpp/app/entry_point.hh>
#include <random>
#include <vector>

namespace simplex {
    class wgt_fire_demo : public simplex::application {
    private:
        static constexpr int FIRE_WIDTH = 160;
        static constexpr int FIRE_HEIGHT = 100;

        std::optional<sdlpp::texture> m_fire_texture;
        std::optional<sdlpp::palette> m_palette;
        std::vector<uint8_t> m_fire_buffer;

        void on_ready() override {
            simplex::application::on_ready();

            auto& r = get_renderer();

            // 1. Create INDEX8 texture (streaming since we update it every frame)
            auto tex_res = sdlpp::texture::create(
                r,
                sdlpp::pixel_format_enum::INDEX8,
                sdlpp::texture_access::streaming,
                FIRE_WIDTH,
                FIRE_HEIGHT
            );
            if (!tex_res) {
                LOG_ERROR("Failed to create INDEX8 texture: ", tex_res.error());
                return;
            }
            m_fire_texture = std::move(*tex_res);
            m_fire_texture->set_scale_mode(sdlpp::scale_mode::linear); // Crisp pixelated scaling

            // 2. Initialize Fire Buffer to 0 (all black)
            m_fire_buffer.assign(FIRE_WIDTH * FIRE_HEIGHT, 0);

            // 3. Define Doom fire colors (37 colors)
            const std::vector<sdlpp::color> fire_colors = {
                {0x07,0x07,0x07, 255}, {0x1F,0x07,0x07, 255}, {0x2F,0x0F,0x07, 255}, {0x47,0x0F,0x07, 255},
                {0x57,0x17,0x07, 255}, {0x67,0x1F,0x07, 255}, {0x77,0x1F,0x07, 255}, {0x8F,0x27,0x07, 255},
                {0x9F,0x2F,0x07, 255}, {0xAF,0x3F,0x07, 255}, {0xBF,0x47,0x07, 255}, {0xC7,0x47,0x07, 255},
                {0xDF,0x4F,0x07, 255}, {0xDF,0x57,0x07, 255}, {0xDF,0x57,0x0F, 255}, {0xD7,0x5F,0x0F, 255},
                {0xD7,0x67,0x0F, 255}, {0xCF,0x6F,0x0F, 255}, {0xCF,0x77,0x0F, 255}, {0xCF,0x7F,0x0F, 255},
                {0xCF,0x87,0x17, 255}, {0xC7,0x8F,0x17, 255}, {0xC7,0x97,0x1F, 255}, {0xBF,0x9F,0x1F, 255},
                {0xBF,0xA7,0x27, 255}, {0xBF,0xAF,0x2F, 255}, {0xB7,0xB7,0x2F, 255}, {0xB7,0xB7,0x37, 255},
                {0xCF,0xCF,0x6F, 255}, {0xDF,0xDF,0x9F, 255}, {0xEF,0xEF,0xC7, 255}, {0xF7,0xF7,0xDF, 255},
                {0xFF,0xFF,0xFF, 255}
            };

            // Create 256-color palette
            auto pal_res = sdlpp::palette::create(256);
            if (!pal_res) {
                LOG_ERROR("Failed to create palette: ", pal_res.error());
                return;
            }
            m_palette = std::move(*pal_res);

            // Populate palette colors, pad rest with white/last color
            std::vector<sdlpp::color> full_palette_colors(256, fire_colors.back());
            for (std::size_t i = 0; i < fire_colors.size(); ++i) {
                full_palette_colors[i] = fire_colors[i];
            }
            m_palette->set_colors(full_palette_colors);

            // 4. Attach Palette to Texture (GPU side)
#if SDL_VERSION_ATLEAST(3, 4, 0)
            auto set_pal_res = m_fire_texture->set_palette(*m_palette);
            if (!set_pal_res) {
                LOG_ERROR("Failed to set texture palette: ", set_pal_res.error());
            } else {
                LOG_INFO("Successfully configured native GPU texture palette swapping.");
            }
#else
            LOG_WARNING("SDL version is too old for GPU texture palettes. Fading will fall back to CPU side.");
#endif
        }

        void update_fire() {
            // Seed bottom row with fire source (max index = 32, which is white)
            for (int x = 0; x < FIRE_WIDTH; ++x) {
                m_fire_buffer[(FIRE_HEIGHT - 1) * FIRE_WIDTH + x] = 32;
            }

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<int> dis_decay(0, 3);

            for (int y = 0; y < FIRE_HEIGHT - 1; ++y) {
                for (int x = 0; x < FIRE_WIDTH; ++x) {
                    int from_idx = (y + 1) * FIRE_WIDTH + x;
                    int val = m_fire_buffer[from_idx];

                    if (val == 0) {
                        m_fire_buffer[y * FIRE_WIDTH + x] = 0;
                    } else {
                        // Decay and drift calculations
                        int decay = dis_decay(gen) & 3;
                        int drift = decay & 1; // 0 or 1
                        int target_x = (x - drift + FIRE_WIDTH) % FIRE_WIDTH;
                        int target_idx = y * FIRE_WIDTH + target_x;

                        int new_val = val - (decay & 1);
                        if (new_val < 0) new_val = 0;

                        m_fire_buffer[target_idx] = static_cast<uint8_t>(new_val);
                    }
                }
            }
        }

        void on_update(float dt) override {
            simplex::application::on_update(dt);
            update_fire();

            // Upload the modified 160x100 INDEX8 fire buffer to the GPU texture
            if (m_fire_texture) {
                m_fire_texture->update(
                    std::optional<sdlpp::rect<int>>{},
                    m_fire_buffer.data(),
                    FIRE_WIDTH * sizeof(uint8_t)
                );
            }
        }

        void on_render(sdlpp::renderer& r) override {
            r.clear();

            if (m_fire_texture) {
                // Draw fire texture scaled to cover the entire window
                auto dest_rect = sdlpp::rect<float>{0.0f, 0.0f, get_width().px(), get_height().px()};
                r.copy(*m_fire_texture, std::optional<sdlpp::rect<float>>{}, std::make_optional(dest_rect));
            }

            simplex::application::on_render(r);
        }

        sdlpp::window_config get_window_config() override {
            auto cfg = simplex::application::get_window_config();
            cfg.title = "GPU-Friendly Classic Fire Effect in SDL3";
            cfg.width = 800;
            cfg.height = 600;
            return cfg;
        }
    };
} // namespace simplex

SDLPP_MAIN(simplex::wgt_fire_demo)
