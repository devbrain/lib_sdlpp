/**
 * @file storage.hh
 * @brief Persistent storage for application data
 */
#pragma once

#include <sdlpp/core/sdl.hh>

#include <sdlpp/detail/pointer.hh>
#include <sdlpp/io/filesystem.hh>
#include <sdlpp/core/error.hh>

#include <sdlpp/detail/expected.hh>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace sdlpp {
    // Forward declarations
    class storage;

    // Storage glob flags
    enum class glob_flags : std::uint32_t {
        none = 0,
        case_insensitive = SDL_GLOB_CASEINSENSITIVE,
    };

    [[nodiscard]] constexpr glob_flags operator|(glob_flags lhs, glob_flags rhs) noexcept {
        return static_cast <glob_flags>(
            static_cast <std::uint32_t>(lhs) | static_cast <std::uint32_t>(rhs)
        );
    }

    [[nodiscard]] constexpr glob_flags operator&(glob_flags lhs, glob_flags rhs) noexcept {
        return static_cast <glob_flags>(
            static_cast <std::uint32_t>(lhs) & static_cast <std::uint32_t>(rhs)
        );
    }

    [[nodiscard]] constexpr bool has_flag(glob_flags flags, glob_flags flag) noexcept {
        return (flags & flag) == flag;
    }

    // Storage enumerate callback
    using enumerate_callback = std::function <SDL_EnumerationResult(const std::string& name)>;

    // Storage interface for custom implementations
    struct storage_interface {
        using close_fn = bool(*)(void* userdata);
        using ready_fn = bool(*)(void* userdata);
        using enumerate_fn = bool(*)(void* userdata, const char* path, SDL_EnumerateDirectoryCallback callback,
                                     void* callback_userdata);
        using info_fn = bool(*)(void* userdata, const char* path, SDL_PathInfo* info);
        using read_file_fn = bool(*)(void* userdata, const char* path, void* destination, std::uint64_t length);
        using write_file_fn = bool(*)(void* userdata, const char* path, const void* source, std::uint64_t length);
        using mkdir_fn = bool(*)(void* userdata, const char* path);
        using remove_fn = bool(*)(void* userdata, const char* path);
        using rename_fn = bool(*)(void* userdata, const char* oldpath, const char* newpath);
        using copy_fn = bool(*)(void* userdata, const char* oldpath, const char* newpath);
        using space_remaining_fn = std::uint64_t(*)(void* userdata);

        close_fn close = nullptr;
        ready_fn ready = nullptr;
        enumerate_fn enumerate = nullptr;
        info_fn info = nullptr;
        read_file_fn read_file = nullptr;
        write_file_fn write_file = nullptr;
        mkdir_fn mkdir = nullptr;
        remove_fn remove = nullptr;
        rename_fn rename = nullptr;
        copy_fn copy = nullptr;
        space_remaining_fn space_remaining = nullptr;

        [[nodiscard]] SDL_StorageInterface to_sdl() const noexcept {
            return SDL_StorageInterface{
                .version = 1,
                .close = close,
                .ready = ready,
                .enumerate = enumerate,
                .info = info,
                .read_file = read_file,
                .write_file = write_file,
                .mkdir = mkdir,
                .remove = remove,
                .rename = rename,
                .copy = copy,
                .space_remaining = space_remaining
            };
        }
    };

    // RAII storage container
    class storage {
        public:
            using storage_ptr = pointer <SDL_Storage, SDL_CloseStorage>;

            // Constructors
            storage() noexcept = default;

            explicit storage(storage_ptr ptr) noexcept
                : ptr_(std::move(ptr)) {
            }

            // Factory methods for different storage types

            /**
             * @brief Open title storage (read-only game content)
             * @param override Optional override path for the title storage root
             * @param props Optional properties for storage configuration
             * @return Storage instance or error
             */
            [[nodiscard]] static expected <storage, std::string> open_title(
                const std::string_view& override = {},
                SDL_PropertiesID props = 0) noexcept {
                const char* path = override.empty() ? nullptr : override.data();
                auto* raw = SDL_OpenTitleStorage(path, props);
                if (!raw) {
                    return make_unexpected(get_error());
                }
                return storage{storage_ptr{raw}};
            }

            /**
             * @brief Open user storage (read/write user data)
             * @param org Organization name
             * @param app Application name
             * @param props Optional properties for storage configuration
             * @return Storage instance or error
             */
            [[nodiscard]] static expected <storage, std::string> open_user(
                const std::string_view& org,
                const std::string_view& app,
                SDL_PropertiesID props = 0) noexcept {
                auto* raw = SDL_OpenUserStorage(org.data(), app.data(), props);
                if (!raw) {
                    return make_unexpected(get_error());
                }
                return storage{storage_ptr{raw}};
            }

            /**
             * @brief Open file storage (general filesystem access)
             * @param path Path to the storage root
             * @return Storage instance or error
             */
            [[nodiscard]] static expected <storage, std::string> open_file(
                const std::filesystem::path& path) noexcept {
                auto* raw = SDL_OpenFileStorage(path.string().c_str());
                if (!raw) {
                    return make_unexpected(get_error());
                }
                return storage{storage_ptr{raw}};
            }

            /**
             * @brief Open custom storage with user-defined interface
             * @param iface Storage interface implementation
             * @param userdata User data passed to interface callbacks
             * @return Storage instance or error
             */
            [[nodiscard]] static expected <storage, std::string> open_custom(
                const storage_interface& iface,
                void* userdata) noexcept {
                auto sdl_iface = iface.to_sdl();
                auto* raw = SDL_OpenStorage(&sdl_iface, userdata);
                if (!raw) {
                    return make_unexpected(get_error());
                }
                return storage{storage_ptr{raw}};
            }

            // Storage operations

            /**
             * @brief Check if storage is ready for use
             * @return true if ready, false otherwise
             */
            [[nodiscard]] bool is_ready() const noexcept {
                return ptr_ && SDL_StorageReady(ptr_.get());
            }

            /**
             * @brief Get remaining storage space
             * @return Remaining space in bytes, or 0 if unavailable
             */
            [[nodiscard]] std::uint64_t get_space_remaining() const noexcept {
                return ptr_ ? SDL_GetStorageSpaceRemaining(ptr_.get()) : 0;
            }

            /**
             * @brief Get the native SDL_Storage handle
             * @return Raw pointer to SDL_Storage
             */
            [[nodiscard]] SDL_Storage* native_handle() const noexcept {
                return ptr_.get();
            }

            // File operations

            /**
             * @brief Get file size
             * @param path Path to the file
             * @return File size or error
             */
            [[nodiscard]] expected <std::uint64_t, std::string> get_file_size(
                const std::string_view& path) const noexcept {
                if (!ptr_) {
                    return make_unexpected("Storage not initialized");
                }

                std::uint64_t size = 0;
                if (!SDL_GetStorageFileSize(ptr_.get(), path.data(), &size)) {
                    return make_unexpected(get_error());
                }
                return size;
            }

            /**
             * @brief Read entire file contents
             * @param path Path to the file
             * @return File contents or error
             */
            [[nodiscard]] expected <std::vector <std::uint8_t>, std::string> read_file(
                const std::string_view& path) const noexcept {
                auto size_result = get_file_size(path);
                if (!size_result) {
                    return make_unexpected(size_result.error());
                }

                std::vector <std::uint8_t> buffer(size_result.value());
                if (!buffer.empty()) {
                    if (!SDL_ReadStorageFile(ptr_.get(), path.data(), buffer.data(), buffer.size())) {
                        return make_unexpected(get_error());
                    }
                }
                return buffer;
            }

            /**
             * @brief Read file into provided buffer
             * @param path Path to the file
             * @param buffer Destination buffer
             * @return true on success, false on error
             */
            [[nodiscard]] bool read_file_into(
                const std::string_view& path,
                std::span <std::uint8_t> buffer) const noexcept {
                return ptr_ && SDL_ReadStorageFile(ptr_.get(), path.data(),
                                                   buffer.data(), buffer.size());
            }

            /**
             * @brief Write data to file
             * @param path Path to the file
             * @param data Data to write
             * @return true on success, false on error
             */
            [[nodiscard]] bool write_file(
                const std::string_view& path,
                std::span <const std::uint8_t> data) const noexcept {
                return ptr_ && SDL_WriteStorageFile(ptr_.get(), path.data(),
                                                    data.data(), data.size());
            }

            /**
             * @brief Write string to file
             * @param path Path to the file
             * @param content String content to write
             * @return true on success, false on error
             */
            [[nodiscard]] bool write_file(
                const std::string_view& path,
                const std::string_view& content) const noexcept {
                return write_file(path, std::span{
                                      reinterpret_cast <const std::uint8_t*>(content.data()),
                                      content.size()
                                  });
            }

            // Directory operations

            /**
             * @brief Create directory
             * @param path Path to the directory
             * @return true on success, false on error
             */
            [[nodiscard]] bool create_directory(const std::string_view& path) const noexcept {
                return ptr_ && SDL_CreateStorageDirectory(ptr_.get(), path.data());
            }

            /**
             * @brief Enumerate directory contents
             * @param path Path to the directory
             * @param callback Callback function called for each entry
             * @return true on success, false on error
             */
            [[nodiscard]] bool enumerate_directory(
                const std::string_view& path,
                enumerate_callback callback) const noexcept {
                if (!ptr_ || !callback) {
                    return false;
                }

                struct callback_data {
                    enumerate_callback* cb;
                };

                callback_data data{&callback};

                auto enum_cb = [](void* userdata, [[maybe_unused]] const char* dirname,
                                  const char* fname) -> SDL_EnumerationResult {
                    auto* cb_data = static_cast <callback_data*>(userdata);
                    return (*cb_data->cb)(fname ? fname : "");
                };

                return SDL_EnumerateStorageDirectory(ptr_.get(), path.data(), enum_cb, &data);
            }

            /**
             * @brief List directory contents
             * @param path Path to the directory
             * @return List of entries or error
             */
            [[nodiscard]] expected <std::vector <std::string>, std::string> list_directory(
                const std::string_view& path) const noexcept {
                std::vector <std::string> entries;

                bool success = enumerate_directory(path, [&entries](const std::string& name) {
                    entries.push_back(name);
                    return SDL_ENUM_CONTINUE;
                });

                if (!success) {
                    return make_unexpected(get_error());
                }
                return entries;
            }

            /**
             * @brief Glob directory contents with pattern matching
             * @param path Path to the directory
             * @param pattern Glob pattern (e.g., "*.txt")
             * @param flags Glob flags
             * @return List of matching entries or error
             */
            [[nodiscard]] expected <std::vector <std::string>, std::string> glob_directory(
                const std::string_view& path,
                const std::string_view& pattern,
                glob_flags flags = glob_flags::none) const noexcept {
                if (!ptr_) {
                    return make_unexpected("Storage not initialized");
                }

                int count = 0;
                auto** paths = SDL_GlobStorageDirectory(ptr_.get(), path.data(), pattern.data(),
                                                        static_cast <SDL_GlobFlags>(flags), &count);

                if (!paths && count < 0) {
                    return make_unexpected(get_error());
                }

                std::vector <std::string> glob_results;
                if (paths && count > 0) {
                    glob_results.reserve(static_cast <std::size_t>(count));
                    // SDL returns a single allocation, iterate through the array
                    for (int i = 0; i < count; ++i) {
                        if (paths[i]) {
                            glob_results.emplace_back(paths[i]);
                        }
                    }
                    SDL_free(paths);
                }
                return glob_results;
            }

            // Path operations

            /**
             * @brief Get path information
             * @param path Path to query
             * @return Path information or error
             */
            [[nodiscard]] expected <path_info, std::string> get_path_info(
                const std::string_view& path) const noexcept {
                if (!ptr_) {
                    return make_unexpected("Storage not initialized");
                }

                SDL_PathInfo info{};
                if (!SDL_GetStoragePathInfo(ptr_.get(), path.data(), &info)) {
                    return make_unexpected(get_error());
                }
                return path_info::from_sdl(info);
            }

            /**
             * @brief Remove file or directory
             * @param path Path to remove
             * @return true on success, false on error
             */
            [[nodiscard]] bool remove_path(const std::string_view& path) const noexcept {
                return ptr_ && SDL_RemoveStoragePath(ptr_.get(), path.data());
            }

            /**
             * @brief Rename file or directory
             * @param old_path Current path
             * @param new_path New path
             * @return true on success, false on error
             */
            [[nodiscard]] bool rename_path(
                const std::string_view& old_path,
                const std::string_view& new_path) const noexcept {
                return ptr_ && SDL_RenameStoragePath(ptr_.get(), old_path.data(), new_path.data());
            }

            /**
             * @brief Copy file
             * @param src_path Source path
             * @param dst_path Destination path
             * @return true on success, false on error
             */
            [[nodiscard]] bool copy_file(
                const std::string_view& src_path,
                const std::string_view& dst_path) const noexcept {
                return ptr_ && SDL_CopyStorageFile(ptr_.get(), src_path.data(), dst_path.data());
            }

            // Path type checking helpers

            /**
             * @brief Check if path exists
             * @param path Path to check
             * @return true if exists, false otherwise
             */
            [[nodiscard]] bool exists(const std::string_view& path) const noexcept {
                auto info = get_path_info(path);
                return info.has_value() && info->type != path_type::none;
            }

            /**
             * @brief Check if path is a file
             * @param path Path to check
             * @return true if file, false otherwise
             */
            [[nodiscard]] bool is_file(const std::string_view& path) const noexcept {
                auto info = get_path_info(path);
                return info.has_value() && info->type == path_type::file;
            }

            /**
             * @brief Check if path is a directory
             * @param path Path to check
             * @return true if directory, false otherwise
             */
            [[nodiscard]] bool is_directory(const std::string_view& path) const noexcept {
                auto info = get_path_info(path);
                return info.has_value() && info->type == path_type::directory;
            }

            // Accessors
            [[nodiscard]] SDL_Storage* get() const noexcept { return ptr_.get(); }
            [[nodiscard]] explicit operator bool() const noexcept { return static_cast <bool>(ptr_); }

        private:
            storage_ptr ptr_;
    };
} // namespace sdlpp
