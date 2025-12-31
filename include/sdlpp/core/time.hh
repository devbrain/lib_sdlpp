#pragma once

/**
 * @file time.hh
 * @brief Calendar time and date utilities
 * 
 * This header provides access to SDL's calendar time functionality with
 * seamless integration with std::chrono types.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/detail/expected.hh>
#include <chrono>
#include <string>
#include <cstring>
#include <optional>
#include <ctime>
#include <iosfwd>

namespace sdlpp {
    /**
     * @brief SDL time type - nanoseconds since Unix epoch
     */
    using sdl_time = Sint64;

    /**
     * @brief Date and time components
     *
     * Represents a broken-down calendar date and time.
     * All fields use the same ranges as SDL_DateTime.
     */
    struct date_time {
        int year{0}; ///< Year (e.g., 2024)
        int month{1}; ///< Month [1-12]
        int day{1}; ///< Day of month [1-31]
        int hour{0}; ///< Hour [0-23]
        int minute{0}; ///< Minute [0-59]
        int second{0}; ///< Second [0-60] (60 for leap seconds)
        int nanosecond{0}; ///< Nanosecond [0-999999999]
        int day_of_week{0}; ///< Day of week [0-6], 0 = Sunday
        int utc_offset{0}; ///< Seconds east of UTC

        /**
         * @brief Default constructor
         */
        date_time() = default;

        /**
         * @brief Construct from SDL_DateTime
         * @param dt SDL date/time structure
         */
        explicit date_time(const SDL_DateTime& dt) noexcept
            : year(dt.year)
              , month(dt.month)
              , day(dt.day)
              , hour(dt.hour)
              , minute(dt.minute)
              , second(dt.second)
              , nanosecond(dt.nanosecond)
              , day_of_week(dt.day_of_week)
              , utc_offset(dt.utc_offset) {
        }

        /**
         * @brief Convert to SDL_DateTime
         * @return SDL date/time structure
         */
        [[nodiscard]] SDL_DateTime to_sdl() const noexcept {
            return SDL_DateTime{
                .year = year,
                .month = month,
                .day = day,
                .hour = hour,
                .minute = minute,
                .second = second,
                .nanosecond = nanosecond,
                .day_of_week = day_of_week,
                .utc_offset = utc_offset
            };
        }

        /**
         * @brief Check if this represents a valid date
         * @return true if the date is valid
         */
        [[nodiscard]] bool is_valid() const noexcept {
            if (month < 1 || month > 12) return false;
            if (day < 1 || day > 31) return false;
            if (hour < 0 || hour > 23) return false;
            if (minute < 0 || minute > 59) return false;
            if (second < 0 || second > 60) return false;
            if (nanosecond < 0 || nanosecond > 999'999'999) return false;
            return true;
        }

        /**
         * @brief Get the number of days in the current month
         * @return Days in month (28-31), or 0 if invalid
         */
        [[nodiscard]] unsigned days_in_month() const noexcept {
            int days = SDL_GetDaysInMonth(year, month);
            return days > 0 ? static_cast<unsigned>(days) : 0;
        }

        /**
         * @brief Get the day of year
         * @return Day of year (1-366)
         */
        [[nodiscard]] expected<unsigned, std::string> day_of_year() const {
            int day_num = SDL_GetDayOfYear(year, month, day);
            if (day_num < 0) {
                return make_unexpectedf("Invalid date");
            }
            return static_cast<unsigned>(day_num);
        }
    };

    /**
     * @brief SDL clock type for use with std::chrono
     *
     * This clock uses SDL_GetCurrentTime() and is compatible with
     * std::chrono time points and durations.
     */
    struct sdl_clock {
        using rep = Sint64;
        using period = std::nano;
        using duration = std::chrono::nanoseconds;
        using time_point = std::chrono::time_point <sdl_clock>;

        static constexpr bool is_steady = false;

        /**
         * @brief Get current time
         * @return Current time as a time_point
         */
        [[nodiscard]] static time_point now() noexcept {
            sdl_time ns;
            SDL_GetCurrentTime(&ns);
            return time_point(duration(ns));
        }

        /**
         * @brief Convert time_point to time_t
         * @param tp Time point to convert
         * @return Equivalent time_t value
         */
        [[nodiscard]] static std::time_t to_time_t(const time_point& tp) noexcept {
            auto ns = tp.time_since_epoch().count();
            return ns / 1'000'000'000;
        }

        /**
         * @brief Convert time_t to time_point
         * @param t time_t value to convert
         * @return Equivalent time_point
         */
        [[nodiscard]] static time_point from_time_t(std::time_t t) noexcept {
            auto ns = static_cast <Sint64>(t) * 1'000'000'000;
            return time_point(duration(ns));
        }
    };

    /**
     * @brief Get current time
     * @return Current UTC time as nanoseconds since Unix epoch
     */
    [[nodiscard]] inline sdl_clock::time_point get_current_time() noexcept {
        return sdl_clock::now();
    }

    /**
     * @brief Convert time_point to date_time components
     * @param tp Time point to convert
     * @return Date/time components
     */
    [[nodiscard]] inline tl::expected <date_time, std::string> time_to_date_time(
        const sdl_clock::time_point& tp) {
        SDL_DateTime dt;
        sdl_time ns = tp.time_since_epoch().count();

        if (!SDL_TimeToDateTime(ns, &dt, true)) {
            // true = adjust for local time
            return make_unexpectedf(get_error());
        }

        return date_time(dt);
    }

    /**
     * @brief Convert date_time components to time_point
     * @param dt Date/time components
     * @return Time point
     */
    [[nodiscard]] inline tl::expected <sdl_clock::time_point, std::string>
    date_time_to_time(const date_time& dt) {
        sdl_time ns;
        SDL_DateTime sdl_dt = dt.to_sdl();

        if (!SDL_DateTimeToTime(&sdl_dt, &ns)) {
            return make_unexpectedf(get_error());
        }

        return sdl_clock::time_point(sdl_clock::duration(ns));
    }

    /**
     * @brief Convert std::chrono::system_clock to SDL clock
     * @param sys_tp System clock time point
     * @return SDL clock time point
     */
    [[nodiscard]] inline sdl_clock::time_point from_system_clock(
        const std::chrono::system_clock::time_point& sys_tp) noexcept {
        // Convert to nanoseconds since epoch
        auto ns = std::chrono::duration_cast <std::chrono::nanoseconds>(
            sys_tp.time_since_epoch()).count();

        return sdl_clock::time_point(sdl_clock::duration(ns));
    }

    /**
     * @brief Convert SDL clock to std::chrono::system_clock
     * @param sdl_tp SDL clock time point
     * @return System clock time point
     */
    [[nodiscard]] inline std::chrono::system_clock::time_point to_system_clock(
        const sdl_clock::time_point& sdl_tp) noexcept {
        auto ns = sdl_tp.time_since_epoch();
        return std::chrono::system_clock::time_point(
            std::chrono::duration_cast <std::chrono::system_clock::duration>(ns));
    }

    /**
     * @brief Day of week enumeration
     */
    enum class day_of_week {
        sunday = 0,
        monday = 1,
        tuesday = 2,
        wednesday = 3,
        thursday = 4,
        friday = 5,
        saturday = 6
    };

    // Forward declare stream operators
    SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, day_of_week value);
    SDLPP_EXPORT std::istream& operator>>(std::istream& is, day_of_week& value);

    /**
     * @brief Get the day of week for a given date
     * @param year Year
     * @param month Month (1-12)
     * @param day Day of month (1-31)
     * @return Day of week
     */
    [[nodiscard]] inline day_of_week get_day_of_week(int year, int month, int day) noexcept {
        return static_cast <day_of_week>(SDL_GetDayOfWeek(year, month, day));
    }

    /**
     * @brief Get the number of days in a month
     * @param year Year (for leap year calculation)
     * @param month Month (1-12)
     * @return Number of days (28-31), or 0 if invalid month
     */
    [[nodiscard]] inline int get_days_in_month(int year, int month) noexcept {
        return SDL_GetDaysInMonth(year, month);
    }

    /**
     * @brief Get the day of year
     * @param year Year
     * @param month Month (1-12)
     * @param day Day of month (1-31)
     * @return Day of year (0-365), or -1 if invalid date
     */
    [[nodiscard]] inline int get_day_of_year(int year, int month, int day) noexcept {
        return SDL_GetDayOfYear(year, month, day);
    }

    /**
     * @brief Check if a year is a leap year
     * @param year Year to check
     * @return true if leap year
     */
    [[nodiscard]] inline bool is_leap_year(int year) noexcept {
        // A year is a leap year if:
        // - It's divisible by 4 AND
        // - Either not divisible by 100 OR divisible by 400
        return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
    }

    /**
     * @brief Date format enumeration
     */
    enum class date_format {
        yyyy_mm_dd = SDL_DATE_FORMAT_YYYYMMDD, ///< Year/Month/Day
        dd_mm_yyyy = SDL_DATE_FORMAT_DDMMYYYY, ///< Day/Month/Year
        mm_dd_yyyy = SDL_DATE_FORMAT_MMDDYYYY ///< Month/Day/Year
    };

    // Forward declare stream operators
    SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, date_format value);
    SDLPP_EXPORT std::istream& operator>>(std::istream& is, date_format& value);

    /**
     * @brief Time format enumeration
     */
    enum class time_format {
        hour_24 = SDL_TIME_FORMAT_24HR, ///< 24-hour format
        hour_12 = SDL_TIME_FORMAT_12HR ///< 12-hour format with AM/PM
    };

    // Forward declare stream operators
    SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, time_format value);
    SDLPP_EXPORT std::istream& operator>>(std::istream& is, time_format& value);


    /**
     * @brief Date/time format preferences
     */
    struct date_time_format {
        date_format date_fmt{date_format::yyyy_mm_dd}; ///< Date format
        time_format time_fmt{time_format::hour_24}; ///< Time format

        /**
         * @brief Default constructor
         */
        date_time_format() = default;

        /**
         * @brief Construct from SDL formats
         * @param df Date format
         * @param tf Time format
         */
        date_time_format(SDL_DateFormat df, SDL_TimeFormat tf) noexcept
            : date_fmt(static_cast <date_format>(df))
              , time_fmt(static_cast <time_format>(tf)) {
        }

        /**
         * @brief Check if using 24-hour time
         * @return true if 24-hour format
         */
        [[nodiscard]] bool is_24_hour() const noexcept {
            return time_fmt == time_format::hour_24;
        }

        /**
         * @brief Get date format string for strftime
         * @return Format string based on date format preference
         */
        [[nodiscard]] std::string get_date_format_string() const {
            switch (date_fmt) {
                case date_format::yyyy_mm_dd: return "%Y-%m-%d";
                case date_format::dd_mm_yyyy: return "%d/%m/%Y";
                case date_format::mm_dd_yyyy: return "%m/%d/%Y";
                default: return "%Y-%m-%d";
            }
        }

        /**
         * @brief Get time format string for strftime
         * @return Format string based on time format preference
         */
        [[nodiscard]] std::string get_time_format_string() const {
            return is_24_hour() ? "%H:%M:%S" : "%I:%M:%S %p";
        }
    };

    /**
     * @brief Get locale-specific date/time format preferences
     * @return Format preferences, or error
     */
    [[nodiscard]] inline tl::expected <date_time_format, std::string>
    get_date_time_locale_preferences() {
        SDL_DateFormat date_fmt;
        SDL_TimeFormat time_fmt;

        if (!SDL_GetDateTimeLocalePreferences(&date_fmt, &time_fmt)) {
            return make_unexpectedf(get_error());
        }

        return date_time_format(date_fmt, time_fmt);
    }


