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

#ifndef SDLPP_APP_SCENE_APP_HH
#define SDLPP_APP_SCENE_APP_HH

#include <sdlpp/app/app.hh>
#include <sdlpp/app/app_impl.hh>
#include <memory>
#include <stack>
#include <unordered_map>
#include <typeindex>

namespace sdlpp {
    
    // Forward declaration
    class scene_application;
    
    /**
     * @brief Base class for scenes
     * 
     * A scene represents a distinct screen or state in your application,
     * such as a menu, gameplay level, or settings screen.
     */
    class scene {
        friend class scene_application;
        
    protected:
        scene_application* app_ = nullptr;
        
    public:
        virtual ~scene() = default;
        
        /**
         * @brief Called when the scene becomes active
         */
        virtual void on_enter() {}
        
        /**
         * @brief Called when the scene becomes inactive
         */
        virtual void on_exit() {}
        
        /**
         * @brief Called when the scene is paused (another scene pushed on top)
         */
        virtual void on_pause() {}
        
        /**
         * @brief Called when the scene is resumed (top scene was popped)
         */
        virtual void on_resume() {}
        
        /**
         * @brief Update the scene
         * @param dt Delta time in seconds
         */
        virtual void update([[maybe_unused]] float dt) {}
        
        /**
         * @brief Render the scene
         * @param r Renderer to use
         */
        virtual void render(renderer& r) = 0;
        
        /**
         * @brief Handle an event
         * @param e The event
         * @return true to continue, false to stop event propagation
         */
        virtual bool handle_event([[maybe_unused]] const event& e) { return true; }
        
        /**
         * @brief Check if this scene should render scenes below it
         * @return true if transparent/overlay scene
         */
        virtual bool is_transparent() const { return false; }
        
    protected:
        /**
         * @brief Get the application
         */
        scene_application& app() { 
            if (!app_) throw std::runtime_error("Scene not attached to application");
            return *app_; 
        }
    };
    
    /**
     * @brief Application with scene management
     * 
     * This class provides a scene stack where scenes can be pushed,
     * popped, and replaced. Useful for games and menu-driven applications.
     */
    class scene_application : public application {
    protected:
        std::stack<std::unique_ptr<scene>> scene_stack_;
        std::unique_ptr<scene> pending_scene_;
        enum class scene_action { none, push, replace, pop };
        scene_action pending_action_ = scene_action::none;
        
    public:
        using application::application;
        
        /**
         * @brief Push a new scene onto the stack
         * @tparam SceneType Type of scene to create
         * @tparam Args Constructor argument types
         * @param args Constructor arguments
         */
        template<typename SceneType, typename... Args>
        void push_scene(Args&&... args) {
            static_assert(std::is_base_of_v<scene, SceneType>, 
                         "SceneType must derive from scene");
            auto new_scene = std::make_unique<SceneType>(std::forward<Args>(args)...);
            new_scene->app_ = this;
            pending_scene_ = std::move(new_scene);
            pending_action_ = scene_action::push;
        }
        
        /**
         * @brief Replace the current scene
         * @tparam SceneType Type of scene to create
         * @tparam Args Constructor argument types
         * @param args Constructor arguments
         */
        template<typename SceneType, typename... Args>
        void replace_scene(Args&&... args) {
            static_assert(std::is_base_of_v<scene, SceneType>, 
                         "SceneType must derive from scene");
            auto new_scene = std::make_unique<SceneType>(std::forward<Args>(args)...);
            new_scene->app_ = this;
            pending_scene_ = std::move(new_scene);
            pending_action_ = scene_action::replace;
        }
        
        /**
         * @brief Pop the current scene
         */
        void pop_scene() {
            pending_action_ = scene_action::pop;
        }
        
        /**
         * @brief Clear all scenes
         */
        void clear_scenes() {
            while (!scene_stack_.empty()) {
                scene_stack_.top()->on_exit();
                scene_stack_.pop();
            }
        }
        
        /**
         * @brief Get the current scene
         * @return Current scene or nullptr
         */
        scene* current_scene() {
            return scene_stack_.empty() ? nullptr : scene_stack_.top().get();
        }
        
        /**
         * @brief Get the current scene as a specific type
         * @tparam SceneType Type to cast to
         * @return Scene pointer or nullptr
         */
        template<typename SceneType>
        SceneType* current_scene_as() {
            if (auto* s = current_scene()) {
                return dynamic_cast<SceneType*>(s);
            }
            return nullptr;
        }
        
