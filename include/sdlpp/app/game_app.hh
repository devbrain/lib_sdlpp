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

#ifndef SDLPP_APP_GAME_APP_HH
#define SDLPP_APP_GAME_APP_HH

#include <sdlpp/app/app.hh>
#include <sdlpp/app/app_impl.hh>
#include <sdlpp/core/timer.hh>
#include <chrono>
#include <deque>
#include <numeric>

namespace sdlpp {
    
    /**
     * @brief Game application with fixed timestep
     * 
     * This class implements a fixed timestep game loop, which provides
     * consistent physics simulation independent of rendering framerate.
     * 
     * Based on the article "Fix Your Timestep!" by Glenn Fiedler
     */
    class game_application : public application {
    public:
        struct game_config : application::config {
            // Fixed update rate (Hz)
            float fixed_update_rate = 60.0f;
            
            // Maximum updates per frame (to prevent spiral of death)
            int max_updates_per_frame = 5;
            
            // Enable interpolation for smooth rendering
            bool enable_interpolation = true;
            
            // Enable frame time smoothing
            bool enable_frame_smoothing = true;
            int frame_smooth_samples = 10;
        };
        
    protected:
        game_config game_config_;
        std::chrono::duration<float> fixed_timestep_;
        std::chrono::duration<float> accumulator_{0};
        
        // Frame time smoothing
        std::deque<float> frame_times_;
        float smoothed_delta_time_ = 0.0f;
        
        // Statistics
        int fixed_updates_this_frame_ = 0;
        int total_fixed_updates_ = 0;
        float interpolation_alpha_ = 0.0f;
        
    public:
        game_application() : game_application(game_config{}) {}
        
        explicit game_application(game_config cfg) 
            : application(cfg)
            , game_config_(std::move(cfg)) {
            fixed_timestep_ = std::chrono::duration<float>(1.0f / game_config_.fixed_update_rate);
        }
        
    protected:
        void on_frame() override {
            // Get raw frame time
            float raw_delta = get_delta_time();
            
            // Apply frame time smoothing if enabled
            float frame_time = raw_delta;
            if (game_config_.enable_frame_smoothing) {
                frame_times_.push_back(raw_delta);
                if (frame_times_.size() > static_cast<size_t>(game_config_.frame_smooth_samples)) {
                    frame_times_.pop_front();
                }
                
                smoothed_delta_time_ = std::accumulate(
                    frame_times_.begin(), frame_times_.end(), 0.0f
                ) / static_cast<float>(frame_times_.size());
                
                frame_time = smoothed_delta_time_;
            }
            
            // Clamp frame time to prevent spiral of death
            frame_time = std::min(frame_time, 
                fixed_timestep_.count() * static_cast<float>(game_config_.max_updates_per_frame));
            
            // Add to accumulator
            accumulator_ += std::chrono::duration<float>(frame_time);
            
            // Fixed timestep updates
            fixed_updates_this_frame_ = 0;
            while (accumulator_ >= fixed_timestep_ && 
                   fixed_updates_this_frame_ < game_config_.max_updates_per_frame) {
                fixed_update(fixed_timestep_.count());
                accumulator_ -= fixed_timestep_;
                fixed_updates_this_frame_++;
                total_fixed_updates_++;
            }
            
            // Calculate interpolation alpha
            interpolation_alpha_ = game_config_.enable_interpolation ? 
                accumulator_.count() / fixed_timestep_.count() : 1.0f;
            
            // Render with interpolation
            render(interpolation_alpha_);
        }
        
        /**
         * @brief Called at fixed timestep for physics/logic
         * @param dt Fixed delta time (always the same)
         */
        virtual void fixed_update([[maybe_unused]] float dt) {}
        
        /**
         * @brief Called once per frame for rendering
         * @param alpha Interpolation factor [0,1] for smooth rendering
         */
        virtual void render([[maybe_unused]] float alpha) {}
        
    public:
        /**
         * @brief Get the fixed timestep duration
         */
        float get_fixed_timestep() const { 
            return fixed_timestep_.count(); 
        }
        
        /**
         * @brief Get number of fixed updates in the last frame
         */
        int get_fixed_updates_per_frame() const { 
            return fixed_updates_this_frame_; 
        }
        
        /**
         * @brief Get total number of fixed updates
         */
        int get_total_fixed_updates() const { 
            return total_fixed_updates_; 
        }
        
        /**
         * @brief Get current interpolation alpha
         */
        float get_interpolation_alpha() const { 
            return interpolation_alpha_; 
        }
        
        /**
         * @brief Get smoothed frame time
         */
        float get_smoothed_delta_time() const { 
            return smoothed_delta_time_; 
        }
    };
    
    /**
     * @brief Game application with variable timestep
     * 
     * For games that don't need fixed timestep physics,
     * this provides a simpler variable timestep loop.
     */
    class variable_timestep_game : public application {
    protected:
        float time_scale_ = 1.0f;
        float max_delta_time_ = 1.0f / 30.0f; // Cap at 30 FPS minimum
        
