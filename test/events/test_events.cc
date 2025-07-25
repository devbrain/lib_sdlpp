#include <doctest/doctest.h>
#include <sdlpp/events/events.hh>
#include <sdlpp/core/core.hh>
#include <cstring>

TEST_SUITE("events") {
    TEST_CASE("event_type_enum") {
        // Test event type values match SDL
        CHECK(static_cast<Uint32>(sdlpp::event_type::quit) == SDL_EVENT_QUIT);
        CHECK(static_cast<Uint32>(sdlpp::event_type::key_down) == SDL_EVENT_KEY_DOWN);
        CHECK(static_cast<Uint32>(sdlpp::event_type::mouse_motion) == SDL_EVENT_MOUSE_MOTION);
        CHECK(static_cast<Uint32>(sdlpp::event_type::window_shown) == SDL_EVENT_WINDOW_SHOWN);
    }
    
    TEST_CASE("keyboard_codes") {
        // Test scancode enum
        CHECK(static_cast<int>(sdlpp::scancode::a) == SDL_SCANCODE_A);
        CHECK(static_cast<int>(sdlpp::scancode::escape) == SDL_SCANCODE_ESCAPE);
        CHECK(static_cast<int>(sdlpp::scancode::space) == SDL_SCANCODE_SPACE);
        
        // Test keycode constants
        CHECK(sdlpp::keycodes::a == SDLK_A);
        CHECK(sdlpp::keycodes::escape == SDLK_ESCAPE);
        CHECK(sdlpp::keycodes::space == SDLK_SPACE);
        
        // Test keymod operations
        auto mod = sdlpp::keymod::shift | sdlpp::keymod::ctrl;
        CHECK(sdlpp::has_keymod(mod, sdlpp::keymod::shift));
        CHECK(sdlpp::has_keymod(mod, sdlpp::keymod::ctrl));
        CHECK(!sdlpp::has_keymod(mod, sdlpp::keymod::alt));
    }
    
    TEST_CASE("mouse_codes") {
        // Test mouse button enum
        CHECK(static_cast<Uint8>(sdlpp::mouse_button::left) == SDL_BUTTON_LEFT);
        CHECK(static_cast<Uint8>(sdlpp::mouse_button::right) == SDL_BUTTON_RIGHT);
        CHECK(static_cast<Uint8>(sdlpp::mouse_button::middle) == SDL_BUTTON_MIDDLE);
        
        // Test button mask operations
        auto mask = sdlpp::mouse_button_mask::left | sdlpp::mouse_button_mask::right;
        CHECK(sdlpp::has_button(mask, sdlpp::mouse_button_mask::left));
        CHECK(sdlpp::has_button(mask, sdlpp::mouse_button_mask::right));
        CHECK(!sdlpp::has_button(mask, sdlpp::mouse_button_mask::middle));
    }
    
    TEST_CASE("event_construction") {
        SDL_Event raw;
        std::memset(&raw, 0, sizeof(raw));
        raw.type = SDL_EVENT_QUIT;
        raw.quit.type = SDL_EVENT_QUIT;
        raw.quit.timestamp = 12345;
        
        sdlpp::event event(raw);
        CHECK(event.type() == sdlpp::event_type::quit);
        CHECK(event.timestamp() == 12345);
    }
    
    TEST_CASE("event_type_checking") {
        SDL_Event raw;
        std::memset(&raw, 0, sizeof(raw));
        
        SUBCASE("keyboard event") {
            raw.type = SDL_EVENT_KEY_DOWN;
            raw.key.type = SDL_EVENT_KEY_DOWN;
            raw.key.key = SDLK_ESCAPE;
            raw.key.scancode = SDL_SCANCODE_ESCAPE;
            raw.key.down = true;
            raw.key.repeat = false;
            
            sdlpp::event event(raw);
            CHECK(event.is<sdlpp::keyboard_event>());
            CHECK(!event.is<sdlpp::mouse_button_event>());
            CHECK(!event.is<sdlpp::quit_event>());
            
            auto* kb = event.as<sdlpp::keyboard_event>();
            REQUIRE(kb != nullptr);
            CHECK(kb->is_pressed());
            CHECK(!kb->is_released());
            CHECK(kb->get_scancode() == sdlpp::scancode::escape);
            CHECK(kb->get_keycode() == sdlpp::keycodes::escape);
        }
        
        SUBCASE("mouse button event") {
            raw.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
            raw.button.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
            raw.button.button = SDL_BUTTON_LEFT;
            raw.button.x = 100.0f;
            raw.button.y = 200.0f;
            raw.button.clicks = 2;
            
            sdlpp::event event(raw);
            CHECK(event.is<sdlpp::mouse_button_event>());
            CHECK(!event.is<sdlpp::keyboard_event>());
            
            auto* mb = event.as<sdlpp::mouse_button_event>();
            REQUIRE(mb != nullptr);
            CHECK(mb->is_pressed());
            CHECK(mb->is_double_click());
            CHECK(mb->get_button() == sdlpp::mouse_button::left);
            CHECK(mb->x == 100.0f);
            CHECK(mb->y == 200.0f);
        }
        
        SUBCASE("window event") {
            raw.type = SDL_EVENT_WINDOW_RESIZED;
            raw.window.type = SDL_EVENT_WINDOW_RESIZED;
            raw.window.windowID = 1;
            raw.window.data1 = 800;
            raw.window.data2 = 600;
            
            sdlpp::event event(raw);
            CHECK(event.is<sdlpp::window_event>());
            
            auto* win = event.as<sdlpp::window_event>();
            REQUIRE(win != nullptr);
            CHECK(win->is_resized());
            CHECK(win->width() == 800);
            CHECK(win->height() == 600);
        }
    }
    
    TEST_CASE("event_handle_method") {
        SDL_Event raw;
        std::memset(&raw, 0, sizeof(raw));
        raw.type = SDL_EVENT_QUIT;
        raw.quit.type = SDL_EVENT_QUIT;
        
        sdlpp::event event(raw);
        
        bool handled = false;
        bool wrong_handled = false;
        
        event.handle<sdlpp::quit_event>([&](const sdlpp::quit_event&) {
            handled = true;
        });
        
        event.handle<sdlpp::keyboard_event>([&](const sdlpp::keyboard_event&) {
            wrong_handled = true;
        });
        
        CHECK(handled);
        CHECK(!wrong_handled);
    }
    
    TEST_CASE("event_direct_access") {
        SDL_Event raw;
        std::memset(&raw, 0, sizeof(raw));
        raw.type = SDL_EVENT_KEY_DOWN;
        raw.key.type = SDL_EVENT_KEY_DOWN;
        raw.key.key = SDLK_SPACE;
        
        sdlpp::event event(raw);
        
        // Direct access (user must check type)
        CHECK(event.key().key == SDLK_SPACE);
    }
    
    TEST_CASE("event_queue_operations") {
        auto init = sdlpp::init(sdlpp::init_flags::events);
        REQUIRE(init.was_init(sdlpp::init_flags::events));
        
        auto& queue = sdlpp::get_event_queue();
        
        SUBCASE("pump and poll") {
            queue.pump();
            
            // Poll should return nullopt when no events
            auto event = queue.poll();
            CHECK(!event.has_value());
        }
        
        SUBCASE("push and poll") {
            SDL_Event raw;
        std::memset(&raw, 0, sizeof(raw));
            raw.type = SDL_EVENT_USER;
            raw.user.type = SDL_EVENT_USER;
            raw.user.code = 42;
            
            sdlpp::event push_event(raw);
            auto result = queue.push(push_event);
            CHECK(result.has_value());
            
            auto event = queue.poll();
            CHECK(event.has_value());
            if (event) {
                CHECK(event->type() == sdlpp::event_type::user);
                auto* user = event->as<sdlpp::user_event>();
                CHECK(user != nullptr);
                if (user) {
                    CHECK(user->code == 42);
                }
            }
        }
        
        SUBCASE("flush events") {
            // Push some events
            SDL_Event raw;
        std::memset(&raw, 0, sizeof(raw));
            raw.type = SDL_EVENT_USER;
            raw.user.type = SDL_EVENT_USER;
            
            for (int i = 0; i < 5; ++i) {
                raw.user.code = i;
                [[maybe_unused]] auto result = queue.push(sdlpp::event(raw));
            }
            
            // Flush user events
            queue.flush(sdlpp::event_type::user);
            
            // Should have no user events
            CHECK(!queue.has_event(sdlpp::event_type::user));
        }
    }
    
    TEST_CASE("event_registry") {
        auto result = sdlpp::event_registry::register_events(5);
        CHECK(result.has_value());
        
        if (result) {
            Uint32 base = *result;
            CHECK(sdlpp::event_registry::is_custom(base));
            CHECK(sdlpp::event_registry::is_custom(base + 1));
            CHECK(sdlpp::event_registry::is_custom(base + 4));
            CHECK(!sdlpp::event_registry::is_custom(SDL_EVENT_QUIT));
        }
    }
    
    TEST_CASE("event_variant_support") {
        SDL_Event raw;
        std::memset(&raw, 0, sizeof(raw));
        raw.type = SDL_EVENT_KEY_DOWN;
        raw.key.type = SDL_EVENT_KEY_DOWN;
        raw.key.key = SDLK_RETURN;
        raw.key.down = true;
        
        sdlpp::event event(raw);
        
        // Test variant access
        bool visited = false;
        event.visit([&](const auto& e) {
            using T = std::decay_t<decltype(e)>;
            if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
                visited = true;
                CHECK(e.get_keycode() == sdlpp::keycodes::return_key);
            }
        });
        
        CHECK(visited);
    }
    
    TEST_CASE("text_input_event") {
        SDL_Event raw;
        std::memset(&raw, 0, sizeof(raw));
        raw.type = SDL_EVENT_TEXT_INPUT;
        raw.text.type = SDL_EVENT_TEXT_INPUT;
        raw.text.text = "Hello";
        
        sdlpp::event event(raw);
        CHECK(event.is<sdlpp::text_input_event>());
        
        auto* text = event.as<sdlpp::text_input_event>();
        REQUIRE(text != nullptr);
        CHECK(text->get_text() == "Hello");
    }
}