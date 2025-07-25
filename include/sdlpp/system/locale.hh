#pragma once

/**
 * @file locale.hh
 * @brief Locale and language preference detection
 * 
 * This header provides access to the user's preferred locales (language and country),
 * allowing applications to provide localized content based on system preferences.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <string>
#include <vector>
#include <optional>
#include <map>
#include <algorithm>

namespace sdlpp {
    /**
     * @brief Locale information structure
     *
     * Represents a locale with language and optional country code.
     * Language is an ISO-639-1 code (e.g., "en" for English).
     * Country is an ISO-3166-1 code (e.g., "US" for United States).
     */
    struct locale {
        std::string language; ///< ISO-639-1 language code (e.g., "en", "fr", "ja")
        std::optional <std::string> country; ///< Optional ISO-3166-1 country code (e.g., "US", "CA", "JP")

        /**
         * @brief Default constructor
         */
        locale() = default;

        /**
         * @brief Construct from SDL_Locale
         * @param sdl_locale SDL locale structure
         */
        explicit locale(const SDL_Locale& sdl_locale) {
            if (sdl_locale.language) {
                language = sdl_locale.language;
            }
            if (sdl_locale.country) {
                country = sdl_locale.country;
            }
        }

        /**
         * @brief Construct from language and optional country
         * @param lang Language code
         * @param ctry Optional country code
         */
        locale(const std::string& lang, const std::optional <std::string>& ctry = std::nullopt)
            : language(lang), country(ctry) {
        }

        /**
         * @brief Get the full locale string
         * @param separator Separator between language and country (default: "-")
         * @return Full locale string (e.g., "en-US", "fr-CA")
         */
        [[nodiscard]] std::string to_string(const std::string& separator = "-") const {
            if (country) {
                return language + separator + *country;
            }
            return language;
        }

        /**
         * @brief Get the locale in standard format
         * @return Locale string with underscore separator (e.g., "en_US")
         */
        [[nodiscard]] std::string to_posix_string() const {
            return to_string("_");
        }

        /**
         * @brief Get the locale in BCP 47 format
         * @return Locale string with hyphen separator (e.g., "en-US")
         */
        [[nodiscard]] std::string to_bcp47_string() const {
            return to_string("-");
        }

        /**
         * @brief Check if this locale matches another (with fallback)
         * @param other Other locale to compare
         * @param allow_language_only If true, match on language alone if country differs
         * @return true if locales match
         */
        [[nodiscard]] bool matches(const locale& other, bool allow_language_only = true) const {
            if (language != other.language) {
                return false;
            }

            // If both have countries, check if they match
            if (country && other.country) {
                if (*country == *other.country) {
                    return true; // Exact match
                }
                // Countries differ - match only if language-only matching is allowed
                return allow_language_only;
            }

            // If neither has country, they match
            if (!country && !other.country) {
                return true;
            }

            // One has country, one doesn't - match if allowed
            return allow_language_only;
        }

        /**
         * @brief Equality operator
         * @param other Other locale to compare
         * @return true if locales are exactly equal
         */
        [[nodiscard]] bool operator==(const locale& other) const {
            return language == other.language && country == other.country;
        }

        /**
         * @brief Inequality operator
         * @param other Other locale to compare
         * @return true if locales are not equal
         */
        [[nodiscard]] bool operator!=(const locale& other) const {
            return !(*this == other);
        }

        /**
         * @brief Less-than operator for sorting
         * @param other Other locale to compare
         * @return true if this locale is less than other
         */
        [[nodiscard]] bool operator<(const locale& other) const {
            if (language != other.language) {
                return language < other.language;
            }
            // Compare countries, treating absence as less than presence
            if (!country && other.country) return true;
            if (country && !other.country) return false;
            if (country && other.country) return *country < *other.country;
            return false;
        }
    };

    /**
     * @brief Get the user's preferred locales
     *
     * Returns a list of locales in order of preference, as reported by the operating system.
     * This typically includes the user's primary language/region and fallback options.
     *
     * @return Vector of preferred locales (may be empty if unable to determine)
     *
     * @note The returned list is ordered by preference, with the most preferred first
     * @note On some platforms, this may include generic language codes without country
     *
     * Example:
     * @code
     * auto locales = sdlpp::get_preferred_locales();
     * for (const auto& loc : locales) {
     *     std::cout << "Locale: " << loc.to_string() << "\n";
     *     if (loc.language == "en") {
     *         // English is supported
     *         if (loc.country == "US") {
     *             // Use US English
     *         } else if (loc.country == "GB") {
     *             // Use British English
     *         } else {
     *             // Use generic English
     *         }
     *     }
     * }
     * @endcode
     */
    [[nodiscard]] inline std::vector <locale> get_preferred_locales() {
        std::vector <locale> locales;

        int count = 0;
        SDL_Locale** sdl_locales = SDL_GetPreferredLocales(&count);
        if (!sdl_locales || count == 0) {
            return locales;
        }

        // Convert SDL locales to our locale objects
        locales.reserve(static_cast <size_t>(count));
        for (int i = 0; i < count; ++i) {
            if (sdl_locales[i] && sdl_locales[i]->language) {
                locales.emplace_back(*sdl_locales[i]);
            }
        }

        // SDL owns the memory, so we don't free it
        return locales;
    }

    /**
     * @brief Get the primary (most preferred) locale
     *
     * @return Optional containing the primary locale, or nullopt if none available
     */
    [[nodiscard]] inline std::optional <locale> get_primary_locale() {
        auto locales = get_preferred_locales();
        if (locales.empty()) {
            return std::nullopt;
        }
        return locales.front();
    }

    /**
     * @brief Find the best matching locale from available options
     *
     * Given a list of available locales (e.g., those your application supports),
     * finds the best match from the user's preferred locales.
     *
     * @param available Vector of available locales
     * @param allow_language_fallback If true, match on language alone if exact match not found
     * @return Optional containing the best matching locale, or nullopt if no match
     *
     * Example:
     * @code
     * // Locales your app supports
     * std::vector<sdlpp::locale> supported = {
     *     {"en", "US"},  // English (US)
     *     {"en", "GB"},  // English (UK)
     *     {"fr", "FR"},  // French (France)
     *     {"fr", "CA"},  // French (Canada)
     *     {"es"},        // Spanish (generic)
     *     {"de"},        // German (generic)
     *     {"ja"}         // Japanese
     * };
     *
     * auto best = sdlpp::find_best_locale(supported);
     * if (best) {
     *     std::cout << "Using locale: " << best->to_string() << "\n";
     * }
     * @endcode
     */
    [[nodiscard]] inline std::optional <locale> find_best_locale(
        const std::vector <locale>& available,
        bool allow_language_fallback = true) {
        auto preferred = get_preferred_locales();
        if (preferred.empty() || available.empty()) {
            return std::nullopt;
        }

        // First pass: look for exact matches
        for (const auto& pref : preferred) {
            for (const auto& avail : available) {
                if (pref == avail) {
                    return avail;
                }
            }
        }

        // Second pass: look for language matches if allowed
        if (allow_language_fallback) {
            for (const auto& pref : preferred) {
                for (const auto& avail : available) {
                    if (pref.matches(avail, true)) {
                        return avail;
                    }
                }
            }
        }

        return std::nullopt;
    }

    /**
     * @brief Locale matching result
     */
    struct locale_match {
        locale matched; ///< The matched locale
        size_t preference_index; ///< Index in preference list (0 = most preferred)
        bool exact_match; ///< True if country also matched
    };

    /**
     * @brief Find all matching locales from available options
     *
     * Returns all available locales that match user preferences, sorted by preference.
     *
     * @param available Vector of available locales
     * @param allow_language_fallback If true, include language-only matches
     * @return Vector of matching locales with match information
     */
    [[nodiscard]] inline std::vector <locale_match> find_all_matching_locales(
        const std::vector <locale>& available,
        bool allow_language_fallback = true) {
        std::vector <locale_match> matches;
        auto preferred = get_preferred_locales();

        if (preferred.empty() || available.empty()) {
            return matches;
        }

        // Track best match for each available locale
        std::map <std::string, locale_match> best_matches;

        // Check each preferred locale
        for (size_t pref_idx = 0; pref_idx < preferred.size(); ++pref_idx) {
            const auto& pref = preferred[pref_idx];

            // Check against available locales
            for (const auto& avail : available) {
                std::string key = avail.to_string();

                // Check if we already have a match for this locale
                auto it = best_matches.find(key);

                if (pref == avail) {
                    // Exact match - always update or add
                    if (it == best_matches.end() || pref_idx < it->second.preference_index ||
                        (pref_idx == it->second.preference_index && !it->second.exact_match)) {
                        best_matches[key] = {avail, pref_idx, true};
                    }
                } else if (allow_language_fallback && pref.matches(avail, true)) {
                    // Language-only match - only add if we don't have a better match
                    if (it == best_matches.end()) {
                        best_matches[key] = {avail, pref_idx, false};
                    } else if (pref_idx < it->second.preference_index) {
                        // Better preference index
                        best_matches[key] = {avail, pref_idx, false};
                    }
                }
            }
        }

        // Extract matches from map
        matches.reserve(best_matches.size());
        for (const auto& [key, match] : best_matches) {
            matches.push_back(match);
        }

        // Sort by preference index, then by exact match
        std::sort(matches.begin(), matches.end(),
                  [](const locale_match& a, const locale_match& b) {
                      if (a.preference_index != b.preference_index) {
                          return a.preference_index < b.preference_index;
                      }
                      return a.exact_match && !b.exact_match;
                  });

        return matches;
    }

    /**
     * @brief Common locale language codes
     *
     * Provides constants for commonly used ISO-639-1 language codes
     */
    namespace languages {
        constexpr const char* english = "en";
        constexpr const char* french = "fr";
        constexpr const char* german = "de";
        constexpr const char* spanish = "es";
        constexpr const char* italian = "it";
        constexpr const char* portuguese = "pt";
        constexpr const char* russian = "ru";
        constexpr const char* japanese = "ja";
        constexpr const char* korean = "ko";
        constexpr const char* chinese = "zh";
        constexpr const char* arabic = "ar";
        constexpr const char* hindi = "hi";
        constexpr const char* dutch = "nl";
        constexpr const char* swedish = "sv";
        constexpr const char* polish = "pl";
        constexpr const char* turkish = "tr";
        constexpr const char* greek = "el";
        constexpr const char* hebrew = "he";
        constexpr const char* czech = "cs";
        constexpr const char* hungarian = "hu";
    }

    /**
     * @brief Common locale country codes
     *
     * Provides constants for commonly used ISO-3166-1 country codes
     */
    namespace countries {
        constexpr const char* united_states = "US";
        constexpr const char* united_kingdom = "GB";
        constexpr const char* canada = "CA";
        constexpr const char* australia = "AU";
        constexpr const char* france = "FR";
        constexpr const char* germany = "DE";
        constexpr const char* spain = "ES";
        constexpr const char* italy = "IT";
        constexpr const char* brazil = "BR";
        constexpr const char* portugal = "PT";
        constexpr const char* russia = "RU";
        constexpr const char* japan = "JP";
        constexpr const char* korea = "KR";
        constexpr const char* china = "CN";
        constexpr const char* taiwan = "TW";
        constexpr const char* india = "IN";
        constexpr const char* mexico = "MX";
        constexpr const char* argentina = "AR";
        constexpr const char* netherlands = "NL";
        constexpr const char* belgium = "BE";
    }

    /**
     * @brief Create common locale combinations
     */
    namespace locales {
        inline locale en_US() { return {languages::english, countries::united_states}; }
        inline locale en_GB() { return {languages::english, countries::united_kingdom}; }
        inline locale en_CA() { return {languages::english, countries::canada}; }
        inline locale en_AU() { return {languages::english, countries::australia}; }
        inline locale fr_FR() { return {languages::french, countries::france}; }
        inline locale fr_CA() { return {languages::french, countries::canada}; }
        inline locale de_DE() { return {languages::german, countries::germany}; }
        inline locale es_ES() { return {languages::spanish, countries::spain}; }
        inline locale es_MX() { return {languages::spanish, countries::mexico}; }
        inline locale pt_BR() { return {languages::portuguese, countries::brazil}; }
        inline locale pt_PT() { return {languages::portuguese, countries::portugal}; }
        inline locale zh_CN() { return {languages::chinese, countries::china}; }
        inline locale zh_TW() { return {languages::chinese, countries::taiwan}; }
        inline locale ja_JP() { return {languages::japanese, countries::japan}; }
        inline locale ko_KR() { return {languages::korean, countries::korea}; }
    }
} // namespace sdlpp
