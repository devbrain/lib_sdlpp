//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file properties.hh
 * @brief Modern C++ wrapper for SDL3 properties system
 * 
 * This header provides RAII-managed wrappers around SDL3's properties system,
 * which allows dynamic creation and management of named properties with
 * type-safe access and automatic cleanup.
 * 
 * Note: The properties API requires SDL 3.2.0 or later. This is enforced at
 * compile-time via static_assert and checked at runtime for dynamic linking.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/core/version.hh>
#include <string>
#include <optional>
#include <functional>
#include <memory>
#include <type_traits>
#include <variant>
#include <vector>

namespace sdlpp {
    // Compile-time check for properties API availability
    static_assert(version_info::features::has_properties,
                  "SDL Properties API requires SDL 3.2.0 or later. Please update your SDL headers.");

    /**
     * @brief Smart pointer type for SDL_PropertiesID with automatic cleanup
     */
    class properties_deleter {
        public:
            void operator()(SDL_PropertiesID id) const {
                if (id != 0) {
                    SDL_DestroyProperties(id);
                }
            }
    };

    using properties_ptr = std::unique_ptr <std::remove_pointer_t <SDL_PropertiesID>, properties_deleter>;

    /**
     * @brief Type-safe property value variant
     *
     * Represents any value that can be stored in SDL properties
     */
    using property_value = std::variant <
        std::nullptr_t, // For null/unset
        void*, // Pointer
        std::string, // String
        int64_t, // Number
        float, // Float
        bool // Boolean
    >;

    /**
     * @brief Property cleanup function type
     */
    using property_cleanup_func = std::function <void(void* userdata, void* value)>;

    /**
     * @brief RAII wrapper for SDL properties
     *
     * This class provides a safe, type-rich interface to SDL's properties system.
     * Properties are automatically destroyed when the object goes out of scope.
     *
     * Example usage:
     * @code
     * auto props = properties::create();
     * if (props) {
     *     props->set("name", "Player 1");
     *     props->set("score", 1000);
     *     props->set("position.x", 42.5f);
     *
     *     auto name = props->get_string("name");
     *     auto score = props->get_number("score");
     * }
     * @endcode
     */
    class properties {
        private:
            SDL_PropertiesID id;

            // Storage for cleanup callbacks
            struct cleanup_data {
                property_cleanup_func func;
                void* original_userdata;

                static void sdl_cleanup(void* userdata, void* value) {
                    auto* data = static_cast <cleanup_data*>(userdata);
                    if (data && data->func) {
                        data->func(data->original_userdata, value);
                    }
                    delete data;
                }
            };

        public:
            /**
             * @brief Default constructor - creates empty properties
             */
            properties()
                : id(0) {
            }

            /**
             * @brief Construct from existing properties ID
             * @param props_id Existing SDL_PropertiesID (takes ownership)
             */
            explicit properties(SDL_PropertiesID props_id)
                : id(props_id) {
            }

            /**
             * @brief Move constructor
             */
            properties(properties&& other) noexcept
                : id(other.id) {
                other.id = 0;
            }

            /**
             * @brief Move assignment operator
             */
            properties& operator=(properties&& other) noexcept {
                if (this != &other) {
                    if (id != 0) {
                        SDL_DestroyProperties(id);
                    }
                    id = other.id;
                    other.id = 0;
                }
                return *this;
            }

            /**
             * @brief Destructor
             */
            ~properties() {
                if (id != 0) {
                    SDL_DestroyProperties(id);
                }
            }

            // Delete copy operations
            properties(const properties&) = delete;
            properties& operator=(const properties&) = delete;

            /**
             * @brief Check if properties are valid
             */
            [[nodiscard]] bool is_valid() const { return id != 0; }
            [[nodiscard]] explicit operator bool() const { return is_valid(); }

            /**
             * @brief Get the underlying SDL_PropertiesID
             */
            [[nodiscard]] SDL_PropertiesID get_id() const { return id; }

            // Setters for different types

            /**
             * @brief Set a pointer property
             * @param name Property name
             * @param value Pointer value
             * @return true if successful
             */
            [[nodiscard]] bool set_pointer(const std::string& name, void* value) {
                return SDL_SetPointerProperty(id, name.c_str(), value);
            }

