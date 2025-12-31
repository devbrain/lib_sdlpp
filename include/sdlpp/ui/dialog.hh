#pragma once

/**
 * @file dialog.hh
 * @brief Native file and folder dialog support
 * 
 * This header provides RAII wrappers for SDL3's file dialog functionality,
 * allowing users to select files and folders through native system dialogs.
 * All dialogs are non-blocking and use callbacks for result handling.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/config/properties.hh>
#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include <optional>
#include <exception>
#include <new>

namespace sdlpp {
    /**
     * @brief File dialog types
     */
    enum class file_dialog_type {
        open_file = SDL_FILEDIALOG_OPENFILE, ///< Open file dialog
        save_file = SDL_FILEDIALOG_SAVEFILE, ///< Save file dialog
        open_folder = SDL_FILEDIALOG_OPENFOLDER ///< Open folder dialog
    };

    /**
     * @brief File filter for dialogs
     *
     * Used to restrict the types of files shown in file dialogs.
     */
    struct dialog_file_filter {
        std::string name; ///< Display name (e.g., "Image files")
        std::string pattern; ///< Pattern (e.g., "*.png;*.jpg")

        /**
         * @brief Convert to SDL filter
         * @return SDL_DialogFileFilter structure
         */
        [[nodiscard]] SDL_DialogFileFilter to_sdl() const noexcept {
            return SDL_DialogFileFilter{
                name.c_str(),
                pattern.c_str()
            };
        }
    };

    /**
     * @brief Result of a file dialog operation
     */
    struct dialog_result {
        bool accepted{false}; ///< True if user accepted (didn't cancel)
        std::vector <std::filesystem::path> paths; ///< Selected file/folder paths

        /**
         * @brief Check if dialog was cancelled
         * @return True if user cancelled the dialog
         */
        [[nodiscard]] bool cancelled() const noexcept {
            return !accepted;
        }

        /**
         * @brief Get the first (or only) selected path
         * @return Optional containing the path if available
         */
        [[nodiscard]] std::optional <std::filesystem::path> get_path() const {
            if (paths.empty()) {
                return std::nullopt;
            }
            return paths.front();
        }
    };

    /**
     * @brief Callback type for dialog results
     */
    using dialog_callback = std::function <void(const dialog_result&)>;

    namespace detail {
        /**
         * @brief C callback wrapper for file dialogs
         */
        inline void SDLCALL dialog_callback_wrapper(void* userdata, const char* const* filelist, int) {
            if (!userdata) return;

            auto* callback = static_cast <dialog_callback*>(userdata);
            try {
                dialog_result result;

                if (filelist) {
                    result.accepted = true;
                    // Convert file list to paths
                    for (int i = 0; filelist[i] != nullptr; ++i) {
                        result.paths.emplace_back(filelist[i]);
                    }
                } else {
                    result.accepted = false;
                }

                (*callback)(result);
            } catch (...) {
            }
            delete callback;
        }
    } // namespace detail

    /**
     * @brief File dialog builder for creating customized file dialogs
     *
     * This class provides a fluent interface for creating file dialogs
     * with various options like filters, default locations, and more.
     *
     * Example:
     * @code
     * sdlpp::file_dialog_builder()
     *     .set_type(sdlpp::file_dialog_type::open_file)
     *     .set_title("Select Image")
     *     .add_filter("Image files", "*.png;*.jpg;*.jpeg")
     *     .add_filter("All files", "*.*")
     *     .set_default_location("/home/user/Pictures")
     *     .allow_multiple(true)
     *     .show([](const sdlpp::dialog_result& result) {
     *         if (result.accepted) {
     *             for (const auto& path : result.paths) {
     *                 std::cout << "Selected: " << path << "\n";
     *             }
     *         }
     *     });
     * @endcode
     */
    class file_dialog_builder {
        private:
            file_dialog_type type_{file_dialog_type::open_file};
            std::string title_;
            std::string accept_label_;
            std::string cancel_label_;
            std::optional <std::filesystem::path> default_location_;
            std::optional <std::string> default_name_;
            std::vector <dialog_file_filter> filters_;
            window* parent_window_{nullptr};
            bool allow_multiple_{false};

        public:
            /**
             * @brief Set the dialog type
             * @param type Dialog type
             * @return Reference to this builder
             */
            file_dialog_builder& set_type(file_dialog_type type) {
                type_ = type;
                return *this;
            }

            /**
             * @brief Set the dialog title
             * @param title Title text
             * @return Reference to this builder
             */
            file_dialog_builder& set_title(const std::string& title) {
                title_ = title;
                return *this;
            }

            /**
             * @brief Set the accept button label
             * @param label Button label
             * @return Reference to this builder
             */
            file_dialog_builder& set_accept_label(const std::string& label) {
                accept_label_ = label;
                return *this;
            }

            /**
             * @brief Set the cancel button label
             * @param label Button label
             * @return Reference to this builder
             */
            file_dialog_builder& set_cancel_label(const std::string& label) {
                cancel_label_ = label;
                return *this;
            }

            /**
             * @brief Set the default location
             * @param path Default directory path
             * @return Reference to this builder
             */
            file_dialog_builder& set_default_location(const std::filesystem::path& path) {
                default_location_ = path;
                return *this;
            }

            /**
             * @brief Set the default file name (for save dialogs)
             * @param name Default file name
             * @return Reference to this builder
             */
            file_dialog_builder& set_default_name(const std::string& name) {
                default_name_ = name;
                return *this;
            }

            /**
             * @brief Set the parent window
             * @param parent Parent window
             * @return Reference to this builder
             */
            file_dialog_builder& set_parent(window& parent) {
                parent_window_ = &parent;
                return *this;
            }

            /**
             * @brief Add a file filter
             * @param name Display name
             * @param pattern File pattern
             * @return Reference to this builder
             */
            file_dialog_builder& add_filter(const std::string& name, const std::string& pattern) {
                filters_.push_back({name, pattern});
                return *this;
            }

            /**
             * @brief Add a file filter
             * @param filter Filter to add
             * @return Reference to this builder
             */
            file_dialog_builder& add_filter(const dialog_file_filter& filter) {
                filters_.push_back(filter);
                return *this;
            }

            /**
             * @brief Allow multiple file selection (for open file dialogs)
             * @param allow Whether to allow multiple selection
             * @return Reference to this builder
             */
            file_dialog_builder& allow_multiple(bool allow = true) {
                allow_multiple_ = allow;
                return *this;
            }

            /**
             * @brief Show the dialog
             * @param callback Callback to invoke with the result
             * @return Expected<void> - empty on success, error message on failure
             * @note The callback will be invoked asynchronously when the user completes the dialog
             */
            [[nodiscard]] expected <void, std::string> show(dialog_callback callback) const noexcept {
                try {
                    // Create properties
                    SDL_PropertiesID props = SDL_CreateProperties();
                    if (props == 0) {
                        return make_unexpectedf(get_error());
                    }

                // Auto-cleanup properties
                struct PropertiesGuard {
                    SDL_PropertiesID id;
                    ~PropertiesGuard() { SDL_DestroyProperties(id); }
                } guard{props};

                // Set window if provided
                if (parent_window_) {
                    SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_WINDOW_POINTER, parent_window_->get());
                }

                // Set title if provided
                if (!title_.empty()) {
                    SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_TITLE_STRING, title_.c_str());
                }

                // Set accept button label
                if (!accept_label_.empty()) {
                    SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_ACCEPT_STRING, accept_label_.c_str());
                }

                // Set cancel button label
                if (!cancel_label_.empty()) {
                    SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_CANCEL_STRING, cancel_label_.c_str());
                }

                // Set default location
                if (default_location_) {
                    SDL_SetStringProperty(props, SDL_PROP_FILE_DIALOG_LOCATION_STRING,
                                          default_location_->string().c_str());
                }

                // Set default name for save dialogs
                if (type_ == file_dialog_type::save_file && default_name_) {
                    // Note: Property name might vary in SDL3 versions
                    SDL_SetStringProperty(props, "SDL.filedialog.default_filename",
                                          default_name_->c_str());
                }

                // Set filters
                if (!filters_.empty()) {
                    std::vector <SDL_DialogFileFilter> sdl_filters;
                    sdl_filters.reserve(filters_.size());
                    for (const auto& filter : filters_) {
                        sdl_filters.push_back(filter.to_sdl());
                    }

                    SDL_SetPointerProperty(props, SDL_PROP_FILE_DIALOG_FILTERS_POINTER, sdl_filters.data());
                    SDL_SetNumberProperty(props, SDL_PROP_FILE_DIALOG_NFILTERS_NUMBER,
                                          static_cast <Sint64>(sdl_filters.size()));
                }

                // Set multiple selection for open file dialog
                if (type_ == file_dialog_type::open_file && allow_multiple_) {
                    SDL_SetBooleanProperty(props, SDL_PROP_FILE_DIALOG_MANY_BOOLEAN, true);
                }

                // Create callback wrapper
                auto* callback_ptr = new dialog_callback(std::move(callback));

                // Show dialog
                SDL_ShowFileDialogWithProperties(static_cast <SDL_FileDialogType>(type_),
                                                 detail::dialog_callback_wrapper,
                                                 callback_ptr,
                                                 props);

                    return {};
                } catch (const std::bad_alloc&) {
                    return make_unexpectedf("Out of memory");
                } catch (const std::exception& e) {
                    return make_unexpectedf(e.what());
                } catch (...) {
                    return make_unexpectedf("Unknown error");
                }
            }
    };

    /**
     * @brief Show an open file dialog
     * @param callback Callback to invoke with the result
     * @param parent Optional parent window
     * @param filters Optional file filters
     * @param allow_multiple Whether to allow multiple file selection
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> show_open_file_dialog(
        dialog_callback callback,
        window* parent = nullptr,
        const std::vector <dialog_file_filter>& filters = {},
        bool allow_multiple = false) noexcept {
        try {
            // Create callback wrapper
            auto* callback_ptr = new dialog_callback(std::move(callback));

            // Convert filters
            std::vector <SDL_DialogFileFilter> sdl_filters;
            if (!filters.empty()) {
                sdl_filters.reserve(filters.size());
                for (const auto& filter : filters) {
                    sdl_filters.push_back(filter.to_sdl());
                }
            }

            SDL_ShowOpenFileDialog(
                detail::dialog_callback_wrapper,
                callback_ptr,
                parent ? parent->get() : nullptr,
                sdl_filters.empty() ? nullptr : sdl_filters.data(),
                static_cast <int>(sdl_filters.size()),
                nullptr, // default location
                allow_multiple
            );

            return {};
        } catch (const std::bad_alloc&) {
            return make_unexpectedf("Out of memory");
        } catch (const std::exception& e) {
            return make_unexpectedf(e.what());
        } catch (...) {
            return make_unexpectedf("Unknown error");
        }
    }

    /**
     * @brief Show a save file dialog
     * @param callback Callback to invoke with the result
     * @param parent Optional parent window
     * @param filters Optional file filters
     * @param default_name Optional default file name
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> show_save_file_dialog(
        dialog_callback callback,
        window* parent = nullptr,
        const std::vector <dialog_file_filter>& filters = {},
        const std::string& default_name = "") noexcept {
        try {
            // Create callback wrapper
            auto* callback_ptr = new dialog_callback(std::move(callback));

            // Convert filters
            std::vector <SDL_DialogFileFilter> sdl_filters;
            if (!filters.empty()) {
                sdl_filters.reserve(filters.size());
                for (const auto& filter : filters) {
                    sdl_filters.push_back(filter.to_sdl());
                }
            }

            SDL_ShowSaveFileDialog(
                detail::dialog_callback_wrapper,
                callback_ptr,
                parent ? parent->get() : nullptr,
                sdl_filters.empty() ? nullptr : sdl_filters.data(),
                static_cast <int>(sdl_filters.size()),
                default_name.empty() ? nullptr : default_name.c_str()
            );

            return {};
        } catch (const std::bad_alloc&) {
            return make_unexpectedf("Out of memory");
        } catch (const std::exception& e) {
            return make_unexpectedf(e.what());
        } catch (...) {
            return make_unexpectedf("Unknown error");
        }
    }

    /**
     * @brief Show an open folder dialog
     * @param callback Callback to invoke with the result
     * @param parent Optional parent window
     * @param allow_multiple Whether to allow multiple folder selection
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> show_open_folder_dialog(
        dialog_callback callback,
        window* parent = nullptr,
        bool allow_multiple = false) noexcept {
        try {
            // Create callback wrapper
            auto* callback_ptr = new dialog_callback(std::move(callback));

            SDL_ShowOpenFolderDialog(
                detail::dialog_callback_wrapper,
                callback_ptr,
                parent ? parent->get() : nullptr,
                nullptr, // default location
                allow_multiple
            );

            return {};
        } catch (const std::bad_alloc&) {
            return make_unexpectedf("Out of memory");
        } catch (const std::exception& e) {
            return make_unexpectedf(e.what());
        } catch (...) {
            return make_unexpectedf("Unknown error");
        }
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for file_dialog_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, file_dialog_type value);

/**
 * @brief Stream input operator for file_dialog_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, file_dialog_type& value);

}
