//
// Created by igor on 12/01/2026.
//

#include <sdlpp/app/game_application.hh>

namespace sdlpp {
    namespace {
        [[nodiscard]] std::optional<std::size_t> mouse_button_index(mouse_button button) noexcept {
            switch (button) {
                case mouse_button::left:
                    return 0;
                case mouse_button::right:
                    return 1;
                case mouse_button::middle:
                    return 2;
                case mouse_button::x1:
                    return 3;
                case mouse_button::x2:
                    return 4;
            }
            return std::nullopt;
        }

        void press_button(button_state& state) noexcept {
            state.pressed = !state.held;
            state.held = true;
        }

        void release_button(button_state& state) noexcept {
            state.released = state.held;
            state.held = false;
        }
    }

    game_application::game_application() = default;
    game_application::~game_application() = default;

    std::optional<surface> game_application::get_window_icon() { return std::nullopt; }

    void game_application::on_config([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {}

    void game_application::on_ready() {}

    void game_application::on_update([[maybe_unused]] float delta_time) {}

    void game_application::on_render([[maybe_unused]] renderer& r) {}

    void game_application::on_window_shown() {}

    void game_application::on_window_hidden() {}

    void game_application::on_window_exposed() {}

    void game_application::on_window_moved([[maybe_unused]] int x, [[maybe_unused]] int y) {}

    void game_application::on_window_resized([[maybe_unused]] int width, [[maybe_unused]] int height) {}

    void game_application::on_window_minimized() {}

    void game_application::on_window_maximized() {}

    void game_application::on_window_restored() {}

    void game_application::on_window_mouse_enter() {}

    void game_application::on_window_mouse_leave() {}

    void game_application::on_window_focus_gained() {}

    void game_application::on_window_focus_lost() {}

    void game_application::on_window_enter_fullscreen() {}

    void game_application::on_window_leave_fullscreen() {}

    void game_application::on_window_display_scale_changed([[maybe_unused]] float scale) {}

    bool game_application::is_fullscreen() const {
        return window_.is_fullscreen();
    }

    bool game_application::set_fullscreen(bool fullscreen) {
        auto mode = fullscreen ? fullscreen_mode::fullscreen : fullscreen_mode::windowed;
        return window_.set_fullscreen(mode).has_value();
    }

    bool game_application::toggle_fullscreen() {
        return set_fullscreen(!is_fullscreen());
    }

    window& game_application::get_window() { return window_; }

    const window& game_application::get_window() const { return window_; }

    renderer& game_application::get_renderer() { return renderer_; }

    const renderer& game_application::get_renderer() const { return renderer_; }

    float game_application::fps() const { return fps_; }

    float game_application::delta_time() const { return delta_time_; }

    int game_application::target_fps() const { return target_fps_; }

    void game_application::set_target_fps(int fps) {
        target_fps_ = fps;
        if (fps > 0) {
            frame_duration_ = duration(1.0f / static_cast<float>(fps));
        }
    }

    bool game_application::is_focused() const noexcept {
        return focused_;
    }

    button_state game_application::get_key(scancode scan) const noexcept {
        const auto index = static_cast<std::size_t>(scan);
        if (index >= keyboard_state_.size()) {
            return {};
        }
        return keyboard_state_[index];
    }

    button_state game_application::get_mouse(mouse_button button) const noexcept {
        const auto index = mouse_button_index(button);
        if (!index) {
            return {};
        }
        return mouse_state_[*index];
    }

    button_state game_application::get_mouse(std::uint32_t button) const noexcept {
        if (button >= mouse_state_.size()) {
            return {};
        }
        return mouse_state_[button];
    }

    int game_application::get_mouse_x() const noexcept {
        return mouse_position_.x;
    }

    int game_application::get_mouse_y() const noexcept {
        return mouse_position_.y;
    }

    point_i game_application::get_mouse_pos() const noexcept {
        return mouse_position_;
    }

    point_i game_application::get_window_mouse() const noexcept {
        return mouse_position_;
    }

    int game_application::get_mouse_wheel() const noexcept {
        return mouse_wheel_delta_;
    }

    void game_application::on_event(const event& e) {
        update_input_from_event(e);

        // Dispatch window events
        if (e.is<window_event>()) {
            auto* we = e.as<window_event>();
            switch (e.type()) {
                case event_type::window_shown:
                    on_window_shown();
                    break;
                case event_type::window_hidden:
                    on_window_hidden();
                    break;
                case event_type::window_exposed:
                    on_window_exposed();
                    break;
                case event_type::window_moved:
                    on_window_moved(we->x(), we->y());
                    break;
                case event_type::window_resized:
                    on_window_resized(we->width(), we->height());
                    break;
                case event_type::window_minimized:
                    on_window_minimized();
                    break;
                case event_type::window_maximized:
                    on_window_maximized();
                    break;
                case event_type::window_restored:
                    on_window_restored();
                    break;
                case event_type::window_mouse_enter:
                    on_window_mouse_enter();
                    break;
                case event_type::window_mouse_leave:
                    on_window_mouse_leave();
                    break;
                case event_type::window_focus_gained:
                    on_window_focus_gained();
                    break;
                case event_type::window_focus_lost:
                    on_window_focus_lost();
                    break;
                case event_type::window_enter_fullscreen:
                    on_window_enter_fullscreen();
                    break;
                case event_type::window_leave_fullscreen:
                    on_window_leave_fullscreen();
                    break;
                case event_type::window_display_scale_changed:
                    on_window_display_scale_changed(window_.display_scale());
                    break;
                default:
                    break;
            }
        }

        // Call base class to handle lifecycle events and dispatch to handle_event
        abstract_application::on_event(e);
    }

    void game_application::on_init(int argc, char* argv[]) {
        // Configure failsafe to use SDL logging
        failsafe::logger::set_backend(create_failsafe_sdl_backend());

        // Let user handle command line args
        on_config(argc, argv);

        // Get window configuration
        auto config = get_window_config();
        target_fps_ = config.target_fps;
        if (target_fps_ > 0) {
            frame_duration_ = duration(1.0f / static_cast<float>(target_fps_));
        }

        // Create window
        auto window_result = window::create(config.title, config.width, config.height, config.flags);
        if (!window_result) {
            throw std::runtime_error("Failed to create window: " + window_result.error());
        }
        window_ = std::move(window_result.value());
        focused_ = (window_.get_flags() & window_flags::input_focus) != window_flags::none;

        // Set icon if provided
        auto icon = get_window_icon();
        if (icon) {
            window_.set_icon(icon->get());
        }

        // Create renderer
        auto renderer_result = renderer::create(window_);
        if (!renderer_result) {
            throw std::runtime_error("Failed to create renderer: " + renderer_result.error());
        }
        renderer_ = std::move(renderer_result.value());

        // Initialize timing
        last_frame_time_ = clock::now();
        fps_update_time_ = last_frame_time_;
        frame_count_ = 0;

        // Let user do post-init
        on_ready();
    }

    void game_application::on_iterate() {
        auto current_time = clock::now();

        // Calculate delta time
        duration delta = current_time - last_frame_time_;
        delta_time_ = delta.count();

        // Update FPS counter
        frame_count_++;
        duration fps_delta = current_time - fps_update_time_;
        if (fps_delta.count() >= 1.0f) {
            fps_ = static_cast<float>(frame_count_) / fps_delta.count();
            frame_count_ = 0;
            fps_update_time_ = current_time;
        }

        // Call user callbacks
        on_update(delta_time_);
        on_render(renderer_);

        clear_transient_input_state();

        // Enforce FPS limit
        if (target_fps_ > 0) {
            auto frame_end = clock::now();
            duration frame_time = frame_end - current_time;

            if (frame_time < frame_duration_) {
                duration sleep_time = frame_duration_ - frame_time;
                std::this_thread::sleep_for(sleep_time);
            }
        }

        last_frame_time_ = current_time;
    }

    void game_application::update_input_from_event(const event& e) {
        const auto app_window_id = window_.get_id();

        if (const auto* we = e.as<window_event>()) {
            if (we->windowID != 0 && app_window_id != 0 && we->windowID != app_window_id) {
                return;
            }
            if (we->is_focus_gained()) {
                focused_ = true;
            } else if (we->is_focus_lost()) {
                focused_ = false;
                for (auto& state : keyboard_state_) {
                    state.held = false;
                }
                for (auto& state : mouse_state_) {
                    state.held = false;
                }
            }
            return;
        }

        if (const auto* key = e.as<keyboard_event>()) {
            if (key->windowID != 0 && app_window_id != 0 && key->windowID != app_window_id) {
                return;
            }

            const auto index = static_cast<std::size_t>(key->scan);
            if (index >= keyboard_state_.size()) {
                return;
            }

            if (key->is_pressed()) {
                if (!key->is_repeat()) {
                    press_button(keyboard_state_[index]);
                } else {
                    keyboard_state_[index].held = true;
                }
            } else if (key->is_released()) {
                release_button(keyboard_state_[index]);
            }
            return;
        }

        if (const auto* motion = e.as<mouse_motion_event>()) {
            if (motion->windowID != 0 && app_window_id != 0 && motion->windowID != app_window_id) {
                return;
            }

            mouse_position_ = {
                static_cast<int>(motion->x),
                static_cast<int>(motion->y)
            };
            return;
        }

        if (const auto* button = e.as<mouse_button_event>()) {
            if (button->windowID != 0 && app_window_id != 0 && button->windowID != app_window_id) {
                return;
            }

            mouse_position_ = {
                static_cast<int>(button->x),
                static_cast<int>(button->y)
            };

            const auto index = mouse_button_index(button->get_button());
            if (!index) {
                return;
            }

            if (button->is_pressed()) {
                press_button(mouse_state_[*index]);
            } else if (button->is_released()) {
                release_button(mouse_state_[*index]);
            }
            return;
        }

        if (const auto* wheel = e.as<mouse_wheel_event>()) {
            if (wheel->windowID != 0 && app_window_id != 0 && wheel->windowID != app_window_id) {
                return;
            }

            mouse_position_ = {
                static_cast<int>(wheel->mouse_x),
                static_cast<int>(wheel->mouse_y)
            };
            mouse_wheel_delta_ += static_cast<int>(wheel->y);
        }
    }

    void game_application::clear_transient_input_state() noexcept {
        for (auto& state : keyboard_state_) {
            state.pressed = false;
            state.released = false;
        }

        for (auto& state : mouse_state_) {
            state.pressed = false;
            state.released = false;
        }

        mouse_wheel_delta_ = 0;
    }
}
