//
// Created by igor on 7/14/25.
//

#include "sdlpp/video/window.hh"
#include "sdlpp/video/renderer.hh"
#include "sdlpp/core/error.hh"
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cctype>

namespace sdlpp {
    expected <renderer, std::string> window::create_renderer(const char* driver_name) const {
        if (!ptr) {
            return make_unexpected("Invalid window");
        }

        SDL_Renderer* r = SDL_CreateRenderer(ptr.get(), driver_name);
        if (!r) {
            return make_unexpected(get_error());
        }

        return renderer(r);
    }
} // namespace sdlpp

// Auto-generated enum stream operators
namespace sdlpp {
    std::ostream& operator<<(std::ostream& os, window_flags value) {
        // Check if hex output is requested
        if (os.flags() & std::ios::hex) {
            // Output as hex value
            os << static_cast <unsigned int>(value);
            return os;
        }

        // Output as human-readable flag names
        std::vector <std::string> active_flags;

        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::fullscreen)) != 0) {
            active_flags.emplace_back("fullscreen");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::opengl)) != 0) {
            active_flags.emplace_back("opengl");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::occluded)) != 0) {
            active_flags.emplace_back("occluded");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::hidden)) != 0) {
            active_flags.emplace_back("hidden");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::borderless)) != 0) {
            active_flags.emplace_back("borderless");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::resizable)) != 0) {
            active_flags.emplace_back("resizable");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::minimized)) != 0) {
            active_flags.emplace_back("minimized");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::maximized)) != 0) {
            active_flags.emplace_back("maximized");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::mouse_grabbed)) != 0) {
            active_flags.emplace_back("mouse_grabbed");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::input_focus)) != 0) {
            active_flags.emplace_back("input_focus");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::mouse_focus)) != 0) {
            active_flags.emplace_back("mouse_focus");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::external)) != 0) {
            active_flags.emplace_back("external");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::modal)) != 0) {
            active_flags.emplace_back("modal");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::high_pixel_density)) != 0) {
            active_flags.emplace_back("high_pixel_density");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::mouse_capture)) != 0) {
            active_flags.emplace_back("mouse_capture");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::always_on_top)) != 0) {
            active_flags.emplace_back("always_on_top");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::utility)) != 0) {
            active_flags.emplace_back("utility");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::tooltip)) != 0) {
            active_flags.emplace_back("tooltip");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::popup_menu)) != 0) {
            active_flags.emplace_back("popup_menu");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::keyboard_grabbed)) != 0) {
            active_flags.emplace_back("keyboard_grabbed");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::vulkan)) != 0) {
            active_flags.emplace_back("vulkan");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::metal)) != 0) {
            active_flags.emplace_back("metal");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::transparent)) != 0) {
            active_flags.emplace_back("transparent");
        }
        if ((static_cast <unsigned int>(value) & static_cast <unsigned int>(window_flags::not_focusable)) != 0) {
            active_flags.emplace_back("not_focusable");
        }

        // Sort flags lexicographically
        std::sort(active_flags.begin(), active_flags.end());

        if (active_flags.empty()) {
            os << "none";
        } else {
            bool first = true;
            for (const auto& flag : active_flags) {
                if (!first) os << " | ";
                os << flag;
                first = false;
            }
        }
        return os;
    }

    std::istream& operator>>(std::istream& is, window_flags& value) {
        std::string str;
        // For flag enums, we need to read the entire content which may contain spaces
        // Read until newline or EOF to support "flag1 | flag2" format
        std::getline(is, str);

        value = static_cast <window_flags>(0);

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned long long_value = std::stoul(str, nullptr, 16);
                value = static_cast <window_flags>(static_cast <unsigned int>(long_value));
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Check for explicit hex format (0x...)
        if (str.substr(0, 2) == "0x" || str.substr(0, 2) == "0X") {
            try {
                unsigned long long_value = std::stoul(str, nullptr, 16);
                value = static_cast <window_flags>(static_cast <unsigned int>(long_value));
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Parse as flag names separated by |
        size_t start = 0;
        size_t pos = 0;

        while (start < str.length()) {
            pos = str.find('|', start);
            std::string flag_name = (pos == std::string::npos) ? str.substr(start) : str.substr(start, pos - start);

            // Trim whitespace
            size_t first = flag_name.find_first_not_of(' ');
            size_t last = flag_name.find_last_not_of(' ');
            if (first != std::string::npos && last != std::string::npos) {
                flag_name = flag_name.substr(first, last - first + 1);
            }

            // Convert to lowercase
            std::transform(flag_name.begin(), flag_name.end(), flag_name.begin(), ::tolower);

            // Match flag name
            bool found = false;
            if (flag_name == "none") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::none));
                found = true;
            }
            if (flag_name == "fullscreen") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::fullscreen));
                found = true;
            }
            if (flag_name == "opengl") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::opengl));
                found = true;
            }
            if (flag_name == "occluded") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::occluded));
                found = true;
            }
            if (flag_name == "hidden") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::hidden));
                found = true;
            }
            if (flag_name == "borderless") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::borderless));
                found = true;
            }
            if (flag_name == "resizable") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::resizable));
                found = true;
            }
            if (flag_name == "minimized") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::minimized));
                found = true;
            }
            if (flag_name == "maximized") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::maximized));
                found = true;
            }
            if (flag_name == "mouse_grabbed") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::mouse_grabbed));
                found = true;
            }
            if (flag_name == "input_focus") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::input_focus));
                found = true;
            }
            if (flag_name == "mouse_focus") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::mouse_focus));
                found = true;
            }
            if (flag_name == "external") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::external));
                found = true;
            }
            if (flag_name == "modal") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::modal));
                found = true;
            }
            if (flag_name == "high_pixel_density") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::high_pixel_density));
                found = true;
            }
            if (flag_name == "mouse_capture") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::mouse_capture));
                found = true;
            }
            if (flag_name == "always_on_top") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::always_on_top));
                found = true;
            }
            if (flag_name == "utility") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::utility));
                found = true;
            }
            if (flag_name == "tooltip") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::tooltip));
                found = true;
            }
            if (flag_name == "popup_menu") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::popup_menu));
                found = true;
            }
            if (flag_name == "keyboard_grabbed") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::keyboard_grabbed));
                found = true;
            }
            if (flag_name == "vulkan") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::vulkan));
                found = true;
            }
            if (flag_name == "metal") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::metal));
                found = true;
            }
            if (flag_name == "transparent") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::transparent));
                found = true;
            }
            if (flag_name == "not_focusable") {
                value = static_cast <window_flags>(
                    static_cast <unsigned int>(value) | static_cast <unsigned int>(window_flags::not_focusable));
                found = true;
            }

            if (!found) {
                // Try to parse as integer
                try {
                    unsigned long long_value = std::stoul(flag_name);
                    value = static_cast <window_flags>(
                        static_cast <unsigned int>(value) | static_cast <unsigned int>(long_value));
                } catch (...) {
                    is.setstate(std::ios::failbit);
                    return is;
                }
            }

            if (pos == std::string::npos) break;
            start = pos + 1;
        }

        return is;
    }

    std::ostream& operator<<(std::ostream& os, fullscreen_mode value) {
        switch (value) {
            case fullscreen_mode::windowed:
                return os << "windowed";
            case fullscreen_mode::fullscreen:
                return os << "fullscreen";
            default:
                return os << "Unknown_fullscreen_mode(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, fullscreen_mode& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <fullscreen_mode>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "windowed") {
            value = fullscreen_mode::windowed;
        } else if (str == "fullscreen") {
            value = fullscreen_mode::fullscreen;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <fullscreen_mode>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }
} // namespace