            /**
             * @brief Set a pointer property with cleanup callback
             * @param name Property name
             * @param value Pointer value
             * @param cleanup Cleanup function to call when property is destroyed
             * @param userdata User data for cleanup function
             * @return true if successful
             */
            [[nodiscard]] bool set_pointer_with_cleanup(const std::string& name,
                                                        void* value,
                                                        property_cleanup_func cleanup,
                                                        void* userdata = nullptr) {
                auto* data = new cleanup_data{std::move(cleanup), userdata};
                return SDL_SetPointerPropertyWithCleanup(id, name.c_str(), value,
                                                         cleanup_data::sdl_cleanup, data);
            }

            /**
             * @brief Set a string property
             * @param name Property name
             * @param value String value
             * @return true if successful
             */
            [[nodiscard]] bool set_string(const std::string& name, const std::string& value) {
                return SDL_SetStringProperty(id, name.c_str(), value.c_str());
            }

            /**
             * @brief Set a number property
             * @param name Property name
             * @param value Number value
             * @return true if successful
             */
            [[nodiscard]] bool set_number(const std::string& name, int64_t value) {
                return SDL_SetNumberProperty(id, name.c_str(), value);
            }

            /**
             * @brief Set a float property
             * @param name Property name
             * @param value Float value
             * @return true if successful
             */
            [[nodiscard]] bool set_float(const std::string& name, float value) {
                return SDL_SetFloatProperty(id, name.c_str(), value);
            }

            /**
             * @brief Set a boolean property
             * @param name Property name
             * @param value Boolean value
             * @return true if successful
             */
            [[nodiscard]] bool set_boolean(const std::string& name, bool value) {
                return SDL_SetBooleanProperty(id, name.c_str(), value);
            }

            // Generic setter using variant

            /**
             * @brief Set a property using variant type
             * @param name Property name
             * @param value Property value
             * @return true if successful
             */
            [[nodiscard]] bool set(const std::string& name, const property_value& value) {
                return std::visit([this, &name](const auto& v) -> bool {
                    using T = std::decay_t <decltype(v)>;
                    if constexpr (std::is_same_v <T, std::nullptr_t>) {
                        return set_pointer(name, nullptr);
                    } else if constexpr (std::is_same_v <T, void*>) {
                        return set_pointer(name, v);
                    } else if constexpr (std::is_same_v <T, std::string>) {
                        return set_string(name, v);
                    } else if constexpr (std::is_same_v <T, int64_t>) {
                        return set_number(name, v);
                    } else if constexpr (std::is_same_v <T, float>) {
                        return set_float(name, v);
                    } else if constexpr (std::is_same_v <T, bool>) {
                        return set_boolean(name, v);
                    }
                    return false;
                }, value);
            }

            // Convenience overloads for common types
            [[nodiscard]] bool set(const std::string& name, int value) {
                return set_number(name, static_cast <int64_t>(value));
            }

            [[nodiscard]] bool set(const std::string& name, double value) {
                return set_float(name, static_cast <float>(value));
            }

            // Getters for different types

            /**
             * @brief Get a pointer property
             * @param name Property name
             * @param default_value Default value if property not found
             * @return Property value or default
             */
            [[nodiscard]] void* get_pointer(const std::string& name, void* default_value = nullptr) const {
                return SDL_GetPointerProperty(id, name.c_str(), default_value);
            }

            /**
             * @brief Get a string property
             * @param name Property name
             * @param default_value Default value if property not found
             * @return Property value or default
             */
            [[nodiscard]] std::string get_string(const std::string& name,
                                                 const std::string& default_value = "") const {
                const char* value = SDL_GetStringProperty(id, name.c_str(), default_value.c_str());
                return value ? value : default_value;
            }

            /**
             * @brief Get a number property
             * @param name Property name
             * @param default_value Default value if property not found
             * @return Property value or default
             */
            [[nodiscard]] int64_t get_number(const std::string& name, int64_t default_value = 0) const {
                return SDL_GetNumberProperty(id, name.c_str(), default_value);
            }

            /**
             * @brief Get a float property
             * @param name Property name
             * @param default_value Default value if property not found
             * @return Property value or default
             */
            [[nodiscard]] float get_float(const std::string& name, float default_value = 0.0f) const {
                return SDL_GetFloatProperty(id, name.c_str(), default_value);
            }

