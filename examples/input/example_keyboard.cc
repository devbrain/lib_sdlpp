#include <sdlpp/core/core.hh>
#include <../include/sdlpp/input/keyboard.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/renderer.hh>
#include <sdlpp/utility/geometry.hh>
#include <../include/sdlpp/core/timer.hh>
#include <../include/sdlpp/core/log.hh>

#include <iostream>
#include <sstream>
#include <iomanip>
#include <array>

using namespace sdlpp;

int main() {
    // Initialize SDL
    auto init = sdlpp::init(init_flags::video | init_flags::events);
    if (!init) {
        logger::error(log_category::application, std::source_location::current(), "Failed to initialize SDL");
        return 1;
    }
    
    // Create window
    auto window_result = window::create("Keyboard Example - Press keys to test", 800, 600);
    if (!window_result) {
        logger::error(log_category::application, std::source_location::current(), "Failed to create window: ", window_result.error());
        return 1;
    }
    auto& win = window_result.value();
    
    // Create renderer
    auto renderer_result = renderer::create(win);
    if (!renderer_result) {
        logger::error(log_category::application, std::source_location::current(), "Failed to create renderer: ", renderer_result.error());
        return 1;
    }
    auto& ren = renderer_result.value();
    
    // Print keyboard information
    std::cout << "\n=== SDL++ Keyboard Example ===\n\n";
    std::cout << "Keyboard Information:\n";
    std::cout << "- Has keyboard: " << (has_keyboard() ? "Yes" : "No") << "\n";
    std::cout << "- Screen keyboard support: " << (has_screen_keyboard_support() ? "Yes" : "No") << "\n";
    
    auto keyboards = get_keyboards();
    std::cout << "- Number of keyboards: " << keyboards.size() << "\n";
    for (size_t i = 0; i < keyboards.size(); ++i) {
        auto name = get_keyboard_name(keyboards[i]);
        std::cout << "  Keyboard " << i << ": " << (name.empty() ? "(unnamed)" : name) 
                  << " (ID: " << keyboards[i] << ")\n";
    }
    std::cout << "\n";
    
    // Test key name conversions
    std::cout << "Sample key name conversions:\n";
    std::cout << "- Keycode 'Space' -> " << get_key_name(keycodes::space) << "\n";
    std::cout << "- Scancode 'A' -> " << get_scancode_name(scancode::a) << "\n";
    std::cout << "- Name 'Escape' -> keycode " << static_cast<int>(get_key_from_name("Escape")) << "\n";
    std::cout << "- Name 'Tab' -> scancode " << static_cast<int>(get_scancode_from_name("Tab")) << "\n";
    std::cout << "\n";
    
    // Instructions
    std::cout << "Instructions:\n";
    std::cout << "- Press any keys to see their names and codes\n";
    std::cout << "- Press 'T' to toggle text input mode\n";
    std::cout << "- Press 'M' to manually set modifier state (Shift+Ctrl)\n";
    std::cout << "- Press 'K' to display keyboard state summary\n";
    std::cout << "- Try shortcuts: Ctrl+C, Ctrl+V, Ctrl+A, Ctrl+S, Ctrl+X, Ctrl+Z\n";
    std::cout << "- Press ESC to quit\n\n";
    
    // Event loop
    event_queue events;
    bool running = true;
    bool text_input_active = false;
    std::optional<text_input_session> text_session;
    std::string text_buffer;
    
    // Keys to monitor for display
    const std::array<std::pair<scancode, const char*>, 8> monitored_keys = {{
        {scancode::w, "W"}, {scancode::a, "A"}, {scancode::s, "S"}, {scancode::d, "D"},
        {scancode::space, "Space"}, {scancode::lshift, "LShift"}, 
        {scancode::lctrl, "LCtrl"}, {scancode::lalt, "LAlt"}
    }};
    
    // Frame limiter
    frame_limiter limiter(60.0);
    
    while (running) {
        // Clear screen
        auto color_result = ren.set_draw_color(color{30, 30, 40, 255});
        if (!color_result) {
            logger::error(log_category::application, std::source_location::current(), "Failed to set color: ", color_result.error());
        }
        auto clear_result = ren.clear();
        if (!clear_result) {
            logger::error(log_category::application, std::source_location::current(), "Failed to clear: ", clear_result.error());
        }
        
        // Poll events
        while (auto event = events.poll()) {
            event->visit([&](auto&& e) {
                using T = std::decay_t<decltype(e)>;
                
                if constexpr (std::is_same_v<T, quit_event>) {
                    running = false;
                }
                else if constexpr (std::is_same_v<T, keyboard_event>) {
                    if (e.is_pressed()) {
                        // Get key information
                        auto key_name = get_key_name(e.key);
                        auto scan_name = get_scancode_name(e.scan);
                        
                        std::cout << "Key DOWN: " << key_name 
                                  << " (keycode=" << static_cast<int>(e.key)
                                  << ", scancode=" << static_cast<int>(e.scan) 
                                  << " [" << scan_name << "]";
                        
                        // Show modifiers
                        if (e.mod != keymod::none) {
                            std::cout << ", mods:";
                            if (has_keymod(e.mod, keymod::shift)) std::cout << " SHIFT";
                            if (has_keymod(e.mod, keymod::ctrl)) std::cout << " CTRL";
                            if (has_keymod(e.mod, keymod::alt)) std::cout << " ALT";
                            if (has_keymod(e.mod, keymod::gui)) std::cout << " GUI";
                        }
                        
                        if (e.repeat) std::cout << " [REPEAT]";
                        std::cout << ")\n";
                        
                        // Handle special keys
                        if (e.scan == scancode::escape) {
                            running = false;
                        }
                        else if (e.scan == scancode::t && !e.repeat) {
                            // Toggle text input mode
                            if (text_input_active) {
                                text_session.reset();
                                text_input_active = false;
                                std::cout << "\n>>> Text input mode: OFF <<<\n\n";
                            } else {
                                text_session.emplace(win);
                                text_input_active = true;
                                text_buffer.clear();
                                std::cout << "\n>>> Text input mode: ON (type some text!) <<<\n";
                                
                                // Set input area for IME
                                sdlpp::rect_i input_area(100, 100, 300, 30);
                                text_session->set_input_area(input_area);
                            }
                        }
                        else if (e.scan == scancode::m && !e.repeat) {
                            // Manually set modifier state
                            std::cout << "\n>>> Setting modifier state to Shift+Ctrl <<<\n";
                            set_mod_state(keymod::shift | keymod::ctrl);
                        }
                        else if (e.scan == scancode::k && !e.repeat) {
                            // Display keyboard state
                            keyboard_state kb;
                            std::cout << "\n--- Current Keyboard State ---\n";
                            std::cout << "Active keys: ";
                            bool any = false;
                            for (const auto& [scan, name] : monitored_keys) {
                                if (kb.is_pressed(scan)) {
                                    std::cout << name << " ";
                                    any = true;
                                }
                            }
                            if (!any) std::cout << "(none)";
                            std::cout << "\nModifiers: ";
                            if (kb.is_ctrl_pressed()) std::cout << "Ctrl ";
                            if (kb.is_shift_pressed()) std::cout << "Shift ";
                            if (kb.is_alt_pressed()) std::cout << "Alt ";
                            if (kb.is_gui_pressed()) std::cout << "GUI ";
                            if (kb.get_mods() == keymod::none) std::cout << "(none)";
                            std::cout << "\n---\n\n";
                        }
                    }
                    else if (e.is_released()) {
                        auto key_name = get_key_name(e.key);
                        std::cout << "Key UP: " << key_name << "\n";
                    }
                }
                else if constexpr (std::is_same_v<T, text_input_event>) {
                    if (text_input_active) {
                        text_buffer += e.text;
                        std::cout << "Text input: \"" << e.text << "\" (buffer: \"" << text_buffer << "\")\n";
                    }
                }
                else if constexpr (std::is_same_v<T, text_editing_event>) {
                    if (text_input_active) {
                        std::cout << "Text editing: \"" << e.text << "\" (cursor: " 
                                  << e.start << ", length: " << e.length << ")\n";
                    }
                }
            });
        }
        
        // Check keyboard state for shortcuts
        keyboard_state kb;
        
        static bool ctrl_c_was_pressed = false;
        static bool ctrl_v_was_pressed = false;
        static bool ctrl_a_was_pressed = false;
        static bool ctrl_s_was_pressed = false;
        static bool ctrl_x_was_pressed = false;
        static bool ctrl_z_was_pressed = false;
        
        // Detect shortcut press (not hold)
        if (kb.is_ctrl_c() && !ctrl_c_was_pressed) {
            std::cout << ">>> Ctrl+C detected (Copy) <<<\n";
            ctrl_c_was_pressed = true;
        } else if (!kb.is_ctrl_c()) {
            ctrl_c_was_pressed = false;
        }
        
        if (kb.is_ctrl_v() && !ctrl_v_was_pressed) {
            std::cout << ">>> Ctrl+V detected (Paste) <<<\n";
            ctrl_v_was_pressed = true;
        } else if (!kb.is_ctrl_v()) {
            ctrl_v_was_pressed = false;
        }
        
        if (kb.is_ctrl_a() && !ctrl_a_was_pressed) {
            std::cout << ">>> Ctrl+A detected (Select All) <<<\n";
            ctrl_a_was_pressed = true;
        } else if (!kb.is_ctrl_a()) {
            ctrl_a_was_pressed = false;
        }
        
        if (kb.is_ctrl_s() && !ctrl_s_was_pressed) {
            std::cout << ">>> Ctrl+S detected (Save) <<<\n";
            ctrl_s_was_pressed = true;
        } else if (!kb.is_ctrl_s()) {
            ctrl_s_was_pressed = false;
        }
        
        if (kb.is_ctrl_x() && !ctrl_x_was_pressed) {
            std::cout << ">>> Ctrl+X detected (Cut) <<<\n";
            ctrl_x_was_pressed = true;
        } else if (!kb.is_ctrl_x()) {
            ctrl_x_was_pressed = false;
        }
        
        if (kb.is_ctrl_z() && !ctrl_z_was_pressed) {
            std::cout << ">>> Ctrl+Z detected (Undo) <<<\n";
            ctrl_z_was_pressed = true;
        } else if (!kb.is_ctrl_z()) {
            ctrl_z_was_pressed = false;
        }
        
        // Test key conversions with modifiers
        static int frame_count = 0;
        if (++frame_count % 300 == 0) { // Every 5 seconds at 60 FPS
            // Test scancode to keycode with current modifiers
            auto current_mods = get_mod_state();
            auto key_a = get_key_from_scancode(scancode::a, current_mods);
            if (current_mods != keymod::none) {
                std::cout << "Current scancode 'A' with mods -> keycode " 
                          << static_cast<int>(key_a) << "\n";
            }
        }
        
        // Present
        auto present_result = ren.present();
        if (!present_result) {
            logger::error(log_category::application, std::source_location::current(), "Failed to present: ", present_result.error());
        }
        
        // Frame limiting
        limiter.wait_for_next_frame();
    }
    
    std::cout << "\nGoodbye!\n";
    return 0;
}