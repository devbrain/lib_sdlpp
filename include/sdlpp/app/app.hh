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

#ifndef SDLPP_APP_HH
#define SDLPP_APP_HH

#include <sdlpp/detail/export.hh>
#include <sdlpp/core/core.hh>
#include <sdlpp/events/events.hh>
#include <atomic>

namespace sdlpp {

    /**
     * @brief Minimal application base class for SDL3 app model
     *
     * Provides SDL initialization and lifecycle management.
     * Derived classes create their own resources (windows, renderers, etc.)
     *
     * Example:
     * @code
     * #include <sdlpp/app/entry_point.hh>
     *
     * class MyApp : public sdlpp::abstract_application {
     *     sdlpp::window window_;
     *     sdlpp::renderer renderer_;
     *
     * public:
     *     void on_init(int argc, char* argv[]) override {
     *         window_ = sdlpp::window::create("My App", 1280, 720).value();
     *         renderer_ = sdlpp::renderer::create(window_).value();
     *     }
     *
     *     void handle_event(const sdlpp::event& e) override {
     *         // Handle input events here
     *     }
     *
     *     void on_iterate() override {
     *         renderer_.set_draw_color(sdlpp::colors::black);
     *         renderer_.clear();
     *         renderer_.present();
     *     }
     * };
     *
     * SDLPP_MAIN(MyApp)
     * @endcode
     */
    class SDLPP_EXPORT abstract_application {
    public:
        /**
         * @brief Construct application with SDL initialization flags
         * @param flags SDL subsystems to initialize (default: video + events)
         */
        explicit abstract_application(init_flags flags = init_flags::video | init_flags::events);

        virtual ~abstract_application() = default;

        // Non-copyable, non-movable
        abstract_application(const abstract_application&) = delete;
        abstract_application& operator=(const abstract_application&) = delete;
        abstract_application(abstract_application&&) = delete;
        abstract_application& operator=(abstract_application&&) = delete;

        /**
         * @brief Called after SDL is initialized
         * @param argc Argument count from command line
         * @param argv Argument values from command line
         * @throws Any exception to signal initialization failure
         */
        virtual void on_init([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]);

        /**
         * @brief Called for each SDL event
         * @param e The event to handle
         * @note Dispatches to lifecycle callbacks, then calls handle_event()
         */
        virtual void on_event(const event& e);

        /**
         * @brief Handle application events
         * @param e The event to handle
         * @note Override this to handle input and other events
         */
        virtual void handle_event([[maybe_unused]] const event& e);

        /**
         * @brief Called when quit is requested (e.g., window close)
         * @note Default implementation calls quit()
         */
        virtual void on_quit_requested();

        /**
         * @brief Called when the app is being terminated
         */
        virtual void on_terminating();

        /**
         * @brief Called when the system is low on memory
         */
        virtual void on_low_memory();

        /**
         * @brief Called when the app is about to enter background
         */
        virtual void on_will_enter_background();

        /**
         * @brief Called when the app entered background
         */
        virtual void on_did_enter_background();

        /**
         * @brief Called when the app is about to enter foreground
         */
        virtual void on_will_enter_foreground();

        /**
         * @brief Called when the app entered foreground
         */
        virtual void on_did_enter_foreground();

        /**
         * @brief Called once per frame
         * @throws Any exception to signal failure
         * @note Call quit() to exit cleanly
         */
        virtual void on_iterate();

        /**
         * @brief Called during shutdown for cleanup
         * @note Must not throw
         */
        virtual void on_quit() noexcept;

        /**
         * @brief Request application to quit
         * @note Thread-safe
         */
        void quit() noexcept;

        /**
         * @brief Check if application is still running
         * @return true if running, false if quit was requested
         * @note Thread-safe
         */
        [[nodiscard]] bool is_running() const noexcept;

        /// @internal Initialize SDL (called by entry_point)
        void init_sdl_();

        /// @internal Shutdown SDL (called by entry_point)
        void shutdown_sdl_() noexcept;

    private:
        init_flags init_flags_;
        std::atomic<bool> running_{true};
        std::optional<init> sdl_init_;
    };

} // namespace sdlpp

#endif // SDLPP_APP_HH