/**
 * @brief Convert SDL time to Windows FILETIME
 * @param tp SDL time point
 * @param filetime_low Output for low 32 bits
 * @param filetime_high Output for high 32 bits
 */
inline void time_to_windows(const sdl_clock::time_point& tp,
                           Uint32& filetime_low,
                           Uint32& filetime_high) noexcept {
    sdl_time ns = tp.time_since_epoch().count();
    SDL_TimeToWindows(ns, &filetime_low, &filetime_high);
}

/**
 * @brief Convert Windows FILETIME to SDL time
 * @param filetime_low Low 32 bits of FILETIME
 * @param filetime_high High 32 bits of FILETIME
 * @return SDL time point
 */
[[nodiscard]] inline sdl_clock::time_point time_from_windows(
    Uint32 filetime_low,
    Uint32 filetime_high) noexcept {

    sdl_time ns = SDL_TimeFromWindows(filetime_low, filetime_high);
    return sdl_clock::time_point(sdl_clock::duration(ns));
}


    /**
     * @brief Time duration conversion utilities
     *
     * These provide std::chrono equivalents to SDL's conversion macros
     */
    namespace chrono_utils {
        /**
         * @brief Convert seconds to nanoseconds
         * @param seconds Number of seconds
         * @return Equivalent nanoseconds
         */
        [[nodiscard]] inline constexpr std::chrono::nanoseconds
        seconds_to_ns(Sint64 seconds) noexcept {
            return std::chrono::seconds(seconds);
        }

        /**
         * @brief Convert nanoseconds to seconds
         * @param ns Nanoseconds
         * @return Equivalent seconds (truncated)
         */
        [[nodiscard]] inline constexpr Sint64
        ns_to_seconds(std::chrono::nanoseconds ns) noexcept {
            return std::chrono::duration_cast <std::chrono::seconds>(ns).count();
        }

        /**
         * @brief Convert milliseconds to nanoseconds
         * @param ms Number of milliseconds
         * @return Equivalent nanoseconds
         */
        [[nodiscard]] inline constexpr std::chrono::nanoseconds
        ms_to_ns(Sint64 ms) noexcept {
            return std::chrono::milliseconds(ms);
        }

        /**
         * @brief Convert nanoseconds to milliseconds
         * @param ns Nanoseconds
         * @return Equivalent milliseconds (truncated)
         */
        [[nodiscard]] inline constexpr Sint64
        ns_to_ms(std::chrono::nanoseconds ns) noexcept {
            return std::chrono::duration_cast <std::chrono::milliseconds>(ns).count();
        }

        /**
         * @brief Convert microseconds to nanoseconds
         * @param us Number of microseconds
         * @return Equivalent nanoseconds
         */
        [[nodiscard]] inline constexpr std::chrono::nanoseconds
        us_to_ns(Sint64 us) noexcept {
            return std::chrono::microseconds(us);
        }

        /**
         * @brief Convert nanoseconds to microseconds
         * @param ns Nanoseconds
         * @return Equivalent microseconds (truncated)
         */
        [[nodiscard]] inline constexpr Sint64
        ns_to_us(std::chrono::nanoseconds ns) noexcept {
            return std::chrono::duration_cast <std::chrono::microseconds>(ns).count();
        }
    }

    /**
     * @brief Format date/time as string
     * @param dt Date/time to format
     * @param format Format string (uses strftime format)
     * @return Formatted string
     *
     * Example:
     * @code
     * auto now = sdlpp::get_current_time();
     * auto dt = sdlpp::time_to_date_time(now);
     * if (dt) {
     *     std::string formatted = sdlpp::format_date_time(*dt, "%Y-%m-%d %H:%M:%S");
     * }
     * @endcode
     */
    [[nodiscard]] inline std::string format_date_time(const date_time& dt,
                                                      const std::string& format = "%Y-%m-%d %H:%M:%S") {
        // Handle %N (nanoseconds) - non-standard extension
        // Replace before strftime since strftime may alter unknown specifiers
        static constexpr const char* ns_placeholder = "\x01NS_PLACEHOLDER\x01";
        std::string modified_format = format;
        bool has_nanoseconds = false;
        size_t ns_pos = modified_format.find("%N");
        if (ns_pos != std::string::npos) {
            has_nanoseconds = true;
            modified_format.replace(ns_pos, 2, ns_placeholder);
        }

        // Convert to std::tm
        std::tm tm{};
        tm.tm_year = dt.year - 1900;
        tm.tm_mon = dt.month - 1;
        tm.tm_mday = dt.day;
        tm.tm_hour = dt.hour;
        tm.tm_min = dt.minute;
        tm.tm_sec = dt.second;
        tm.tm_wday = dt.day_of_week;
        auto day_of_year_result = dt.day_of_year();
        tm.tm_yday = day_of_year_result ? static_cast<int>(*day_of_year_result) : 0; // SDL already returns 0-based

        // Format using strftime
        char buffer[256];
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif
        size_t len = std::strftime(buffer, sizeof(buffer), modified_format.c_str(), &tm);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

        if (len == 0) {
            return ""; // Format error
        }

        std::string formatted(buffer, len);

        // Replace placeholder with actual nanoseconds
        if (has_nanoseconds) {
            size_t pos = formatted.find(ns_placeholder);
            if (pos != std::string::npos) {
                formatted.replace(pos, std::strlen(ns_placeholder), std::to_string(dt.nanosecond));
            }
        }

        return formatted;
    }
} // namespace sdlpp
