//
// SDL++
// Copyright (C) 2024 Igor Molchanov
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef SDLPP_GAME_APPLICATION_HH
#define SDLPP_GAME_APPLICATION_HH

#include <sdlpp/detail/export.hh>
#include <sdlpp/app/app.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/core/failsafe_backend.hh>
#include <chrono>
#include <optional>
#include <thread>

namespace sdlpp {

    /**
     * @brief Window configuration for game_application
     */
    struct SDLPP_EXPORT window_config {
        std::string title;
        int width;
        int height;
        window_flags flags = window_flags::none;
        int target_fps = 60;  ///< Target FPS (0 = unlimited/vsync)
    };

    /**
     * @brief Game application base class with window, renderer, and game loop
     *
     * Provides:
     * - Single window and renderer management
     * - Delta time calculation using std::chrono
     * - FPS tracking and enforcement
     * - Separate update and render callbacks
     * - Automatic failsafe logger configuration
     *
     * Example:
     * @code
     * #include <sdlpp/app/entry_point.hh>
     * #include <sdlpp/app/game_application.hh>
     *
     * class MyGame : public sdlpp::game_application {
     * protected:
     *     sdlpp::window_config get_window_config() override {
     *         return {"My Game", 1280, 720, sdlpp::window_flags::none, 60};
     *     }
     *
     *     void on_update(float dt) override {
     *         // Update game logic
     *     }
     *
     *     void on_render(sdlpp::renderer& r) override {
     *         r.set_draw_color(sdlpp::colors::black);
     *         r.clear();
     *         // Draw game
     *         r.present();
     *     }
     * };
     *
     * SDLPP_MAIN(MyGame)
     * @endcode
     */
    class SDLPP_EXPORT game_application : public abstract_application {
    public:
        using clock = std::chrono::steady_clock;
        using duration = std::chrono::duration<float>;
        using time_point = clock::time_point;

        game_application();
        ~game_application() override;

    protected:
        /**
         * @brief Get window configuration (pure virtual)
         * @return Window configuration
         */
        virtual window_config get_window_config() = 0;

        /**
         * @brief Get window icon surface (optional)
         * @return Surface to use as window icon, or nullopt for no icon
         */
        virtual std::optional<surface> get_window_icon();

        /**
         * @brief Called first with command line arguments
         * @param argc Argument count
         * @param argv Argument values
         */
        virtual void on_config([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]);

        /**
         * @brief Called after window and renderer are created
         */
        virtual void on_ready();

        /**
         * @brief Called every frame with delta time
         * @param delta_time Time since last frame in seconds
         */
        virtual void on_update([[maybe_unused]] float delta_time);

        /**
         * @brief Called every frame for rendering
         * @param r The renderer to draw with
         */
        virtual void on_render([[maybe_unused]] renderer& r);

        // Window event callbacks

        /**
         * @brief Called when window is shown
         */
        virtual void on_window_shown();

        /**
         * @brief Called when window is hidden
         */
        virtual void on_window_hidden();

        /**
         * @brief Called when window needs to be redrawn
         */
        virtual void on_window_exposed();

        /**
         * @brief Called when window is moved
         * @param x New x position
         * @param y New y position
         */
        virtual void on_window_moved([[maybe_unused]] int x, [[maybe_unused]] int y);

        /**
         * @brief Called when window is resized
         * @param width New width
         * @param height New height
         */
        virtual void on_window_resized([[maybe_unused]] int width, [[maybe_unused]] int height);

        /**
         * @brief Called when window is minimized
         */
        virtual void on_window_minimized();

        /**
         * @brief Called when window is maximized
         */
        virtual void on_window_maximized();

        /**
         * @brief Called when window is restored from minimized/maximized
         */
        virtual void on_window_restored();

        /**
         * @brief Called when mouse enters the window
         */
        virtual void on_window_mouse_enter();

        /**
         * @brief Called when mouse leaves the window
         */
        virtual void on_window_mouse_leave();

        /**
         * @brief Called when window gains keyboard focus
         */
        virtual void on_window_focus_gained();

        /**
         * @brief Called when window loses keyboard focus
         */
        virtual void on_window_focus_lost();

        /**
         * @brief Called when window enters fullscreen mode
         */
        virtual void on_window_enter_fullscreen();

        /**
         * @brief Called when window leaves fullscreen mode
         */
        virtual void on_window_leave_fullscreen();

        /**
         * @brief Called when window display scale changes (DPI change)
         * @param scale New display scale factor
         */
        virtual void on_window_display_scale_changed([[maybe_unused]] float scale);

        // Fullscreen control

        /**
         * @brief Check if window is in fullscreen mode
         * @return true if fullscreen
         */
        [[nodiscard]] bool is_fullscreen() const;

        /**
         * @brief Set fullscreen mode
         * @param fullscreen true for fullscreen, false for windowed
         * @return true on success
         */
        bool set_fullscreen(bool fullscreen);

        /**
         * @brief Toggle fullscreen mode
         * @return true on success
         */
        bool toggle_fullscreen();

        /**
         * @brief Get the window
         * @return Reference to the window
         */
        [[nodiscard]] window& get_window();
        [[nodiscard]] const window& get_window() const;

        /**
         * @brief Get the renderer
         * @return Reference to the renderer
         */
        [[nodiscard]] renderer& get_renderer();
        [[nodiscard]] const renderer& get_renderer() const;

        /**
         * @brief Get current FPS
         * @return Frames per second
         */
        [[nodiscard]] float fps() const;

        /**
         * @brief Get last frame's delta time
         * @return Delta time in seconds
         */
        [[nodiscard]] float delta_time() const;

        /**
         * @brief Get target FPS
         * @return Target frames per second (0 = unlimited)
         */
        [[nodiscard]] int target_fps() const;

        /**
         * @brief Set target FPS at runtime
         * @param fps Target frames per second (0 = unlimited)
         */
        void set_target_fps(int fps);

    private:
        void on_event(const event& e) override;

        void on_init(int argc, char* argv[]) final;

        void on_iterate() final;

    private:
        window window_;
        renderer renderer_;

        // Timing
        time_point last_frame_time_;
        time_point fps_update_time_;
        duration frame_duration_{1.0f / 60.0f};
        float delta_time_ = 0.0f;
        float fps_ = 0.0f;
        int frame_count_ = 0;
        int target_fps_ = 60;
    };

} // namespace sdlpp

#endif // SDLPP_GAME_APPLICATION_HH
