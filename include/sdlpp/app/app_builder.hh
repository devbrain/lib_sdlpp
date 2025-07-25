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

#ifndef SDLPP_APP_APP_BUILDER_HH
#define SDLPP_APP_APP_BUILDER_HH

#include <sdlpp/app/app.hh>
#include <sdlpp/app/app_impl.hh>
#include <functional>
#include <memory>

namespace sdlpp {
    
    /**
     * @brief Function-based application callbacks
     * 
     * For applications that don't need a full class hierarchy,
     * this provides a simple function-based interface.
     */
    struct app_callbacks {
        std::function<bool(int, char*[])> init;
        std::function<bool()> iterate;  
        std::function<bool(const event&)> event;
        std::function<void()> quit;
        
        // Optional callbacks
        std::function<void(const std::string&)> error;
        std::function<bool(int, char*[])> parse_args;
        std::function<float()> get_delta_time;
    };
    
    /**
     * @brief Adapter that wraps callbacks into app_interface
     */
    class callback_application final : public app_interface {
        app_callbacks callbacks_;
        std::chrono::steady_clock::time_point last_frame_time_;
        std::chrono::steady_clock::time_point start_time_;
        
    public:
        explicit callback_application(app_callbacks callbacks) 
            : callbacks_(std::move(callbacks)) {
            start_time_ = std::chrono::steady_clock::now();
            last_frame_time_ = start_time_;
        }
        
        bool init(int argc, char* argv[]) override {
            if (callbacks_.parse_args && !callbacks_.parse_args(argc, argv)) {
                return false;
            }
            return callbacks_.init ? callbacks_.init(argc, argv) : true;
        }
        
        bool iterate() override {
            // Update timing for delta_time callback
            auto now = std::chrono::steady_clock::now();
            auto delta = now - last_frame_time_;
            last_frame_time_ = now;
            
            if (callbacks_.get_delta_time) {
                // Store delta for get_delta_time callback
                float dt = std::chrono::duration<float>(delta).count();
                callbacks_.get_delta_time = [dt]() { return dt; };
            }
            
            return callbacks_.iterate ? callbacks_.iterate() : true;
        }
        
        bool event(const sdlpp::event& e) override {
            return callbacks_.event ? callbacks_.event(e) : true;
        }
        
        void quit() override {
            if (callbacks_.quit) callbacks_.quit();
        }
    };
    
    /**
     * @brief Builder for creating applications with method chaining
     */
    class app_builder {
        struct impl {
            app_callbacks callbacks;
            application::config app_config;
            std::optional<init> sdl_init;
            std::optional<window> main_window;
            std::optional<renderer> main_renderer;
            bool running = true;
        };
        
        std::shared_ptr<impl> impl_;
        
    public:
        app_builder() : impl_(std::make_shared<impl>()) {}
        
        /**
         * @brief Set initialization callback
         */
        app_builder& on_init(std::function<bool(int, char*[])> fn) {
            impl_->callbacks.init = [this, fn](int argc, char* argv[]) {
                // Initialize SDL
                try {
                    impl_->sdl_init.emplace(impl_->app_config.sdl_flags);
                } catch (const std::exception& e) {
                    if (impl_->callbacks.error) {
                        impl_->callbacks.error(std::string("SDL init failed: ") + e.what());
                    }
                    return false;
                }
                
                // Create window if requested
                if (impl_->app_config.auto_create_window) {
                    auto window_result = window::create(
                        impl_->app_config.window_title,
                        impl_->app_config.window_width,
                        impl_->app_config.window_height,
                        impl_->app_config.window_flags
                    );
                    
                    if (!window_result) {
                        if (impl_->callbacks.error) {
                            impl_->callbacks.error("Failed to create window: " + window_result.error());
                        }
                        return false;
                    }
                    
                    impl_->main_window = std::move(*window_result);
                    
                    // Create renderer if requested
                    if (impl_->app_config.auto_create_renderer) {
                        auto renderer_result = renderer::create(*impl_->main_window, impl_->app_config.renderer_driver);
                        if (renderer_result && impl_->app_config.vsync != 0) {
                            [[maybe_unused]] auto vsync_result = (*renderer_result).set_vsync(impl_->app_config.vsync);
                        }
                        if (!renderer_result) {
                            if (impl_->callbacks.error) {
                                impl_->callbacks.error("Failed to create renderer: " + renderer_result.error());
                            }
                            return false;
                        }
                        impl_->main_renderer = std::move(*renderer_result);
                    }
                }
                
                // Call user init
                return fn ? fn(argc, argv) : true;
            };
            return *this;
        }
        
