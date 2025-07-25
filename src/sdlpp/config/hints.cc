/**
 * @file hints.cc
 * @brief Stream operator implementations for enums
 * @note This file is auto-generated by generate_enum_operators.py
 */

#include <sdlpp/config/hints.hh>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>


namespace sdlpp {
    std::ostream& operator<<(std::ostream& os, hint_priority value) {
        switch (value) {
            case hint_priority::default_priority:
                return os << "default_priority";
            case hint_priority::normal:
                return os << "normal";
            case hint_priority::override_priority:
                return os << "override_priority";
            default:
                return os << "Unknown_hint_priority(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, hint_priority& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <hint_priority>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "default_priority") {
            value = hint_priority::default_priority;
        } else if (str == "normal") {
            value = hint_priority::normal;
        } else if (str == "override_priority") {
            value = hint_priority::override_priority;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <hint_priority>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }
} // namespace
