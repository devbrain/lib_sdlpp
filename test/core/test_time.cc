#include <doctest/doctest.h>
#include <../include/sdlpp/core/time.hh>
#include <sdlpp/core/core.hh>
#include <thread>
#include <iostream>

TEST_SUITE("time") {
    TEST_CASE("date_time structure") {
        // Default construction
        sdlpp::date_time dt;
        CHECK(dt.year == 0);
        CHECK(dt.month == 1);
        CHECK(dt.day == 1);
        CHECK(dt.hour == 0);
        CHECK(dt.minute == 0);
        CHECK(dt.second == 0);
        CHECK(dt.nanosecond == 0);
        
        // Validity check
        CHECK(dt.is_valid()); // Year 0 is valid in SDL (represents 1 BCE)
        
        // Valid date
        sdlpp::date_time valid_dt;
        valid_dt.year = 2024;
        valid_dt.month = 7;
        valid_dt.day = 15;
        valid_dt.hour = 14;
        valid_dt.minute = 30;
        valid_dt.second = 45;
        CHECK(valid_dt.is_valid());
        
        // Invalid dates
        sdlpp::date_time invalid_month;
        invalid_month.year = 2024;
        invalid_month.month = 13; // Invalid
        CHECK(!invalid_month.is_valid());
        
        sdlpp::date_time invalid_hour;
        invalid_hour.year = 2024;
        invalid_hour.hour = 24; // Invalid
        CHECK(!invalid_hour.is_valid());
    }
    
    TEST_CASE("sdl_clock") {
        // Get current time
        auto now1 = sdlpp::sdl_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto now2 = sdlpp::sdl_clock::now();
        
        // Time should advance
        CHECK(now2 > now1);
        
        // Duration calculation
        auto diff = now2 - now1;
        CHECK(diff.count() > 0);
        CHECK(diff >= std::chrono::milliseconds(10));
        
        // time_t conversion
        auto now = sdlpp::sdl_clock::now();
        std::time_t tt = sdlpp::sdl_clock::to_time_t(now);
        auto back = sdlpp::sdl_clock::from_time_t(tt);
        
        // Should be within 1 second (due to time_t precision)
        auto diff_ns = std::abs((now - back).count());
        CHECK(diff_ns < 1'000'000'000); // Less than 1 second
    }
    
    TEST_CASE("get_current_time") {
        auto time1 = sdlpp::get_current_time();
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        auto time2 = sdlpp::get_current_time();
        
        CHECK(time2 > time1);
        
        // Should be at least 5ms difference
        auto diff = time2 - time1;
        CHECK(diff >= std::chrono::milliseconds(5));
    }
    
    TEST_CASE("time conversions") {
        // Get current time and convert to date_time
        auto now = sdlpp::get_current_time();
        auto dt_result = sdlpp::time_to_date_time(now);
        
        CHECK(dt_result.has_value());
        if (dt_result) {
            auto& dt = *dt_result;
            CHECK(dt.is_valid());
            CHECK(dt.year >= 2024); // Assuming tests run in 2024 or later
            CHECK(dt.month >= 1);
            CHECK(dt.month <= 12);
            CHECK(dt.day >= 1);
            CHECK(dt.day <= 31);
            
            INFO("Current date/time: " << dt.year << "-" << dt.month << "-" << dt.day 
                 << " " << dt.hour << ":" << dt.minute << ":" << dt.second);
            
            // Convert back to time
            auto time_result = sdlpp::date_time_to_time(dt);
            CHECK(time_result.has_value());
            
            // Should be close to original (may differ due to timezone adjustments)
            if (time_result) {
                auto diff = std::abs((now - *time_result).count());
                // Allow up to 24 hours difference for timezone
                CHECK(diff < 24LL * 60 * 60 * 1'000'000'000);
            }
        }
    }
    
    TEST_CASE("system clock conversion") {
        // Get system time
        auto sys_now = std::chrono::system_clock::now();
        
        // Convert to SDL clock
        auto sdl_time = sdlpp::from_system_clock(sys_now);
        
        // Convert back
        auto sys_back = sdlpp::to_system_clock(sdl_time);
        
        // Should be very close (within microseconds)
        auto diff = std::chrono::abs(sys_now - sys_back);
        CHECK(diff < std::chrono::microseconds(1));
    }
    
    TEST_CASE("date utilities") {
        // Days in month
        CHECK(sdlpp::get_days_in_month(2024, 1) == 31);  // January
        CHECK(sdlpp::get_days_in_month(2024, 2) == 29);  // February (leap year)
        CHECK(sdlpp::get_days_in_month(2023, 2) == 28);  // February (non-leap)
        CHECK(sdlpp::get_days_in_month(2024, 4) == 30);  // April
        CHECK(sdlpp::get_days_in_month(2024, 13) == -1);  // Invalid month returns -1
        
        // Day of week
        auto dow = sdlpp::get_day_of_week(2024, 7, 15); // July 15, 2024
        CHECK(static_cast<int>(dow) >= 0);
        CHECK(static_cast<int>(dow) <= 6);
        
        // Day of year (0-based: 0-365)
        CHECK(sdlpp::get_day_of_year(2024, 1, 1) == 0);    // Jan 1
        CHECK(sdlpp::get_day_of_year(2024, 12, 31) == 365); // Dec 31 (leap year)
        CHECK(sdlpp::get_day_of_year(2023, 12, 31) == 364); // Dec 31 (non-leap)
        CHECK(sdlpp::get_day_of_year(2024, 7, 15) == 196);  // July 15
        
        // Leap year
        CHECK(sdlpp::is_leap_year(2024) == true);
        CHECK(sdlpp::is_leap_year(2023) == false);
        CHECK(sdlpp::is_leap_year(2000) == true);  // Divisible by 400
        CHECK(sdlpp::is_leap_year(1900) == false); // Divisible by 100 but not 400
    }
    
    TEST_CASE("date_time methods") {
        sdlpp::date_time dt;
        dt.year = 2024;
        dt.month = 7;
        dt.day = 15;
        
        CHECK(dt.days_in_month() == 31);
        auto day_result = dt.day_of_year();
        CHECK(day_result.has_value());
        CHECK(*day_result == 196);  // 0-based
        
        // February in leap year
        dt.month = 2;
        dt.day = 29;
        CHECK(dt.days_in_month() == 29u);
        CHECK(dt.day_of_year() == 59u);  // 0-based (31 + 29 - 1)
    }
    
    TEST_CASE("date_time_format preferences") {
        auto prefs = sdlpp::get_date_time_locale_preferences();
        
        // May fail on some systems
        if (prefs) {
            INFO("Date format: " << static_cast<int>(prefs->date_fmt));
            INFO("Time format: " << static_cast<int>(prefs->time_fmt));
            INFO("24-hour format: " << prefs->is_24_hour());
            
            // Check format strings
            auto date_str = prefs->get_date_format_string();
            auto time_str = prefs->get_time_format_string();
            
            CHECK(!date_str.empty());
            CHECK(!time_str.empty());
            
            INFO("Date format string: " << date_str);
            INFO("Time format string: " << time_str);
        } else {
            INFO("Date/time locale preferences not available: " << prefs.error());
        }
    }
    
    TEST_CASE("chrono_utils conversions") {
        using namespace sdlpp::chrono_utils;
        
        // Seconds to nanoseconds
        CHECK(seconds_to_ns(1) == std::chrono::nanoseconds(1'000'000'000));
        CHECK(seconds_to_ns(5) == std::chrono::nanoseconds(5'000'000'000));
        
        // Nanoseconds to seconds
        CHECK(ns_to_seconds(std::chrono::nanoseconds(1'000'000'000)) == 1);
        CHECK(ns_to_seconds(std::chrono::nanoseconds(2'500'000'000)) == 2); // Truncated
        
        // Milliseconds to nanoseconds
        CHECK(ms_to_ns(1) == std::chrono::nanoseconds(1'000'000));
        CHECK(ms_to_ns(1000) == std::chrono::nanoseconds(1'000'000'000));
        
        // Nanoseconds to milliseconds
        CHECK(ns_to_ms(std::chrono::nanoseconds(1'000'000)) == 1);
        CHECK(ns_to_ms(std::chrono::nanoseconds(1'500'000)) == 1); // Truncated
        
        // Microseconds conversions
        CHECK(us_to_ns(1) == std::chrono::nanoseconds(1'000));
        CHECK(ns_to_us(std::chrono::nanoseconds(5'000)) == 5);
    }
    
    TEST_CASE("format_date_time") {
        sdlpp::date_time dt;
        dt.year = 2024;
        dt.month = 7;
        dt.day = 15;
        dt.hour = 14;
        dt.minute = 30;
        dt.second = 45;
        dt.nanosecond = 123'456'789;
        
        // Default format
        auto formatted = sdlpp::format_date_time(dt);
        CHECK(formatted == "2024-07-15 14:30:45");
        
        // Custom formats
        CHECK(sdlpp::format_date_time(dt, "%Y-%m-%d") == "2024-07-15");
        CHECK(sdlpp::format_date_time(dt, "%H:%M:%S") == "14:30:45");
        CHECK(sdlpp::format_date_time(dt, "%Y%m%d") == "20240715");
        
        // With nanoseconds (custom extension)
        auto with_ns = sdlpp::format_date_time(dt, "%H:%M:%S.%N");
        CHECK(with_ns == "14:30:45.123456789");
        
        // Month names (locale-dependent, just check it doesn't crash)
        auto month_name = sdlpp::format_date_time(dt, "%B %Y");
        CHECK(!month_name.empty());
    }
    
    #ifdef SDL_PLATFORM_WINDOWS
    TEST_CASE("windows time conversion") {
        auto now = sdlpp::get_current_time();
        
        // Convert to Windows FILETIME
        Uint32 low, high;
        sdlpp::time_to_windows(now, low, high);
        
        // Convert back
        auto back = sdlpp::time_from_windows(low, high);
        
        // Should be identical
        CHECK(now == back);
    }
    #endif
}