    protected:
        void on_frame() override {
            // Handle pending scene changes
            handle_scene_transitions();
            
            // Find all scenes to render (for transparent scenes)
            std::vector<scene*> scenes_to_render;
            if (!scene_stack_.empty()) {
                // Build from top down, stopping at first non-transparent scene
                std::stack<std::unique_ptr<scene>> temp_stack;
                std::swap(temp_stack, scene_stack_);
                
                while (!temp_stack.empty()) {
                    auto& current = temp_stack.top();
                    scenes_to_render.push_back(current.get());
                    scene_stack_.push(std::move(current));
                    temp_stack.pop();
                    
                    if (!scenes_to_render.back()->is_transparent()) {
                        // Move remaining scenes back
                        while (!temp_stack.empty()) {
                            scene_stack_.push(std::move(temp_stack.top()));
                            temp_stack.pop();
                        }
                        break;
                    }
                }
            }
            
            // Update top scene
            if (!scene_stack_.empty()) {
                scene_stack_.top()->update(get_delta_time());
            }
            
            // Render scenes bottom to top
            if (main_renderer_) {
                for (auto it = scenes_to_render.rbegin(); it != scenes_to_render.rend(); ++it) {
                    (*it)->render(*main_renderer_);
                }
            }
        }
        
        bool on_event(const sdlpp::event& e) override {
            if (!scene_stack_.empty()) {
                return scene_stack_.top()->handle_event(e);
            }
            return true;
        }
        
        void on_quit() override {
            clear_scenes();
        }
        
    private:
        void handle_scene_transitions() {
            switch (pending_action_) {
                case scene_action::push:
                    if (pending_scene_) {
                        if (!scene_stack_.empty()) {
                            scene_stack_.top()->on_pause();
                        }
                        pending_scene_->on_enter();
                        scene_stack_.push(std::move(pending_scene_));
                    }
                    break;
                    
                case scene_action::replace:
                    if (pending_scene_) {
                        if (!scene_stack_.empty()) {
                            scene_stack_.top()->on_exit();
                            scene_stack_.pop();
                        }
                        pending_scene_->on_enter();
                        scene_stack_.push(std::move(pending_scene_));
                    }
                    break;
                    
                case scene_action::pop:
                    if (!scene_stack_.empty()) {
                        scene_stack_.top()->on_exit();
                        scene_stack_.pop();
                        if (!scene_stack_.empty()) {
                            scene_stack_.top()->on_resume();
                        }
                    }
                    break;
                    
                case scene_action::none:
                    break;
            }
            
            pending_action_ = scene_action::none;
            pending_scene_.reset();
        }
    };
    
    /**
     * @brief Transition effects between scenes
     */
    class scene_transition : public scene {
    protected:
        std::unique_ptr<scene> from_scene_;
        std::unique_ptr<scene> to_scene_;
        float duration_;
        float elapsed_ = 0.0f;
        
    public:
        scene_transition(std::unique_ptr<scene> from, 
                        std::unique_ptr<scene> to, 
                        float duration)
            : from_scene_(std::move(from))
            , to_scene_(std::move(to))
            , duration_(duration) {}
        
        void update(float dt) override {
            elapsed_ += dt;
            if (elapsed_ >= duration_) {
                // Transition complete
                if (to_scene_) {
                    // Replace this transition with the target scene
                    // (Implementation would need scene_application support)
                }
            }
        }
        
        bool is_transparent() const override { return true; }
        
    protected:
        float progress() const { 
            return std::min(1.0f, elapsed_ / duration_); 
        }
    };
    
    /**
     * @brief Fade transition between scenes
     */
    class fade_transition : public scene_transition {
    public:
        using scene_transition::scene_transition;
        
        void render(renderer& r) override {
            // Render the appropriate scene
            if (progress() < 0.5f) {
                if (from_scene_) from_scene_->render(r);
            } else {
                if (to_scene_) to_scene_->render(r);
            }
            
            // Render fade overlay
            auto alpha = static_cast<Uint8>(255 * (1.0f - std::abs(progress() - 0.5f) * 2.0f));
            if (auto res = r.set_draw_color({0, 0, 0, alpha}); !res) {
                // Ignore error
            }
            if (auto res = r.set_draw_blend_mode(blend_mode::blend); !res) {
                // Ignore blend mode error
            }
            if (auto viewport = r.get_viewport()) {
                if (auto res = r.fill_rect(*viewport); !res) {
                    // Ignore error
                }
            }
            if (auto res = r.set_draw_blend_mode(blend_mode::none); !res) {
                // Ignore blend mode error
            }
        }
    };
    
} // namespace sdlpp

#endif // SDLPP_APP_SCENE_APP_HH