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

#ifndef SDLPP_APP_APP_ADVANCED_HH
#define SDLPP_APP_APP_ADVANCED_HH

#include <sdlpp/app/app.hh>
#include <sdlpp/app/app_types.hh>

namespace sdlpp {
    
    /**
     * @brief Advanced application interface with fine-grained control
     * 
     * This interface allows returning app_result instead of bool,
     * giving more control over the application lifecycle.
     */
    class app_interface_advanced {
    public:
        virtual ~app_interface_advanced() = default;
        
        /**
         * @brief Initialize the application
         * @param argc Argument count
         * @param argv Argument values
         * @return app_result::continue_ on success, app_result::failure to quit
         */
        virtual app_result init(int argc, char* argv[]) = 0;
        
        /**
         * @brief Called once per frame
         * @return app_result::continue_ to continue, app_result::success to quit gracefully
         */
        virtual app_result iterate() = 0;
        
        /**
         * @brief Handle an SDL event
         * @param e The event to handle
         * @return app_result::continue_ to continue, app_result::success to quit
         */
        virtual app_result event(const sdlpp::event& e) = 0;
        
        /**
         * @brief Clean up the application
         */
        virtual void quit() = 0;
    };
    
    /**
     * @brief Adapter to use app_interface with SDL callbacks that expect app_result
     */
    class app_interface_adapter : public app_interface_advanced {
        app_interface* wrapped_;
        
    public:
        explicit app_interface_adapter(app_interface* app) : wrapped_(app) {}
        
        app_result init(int argc, char* argv[]) override {
            return wrapped_->init(argc, argv) ? app_result::continue_ : app_result::failure;
        }
        
        app_result iterate() override {
            return wrapped_->iterate() ? app_result::continue_ : app_result::success;
        }
        
        app_result event(const sdlpp::event& e) override {
            return wrapped_->event(e) ? app_result::continue_ : app_result::success;
        }
        
        void quit() override {
            wrapped_->quit();
        }
    };
    
    /**
     * @brief Advanced application base class
     * 
     * Similar to application but with app_result returns
     */
    class application_advanced : public application, public app_interface_advanced {
    public:
        using application::application;
        
        // Override app_interface methods to delegate to app_interface_advanced
        bool init(int argc, char* argv[]) override final {
            auto result = init_advanced(argc, argv);
            return result == app_result::continue_;
        }
        
        bool iterate() override final {
            auto result = iterate_advanced();
            return result == app_result::continue_;
        }
        
        bool event(const sdlpp::event& e) override final {
            auto result = event_advanced(e);
            return result == app_result::continue_;
        }
        
        // Implement app_interface_advanced with delegation to base
        app_result init_advanced(int argc, char* argv[]) {
            return application::init(argc, argv) ? app_result::continue_ : app_result::failure;
        }
        
        app_result iterate_advanced() {
            return application::iterate() ? app_result::continue_ : app_result::success;
        }
        
        app_result event_advanced(const sdlpp::event& e) {
            return application::event(e) ? app_result::continue_ : app_result::success;
        }
        
    protected:
        // Hooks that can return app_result
        virtual app_result on_init_result() { 
            return on_init() ? app_result::continue_ : app_result::failure; 
        }
        
        virtual app_result on_frame_result() { 
            on_frame();
            return is_running() ? app_result::continue_ : app_result::success;
        }
        
        virtual app_result on_event_result(const event& e) { 
            return on_event(e) ? app_result::continue_ : app_result::success;
        }
    };
    
} // namespace sdlpp

// Alternative SDL3 callbacks for advanced interface
namespace sdlpp::detail {
    extern "C" {
        inline SDL_AppResult SDL_AppInit_Advanced(void** appstate, int argc, char* argv[]) {
            if (!g_current_app) return SDL_APP_FAILURE;
            
            // Check if it's an advanced app
            if (auto* advanced = dynamic_cast<app_interface_advanced*>(g_current_app)) {
                *appstate = advanced;
                return to_sdl_result(advanced->init(argc, argv));
            }
            
            // Fall back to adapter
            static app_interface_adapter adapter(g_current_app);
            *appstate = &adapter;
            return to_sdl_result(adapter.init(argc, argv));
        }
        
        inline SDL_AppResult SDL_AppIterate_Advanced(void* appstate) {
            auto* app = static_cast<app_interface_advanced*>(appstate);
            return to_sdl_result(app->iterate());
        }
        
        inline SDL_AppResult SDL_AppEvent_Advanced(void* appstate, const SDL_Event* evt) {
            auto* app = static_cast<app_interface_advanced*>(appstate);
            sdlpp::event wrapped_event(*evt);
            return to_sdl_result(app->event(wrapped_event));
        }
        
        inline void SDL_AppQuit_Advanced(void* appstate) {
            auto* app = static_cast<app_interface_advanced*>(appstate);
            app->quit();
        }
    }
}

#endif // SDLPP_APP_APP_ADVANCED_HH