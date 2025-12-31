#pragma once

/**
 * @file clipboard.hh
 * @brief System clipboard functionality
 * 
 * This header provides C++ wrappers around SDL3's clipboard API,
 * offering cross-platform access to the system clipboard for text
 * and other data types.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>

#include <string>
#include <vector>
#include <span>
#include <cstdint>
#include <functional>

namespace sdlpp {
    /**
     * @brief Clipboard data callback function type
     *
     * Called when another application requests clipboard data that was
     * set with a callback. Should return the data in the requested format.
     *
     * @param userdata User data passed when setting the callback
     * @param mime_type The MIME type of the requested data
     * @param size Output parameter for the size of returned data
     * @return Pointer to the data, or nullptr on error
     */
    using clipboard_data_callback = std::function <const void*(void* userdata, const char* mime_type, size_t* size)>;

    /**
     * @brief Clipboard cleanup callback function type
     *
     * Called when clipboard data is being replaced or cleared.
     *
     * @param userdata User data passed when setting the callback
     */
    using clipboard_cleanup_callback = std::function <void(void* userdata)>;

    namespace clipboard {
        /**
         * @brief Set text to the clipboard
         *
         * @param text The text to put on the clipboard
         * @return Expected<void> - empty on success, error message on failure
         *
         * Example:
         * @code
         * auto result = sdlpp::clipboard::set_text("Hello, clipboard!");
         * if (!result) {
         *     std::cerr << "Failed to set clipboard: " << result.error() << "\n";
         * }
         * @endcode
         */
        [[nodiscard]] inline expected <void, std::string> set_text(const std::string& text) {
            if (!SDL_SetClipboardText(text.c_str())) {
                return make_unexpectedf(get_error());
            }
            return {};
        }

        /**
         * @brief Get text from the clipboard
         *
         * @return The clipboard text, or empty string if none/error
         *
         * Example:
         * @code
         * std::string text = sdlpp::clipboard::get_text();
         * if (!text.empty()) {
         *     std::cout << "Clipboard contains: " << text << "\n";
         * }
         * @endcode
         */
        [[nodiscard]] inline std::string get_text() {
            char* text = SDL_GetClipboardText();
            if (!text) {
                return "";
            }
            std::string result(text);
            SDL_free(text);
            return result;
        }

        /**
         * @brief Check if the clipboard has text
         *
         * @return true if clipboard contains text
         */
        [[nodiscard]] inline bool has_text() noexcept {
            return SDL_HasClipboardText();
        }

        /**
         * @brief Set primary selection text (X11)
         *
         * This sets the primary selection on X11 platforms, which is the
         * text selected with the mouse that can be pasted with middle-click.
         * On other platforms, this may do nothing.
         *
         * @param text The text to put in primary selection
         * @return Expected<void> - empty on success, error message on failure
         */
        [[nodiscard]] inline expected <void, std::string> set_primary_selection_text(const std::string& text) {
            if (!SDL_SetPrimarySelectionText(text.c_str())) {
                return make_unexpectedf(get_error());
            }
            return {};
        }

        /**
         * @brief Get primary selection text (X11)
         *
         * @return The primary selection text, or empty string if none/error
         */
        [[nodiscard]] inline std::string get_primary_selection_text() {
            char* text = SDL_GetPrimarySelectionText();
            if (!text) {
                return "";
            }
            std::string result(text);
            SDL_free(text);
            return result;
        }

        /**
         * @brief Check if primary selection has text
         *
         * @return true if primary selection contains text
         */
        [[nodiscard]] inline bool has_primary_selection_text() noexcept {
            return SDL_HasPrimarySelectionText();
        }

        /**
         * @brief Clear the clipboard
         *
         * @return Expected<void> - empty on success, error message on failure
         */
        [[nodiscard]] inline expected <void, std::string> clear() {
            if (!SDL_ClearClipboardData()) {
                return make_unexpectedf(get_error());
            }
            return {};
        }

        /**
         * @brief Set clipboard data with specific MIME types
         *
         * This allows setting multiple data formats at once. The data is copied
         * immediately, so the input spans don't need to remain valid.
         *
         * @param mime_types Array of MIME type strings
         * @param data Array of data spans corresponding to each MIME type
         * @param num_mime_types Number of MIME types (must match array sizes)
         * @return Expected<void> - empty on success, error message on failure
         *
         * Example:
         * @code
         * const char* types[] = {"text/plain", "text/html"};
         * std::string plain = "Hello";
         * std::string html = "<b>Hello</b>";
         * std::span<const uint8_t> data[] = {
         *     {reinterpret_cast<const uint8_t*>(plain.data()), plain.size()},
         *     {reinterpret_cast<const uint8_t*>(html.data()), html.size()}
         * };
         * auto result = sdlpp::clipboard::set_data(types, data, 2);
         * @endcode
         */
        [[nodiscard]] inline expected <void, std::string> set_data(
            const char** mime_types,
            const std::span <const uint8_t>* data,
            size_t num_mime_types) {
            // SDL3 doesn't support setting raw data directly anymore
            // We need to use the callback mechanism
            struct raw_data {
                std::vector <std::vector <uint8_t>> buffers;
                std::vector <std::string> types;
            };

            auto* data_holder = new raw_data;
            data_holder->types.reserve(num_mime_types);
            data_holder->buffers.reserve(num_mime_types);

            for (size_t i = 0; i < num_mime_types; ++i) {
                data_holder->types.emplace_back(mime_types[i]);
                data_holder->buffers.emplace_back(data[i].begin(), data[i].end());
            }

            auto data_callback = [](void* userdata, const char* mime_type, size_t* size) -> const void* {
                auto* holder = static_cast <raw_data*>(userdata);
                for (size_t i = 0; i < holder->types.size(); ++i) {
                    if (holder->types[i] == mime_type) {
                        *size = holder->buffers[i].size();
                        return holder->buffers[i].data();
                    }
                }
                return nullptr;
            };

            auto cleanup_callback = [](void* userdata) {
                delete static_cast <raw_data*>(userdata);
            };

            if (!SDL_SetClipboardData(data_callback, cleanup_callback, data_holder,
                                      mime_types, num_mime_types)) {
                delete data_holder;
                return make_unexpectedf(get_error());
            }
            return {};
        }

        /**
         * @brief Get clipboard data for a specific MIME type
         *
         * @param mime_type The MIME type to retrieve
         * @return Vector containing the data, or empty vector on error
         *
         * Example:
         * @code
         * auto data = sdlpp::clipboard::get_data("text/html");
         * if (!data.empty()) {
         *     std::string html(data.begin(), data.end());
         *     std::cout << "HTML: " << html << "\n";
         * }
         * @endcode
         */
        [[nodiscard]] inline std::vector <uint8_t> get_data(const char* mime_type) {
            size_t size = 0;
            void* data = SDL_GetClipboardData(mime_type, &size);
            if (!data || size == 0) {
                return {};
            }

            std::vector <uint8_t> result(static_cast <const uint8_t*>(data),
                                         static_cast <const uint8_t*>(data) + size);
            SDL_free(data);
            return result;
        }

        /**
         * @brief Check if clipboard has data for a specific MIME type
         *
         * @param mime_type The MIME type to check for
         * @return true if the clipboard contains data of this type
         */
        [[nodiscard]] inline bool has_data(const char* mime_type) noexcept {
            return SDL_HasClipboardData(mime_type);
        }

        /**
         * @brief Get list of available MIME types in clipboard
         *
         * @return Vector of MIME type strings
         *
         * Example:
         * @code
         * auto types = sdlpp::clipboard::get_mime_types();
         * std::cout << "Clipboard contains " << types.size() << " formats:\n";
         * for (const auto& type : types) {
         *     std::cout << "  " << type << "\n";
         * }
         * @endcode
         */
        [[nodiscard]] inline std::vector <std::string> get_mime_types() {
            size_t count = 0;
            char** types = SDL_GetClipboardMimeTypes(&count);
            if (!types || count == 0) {
                return {};
            }

            std::vector <std::string> result;
            result.reserve(count);

            for (size_t i = 0; i < count; ++i) {
                if (types[i]) {
                    result.emplace_back(types[i]);
                }
            }

            SDL_free(types);
            return result;
        }

        /**
         * @brief Clipboard data provider
         *
         * RAII wrapper for providing clipboard data via callbacks.
         * This allows lazy generation of clipboard data only when requested.
         */
        class data_provider {
            private:
                struct callback_data {
                    clipboard_data_callback data_cb;
                    clipboard_cleanup_callback cleanup_cb;
                    void* userdata;
                };

                static const void* sdl_data_callback(void* userdata, const char* mime_type, size_t* size) {
                    auto* data = static_cast <callback_data*>(userdata);
                    if (data && data->data_cb) {
                        return data->data_cb(data->userdata, mime_type, size);
                    }
                    return nullptr;
                }

                static void sdl_cleanup_callback(void* userdata) {
                    auto* data = static_cast <callback_data*>(userdata);
                    if (data) {
                        if (data->cleanup_cb) {
                            data->cleanup_cb(data->userdata);
                        }
                        delete data;
                    }
                }

            public:
                /**
                 * @brief Set clipboard data with callbacks
                 *
                 * The callbacks will be invoked when another application requests
                 * the clipboard data. This allows generating data on-demand.
                 *
                 * @param mime_types Vector of supported MIME types
                 * @param data_cb Callback to provide data
                 * @param cleanup_cb Optional cleanup callback
                 * @param userdata Optional user data passed to callbacks
                 * @return Expected<void> - empty on success, error message on failure
                 *
                 * Example:
                 * @code
                 * auto provider = [](void* ud, const char* type, size_t* size) -> const void* {
                 *     static std::string data = "Generated data";
                 *     *size = data.size();
                 *     return data.data();
                 * };
                 *
                 * std::vector<std::string> types = {"text/plain"};
                 * auto result = sdlpp::clipboard::data_provider::set(types, provider);
                 * @endcode
                 */
                static expected <void, std::string> set(
                    const std::vector <std::string>& mime_types,
                    clipboard_data_callback data_cb,
                    clipboard_cleanup_callback cleanup_cb = nullptr,
                    void* userdata = nullptr) {
                    if (mime_types.empty() || !data_cb) {
                        return make_unexpectedf("Invalid parameters");
                    }

                    // Convert strings to C-style array
                    std::vector <const char*> type_ptrs;
                    type_ptrs.reserve(mime_types.size());
                    for (const auto& type : mime_types) {
                        type_ptrs.push_back(type.c_str());
                    }

                    // Create callback data
                    auto* cb_data = new callback_data{data_cb, cleanup_cb, userdata};

                    if (!SDL_SetClipboardData(sdl_data_callback, sdl_cleanup_callback,
                                              cb_data, type_ptrs.data(),
                                              type_ptrs.size())) {
                        delete cb_data;
                        return make_unexpectedf(get_error());
                    }

                    return {};
                }
        };
    } // namespace clipboard
} // namespace sdlpp