            /**
             * @brief Get a boolean property
             * @param name Property name
             * @param default_value Default value if property not found
             * @return Property value or default
             */
            [[nodiscard]] bool get_boolean(const std::string& name, bool default_value = false) const {
                return SDL_GetBooleanProperty(id, name.c_str(), default_value);
            }

            /**
             * @brief Get property as variant
             * @param name Property name
             * @return Property value as variant, or nullopt if not found
             */
            [[nodiscard]] std::optional <property_value> get(const std::string& name) const {
                // SDL doesn't provide a way to query the type, so we need to try each type
                // This is a limitation of the SDL API

                // First check if property exists
                if (!has(name)) {
                    return std::nullopt;
                }

                // Try to determine type by getting different values
                // This is not perfect but works for most cases
                void* ptr = get_pointer(name);
                if (ptr != nullptr) {
                    return property_value{ptr};
                }

                // Try string (SDL returns "" for non-string properties)
                const char* str = SDL_GetStringProperty(id, name.c_str(), nullptr);
                if (str != nullptr) {
                    return property_value{std::string(str)};
                }

                // For numbers, we can't distinguish from 0, so we assume it's a number
                // if other types failed
                return property_value{get_number(name)};
            }

            /**
             * @brief Check if a property exists
             * @param name Property name
             * @return true if property exists
             */
            [[nodiscard]] bool has(const std::string& name) const {
                return SDL_HasProperty(id, name.c_str());
            }

            /**
             * @brief Clear (remove) a property
             * @param name Property name
             * @return true if successful
             */
            [[nodiscard]] bool clear(const std::string& name) {
                return SDL_ClearProperty(id, name.c_str());
            }

            /**
             * @brief Get property type hint
             * @param name Property name
             * @return Type of the property
             */
            [[nodiscard]] SDL_PropertyType get_type(const std::string& name) const {
                return SDL_GetPropertyType(id, name.c_str());
            }

            /**
             * @brief Lock properties for thread-safe access
             * @return true if successful
             */
            [[nodiscard]] bool lock() {
                return SDL_LockProperties(id);
            }

            /**
             * @brief Unlock properties
             */
            void unlock() {
                SDL_UnlockProperties(id);
            }

            /**
             * @brief RAII lock guard for properties
             */
            class lock_guard {
                private:
                    properties* props;
                    bool locked;

                public:
                    explicit lock_guard(properties& p)
                        : props(&p), locked(false) {
                        locked = props->lock();
                    }

                    ~lock_guard() {
                        if (locked && props) {
                            props->unlock();
                        }
                    }

                    [[nodiscard]] bool is_locked() const { return locked; }

                    // Non-copyable, non-movable
                    lock_guard(const lock_guard&) = delete;
                    lock_guard& operator=(const lock_guard&) = delete;
                    lock_guard(lock_guard&&) = delete;
                    lock_guard& operator=(lock_guard&&) = delete;
            };

            /**
             * @brief Enumerate all properties
             * @param callback Function called for each property
             * @return true if enumeration completed successfully
             */
            template<typename Func>
            bool enumerate(Func&& callback) const {
                struct enum_data {
                    Func* func;

                    static void sdl_callback(void* userdata, [[maybe_unused]] SDL_PropertiesID props,
                                             const char* name) {
                        auto* data = static_cast <enum_data*>(userdata);
                        (*data->func)(std::string(name));
                    }
                };

                Func func_copy = std::forward <Func>(callback);
                enum_data data{&func_copy};

                return SDL_EnumerateProperties(id, enum_data::sdl_callback, &data);
            }

            /**
             * @brief Get all property names
             * @return Vector of property names
             */
            [[nodiscard]] std::vector <std::string> get_names() const {
                std::vector <std::string> names;
                enumerate([&names](const std::string& name) {
                    names.push_back(name);
                });
                return names;
            }

            // Static factory methods

            /**
             * @brief Create a new properties group
             * @return Expected containing new properties, or error message
             */
            [[nodiscard]] static expected <properties, std::string> create() {
                // Runtime check (in case of dynamic linking with older SDL)
                if (!version_info::features::available_at_runtime(3, 2, 0)) {
                    return make_unexpected("SDL Properties API requires SDL 3.2.0 or later at runtime");
                }

                SDL_PropertiesID id = SDL_CreateProperties();
                if (id == 0) {
                    return make_unexpected(get_error());
                }
                return properties(id);
            }

