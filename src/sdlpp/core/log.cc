/**
 * @file log.cc
 * @brief Stream operator implementations for enums
 * @note This file is auto-generated by generate_enum_operators.py
 */

#include <sdlpp/core/log.hh>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

namespace sdlpp {
    std::ostream& operator<<(std::ostream& os, log_priority value) {
        switch (value) {
            case log_priority::invalid:
                return os << "invalid";
            case log_priority::trace:
                return os << "trace";
            case log_priority::verbose:
                return os << "verbose";
            case log_priority::debug:
                return os << "debug";
            case log_priority::info:
                return os << "info";
            case log_priority::warn:
                return os << "warn";
            case log_priority::error:
                return os << "error";
            case log_priority::critical:
                return os << "critical";
            default:
                return os << "Unknown_log_priority(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, log_priority& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <log_priority>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "invalid") {
            value = log_priority::invalid;
        } else if (str == "trace") {
            value = log_priority::trace;
        } else if (str == "verbose") {
            value = log_priority::verbose;
        } else if (str == "debug") {
            value = log_priority::debug;
        } else if (str == "info") {
            value = log_priority::info;
        } else if (str == "warn") {
            value = log_priority::warn;
        } else if (str == "error") {
            value = log_priority::error;
        } else if (str == "critical") {
            value = log_priority::critical;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <log_priority>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }

    std::ostream& operator<<(std::ostream& os, log_category value) {
        switch (value) {
            case log_category::application:
                return os << "application";
            case log_category::error:
                return os << "error";
            case log_category::assert:
                return os << "assert";
            case log_category::system:
                return os << "system";
            case log_category::audio:
                return os << "audio";
            case log_category::video:
                return os << "video";
            case log_category::render:
                return os << "render";
            case log_category::input:
                return os << "input";
            case log_category::test:
                return os << "test";
            case log_category::gpu:
                return os << "gpu";
            case log_category::custom:
                return os << "custom";
            default:
                return os << "Unknown_log_category(" << static_cast <int>(value) << ")";
        }
    }

    std::istream& operator>>(std::istream& is, log_category& value) {
        std::string str;
        is >> str;

        // Check if hex input is expected based on stream flags
        if (is.flags() & std::ios::hex) {
            try {
                unsigned int int_value = static_cast<unsigned int>(std::stoul(str, nullptr, 16));
                value = static_cast <log_category>(int_value);
                return is;
            } catch (...) {
                is.setstate(std::ios::failbit);
                return is;
            }
        }

        // Convert to lowercase for case-insensitive comparison
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);

        if (str == "application") {
            value = log_category::application;
        } else if (str == "error") {
            value = log_category::error;
        } else if (str == "assert") {
            value = log_category::assert;
        } else if (str == "system") {
            value = log_category::system;
        } else if (str == "audio") {
            value = log_category::audio;
        } else if (str == "video") {
            value = log_category::video;
        } else if (str == "render") {
            value = log_category::render;
        } else if (str == "input") {
            value = log_category::input;
        } else if (str == "test") {
            value = log_category::test;
        } else if (str == "gpu") {
            value = log_category::gpu;
        } else if (str == "custom") {
            value = log_category::custom;
        } else {
            // Try to parse as integer
            try {
                int int_value = std::stoi(str);
                value = static_cast <log_category>(int_value);
            } catch (...) {
                is.setstate(std::ios::failbit);
            }
        }

        return is;
    }
} // namespace
