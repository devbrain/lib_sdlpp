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

#ifndef SDLPP_APP_STATEFUL_APP_HH
#define SDLPP_APP_STATEFUL_APP_HH

#include <sdlpp/app/app.hh>
#include <sdlpp/app/app_impl.hh>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <any>

namespace sdlpp {
    
    /**
     * @brief Application with managed state
     * @tparam StateType The type of state to manage
     * 
     * This class extends the basic application with state management
     * capabilities, making it easier to organize application data.
     */
    template<typename StateType>
    class stateful_application : public application {
    protected:
        StateType state_;
        
    public:
        stateful_application() = default;
        
        explicit stateful_application(StateType initial_state) 
            : state_(std::move(initial_state)) {}
        
        stateful_application(application::config cfg, StateType initial_state)
            : application(std::move(cfg)), state_(std::move(initial_state)) {}
        
        /**
         * @brief Get mutable reference to state
         */
        StateType& state() { return state_; }
        
        /**
         * @brief Get const reference to state
         */
        const StateType& state() const { return state_; }
        
        /**
         * @brief Replace the entire state
         */
        void set_state(StateType new_state) { 
            state_ = std::move(new_state); 
        }
    };
    
    /**
     * @brief Base class for state machines
     */
    class state_base {
    public:
        virtual ~state_base() = default;
        
        /**
         * @brief Called when entering this state
         */
        virtual void enter() {}
        
        /**
         * @brief Called when exiting this state
         */
        virtual void exit() {}
        
        /**
         * @brief Update the state
         * @param dt Delta time in seconds
         */
        virtual void update(float dt) {}
        
        /**
         * @brief Render the state
         * @param r Renderer to use
         */
        virtual void render(renderer& r) {}
        
        /**
         * @brief Handle an event
         * @param e The event
         * @return true to continue, false to stop event propagation
         */
        virtual bool handle_event(const event& e) { return true; }
    };
    
    /**
     * @brief Application with state machine
     * 
     * This class provides a state machine where different states
     * can be active and transitioned between.
     */
    class state_machine_application : public application {
    protected:
        std::unordered_map<std::type_index, std::unique_ptr<state_base>> states_;
        state_base* current_state_ = nullptr;
        state_base* next_state_ = nullptr;
        
    public:
        using application::application;
        
        /**
         * @brief Register a state
         * @tparam StateType Type of the state
         * @tparam Args Constructor argument types
         * @param args Constructor arguments
         */
        template<typename StateType, typename... Args>
        void register_state(Args&&... args) {
            static_assert(std::is_base_of_v<state_base, StateType>, 
                         "State must derive from state_base");
            states_[std::type_index(typeid(StateType))] = 
                std::make_unique<StateType>(std::forward<Args>(args)...);
        }
        
        /**
         * @brief Transition to a state
         * @tparam StateType Type of state to transition to
         */
        template<typename StateType>
        void transition_to() {
            auto it = states_.find(std::type_index(typeid(StateType)));
            if (it != states_.end()) {
                next_state_ = it->second.get();
            }
        }
        
        /**
         * @brief Get a state by type
         * @tparam StateType Type of state to get
         * @return Pointer to state or nullptr if not found
         */
        template<typename StateType>
        StateType* get_state() {
            auto it = states_.find(std::type_index(typeid(StateType)));
            if (it != states_.end()) {
                return static_cast<StateType*>(it->second.get());
            }
            return nullptr;
        }
        
    protected:
        void on_frame() override {
            // Handle state transitions
            if (next_state_ && next_state_ != current_state_) {
                if (current_state_) {
                    current_state_->exit();
                }
                current_state_ = next_state_;
                current_state_->enter();
                next_state_ = nullptr;
            }
            
            // Update current state
            if (current_state_) {
                current_state_->update(get_delta_time());
                if (main_renderer_) {
                    current_state_->render(*main_renderer_);
                }
            }
        }
        
        bool on_event(const event& e) override {
            if (current_state_) {
                return current_state_->handle_event(e);
            }
            return true;
        }
    };
    
    /**
     * @brief Simple key-value store for application data
     */
    class app_data_store {
        std::unordered_map<std::string, std::any> data_;
        
    public:
        /**
         * @brief Store a value
         * @tparam T Type of value
         * @param key Key to store under
         * @param value Value to store
         */
        template<typename T>
        void set(const std::string& key, T value) {
            data_[key] = std::move(value);
        }
        
        /**
         * @brief Get a value
         * @tparam T Type of value
         * @param key Key to retrieve
         * @return Pointer to value or nullptr if not found/wrong type
         */
        template<typename T>
        T* get(const std::string& key) {
            auto it = data_.find(key);
            if (it != data_.end()) {
                return std::any_cast<T>(&it->second);
            }
            return nullptr;
        }
        
        /**
         * @brief Get a value with default
         * @tparam T Type of value
         * @param key Key to retrieve
         * @param default_value Default if not found
         * @return Value or default
         */
        template<typename T>
        T get_or(const std::string& key, const T& default_value) {
            if (auto* value = get<T>(key)) {
                return *value;
            }
            return default_value;
        }
        
        /**
         * @brief Check if a key exists
         */
        bool has(const std::string& key) const {
            return data_.find(key) != data_.end();
        }
        
        /**
         * @brief Remove a key
         */
        void remove(const std::string& key) {
            data_.erase(key);
        }
        
        /**
         * @brief Clear all data
         */
        void clear() {
            data_.clear();
        }
    };
    
    /**
     * @brief Application with global data store
     * 
     * Provides a simple key-value store for application-wide data.
     */
    class data_store_application : public application {
    protected:
        app_data_store data_store_;
        
    public:
        using application::application;
        
        /**
         * @brief Get the data store
         */
        app_data_store& data() { return data_store_; }
        const app_data_store& data() const { return data_store_; }
    };
    
} // namespace sdlpp

#endif // SDLPP_APP_STATEFUL_APP_HH