            /**
             * @brief Get global properties
             * @return Global properties (not owned, do not destroy)
             */
            [[nodiscard]] static properties get_global() {
                // Global properties should not be destroyed, so we create a non-owning wrapper
                return properties(SDL_GetGlobalProperties());
            }
    };

    /**
     * @brief Property builder for fluent interface
     *
     * Allows building properties with a fluent API:
     * @code
     * auto props = property_builder()
     *     .add("name", "Player")
     *     .add("level", 10)
     *     .add("position.x", 100.0f)
     *     .build();
     * @endcode
     */
    class property_builder {
        private:
            std::vector <std::pair <std::string, property_value>> values;

        public:
            /**
             * @brief Add a property
             * @param name Property name
             * @param value Property value
             * @return Builder reference for chaining
             */
            template<typename T>
            property_builder& add(const std::string& name, T&& value) {
                if constexpr (std::is_same_v <std::decay_t <T>, const char*>) {
                    values.emplace_back(name, std::string(value));
                } else if constexpr (std::is_integral_v <std::decay_t <T>> &&
                                     !std::is_same_v <std::decay_t <T>, bool>) {
                    values.emplace_back(name, static_cast <int64_t>(value));
                } else if constexpr (std::is_floating_point_v <std::decay_t <T>>) {
                    values.emplace_back(name, static_cast <float>(value));
                } else {
                    values.emplace_back(name, std::forward <T>(value));
                }
                return *this;
            }

            /**
             * @brief Build the properties
             * @return Expected containing properties, or error message
             */
            [[nodiscard]] expected <properties, std::string> build() {
                auto props = properties::create();
                if (!props) {
                    return props;
                }

                for (const auto& [name, value] : values) {
                    if (!props->set(name, value)) {
                        return make_unexpected("Failed to set property: " + name);
                    }
                }

                return props;
            }
    };

    /**
     * @brief Type-safe property accessor template
     *
     * Provides compile-time type checking for property access:
     * @code
     * property_accessor<int> score(props, "score");
     * score = 100;
     * int value = score;
     * @endcode
     */
    template<typename T>
    class property_accessor {
        private:
            properties* props;
            std::string name;
            T default_value;

        public:
            property_accessor(properties& p, const std::string& n, T def = T{})
                : props(&p), name(n), default_value(def) {
            }

            /**
             * @brief Get property value
             */
            [[nodiscard]] operator T() const {
                if constexpr (std::is_same_v <T, std::string>) {
                    return props->get_string(name, default_value);
                } else if constexpr (std::is_integral_v <T> && !std::is_same_v <T, bool>) {
                    return static_cast <T>(props->get_number(name, default_value));
                } else if constexpr (std::is_floating_point_v <T>) {
                    return static_cast <T>(props->get_float(name, default_value));
                } else if constexpr (std::is_same_v <T, bool>) {
                    return props->get_boolean(name, default_value);
                } else if constexpr (std::is_pointer_v <T>) {
                    return static_cast <T>(props->get_pointer(name, default_value));
                }
            }

            /**
             * @brief Set property value
             */
            property_accessor& operator=(const T& value) {
                if constexpr (std::is_same_v <T, std::string>) {
                    [[maybe_unused]] auto set_result = props->set_string(name, value);
                } else if constexpr (std::is_integral_v <T> && !std::is_same_v <T, bool>) {
                    [[maybe_unused]] auto set_result = props->set_number(name, value);
                } else if constexpr (std::is_floating_point_v <T>) {
                    [[maybe_unused]] auto set_result = props->set_float(name, value);
                } else if constexpr (std::is_same_v <T, bool>) {
                    [[maybe_unused]] auto set_result = props->set_boolean(name, value);
                } else if constexpr (std::is_pointer_v <T>) {
                    [[maybe_unused]] auto set_result = props->set_pointer(name, value);
                }
                return *this;
            }

            /**
             * @brief Check if property exists
             */
            [[nodiscard]] bool exists() const {
                return props->has(name);
            }

            /**
             * @brief Clear the property
             */
            bool clear() {
                return props->clear(name);
            }
    };
} // namespace sdlpp