    public:
        using application::application;
        
    protected:
        void on_frame() override {
            // Get clamped delta time
            float dt = std::min(get_delta_time(), max_delta_time_) * time_scale_;
            
            // Update and render
            update(dt);
            render();
        }
        
        /**
         * @brief Update the game
         * @param dt Delta time (variable, scaled)
         */
        virtual void update([[maybe_unused]] float dt) {}
        
        /**
         * @brief Render the game
         */
        virtual void render() {}
        
    public:
        /**
         * @brief Set time scale (for slow motion, etc.)
         */
        void set_time_scale(float scale) { 
            time_scale_ = scale; 
        }
        
        /**
         * @brief Get current time scale
         */
        float get_time_scale() const { 
            return time_scale_; 
        }
        
        /**
         * @brief Set maximum delta time
         */
        void set_max_delta_time(float max_dt) { 
            max_delta_time_ = max_dt; 
        }
    };
    
    /**
     * @brief Performance monitoring for games
     */
    class performance_monitor {
        struct frame_stats {
            float frame_time = 0.0f;
            float update_time = 0.0f;
            float render_time = 0.0f;
            int draw_calls = 0;
        };
        
        std::deque<frame_stats> history_;
        size_t max_history_ = 120; // 2 seconds at 60 FPS
        frame_stats current_frame_;
        
        std::chrono::steady_clock::time_point frame_start_;
        std::chrono::steady_clock::time_point update_start_;
        std::chrono::steady_clock::time_point render_start_;
        
    public:
        void begin_frame() {
            frame_start_ = std::chrono::steady_clock::now();
            current_frame_ = {};
        }
        
        void begin_update() {
            update_start_ = std::chrono::steady_clock::now();
        }
        
        void end_update() {
            auto now = std::chrono::steady_clock::now();
            current_frame_.update_time = 
                std::chrono::duration<float, std::milli>(now - update_start_).count();
        }
        
        void begin_render() {
            render_start_ = std::chrono::steady_clock::now();
        }
        
        void end_render() {
            auto now = std::chrono::steady_clock::now();
            current_frame_.render_time = 
                std::chrono::duration<float, std::milli>(now - render_start_).count();
        }
        
        void end_frame() {
            auto now = std::chrono::steady_clock::now();
            current_frame_.frame_time = 
                std::chrono::duration<float, std::milli>(now - frame_start_).count();
            
            history_.push_back(current_frame_);
            if (history_.size() > max_history_) {
                history_.pop_front();
            }
        }
        
        void increment_draw_calls() {
            current_frame_.draw_calls++;
        }
        
        // Statistics
        float get_average_fps() const {
            if (history_.empty()) return 0.0f;
            
            float avg_frame_time = 0.0f;
            for (const auto& frame : history_) {
                avg_frame_time += frame.frame_time;
            }
            avg_frame_time /= static_cast<float>(history_.size());
            
            return avg_frame_time > 0 ? 1000.0f / avg_frame_time : 0.0f;
        }
        
        float get_average_frame_time() const {
            if (history_.empty()) return 0.0f;
            
            float sum = 0.0f;
            for (const auto& frame : history_) {
                sum += frame.frame_time;
            }
            return sum / static_cast<float>(history_.size());
        }
        
        std::pair<float, float> get_min_max_frame_time() const {
            if (history_.empty()) return {0.0f, 0.0f};
            
            float min_time = std::numeric_limits<float>::max();
            float max_time = 0.0f;
            
            for (const auto& frame : history_) {
                min_time = std::min(min_time, frame.frame_time);
                max_time = std::max(max_time, frame.frame_time);
            }
            
            return {min_time, max_time};
        }
    };
    
    /**
     * @brief Game application with built-in performance monitoring
     */
    template<typename BaseApp = game_application>
    class monitored_game_application : public BaseApp {
    protected:
        performance_monitor monitor_;
        bool monitoring_enabled_ = true;
        
    public:
        using BaseApp::BaseApp;
        
    protected:
        void on_frame() override {
            if (monitoring_enabled_) {
                monitor_.begin_frame();
                monitor_.begin_update();
            }
            
            // Let base class handle the frame
            BaseApp::on_frame();
            
            if (monitoring_enabled_) {
                monitor_.end_update();
                monitor_.end_frame();
            }
        }
        
        void render(float alpha) override {
            if (monitoring_enabled_) {
                monitor_.begin_render();
            }
            
            BaseApp::render(alpha);
            
            if (monitoring_enabled_) {
                monitor_.end_render();
            }
        }
        
    public:
        performance_monitor& get_monitor() { return monitor_; }
        const performance_monitor& get_monitor() const { return monitor_; }
        
        void set_monitoring_enabled(bool enabled) { 
            monitoring_enabled_ = enabled; 
        }
    };
    
} // namespace sdlpp

#endif // SDLPP_APP_GAME_APP_HH