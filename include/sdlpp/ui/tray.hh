#pragma once

/**
 * @file tray.hh
 * @brief System tray icon support
 * 
 * This header provides RAII wrappers for SDL3's system tray functionality,
 * allowing applications to add icons to the system tray with menus and actions.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/video/surface.hh>
#include <string>
#include <functional>
#include <memory>
#include <vector>
#include <optional>

namespace sdlpp {
    // Forward declarations
    class tray;
    class tray_menu;
    class tray_entry;

    /**
     * @brief Tray entry flags
     */
    enum class tray_entry_flags : Uint32 {
        none = 0,
        checked = SDL_TRAYENTRY_CHECKED, ///< Entry is checked (has checkmark)
        disabled = SDL_TRAYENTRY_DISABLED ///< Entry is disabled (grayed out)
    };

    /**
     * @brief Callback type for tray entry activation
     */
    using tray_entry_callback = std::function <void(tray_entry&)>;

    /**
     * @brief RAII wrapper for a tray menu entry
     *
     * Represents a single item in a tray menu, which can be clicked,
     * have a submenu, or be a separator.
     */
    class tray_entry {
        private:
            SDL_TrayEntry* entry_{nullptr};
            std::unique_ptr <tray_entry_callback> callback_;

            friend class tray_menu;

            // C callback wrapper
            static void SDLCALL entry_callback_wrapper(void* userdata, SDL_TrayEntry* entry) {
                if (!userdata) return;
                auto* callback = static_cast <tray_entry_callback*>(userdata);
                tray_entry wrapper(entry);
                (*callback)(wrapper);
            }

        public:
            /**
             * @brief Default constructor - creates an invalid entry
             */
            tray_entry() = default;

            /**
             * @brief Construct from SDL entry
             */
            explicit tray_entry(SDL_TrayEntry* entry)
                : entry_(entry) {
            }

            /**
             * @brief Get the raw SDL handle
             * @return Raw SDL_TrayEntry pointer
             */
            [[nodiscard]] SDL_TrayEntry* get() const noexcept { return entry_; }

            /**
             * @brief Check if the entry is valid
             * @return True if valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return entry_ != nullptr; }

            /**
             * @brief Get the entry label
             * @return Optional containing the label, or nullopt on error
             */
            [[nodiscard]] std::optional <std::string> get_label() const noexcept {
                if (!entry_) return std::nullopt;
                const char* label = SDL_GetTrayEntryLabel(entry_);
                if (!label) return std::nullopt;
                return std::string(label);
            }

            /**
             * @brief Set the entry label
             * @param label New label text
             * @return Expected<void> - empty on success, error on failure
             */
            expected <void, std::string> set_label(const std::string& label) noexcept {
                if (!entry_) return make_unexpected("Invalid entry");
                SDL_SetTrayEntryLabel(entry_, label.c_str());
                return {};
            }

            /**
             * @brief Check if the entry is checked
             * @return True if checked
             */
            [[nodiscard]] bool is_checked() const noexcept {
                if (!entry_) return false;
                return SDL_GetTrayEntryChecked(entry_);
            }

            /**
             * @brief Set the checked state
             * @param checked Whether to check the entry
             * @return Expected<void> - empty on success, error on failure
             */
            expected <void, std::string> set_checked(bool checked) noexcept {
                if (!entry_) return make_unexpected("Invalid entry");
                SDL_SetTrayEntryChecked(entry_, checked);
                return {};
            }

            /**
             * @brief Check if the entry is enabled
             * @return True if enabled
             */
            [[nodiscard]] bool is_enabled() const noexcept {
                if (!entry_) return false;
                return SDL_GetTrayEntryEnabled(entry_);
            }

            /**
             * @brief Set the enabled state
             * @param enabled Whether to enable the entry
             * @return Expected<void> - empty on success, error on failure
             */
            expected <void, std::string> set_enabled(bool enabled) noexcept {
                if (!entry_) return make_unexpected("Invalid entry");
                SDL_SetTrayEntryEnabled(entry_, enabled);
                return {};
            }

            /**
             * @brief Set a callback for when the entry is clicked
             * @param callback Callback function
             * @return Expected<void> - empty on success, error on failure
             */
            expected <void, std::string> set_callback(tray_entry_callback callback) noexcept {
                if (!entry_) return make_unexpected("Invalid entry");

                callback_ = std::make_unique <tray_entry_callback>(std::move(callback));
                SDL_SetTrayEntryCallback(entry_, entry_callback_wrapper, callback_.get());
                return {};
            }

            /**
             * @brief Click/activate the entry programmatically
             * @return Expected<void> - empty on success, error on failure
             */
            expected <void, std::string> click() noexcept {
                if (!entry_) return make_unexpected("Invalid entry");
                SDL_ClickTrayEntry(entry_);
                return {};
            }

            /**
             * @brief Get the submenu for this entry
             * @return Pointer to submenu, or nullptr if no submenu
             */
            [[nodiscard]] SDL_TrayMenu* get_submenu() const noexcept {
                if (!entry_) return nullptr;
                return SDL_GetTraySubmenu(entry_);
            }
    };

    /**
     * @brief RAII wrapper for a tray menu
     *
     * Represents a menu that can contain entries, separators, and submenus.
     */
    class tray_menu {
        private:
            SDL_TrayMenu* menu_{nullptr};
            std::vector <std::unique_ptr <tray_entry_callback>> callbacks_;
            bool owned_{false};

            // Private constructor for non-owned menus
            explicit tray_menu(SDL_TrayMenu* menu, bool owned = false)
                : menu_(menu), owned_(owned) {
            }

            friend class tray;
            friend class tray_entry;

        public:
            /**
             * @brief Default constructor - creates an invalid menu
             */
            tray_menu() = default;

            /**
             * @brief Move constructor
             */
            tray_menu(tray_menu&& other) noexcept
                : menu_(other.menu_)
                  , callbacks_(std::move(other.callbacks_))
                  , owned_(other.owned_) {
                other.menu_ = nullptr;
                other.owned_ = false;
            }

            /**
             * @brief Move assignment operator
             */
            tray_menu& operator=(tray_menu&& other) noexcept {
                if (this != &other) {
                    reset();
                    menu_ = other.menu_;
                    callbacks_ = std::move(other.callbacks_);
                    owned_ = other.owned_;
                    other.menu_ = nullptr;
                    other.owned_ = false;
                }
                return *this;
            }

            /**
             * @brief Destructor
             */
            ~tray_menu() {
                reset();
            }

            // Delete copy operations
            tray_menu(const tray_menu&) = delete;
            tray_menu& operator=(const tray_menu&) = delete;

            /**
             * @brief Create a new menu
             * @return Expected containing the menu, or error
             */
            [[nodiscard]] static expected <tray_menu, std::string> create(SDL_Tray* tray) noexcept {
                SDL_TrayMenu* menu = SDL_CreateTrayMenu(tray);
                if (!menu) {
                    return make_unexpected(get_error());
                }
                return tray_menu(menu, true);
            }

            /**
             * @brief Reset/destroy the menu
             */
            void reset() noexcept {
                callbacks_.clear();
                menu_ = nullptr;
                owned_ = false;
            }

            /**
             * @brief Get the raw SDL handle
             * @return Raw SDL_TrayMenu pointer
             */
            [[nodiscard]] SDL_TrayMenu* get() const noexcept { return menu_; }

            /**
             * @brief Check if the menu is valid
             * @return True if valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return menu_ != nullptr; }

            /**
             * @brief Add a regular menu item
             * @param label Item label
             * @param callback Optional callback for when clicked
             * @param flags Optional flags (checked, disabled)
             * @return Expected containing the entry, or error
             */
            expected <tray_entry, std::string> add_item(
                const std::string& label,
                std::optional <tray_entry_callback> callback = std::nullopt,
                tray_entry_flags flags = tray_entry_flags::none) noexcept {
                if (!menu_) return make_unexpected("Invalid menu");

                SDL_TrayEntry* entry = SDL_InsertTrayEntryAt(menu_, -1, label.c_str(),
                                                             static_cast <Uint32>(flags));
                if (!entry) {
                    return make_unexpected(get_error());
                }

                tray_entry entry_wrapper(entry);

                // Set callback if provided
                if (callback) {
                    auto cb_result = entry_wrapper.set_callback(*callback);
                    if (!cb_result) {
                        SDL_RemoveTrayEntry(entry);
                        return make_unexpected(cb_result.error());
                    }

                    // Store the callback to keep it alive
                    callbacks_.push_back(std::move(entry_wrapper.callback_));
                }

                return entry_wrapper;
            }

            /**
             * @brief Add a separator
             * @return Expected<void> - empty on success, error on failure
             */
            expected <void, std::string> add_separator() noexcept {
                if (!menu_) return make_unexpected("Invalid menu");

                SDL_TrayEntry* entry = SDL_InsertTrayEntryAt(menu_, -1, nullptr, 0);
                if (!entry) {
                    return make_unexpected(get_error());
                }
                return {};
            }

            /**
             * @brief Add a submenu
             * @param label Submenu label
             * @return Expected containing the submenu, or error
             * @note This is a placeholder - SDL3 submenu API may vary
             */
            expected <tray_menu, std::string> add_submenu(
                [[maybe_unused]] const std::string& label) noexcept {
                if (!menu_) return make_unexpected("Invalid menu");

                // Note: SDL_CreateTraySubmenu API may vary between SDL3 versions
                // For now, return an error until we can determine the correct API
                return make_unexpected("Submenu creation not yet implemented");
            }

            /**
             * @brief Get all entries in the menu
             * @return Vector of entry pointers
             */
            [[nodiscard]] std::vector <SDL_TrayEntry*> get_entries() const noexcept {
                if (!menu_) return {};

                int count = 0;
                auto** entries = const_cast <SDL_TrayEntry**>(SDL_GetTrayEntries(menu_, &count));
                if (!entries || count == 0) return {};

                std::vector <SDL_TrayEntry*> entries_list;
                entries_list.reserve(static_cast<size_t>(count));
                for (int i = 0; i < count; ++i) {
                    entries_list.push_back(entries[i]);
                }

                SDL_free(entries);
                return entries_list;
            }

            /**
             * @brief Remove an entry from the menu
             * @param entry Entry to remove
             * @return Expected<void> - empty on success, error on failure
             */
            expected <void, std::string> remove_entry(const tray_entry& entry) noexcept {
                if (!menu_) return make_unexpected("Invalid menu");
                if (!entry.is_valid()) return make_unexpected("Invalid entry");

                SDL_RemoveTrayEntry(entry.get());
                return {};
            }
    };

    /**
     * @brief RAII wrapper for system tray functionality
     *
     * This class manages a system tray icon with associated menus and actions.
     *
     * Example:
     * @code
     * auto tray_result = sdlpp::tray::create(icon_surface, "My Application");
     * if (!tray_result) {
     *     std::cerr << "Failed to create tray: " << tray_result.error() << "\n";
     *     return;
     * }
     *
     * auto& tray = tray_result.value();
     * auto menu = tray.get_menu();
     *
     * menu.add_item("Show Window", [&](auto&) {
     *     window.show();
     * });
     *
     * menu.add_separator();
     *
     * menu.add_item("Quit", [&](auto&) {
     *     running = false;
     * });
     * @endcode
     */
    class tray {
        private:
            SDL_Tray* tray_{nullptr};
            tray_menu menu_;

        public:
            /**
             * @brief Default constructor - creates an invalid tray
             */
            tray() = default;

            /**
             * @brief Move constructor
             */
            tray(tray&& other) noexcept
                : tray_(other.tray_)
                  , menu_(std::move(other.menu_)) {
                other.tray_ = nullptr;
            }

            /**
             * @brief Move assignment operator
             */
            tray& operator=(tray&& other) noexcept {
                if (this != &other) {
                    reset();
                    tray_ = other.tray_;
                    menu_ = std::move(other.menu_);
                    other.tray_ = nullptr;
                }
                return *this;
            }

            /**
             * @brief Destructor
             */
            ~tray() {
                reset();
            }

            // Delete copy operations
            tray(const tray&) = delete;
            tray& operator=(const tray&) = delete;

            /**
             * @brief Create a system tray icon
             * @param icon Icon surface
             * @param tooltip Optional tooltip text
             * @return Expected containing the tray, or error
             */
            [[nodiscard]] static expected <tray, std::string> create(
                const surface& icon,
                const std::string& tooltip = "") noexcept {
                SDL_Tray* sdl_tray = SDL_CreateTray(icon.get(),
                                                    tooltip.empty() ? nullptr : tooltip.c_str());
                if (!sdl_tray) {
                    return make_unexpected(get_error());
                }

                tray tray_instance;
                tray_instance.tray_ = sdl_tray;

                // Get the menu
                SDL_TrayMenu* menu = SDL_GetTrayMenu(sdl_tray);
                if (menu) {
                    tray_instance.menu_ = tray_menu(menu, false); // Not owned
                }

                return tray_instance;
            }

            /**
             * @brief Reset/destroy the tray
             */
            void reset() noexcept {
                if (tray_) {
                    SDL_DestroyTray(tray_);
                    tray_ = nullptr;
                }
                menu_.reset();
            }

            /**
             * @brief Get the raw SDL handle
             * @return Raw SDL_Tray pointer
             */
            [[nodiscard]] SDL_Tray* get() const noexcept { return tray_; }

            /**
             * @brief Check if the tray is valid
             * @return True if valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return tray_ != nullptr; }

            /**
             * @brief Check if the tray is valid (conversion operator)
             */
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the tray menu
             * @return Reference to the menu
             */
            [[nodiscard]] tray_menu& get_menu() noexcept { return menu_; }

            /**
             * @brief Get the tray menu (const)
             * @return Const reference to the menu
             */
            [[nodiscard]] const tray_menu& get_menu() const noexcept { return menu_; }

            /**
             * @brief Set the tray icon
             * @param icon New icon surface
             * @return Expected<void> - empty on success, error on failure
             */
            expected <void, std::string> set_icon(const surface& icon) noexcept {
                if (!tray_) return make_unexpected("Invalid tray");
                SDL_SetTrayIcon(tray_, icon.get());
                return {};
            }

            /**
             * @brief Set the tray tooltip
             * @param tooltip New tooltip text
             * @return Expected<void> - empty on success, error on failure
             */
            expected <void, std::string> set_tooltip(const std::string& tooltip) noexcept {
                if (!tray_) return make_unexpected("Invalid tray");
                SDL_SetTrayTooltip(tray_, tooltip.c_str());
                return {};
            }
    };

    /**
     * @brief Update all system trays
     *
     * This should be called periodically to ensure tray updates are processed.
     */
    inline void update_trays() noexcept {
        SDL_UpdateTrays();
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for tray_entry_flags
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, tray_entry_flags value);

/**
 * @brief Stream input operator for tray_entry_flags
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, tray_entry_flags& value);

}