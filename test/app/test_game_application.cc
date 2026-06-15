#include <doctest/doctest.h>

#include <sdlpp/app/game_application.hh>

namespace {
    class input_probe final : public sdlpp::game_application {
        public:
            using sdlpp::game_application::get_key;
            using sdlpp::game_application::get_mouse;
            using sdlpp::game_application::get_mouse_pos;
            using sdlpp::game_application::get_mouse_wheel;
            using sdlpp::game_application::get_mouse_x;
            using sdlpp::game_application::get_mouse_y;
            using sdlpp::game_application::get_window_mouse;
            using sdlpp::game_application::is_focused;

        protected:
            sdlpp::window_config get_window_config() override {
                return {"input probe", 64, 64, sdlpp::window_flags::none, 60};
            }
    };
}

TEST_SUITE("game_application") {
    TEST_CASE("button_state defaults") {
        const sdlpp::button_state state;

        CHECK_FALSE(state.pressed);
        CHECK_FALSE(state.released);
        CHECK_FALSE(state.held);
        CHECK_FALSE(static_cast<bool>(state));
    }

    TEST_CASE("input accessors are available before input") {
        input_probe app;

        CHECK(app.is_focused());
        CHECK_FALSE(app.get_key(sdlpp::scancode::space).pressed);
        CHECK_FALSE(app.get_key(sdlpp::scancode::space).released);
        CHECK_FALSE(app.get_key(sdlpp::scancode::space).held);

        CHECK_FALSE(app.get_mouse(sdlpp::mouse_button::left).held);
        CHECK_FALSE(app.get_mouse(0).held);
        CHECK_FALSE(app.get_mouse(99).held);

        CHECK(app.get_mouse_x() == 0);
        CHECK(app.get_mouse_y() == 0);
        CHECK(app.get_mouse_pos() == sdlpp::point_i{0, 0});
        CHECK(app.get_window_mouse() == sdlpp::point_i{0, 0});
        CHECK(app.get_mouse_wheel() == 0);
    }
}
