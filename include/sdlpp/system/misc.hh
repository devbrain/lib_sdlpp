#pragma once

/**
 * @file misc.hh
 * @brief Miscellaneous SDL functionality
 * 
 * This header provides access to SDL functionality that doesn't fit
 * into other categories, currently just URL/URI opening.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <string>
#include <string_view>

namespace sdlpp {
    /**
     * @brief Open a URL/URI in the appropriate external application
     *
     * Opens a URL in the default application registered for that protocol.
     * This is typically used to:
     * - Open web pages in the default browser
     * - Open file:// URLs in the file manager
     * - Open mailto: links in the email client
     * - Launch other protocol handlers
     *
     * @param url The URL to open (e.g., "https://example.com", "file:///home/user", "mailto:user@example.com")
     * @return Expected containing success or error message
     *
     * @note This may cause your application to lose focus or be backgrounded, especially on mobile platforms.
     * @note Success only means SDL successfully requested the OS to open the URL, not that the URL actually loaded.
     * @note Platform behavior varies significantly. Some platforms may not support certain URL types.
     *
     * Example:
     * @code
     * // Open a website
     * auto result = sdlpp::open_url("https://www.libsdl.org");
     * if (!result) {
     *     std::cerr << "Failed to open URL: " << result.error() << "\n";
     * }
     *
     * // Open a local file or directory
     * auto result2 = sdlpp::open_url("file:///home/user/documents");
     *
     * // Open email client
     * auto result3 = sdlpp::open_url("mailto:support@example.com?subject=Help");
     * @endcode
     */
    [[nodiscard]] inline expected <void, std::string> open_url(std::string_view url) {
        if (SDL_OpenURL(std::string(url).c_str())) {
            return {};
        }
        return make_unexpectedf(get_error());
    }

    /**
     * @brief URL/URI helper utilities
     */
    namespace url {
        /**
         * @brief Check if a URL string starts with a known protocol
         * @param url The URL to check
         * @return true if the URL has a protocol prefix
         *
         * Example:
         * @code
         * if (!sdlpp::url::has_protocol("example.com")) {
         *     // Add https:// prefix
         * }
         * @endcode
         */
        [[nodiscard]] inline bool has_protocol(std::string_view url) noexcept {
            // Common protocols
            const char* protocols[] = {
                "http://", "https://", "file://", "ftp://", "ftps://",
                "mailto:", "tel:", "sms:", "geo:", "maps:",
                "steam://", "discord://", "slack://", "zoom://",
                "market://", "itms://", "itms-apps://"
            };

            for (const char* protocol : protocols) {
                if (url.find(protocol) == 0) {
                    return true;
                }
            }
            return false;
        }

        /**
         * @brief Add https:// protocol if no protocol is present
         * @param url The URL to process
         * @return URL with protocol prefix
         *
         * Example:
         * @code
         * std::string url = sdlpp::url::ensure_protocol("example.com");
         * // Returns: "https://example.com"
         * @endcode
         */
        [[nodiscard]] inline std::string ensure_protocol(std::string_view url) {
            if (has_protocol(url)) {
                return std::string(url);
            }
            return "https://" + std::string(url);
        }

        /**
         * @brief Create a mailto: URL with optional parameters
         * @param email Email address
         * @param subject Optional email subject
         * @param body Optional email body
         * @return Formatted mailto: URL
         *
         * @note The subject and body are not URL-encoded by this function.
         *       You may need to encode them if they contain special characters.
         *
         * Example:
         * @code
         * auto url = sdlpp::url::make_mailto("support@example.com",
         *                                    "Bug Report",
         *                                    "I found a bug...");
         * sdlpp::open_url(url);
         * @endcode
         */
        [[nodiscard]] inline std::string make_mailto(const std::string& email,
                                                     const std::string& subject = "",
                                                     const std::string& body = "") {
            std::string result = "mailto:" + email;

            bool first_param = true;

            if (!subject.empty()) {
                result += "?subject=" + subject;
                first_param = false;
            }

            if (!body.empty()) {
                result += (first_param ? "?" : "&");
                result += "body=" + body;
            }

            return result;
        }

        /**
         * @brief Create a file:// URL from a local path
         * @param path Local filesystem path
         * @return file:// URL
         *
         * @note This performs basic conversion only. The path should be absolute.
         * @note On Windows, converts backslashes to forward slashes.
         *
         * Example:
         * @code
         * auto url = sdlpp::url::make_file_url("/home/user/document.pdf");
         * // Returns: "file:///home/user/document.pdf"
         * @endcode
         */
        [[nodiscard]] inline std::string make_file_url(std::string path) {
            // Convert backslashes to forward slashes on Windows
            for (char& c : path) {
                if (c == '\\') {
                    c = '/';
                }
            }

            // Ensure it starts with a slash
            if (!path.empty() && path[0] != '/') {
                path = "/" + path;
            }

            return "file://" + path;
        }
    }

    /**
     * @brief Platform-specific URL behavior notes
     *
     * Different platforms handle URL opening differently:
     *
     * - **Windows**: Uses ShellExecute, supports most common protocols
     * - **macOS**: Uses NSWorkspace, excellent protocol support
     * - **Linux**: Uses xdg-open or similar, depends on desktop environment
     * - **iOS**: Uses UIApplication openURL, may require app switching
     * - **Android**: Uses Intent system, may require permissions
     * - **Emscripten**: Uses window.open(), subject to popup blocking
     *
     * Always test URL opening on your target platforms, as behavior
     * can vary significantly.
     */
} // namespace sdlpp