        /**
         * @brief Set frame callback
         */
        app_builder& on_frame(std::function<bool()> fn) {
            impl_->callbacks.iterate = [this, fn]() {
                if (!impl_->running) return false;
                
                // Process events
                auto& queue = get_event_queue();
                while (auto evt = queue.poll()) {
                    if (impl_->app_config.handle_quit_event && evt->is<quit_event>()) {
                        impl_->running = false;
                        return false;
                    }
                    
                    if (impl_->callbacks.event && !impl_->callbacks.event(*evt)) {
                        impl_->running = false;
                        return false;
                    }
                }
                
                // Call user frame callback
                bool frame_result = fn ? fn() : true;
                
                // Present renderer if we have one
                if (impl_->main_renderer) {
                    if (auto res = impl_->main_renderer->present(); !res && impl_->callbacks.error) {
                        impl_->callbacks.error("Renderer present failed: " + res.error());
                    }
                }
                
                return frame_result && impl_->running;
            };
            return *this;
        }
        
        /**
         * @brief Set event callback
         */
        app_builder& on_event(std::function<bool(const event&)> fn) {
            impl_->callbacks.event = fn;
            return *this;
        }
        
        /**
         * @brief Set quit callback
         */
        app_builder& on_quit(std::function<void()> fn) {
            impl_->callbacks.quit = [this, fn]() {
                if (fn) fn();
                
                // Clean up
                impl_->main_renderer.reset();
                impl_->main_window.reset();
                impl_->sdl_init.reset();
            };
            return *this;
        }
        
        /**
         * @brief Set error callback
         */
        app_builder& on_error(std::function<void(const std::string&)> fn) {
            impl_->callbacks.error = fn;
            return *this;
        }
        
        /**
         * @brief Configure the application
         */
        app_builder& with_config(application::config cfg) {
            impl_->app_config = std::move(cfg);
            return *this;
        }
        
        /**
         * @brief Set window configuration
         */
        app_builder& with_window(const std::string& title, int width, int height) {
            impl_->app_config.window_title = title;
            impl_->app_config.window_width = width;
            impl_->app_config.window_height = height;
            impl_->app_config.auto_create_window = true;
            return *this;
        }
        
        /**
         * @brief Set window flags
         */
        app_builder& with_window_flags(window_flags flags) {
            impl_->app_config.window_flags = flags;
            return *this;
        }
        
        /**
         * @brief Enable renderer creation
         */
        app_builder& with_renderer(int vsync = 1, const char* driver = nullptr) {
            impl_->app_config.auto_create_renderer = true;
            impl_->app_config.vsync = vsync;
            impl_->app_config.renderer_driver = driver;
            return *this;
        }
        
        /**
         * @brief Disable automatic window creation
         */
        app_builder& no_window() {
            impl_->app_config.auto_create_window = false;
            impl_->app_config.auto_create_renderer = false;
            return *this;
        }
        
        /**
         * @brief Get the window (if created)
         */
        sdlpp::window& get_window() {
            if (!impl_->main_window) throw std::runtime_error("No window created");
            return *impl_->main_window;
        }
        
        /**
         * @brief Get the renderer (if created)
         */
        sdlpp::renderer& get_renderer() {
            if (!impl_->main_renderer) throw std::runtime_error("No renderer created");
            return *impl_->main_renderer;
        }
        
        /**
         * @brief Build and run the application
         * @param argc Argument count
         * @param argv Argument values
         * @return Exit code
         */
        int run([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
            callback_application app(impl_->callbacks);
            return sdlpp::run(app);
        }
    };
    
    /**
     * @brief Run an application with callbacks
     * @param argc Argument count
     * @param argv Argument values
     * @param callbacks Application callbacks
     * @return Exit code
     */
    inline int run([[maybe_unused]] int argc, [[maybe_unused]] char* argv[], app_callbacks callbacks) {
        callback_application app(std::move(callbacks));
        detail::g_current_app = &app;
        return 0;
    }
    
} // namespace sdlpp

#endif // SDLPP_APP_APP_BUILDER_HH