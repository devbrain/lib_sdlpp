#pragma once

/**
 * @file shared_object.hh
 * @brief Dynamic library loading and function resolution
 * 
 * This header provides RAII wrappers for loading shared libraries
 * (DLLs on Windows, .so on Linux, .dylib on macOS) and resolving
 * symbols from them at runtime.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <string>
#include <string_view>
#include <filesystem>
#include <type_traits>

namespace sdlpp {
    /**
     * @brief RAII wrapper for dynamically loaded shared objects
     *
     * This class provides a safe interface for loading shared libraries
     * and resolving symbols from them. The library is automatically
     * unloaded when the object is destroyed.
     *
     * Example usage:
     * @code
     * auto lib_result = sdlpp::shared_object::load("mylib.so");
     * if (!lib_result) {
     *     // Handle error
     *     return;
     * }
     * auto& lib = lib_result.value();
     *
     * // Get a function pointer
     * auto func_ptr = lib.get_function<int(const char*)>("my_function");
     * if (func_ptr) {
     *     int result = func_ptr.value()("hello");
     * }
     *
     * // Get a data pointer
     * auto data_ptr = lib.get_data<int>("my_global_var");
     * if (data_ptr) {
     *     int value = *data_ptr.value();
     * }
     * @endcode
     */
    class shared_object {
        public:
            /**
             * @brief Default constructor creates an invalid shared object
             */
            shared_object() noexcept = default;

            /**
             * @brief Move constructor
             */
            shared_object(shared_object&& other) noexcept
                : handle_(other.handle_) {
                other.handle_ = nullptr;
            }

            /**
             * @brief Move assignment operator
             */
            shared_object& operator=(shared_object&& other) noexcept {
                if (this != &other) {
                    reset();
                    handle_ = other.handle_;
                    other.handle_ = nullptr;
                }
                return *this;
            }

            /**
             * @brief Destructor unloads the shared object
             */
            ~shared_object() {
                reset();
            }

            // Delete copy operations
            shared_object(const shared_object&) = delete;
            shared_object& operator=(const shared_object&) = delete;

            /**
             * @brief Load a shared object from file
             * @param path Path to the shared library file
             * @return Expected containing the shared object or error message
             * @note The path can be a full path or just a library name.
             *       If just a name is provided, the system will search
             *       standard locations.
             */
            [[nodiscard]] static expected <shared_object, std::string> load(
                const std::filesystem::path& path) noexcept {
                SDL_SharedObject* handle = SDL_LoadObject(path.string().c_str());
                if (!handle) {
                    return make_unexpectedf(get_error());
                }

                shared_object obj;
                obj.handle_ = handle;
                return obj;
            }

            /**
             * @brief Load a shared object from file
             * @param name Library name (without extension)
             * @return Expected containing the shared object or error message
             * @note This is a convenience overload for string names
             */
            [[nodiscard]] static expected <shared_object, std::string> load(
                const char* name) noexcept {
                return load(std::filesystem::path(name));
            }

            /**
             * @brief Get a function pointer from the shared object
             * @tparam Func Function type (e.g., int(const char*))
             * @param name Symbol name of the function
             * @return Expected containing function pointer or error message
             * @note The function type must match the actual function signature
             */
            template<typename Func>
            [[nodiscard]] expected <Func*, std::string> get_function(
                const std::string& name) const noexcept {
                static_assert(std::is_function_v <Func>,
                              "Template parameter must be a function type");

                if (!handle_) {
                    return make_unexpectedf("Shared object not loaded");
                }

                SDL_FunctionPointer symbol = SDL_LoadFunction(handle_, name.c_str());
                if (!symbol) {
                    return make_unexpectedf(get_error());
                }

                return reinterpret_cast <Func*>(reinterpret_cast <void*>(symbol));
            }

            /**
             * @brief Get a data pointer from the shared object
             * @tparam T Data type
             * @param name Symbol name of the data
             * @return Expected containing data pointer or error message
             * @note The data type must match the actual data type
             */
            template<typename T>
            [[nodiscard]] expected <T*, std::string> get_data(
                const std::string& name) const noexcept {
                if (!handle_) {
                    return make_unexpectedf("Shared object not loaded");
                }

                SDL_FunctionPointer symbol = SDL_LoadFunction(handle_, name.c_str());
                if (!symbol) {
                    return make_unexpectedf(get_error());
                }

                return reinterpret_cast <T*>(reinterpret_cast <void*>(symbol));
            }

            /**
             * @brief Get a raw symbol pointer from the shared object
             * @param name Symbol name
             * @return Expected containing void pointer or error message
             * @note This is the low-level interface; prefer get_function or get_data
             */
            [[nodiscard]] expected <void*, std::string> get_symbol(
                const std::string& name) const noexcept {
                if (!handle_) {
                    return make_unexpectedf("Shared object not loaded");
                }

                SDL_FunctionPointer symbol = SDL_LoadFunction(handle_, name.c_str());
                if (!symbol) {
                    return make_unexpectedf(get_error());
                }

                return reinterpret_cast <void*>(symbol);
            }

            /**
             * @brief Check if a symbol exists in the shared object
             * @param name Symbol name
             * @return true if the symbol exists
             */
            [[nodiscard]] bool has_symbol(const std::string& name) const noexcept {
                if (!handle_) {
                    return false;
                }

                SDL_FunctionPointer symbol = SDL_LoadFunction(handle_, name.c_str());
                return symbol != nullptr;
            }

            /**
             * @brief Check if the shared object is valid
             * @return true if a library is loaded
             */
            [[nodiscard]] bool is_valid() const noexcept {
                return handle_ != nullptr;
            }

            /**
             * @brief Check if the shared object is valid (explicit bool conversion)
             */
            [[nodiscard]] explicit operator bool() const noexcept {
                return is_valid();
            }

            /**
             * @brief Unload the shared object
             */
            void reset() noexcept {
                if (handle_) {
                    SDL_UnloadObject(handle_);
                    handle_ = nullptr;
                }
            }

            /**
             * @brief Release ownership of the handle without unloading
             * @return The raw handle (caller takes ownership)
             */
            [[nodiscard]] SDL_SharedObject* release() noexcept {
                SDL_SharedObject* temp = handle_;
                handle_ = nullptr;
                return temp;
            }

            /**
             * @brief Get the raw handle (does not transfer ownership)
             * @return The raw handle
             */
            [[nodiscard]] SDL_SharedObject* get() const noexcept {
                return handle_;
            }

        private:
            SDL_SharedObject* handle_ = nullptr;
    };

    /**
     * @brief Helper class for automatic symbol resolution
     *
     * This class template helps with resolving multiple symbols from
     * a shared object and storing them as member function pointers.
     *
     * Example usage:
     * @code
     * struct MyAPI : sdlpp::symbol_resolver<MyAPI> {
     *     using init_func = int();
     *     using process_func = void(const char*, int);
     *
     *     init_func* init = nullptr;
     *     process_func* process = nullptr;
     *
     *     static constexpr auto symbols() {
     *         return std::make_tuple(
     *             bind("init", &MyAPI::init),
     *             bind("process", &MyAPI::process)
     *         );
     *     }
     * };
     *
     * MyAPI api;
     * auto result = api.load_from(shared_obj);
     * if (result) {
     *     api.init();
     *     api.process("data", 42);
     * }
     * @endcode
     */
    template<typename Derived>
    class symbol_resolver {
        public:
            /**
             * @brief Symbol binding information
             */
            template<typename T>
            struct symbol_binding {
                const char* name;
                T Derived::* member;
            };

            /**
             * @brief Helper to create symbol_binding with type deduction
             * @note Use this instead of direct construction for GCC 11 compatibility
             */
            template<typename T>
            static constexpr symbol_binding<T> bind(const char* name, T Derived::* member) noexcept {
                return symbol_binding<T>{name, member};
            }

            /**
             * @brief Load all symbols from a shared object
             * @param obj The shared object to load from
             * @return Expected containing success or error message
             */
            [[nodiscard]] expected <void, std::string> load_from(
                const shared_object& obj) noexcept {
                if (!obj.is_valid()) {
                    return make_unexpectedf("Invalid shared object");
                }

                return load_symbols(obj, std::make_index_sequence <
                                        std::tuple_size_v <decltype(Derived::symbols())>>{});
            }

        private:
            template<std::size_t... Is>
            [[nodiscard]] expected <void, std::string> load_symbols(
                const shared_object& obj,
                std::index_sequence <Is...>) noexcept {
                auto symbols = Derived::symbols();
                std::string error;

                bool success = (load_symbol(obj, std::get <Is>(symbols), error) && ...);

                if (!success) {
                    return make_unexpectedf(error);
                }

                return {};
            }

            template<typename T>
            bool load_symbol(const shared_object& obj,
                             const symbol_binding <T>& binding,
                             std::string& error) noexcept {
                auto symbol = obj.get_symbol(binding.name);
                if (!symbol) {
                    error = "Failed to load symbol '" + std::string(binding.name) +
                            "': " + symbol.error();
                    return false;
                }

                auto* derived = static_cast <Derived*>(this);
                derived->*(binding.member) = reinterpret_cast <
                    std::remove_reference_t <decltype(derived->*(binding.member))>>(
                    symbol.value());

                return true;
            }
    };

    /**
     * @brief Convenience function to create a shared object
     * @param path Path to the shared library
     * @return Expected containing shared object or error
     */
    [[nodiscard]] inline expected <shared_object, std::string> load_shared_object(
        const std::filesystem::path& path) noexcept {
        return shared_object::load(path);
    }

    /**
     * @brief Convenience function to create a shared object
     * @param name Library name
     * @return Expected containing shared object or error
     */
    [[nodiscard]] inline expected <shared_object, std::string> load_shared_object(
        const char* name) noexcept {
        return shared_object::load(name);
    }
} // namespace sdlpp
