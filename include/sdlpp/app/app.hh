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

#ifndef SDLPP_APP_APP_HH
#define SDLPP_APP_APP_HH

#include <sdlpp/core/core.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <optional>
#include <functional>
#include <iostream>
#include <string>
#include <chrono>

namespace sdlpp {
    /**
     * @brief Pure interface for SDL3 application model
     * 
     * This interface defines the contract for SDL3's new application
     * lifecycle callbacks (SDL_AppInit, SDL_AppIterate, etc.)
     */
    class app_interface {
    public:
        virtual ~app_interface() = default;
        
        /**
         * @brief Initialize the application
         * @param argc Argument count
         * @param argv Argument values
         * @return true on success, false to quit
         */
        virtual bool init(int argc, char* argv[]) = 0;
        
        /**
         * @brief Called once per frame
         * @return true to continue, false to quit
         */
        virtual bool iterate() = 0;
        
        /**
         * @brief Handle an SDL event
         * @param e The event to handle
         * @return true to continue, false to quit
         */
        virtual bool event(const sdlpp::event& e) = 0;
        
        /**
         * @brief Clean up the application
         */
        virtual void quit() = 0;
    };
    
    /**
     * @brief Base application class with common functionality
     * 
     * Provides sensible defaults and common resources for typical SDL applications.
     * Derived classes can override virtual methods to customize behavior.
     */
    class application : public app_interface {
    public:
        /**
         * @brief Application configuration
         */
        struct config {
            init_flags sdl_flags = init_flags::video | init_flags::events;
            std::string window_title = "SDL++ Application";
            int window_width = 1280;
            int window_height = 720;
            sdlpp::window_flags window_flags = sdlpp::window_flags::resizable | sdlpp::window_flags::high_pixel_density;
            int vsync = 1; // 1 = enabled, 0 = disabled, -1 = adaptive
            bool auto_create_window = true;
            bool auto_create_renderer = true;
            const char* renderer_driver = nullptr; // nullptr = auto-select
            bool handle_quit_event = true;
        };
        
    private:
        bool running_ = true;
        bool initialized_ = false;
        std::optional<sdlpp::init> sdl_init_;
        config config_;
        
    protected:
        // Common resources
        std::optional<window> main_window_;
        std::optional<renderer> main_renderer_;
        event_queue* event_queue_ = nullptr;
        
        // Timing
        std::chrono::steady_clock::time_point frame_start_;
        std::chrono::steady_clock::time_point last_frame_time_;
        std::chrono::duration<float> delta_time_{0};
        std::chrono::duration<float> total_time_{0};
        
    public:
        application() = default;
        explicit application(config cfg) : config_(std::move(cfg)) {}
        
        // app_interface implementation
        bool init(int argc, char* argv[]) override;
        bool iterate() override;
        bool event(const sdlpp::event& e) override;
        void quit() override;
        
    protected:
        // Hooks for derived classes
        
        /**
         * @brief Parse command line arguments
         * @param argc Argument count
         * @param argv Argument values
         * @return true to continue, false to abort initialization
         */
        virtual bool parse_args([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
            return true;
        }
        
        /**
         * @brief Called after SDL and window/renderer are initialized
         * @return true on success, false to quit
         */
        virtual bool on_init() { return true; }
        
        /**
         * @brief Called once per frame
         */
        virtual void on_frame() {}
        
        /**
         * @brief Handle an event
         * @param e The event
         * @return true to continue, false to quit
         */
        virtual bool on_event([[maybe_unused]] const sdlpp::event& e) { return true; }
        
        /**
         * @brief Called before cleanup
         */
        virtual void on_quit() {}
        
        /**
         * @brief Handle errors
         * @param error Error message
         */
        virtual void on_error(const std::string& error) {
            std::cerr << "Application error: " << error << std::endl;
        }
        
    public:
        // Utility methods
        
        /**
         * @brief Request the application to quit
         */
        void request_quit() { running_ = false; }
        
        /**
         * @brief Check if the application is running
         */
        bool is_running() const { return running_ && initialized_; }
        
        /**
         * @brief Get the main window
         * @throws std::runtime_error if no window exists
         */
        window& get_window() { 
            if (!main_window_) throw std::runtime_error("No window created");
            return *main_window_; 
        }
        
        const window& get_window() const { 
            if (!main_window_) throw std::runtime_error("No window created");
            return *main_window_; 
        }
        
        /**
         * @brief Get the main renderer
         * @throws std::runtime_error if no renderer exists
         */
        renderer& get_renderer() { 
            if (!main_renderer_) throw std::runtime_error("No renderer created");
            return *main_renderer_; 
        }
        
        const renderer& get_renderer() const { 
            if (!main_renderer_) throw std::runtime_error("No renderer created");
            return *main_renderer_; 
        }
        
        /**
         * @brief Get the configuration
         */
        const config& get_config() const { return config_; }
        
        /**
         * @brief Get frame delta time in seconds
         */
        float get_delta_time() const { return delta_time_.count(); }
        
        /**
         * @brief Get total elapsed time in seconds
         */
        float get_total_time() const { return total_time_.count(); }
        
        /**
         * @brief Get current FPS
         */
        float get_fps() const { 
            return delta_time_.count() > 0 ? 1.0f / delta_time_.count() : 0.0f; 
        }
    };
    
} // namespace sdlpp

#endif // SDLPP_APP_APP_HH