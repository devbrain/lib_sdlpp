/**
 * @file filesystem.hh
 * @brief Filesystem operations wrapper
 */
#pragma once

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/detail/pointer.hh>

#include <sdlpp/detail/expected.hh>

#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace sdlpp {
    enum class folder_type {
        home = SDL_FOLDER_HOME,
        desktop = SDL_FOLDER_DESKTOP,
        documents = SDL_FOLDER_DOCUMENTS,
        downloads = SDL_FOLDER_DOWNLOADS,
        music = SDL_FOLDER_MUSIC,
        pictures = SDL_FOLDER_PICTURES,
        publicshare = SDL_FOLDER_PUBLICSHARE,
        savedgames = SDL_FOLDER_SAVEDGAMES,
        screenshots = SDL_FOLDER_SCREENSHOTS,
        templates = SDL_FOLDER_TEMPLATES,
        videos = SDL_FOLDER_VIDEOS,
    };

    enum class path_type {
        none = SDL_PATHTYPE_NONE,
        file = SDL_PATHTYPE_FILE,
        directory = SDL_PATHTYPE_DIRECTORY,
        other = SDL_PATHTYPE_OTHER,
    };

    struct path_info {
        path_type type{path_type::none};
        std::uint64_t size{0};
        std::int64_t create_time{0};
        std::int64_t modify_time{0};
        std::int64_t access_time{0};

        [[nodiscard]] static path_info from_sdl(const SDL_PathInfo& info) noexcept {
            return {
                .type = static_cast <path_type>(info.type),
                .size = info.size,
                .create_time = info.create_time,
                .modify_time = info.modify_time,
                .access_time = info.access_time,
            };
        }
    };

    namespace filesystem {
        [[nodiscard]] inline expected <std::filesystem::path, std::string> get_base_path() noexcept {
            const char* path = SDL_GetBasePath();
            if (!path) {
                return make_unexpectedf(SDL_GetError());
            }
            return std::filesystem::path{path};
        }

        [[nodiscard]] inline expected <std::filesystem::path, std::string> get_pref_path(
            const std::string& org, const std::string& app) noexcept {
            using path_ptr = pointer <char, SDL_free>;

            path_ptr path{SDL_GetPrefPath(org.c_str(), app.c_str())};
            if (!path) {
                return make_unexpectedf(SDL_GetError());
            }
            return std::filesystem::path{path.get()};
        }

        [[nodiscard]] inline expected <std::filesystem::path, std::string> get_user_folder(
            folder_type folder) noexcept {
            const char* path = SDL_GetUserFolder(static_cast <SDL_Folder>(folder));
            if (!path) {
                return make_unexpectedf(SDL_GetError());
            }
            return std::filesystem::path{path};
        }

        [[nodiscard]] inline expected <std::filesystem::path, std::string> get_current_directory() noexcept {
            using path_ptr = pointer <char, SDL_free>;

            path_ptr path{SDL_GetCurrentDirectory()};
            if (!path) {
                return make_unexpectedf(SDL_GetError());
            }
            return std::filesystem::path{path.get()};
        }

        [[nodiscard]] inline expected <void, std::string> create_directory(
            const std::filesystem::path& path) noexcept {
            if (!SDL_CreateDirectory(path.string().c_str())) {
                return make_unexpectedf(SDL_GetError());
            }
            return {};
        }

        [[nodiscard]] inline expected <void, std::string> remove_path(
            const std::filesystem::path& path) noexcept {
            if (!SDL_RemovePath(path.string().c_str())) {
                return make_unexpectedf(SDL_GetError());
            }
            return {};
        }

        [[nodiscard]] inline expected <void, std::string> rename_path(
            const std::filesystem::path& old_path,
            const std::filesystem::path& new_path) noexcept {
            if (!SDL_RenamePath(old_path.string().c_str(), new_path.string().c_str())) {
                return make_unexpectedf(SDL_GetError());
            }
            return {};
        }

        [[nodiscard]] inline expected <void, std::string> copy_file(
            const std::filesystem::path& old_path,
            const std::filesystem::path& new_path) noexcept {
            if (!SDL_CopyFile(old_path.string().c_str(), new_path.string().c_str())) {
                return make_unexpectedf(SDL_GetError());
            }
            return {};
        }

        [[nodiscard]] inline expected <path_info, std::string> get_path_info(
            const std::filesystem::path& path) noexcept {
            SDL_PathInfo info;
            if (!SDL_GetPathInfo(path.string().c_str(), &info)) {
                return make_unexpectedf(SDL_GetError());
            }
            return path_info::from_sdl(info);
        }

        enum class glob_flags : std::uint32_t {
            none = 0,
            case_insensitive = SDL_GLOB_CASEINSENSITIVE,
        };

        [[nodiscard]] inline constexpr glob_flags operator|(glob_flags lhs, glob_flags rhs) noexcept {
            return static_cast <glob_flags>(
                static_cast <std::uint32_t>(lhs) | static_cast <std::uint32_t>(rhs)
            );
        }

        [[nodiscard]] inline constexpr glob_flags operator&(glob_flags lhs, glob_flags rhs) noexcept {
            return static_cast <glob_flags>(
                static_cast <std::uint32_t>(lhs) & static_cast <std::uint32_t>(rhs)
            );
        }

        class glob_result {
            public:
                using paths_ptr = std::unique_ptr <char*[], void(*)(char**)>;

                glob_result() noexcept
                    : paths_{nullptr, &free_paths}, count_{0} {
                }

                glob_result(char** paths, int count) noexcept
                    : paths_{paths, &free_paths}, count_{count > 0 ? static_cast<size_t>(count) : 0} {
                }

                glob_result(glob_result&&) noexcept = default;
                glob_result& operator=(glob_result&&) noexcept = default;

                glob_result(const glob_result&) = delete;
                glob_result& operator=(const glob_result&) = delete;

                [[nodiscard]] std::vector <std::filesystem::path> to_vector() const {
                    std::vector <std::filesystem::path> paths_vec;
                    paths_vec.reserve(count_);

                    char** paths = paths_.get();
                    for (size_t i = 0; i < count_; ++i) {
                        paths_vec.emplace_back(paths[i]);
                    }

                    return paths_vec;
                }

                [[nodiscard]] size_t size() const noexcept { return count_; }
                [[nodiscard]] bool empty() const noexcept { return count_ == 0; }

            private:
                static void free_paths(char** paths) noexcept {
                    if (paths) {
                        SDL_free(paths);
                    }
                }

                paths_ptr paths_;
                size_t count_;
        };

        [[nodiscard]] inline expected <glob_result, std::string> glob_directory(
            const std::filesystem::path& path,
            const std::string& pattern = "*",
            glob_flags flags = glob_flags::none) noexcept {
            int count = 0;
            char** paths = SDL_GlobDirectory(
                path.string().c_str(),
                pattern.c_str(),
                static_cast <SDL_GlobFlags>(flags),
                &count
            );

            if (!paths && count < 0) {
                return make_unexpectedf(SDL_GetError());
            }

            return glob_result{paths, count};
        }

        using enumerate_callback = std::function <SDL_EnumerationResult(const std::string& name)>;

        namespace detail {
            inline SDL_EnumerationResult enumerate_callback_wrapper(
                void* userdata, [[maybe_unused]] const char* dirname, const char* fname) {
                auto* callback = static_cast <enumerate_callback*>(userdata);
                return (*callback)(fname);
            }
        } // namespace detail

        [[nodiscard]] inline expected <void, std::string> enumerate_directory(
            const std::filesystem::path& path,
            enumerate_callback callback) noexcept {
            if (!SDL_EnumerateDirectory(
                path.string().c_str(),
                detail::enumerate_callback_wrapper,
                &callback)) {
                return make_unexpectedf(SDL_GetError());
            }
            return {};
        }
    } // namespace filesystem
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for folder_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, folder_type value);

/**
 * @brief Stream input operator for folder_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, folder_type& value);

/**
 * @brief Stream output operator for path_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, path_type value);

/**
 * @brief Stream input operator for path_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, path_type& value);

}