#pragma once

/**
 * @file message_box.hh
 * @brief Native message box dialog support
 * 
 * This header provides RAII wrappers for SDL3's message box functionality,
 * allowing display of native system dialogs for alerts and simple user interaction.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/video/window.hh>
#include <string>
#include <vector>
#include <array>
#include <optional>

namespace sdlpp {
    /**
     * @brief Message box flags
     */
    enum class message_box_flags : Uint32 {
        error = SDL_MESSAGEBOX_ERROR, ///< Error dialog
        warning = SDL_MESSAGEBOX_WARNING, ///< Warning dialog
        information = SDL_MESSAGEBOX_INFORMATION ///< Informational dialog
    };

    /**
     * @brief Message box button flags
     */
    enum class message_box_button_flags : Uint32 {
        none = 0,
        return_key_default = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, ///< Default for Return key
        escape_key_default = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT ///< Default for Escape key
    };

    /**
     * @brief Color types for message box color scheme
     */
    enum class message_box_color_type {
        background = SDL_MESSAGEBOX_COLOR_BACKGROUND,
        text = SDL_MESSAGEBOX_COLOR_TEXT,
        button_border = SDL_MESSAGEBOX_COLOR_BUTTON_BORDER,
        button_background = SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND,
        button_selected = SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED,
        max = SDL_MESSAGEBOX_COLOR_COUNT
    };

    /**
     * @brief Button data for message box
     */
    struct message_box_button {
        message_box_button_flags flags{message_box_button_flags::none}; ///< Button flags
        int id{0}; ///< Button ID (returned when clicked)
        std::string text; ///< Button text

        /**
         * @brief Convert to SDL button data
         * @return SDL_MessageBoxButtonData structure
         */
        [[nodiscard]] SDL_MessageBoxButtonData to_sdl() const noexcept {
            return SDL_MessageBoxButtonData{
                static_cast <Uint32>(flags),
                id,
                text.c_str()
            };
        }
    };

    /**
     * @brief Color specification for message box
     */
    struct message_box_color {
        Uint8 r{255}; ///< Red component (0-255)
        Uint8 g{255}; ///< Green component (0-255)
        Uint8 b{255}; ///< Blue component (0-255)

        /**
         * @brief Convert to SDL color
         * @return SDL_MessageBoxColor structure
         */
        [[nodiscard]] SDL_MessageBoxColor to_sdl() const noexcept {
            return SDL_MessageBoxColor{r, g, b};
        }
    };

    /**
     * @brief Color scheme for message box
     */
    class message_box_color_scheme {
        private:
            std::array <message_box_color, static_cast <size_t>(message_box_color_type::max)> colors;

        public:
            /**
             * @brief Default constructor with default colors
             */
            message_box_color_scheme() = default;

            /**
             * @brief Set color for a specific element
             * @param type Color type to set
             * @param color Color value
             * @return Reference to this scheme for chaining
             */
            message_box_color_scheme& set_color(message_box_color_type type, const message_box_color& col) {
                auto index = static_cast <size_t>(type);
                if (index < colors.size()) {
                    colors[index] = col;
                }
                return *this;
            }

            /**
             * @brief Get color for a specific element
             * @param type Color type to get
             * @return Color value
             */
            [[nodiscard]] const message_box_color& get_color(message_box_color_type type) const {
                auto index = static_cast <size_t>(type);
                return colors[index];
            }

            /**
             * @brief Convert to SDL color scheme
             * @return SDL_MessageBoxColorScheme structure
             */
            [[nodiscard]] SDL_MessageBoxColorScheme to_sdl() const noexcept {
                SDL_MessageBoxColorScheme scheme;
                for (size_t i = 0; i < colors.size(); ++i) {
                    scheme.colors[i] = colors[i].to_sdl();
                }
                return scheme;
            }
    };

    /**
     * @brief Builder for creating complex message boxes
     *
     * This class provides a fluent interface for creating message boxes
     * with custom buttons and optional color schemes.
     *
     * Example:
     * @code
     * auto result = sdlpp::message_box_builder()
     *     .set_title("Confirm Action")
     *     .set_message("Are you sure you want to proceed?")
     *     .set_type(sdlpp::message_box_flags::warning)
     *     .add_button(1, "Yes", true)  // Default for Return key
     *     .add_button(0, "No", false, true)  // Default for Escape key
     *     .show();
     *
     * if (result && result.value() == 1) {
     *     // User clicked "Yes"
     * }
     * @endcode
     */
    class message_box_builder {
        private:
            message_box_flags flags_{message_box_flags::information};
            std::string title_;
            std::string message_;
            std::vector <message_box_button> buttons_;
            std::optional <message_box_color_scheme> color_scheme_;
            window* parent_window_{nullptr};

        public:
            /**
             * @brief Set the message box type/flags
             * @param flags Message box flags
             * @return Reference to this builder
             */
            message_box_builder& set_type(message_box_flags flags) {
                flags_ = flags;
                return *this;
            }

            /**
             * @brief Set the window title
             * @param title Title text
             * @return Reference to this builder
             */
            message_box_builder& set_title(const std::string& title) {
                title_ = title;
                return *this;
            }

            /**
             * @brief Set the message text
             * @param message Message text
             * @return Reference to this builder
             */
            message_box_builder& set_message(const std::string& message) {
                message_ = message;
                return *this;
            }

            /**
             * @brief Set the parent window (for modal behavior)
             * @param parent Parent window
             * @return Reference to this builder
             */
            message_box_builder& set_parent(window& parent) {
                parent_window_ = &parent;
                return *this;
            }

            /**
             * @brief Add a button to the message box
             * @param id Button ID (returned when clicked)
             * @param text Button text
             * @param is_return_default Make this the default for Return key
             * @param is_escape_default Make this the default for Escape key
             * @return Reference to this builder
             */
            message_box_builder& add_button(int id, const std::string& text,
                                            bool is_return_default = false,
                                            bool is_escape_default = false) {
                message_box_button button;
                button.id = id;
                button.text = text;

                if (is_return_default) {
                    button.flags = message_box_button_flags::return_key_default;
                } else if (is_escape_default) {
                    button.flags = message_box_button_flags::escape_key_default;
                }

                buttons_.push_back(std::move(button));
                return *this;
            }

            /**
             * @brief Add a custom button
             * @param button Button data
             * @return Reference to this builder
             */
            message_box_builder& add_button(const message_box_button& button) {
                buttons_.push_back(button);
                return *this;
            }

            /**
             * @brief Set a custom color scheme
             * @param scheme Color scheme to use
             * @return Reference to this builder
             */
            message_box_builder& set_color_scheme(const message_box_color_scheme& scheme) {
                color_scheme_ = scheme;
                return *this;
            }

            /**
             * @brief Show the message box
             * @return Expected containing the ID of the clicked button, or error message
             */
            [[nodiscard]] expected <int, std::string> show() const noexcept {
                if (buttons_.empty()) {
                    // Add default OK button if none specified
                    message_box_button ok_button;
                    ok_button.id = 0;
                    ok_button.text = "OK";
                    ok_button.flags = message_box_button_flags::return_key_default;

                    SDL_MessageBoxButtonData button_data = ok_button.to_sdl();

                    SDL_MessageBoxData data;
                    data.flags = static_cast <Uint32>(flags_);
                    data.window = parent_window_ ? parent_window_->get() : nullptr;
                    data.title = title_.c_str();
                    data.message = message_.c_str();
                    data.numbuttons = 1;
                    data.buttons = &button_data;
                    data.colorScheme = nullptr;

                    int button_id = 0;
                    if (!SDL_ShowMessageBox(&data, &button_id)) {
                        return make_unexpected(get_error());
                    }
                    return button_id;
                }

                // Convert buttons to SDL format
                std::vector <SDL_MessageBoxButtonData> sdl_buttons;
                sdl_buttons.reserve(buttons_.size());
                for (const auto& button : buttons_) {
                    sdl_buttons.push_back(button.to_sdl());
                }

                SDL_MessageBoxData data;
                data.flags = static_cast <Uint32>(flags_);
                data.window = parent_window_ ? parent_window_->get() : nullptr;
                data.title = title_.c_str();
                data.message = message_.c_str();
                data.numbuttons = static_cast <int>(sdl_buttons.size());
                data.buttons = sdl_buttons.data();

                // Set color scheme if provided
                SDL_MessageBoxColorScheme sdl_scheme;
                if (color_scheme_) {
                    sdl_scheme = color_scheme_->to_sdl();
                    data.colorScheme = &sdl_scheme;
                } else {
                    data.colorScheme = nullptr;
                }

                int button_id = 0;
                if (!SDL_ShowMessageBox(&data, &button_id)) {
                    return make_unexpected(get_error());
                }

                return button_id;
            }
    };

    /**
     * @brief Show a simple message box
     * @param flags Message box type
     * @param title Window title
     * @param message Message text
     * @param parent Optional parent window
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> show_simple_message_box(
        message_box_flags flags,
        const std::string& title,
        const std::string& message,
        window* parent = nullptr) noexcept {
        SDL_Window* parent_window = parent ? parent->get() : nullptr;

        if (!SDL_ShowSimpleMessageBox(
            static_cast <Uint32>(flags),
            title.c_str(),
            message.c_str(),
            parent_window)) {
            return make_unexpected(get_error());
        }

        return {};
    }

    /**
     * @brief Show an error message box
     * @param title Window title
     * @param message Error message
     * @param parent Optional parent window
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> show_error_box(
        const std::string& title,
        const std::string& message,
        window* parent = nullptr) noexcept {
        return show_simple_message_box(message_box_flags::error, title, message, parent);
    }

    /**
     * @brief Show a warning message box
     * @param title Window title
     * @param message Warning message
     * @param parent Optional parent window
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> show_warning_box(
        const std::string& title,
        const std::string& message,
        window* parent = nullptr) noexcept {
        return show_simple_message_box(message_box_flags::warning, title, message, parent);
    }

    /**
     * @brief Show an information message box
     * @param title Window title
     * @param message Information message
     * @param parent Optional parent window
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> show_info_box(
        const std::string& title,
        const std::string& message,
        window* parent = nullptr) noexcept {
        return show_simple_message_box(message_box_flags::information, title, message, parent);
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for message_box_flags
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, message_box_flags value);

/**
 * @brief Stream input operator for message_box_flags
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, message_box_flags& value);

/**
 * @brief Stream output operator for message_box_button_flags
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, message_box_button_flags value);

/**
 * @brief Stream input operator for message_box_button_flags
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, message_box_button_flags& value);

/**
 * @brief Stream output operator for message_box_color_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, message_box_color_type value);

/**
 * @brief Stream input operator for message_box_color_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, message_box_color_type& value);

}