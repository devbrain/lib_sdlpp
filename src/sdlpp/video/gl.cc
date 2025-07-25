/**
 * @file gl.cc
 * @brief Stream operator implementations for enums
 * @note This file is auto-generated by generate_enum_operators.py
 */

#include <sdlpp/video/gl.hh>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

namespace sdlpp {
    std::ostream& operator<<(std::ostream& os, gl_profile value) {
        switch (value) {
            case gl_profile::core:
                return os << "core";
            case gl_profile::compatibility:
                return os << "compatibility";
            case gl_profile::es:
                return os << "es";
            default:
                return os << "Unknown_gl_profile(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, gl_profile& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <gl_profile>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "core") {
            value = gl_profile::core;
        } else if (str == "compatibility") {
            value = gl_profile::compatibility;
        } else if (str == "es") {
            value = gl_profile::es;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <gl_profile>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }

    std::ostream& operator<<(std::ostream& os, gl_context_flag value) {
        switch (value) {
            case gl_context_flag::debug:
                return os << "debug";
            case gl_context_flag::forward_compatible:
                return os << "forward_compatible";
            case gl_context_flag::robust_access:
                return os << "robust_access";
            case gl_context_flag::reset_isolation:
                return os << "reset_isolation";
            default:
                return os << "Unknown_gl_context_flag(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, gl_context_flag& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <gl_context_flag>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "debug") {
            value = gl_context_flag::debug;
        } else if (str == "forward_compatible") {
            value = gl_context_flag::forward_compatible;
        } else if (str == "robust_access") {
            value = gl_context_flag::robust_access;
        } else if (str == "reset_isolation") {
            value = gl_context_flag::reset_isolation;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <gl_context_flag>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }

    std::ostream& operator<<(std::ostream& os, gl_release_behavior value) {
        switch (value) {
            case gl_release_behavior::none:
                return os << "none";
            case gl_release_behavior::flush:
                return os << "flush";
            default:
                return os << "Unknown_gl_release_behavior(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, gl_release_behavior& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <gl_release_behavior>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "none") {
            value = gl_release_behavior::none;
        } else if (str == "flush") {
            value = gl_release_behavior::flush;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <gl_release_behavior>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }

    std::ostream& operator<<(std::ostream& os, gl_reset_notification value) {
        switch (value) {
            case gl_reset_notification::no_notification:
                return os << "no_notification";
            case gl_reset_notification::lose_context:
                return os << "lose_context";
            default:
                return os << "Unknown_gl_reset_notification(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, gl_reset_notification& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <gl_reset_notification>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "no_notification") {
            value = gl_reset_notification::no_notification;
        } else if (str == "lose_context") {
            value = gl_reset_notification::lose_context;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <gl_reset_notification>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }

    std::ostream& operator<<(std::ostream& os, gl_attr value) {
        switch (value) {
            case gl_attr::red_size:
                return os << "red_size";
            case gl_attr::green_size:
                return os << "green_size";
            case gl_attr::blue_size:
                return os << "blue_size";
            case gl_attr::alpha_size:
                return os << "alpha_size";
            case gl_attr::buffer_size:
                return os << "buffer_size";
            case gl_attr::doublebuffer:
                return os << "doublebuffer";
            case gl_attr::depth_size:
                return os << "depth_size";
            case gl_attr::stencil_size:
                return os << "stencil_size";
            case gl_attr::accum_red_size:
                return os << "accum_red_size";
            case gl_attr::accum_green_size:
                return os << "accum_green_size";
            case gl_attr::accum_blue_size:
                return os << "accum_blue_size";
            case gl_attr::accum_alpha_size:
                return os << "accum_alpha_size";
            case gl_attr::stereo:
                return os << "stereo";
            case gl_attr::multisamplebuffers:
                return os << "multisamplebuffers";
            case gl_attr::multisamplesamples:
                return os << "multisamplesamples";
            case gl_attr::accelerated_visual:
                return os << "accelerated_visual";
            case gl_attr::context_major_version:
                return os << "context_major_version";
            case gl_attr::context_minor_version:
                return os << "context_minor_version";
            case gl_attr::context_flags:
                return os << "context_flags";
            case gl_attr::context_profile_mask:
                return os << "context_profile_mask";
            case gl_attr::share_with_current_context:
                return os << "share_with_current_context";
            case gl_attr::framebuffer_srgb_capable:
                return os << "framebuffer_srgb_capable";
            case gl_attr::context_release_behavior:
                return os << "context_release_behavior";
            case gl_attr::context_reset_notification:
                return os << "context_reset_notification";
            case gl_attr::context_no_error:
                return os << "context_no_error";
            case gl_attr::floatbuffers:
                return os << "floatbuffers";
            case gl_attr::egl_platform:
                return os << "egl_platform";
            default:
                return os << "Unknown_gl_attr(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, gl_attr& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <gl_attr>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "red_size") {
            value = gl_attr::red_size;
        } else if (str == "green_size") {
            value = gl_attr::green_size;
        } else if (str == "blue_size") {
            value = gl_attr::blue_size;
        } else if (str == "alpha_size") {
            value = gl_attr::alpha_size;
        } else if (str == "buffer_size") {
            value = gl_attr::buffer_size;
        } else if (str == "doublebuffer") {
            value = gl_attr::doublebuffer;
        } else if (str == "depth_size") {
            value = gl_attr::depth_size;
        } else if (str == "stencil_size") {
            value = gl_attr::stencil_size;
        } else if (str == "accum_red_size") {
            value = gl_attr::accum_red_size;
        } else if (str == "accum_green_size") {
            value = gl_attr::accum_green_size;
        } else if (str == "accum_blue_size") {
            value = gl_attr::accum_blue_size;
        } else if (str == "accum_alpha_size") {
            value = gl_attr::accum_alpha_size;
        } else if (str == "stereo") {
            value = gl_attr::stereo;
        } else if (str == "multisamplebuffers") {
            value = gl_attr::multisamplebuffers;
        } else if (str == "multisamplesamples") {
            value = gl_attr::multisamplesamples;
        } else if (str == "accelerated_visual") {
            value = gl_attr::accelerated_visual;
        } else if (str == "context_major_version") {
            value = gl_attr::context_major_version;
        } else if (str == "context_minor_version") {
            value = gl_attr::context_minor_version;
        } else if (str == "context_flags") {
            value = gl_attr::context_flags;
        } else if (str == "context_profile_mask") {
            value = gl_attr::context_profile_mask;
        } else if (str == "share_with_current_context") {
            value = gl_attr::share_with_current_context;
        } else if (str == "framebuffer_srgb_capable") {
            value = gl_attr::framebuffer_srgb_capable;
        } else if (str == "context_release_behavior") {
            value = gl_attr::context_release_behavior;
        } else if (str == "context_reset_notification") {
            value = gl_attr::context_reset_notification;
        } else if (str == "context_no_error") {
            value = gl_attr::context_no_error;
        } else if (str == "floatbuffers") {
            value = gl_attr::floatbuffers;
        } else if (str == "egl_platform") {
            value = gl_attr::egl_platform;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <gl_attr>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }
} // namespace
