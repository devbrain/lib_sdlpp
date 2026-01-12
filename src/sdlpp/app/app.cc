//
// Created by igor on 12/01/2026.
//
#include <sdlpp/app/app.hh>

namespace sdlpp {
    abstract_application::abstract_application(init_flags flags): init_flags_(flags) {}

    void abstract_application::on_init([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {}

    void abstract_application::on_event(const event& e) {
        // Dispatch lifecycle events to specific callbacks
        switch (e.type()) {
            case event_type::quit:
                on_quit_requested();
                break;
            case event_type::terminating:
                on_terminating();
                break;
            case event_type::low_memory:
                on_low_memory();
                break;
            case event_type::will_enter_background:
                on_will_enter_background();
                break;
            case event_type::did_enter_background:
                on_did_enter_background();
                break;
            case event_type::will_enter_foreground:
                on_will_enter_foreground();
                break;
            case event_type::did_enter_foreground:
                on_did_enter_foreground();
                break;
            default:
                break;
        }
        handle_event(e);
    }

    void abstract_application::handle_event([[maybe_unused]] const event& e) {}

    void abstract_application::on_quit_requested() { quit(); }

    void abstract_application::on_terminating() {}

    void abstract_application::on_low_memory() {}

    void abstract_application::on_will_enter_background() {}

    void abstract_application::on_did_enter_background() {}

    void abstract_application::on_will_enter_foreground() {}

    void abstract_application::on_did_enter_foreground() {}

    void abstract_application::on_iterate() {}

    void abstract_application::on_quit() noexcept {}

    void abstract_application::quit() noexcept {
        running_.store(false, std::memory_order_release);
    }

    bool abstract_application::is_running() const noexcept {
        return running_.load(std::memory_order_acquire);
    }

    void abstract_application::init_sdl_() { sdl_init_.emplace(init_flags_); }

    void abstract_application::shutdown_sdl_() noexcept { sdl_init_.reset(); }
}
