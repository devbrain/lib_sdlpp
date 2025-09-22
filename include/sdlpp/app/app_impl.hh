#ifndef SDLPP_APP_APP_IMPL_HH
#define SDLPP_APP_APP_IMPL_HH

#include <sdlpp/app/app.hh>
#include <sdlpp/app/app_types.hh>
#include <SDL3/SDL_main.h>

namespace sdlpp {
    
    // Implementation of application methods
    inline bool application::init(int argc, char* argv[]) {
        // Parse command line
        if (!parse_args(argc, argv)) {
            return false;
        }
        
        // Initialize SDL
        try {
            sdl_init_.emplace(config_.sdl_flags);
        } catch (const std::exception& e) {
            on_error(std::string("SDL initialization failed: ") + e.what());
            return false;
        }
        
        // Auto-create window if requested
        if (config_.auto_create_window) {
            auto window_result = window::create(
                config_.window_title,
                config_.window_width,
                config_.window_height,
                config_.window_flags
            );
            
            if (!window_result) {
                on_error("Failed to create window: " + window_result.error());
                return false;
            }
            
            main_window_ = std::move(*window_result);
            
            // Auto-create renderer if requested
            if (config_.auto_create_renderer) {
                auto renderer_result = renderer::create(*main_window_, config_.renderer_driver);
                if (!renderer_result) {
                    on_error("Failed to create renderer: " + renderer_result.error());
                    return false;
                }
                main_renderer_ = std::move(*renderer_result);
                
                // Set vsync if requested
                if (config_.vsync != 0) {
                    if (auto vsync_result = main_renderer_->set_vsync(config_.vsync); !vsync_result) {
                        on_error("Warning: Failed to set vsync: " + vsync_result.error());
                    }
                }
            }
        }
        
        // Get event queue
        event_queue_ = &get_event_queue();
        
        // Initialize timing
        frame_start_ = std::chrono::steady_clock::now();
        last_frame_time_ = frame_start_;
        
        // Call derived class initialization
        initialized_ = on_init();
        return initialized_;
    }
    
    inline bool application::iterate() {
        if (!running_ || !initialized_) {
            return false;
        }
        
        // Update timing
        auto now = std::chrono::steady_clock::now();
        delta_time_ = now - last_frame_time_;
        last_frame_time_ = now;
        total_time_ = now - frame_start_;
        
        // Process all pending events
        while (auto event = event_queue_->poll()) {
            // Handle built-in quit event if enabled
            if (config_.handle_quit_event && event->is<quit_event>()) {
                running_ = false;
                return false;
            }
            
            // Let derived class handle the event
            if (!on_event(*event)) {
                running_ = false;
                return false;
            }
        }
        
        // Call frame update
        on_frame();
        
        // Present renderer if we have one
        if (main_renderer_) {
            if (auto res = main_renderer_->present(); !res) {
                on_error("Renderer present failed: " + res.error());
            }
        }
        
        return running_;
    }
    
    inline bool application::event(const sdlpp::event& e) {
        // Handle built-in quit event if enabled
        if (config_.handle_quit_event && e.is<quit_event>()) {
            running_ = false;
            return false;
        }
        
        // Forward to derived class
        return on_event(e);
    }
    
    inline void application::quit() {
        running_ = false;
        on_quit();
        
        // Clean up in reverse order
        main_renderer_.reset();
        main_window_.reset();
        sdl_init_.reset();
    }
    
    namespace detail {
        // Global app pointer for SDL callbacks
        inline app_interface* g_current_app = nullptr;
    }
    
    /**
     * @brief Run an application (class-based)
     * @tparam App Application class type (must derive from app_interface)
     * @param argc Argument count
     * @param argv Argument values
     * @return Exit code (0 for success)
     */
    template<typename App>
    int run([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
        App app;
        detail::g_current_app = &app;
        // SDL_main will call the SDL_App* functions
        return 0;
    }
    
    /**
     * @brief Run an application with custom config
     * @tparam App Application class type
     * @param argc Argument count
     * @param argv Argument values
     * @param config Application configuration
     * @return Exit code
     */
    template<typename App>
    int run(int argc, char* argv[], typename App::config config) {
        App app(std::move(config));
        detail::g_current_app = &app;
        return 0;
    }
    
    /**
     * @brief Run an existing application instance
     * @param app Application instance
     * @return Exit code
     */
    inline int run(app_interface& app) {
        detail::g_current_app = &app;
        return 0;
    }
    
} // namespace sdlpp

// SDL3 callback implementations
extern "C" {
    inline SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) {
        if (!sdlpp::detail::g_current_app) return SDL_APP_FAILURE;
        *appstate = sdlpp::detail::g_current_app;
        return sdlpp::detail::g_current_app->init(argc, argv) ? SDL_APP_CONTINUE : SDL_APP_FAILURE;
    }
    
    inline SDL_AppResult SDL_AppIterate(void* appstate) {
        auto* app = static_cast<sdlpp::app_interface*>(appstate);
        return app->iterate() ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
    }
    
    inline SDL_AppResult SDL_AppEvent(void* appstate, const SDL_Event* evt) {
        auto* app = static_cast<sdlpp::app_interface*>(appstate);
        sdlpp::event wrapped_event(*evt);
        return app->event(wrapped_event) ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
    }
    
    inline void SDL_AppQuit(void* appstate) {
        auto* app = static_cast<sdlpp::app_interface*>(appstate);
        app->quit();
    }
}

// Convenience macros
#define SDLPP_MAIN(AppClass) \
    int main(int argc, char* argv[]) { \
        return sdlpp::run<AppClass>(argc, argv); \
    }

#define SDLPP_MAIN_WITH_CONFIG(AppClass, ...) \
    int main(int argc, char* argv[]) { \
        return sdlpp::run<AppClass>(argc, argv, {__VA_ARGS__}); \
    }

#endif // SDLPP_APP_APP_IMPL_HH