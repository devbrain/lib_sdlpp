#include <iostream>
#include <iomanip>
#include <thread>
#include <vector>
#include <sdlpp/core/core.hh>
#include <../include/sdlpp/core/time.hh>
#include <../include/sdlpp/core/timer.hh>

void demonstrate_date_time() {
    std::cout << "=== Date/Time Operations ===\n\n";
    
    // Get current time
    auto now = sdlpp::get_current_time();
    std::cout << "Current time (nanoseconds since epoch): " 
              << now.time_since_epoch().count() << "\n";
    
    // Convert to date/time components
    auto dt_result = sdlpp::time_to_date_time(now);
    if (dt_result) {
        const auto& dt = *dt_result;
        std::cout << "\nCurrent date/time:\n";
        std::cout << "  Date: " << dt.year << "-" 
                  << std::setfill('0') << std::setw(2) << dt.month << "-"
                  << std::setw(2) << dt.day << "\n";
        std::cout << "  Time: " << std::setw(2) << dt.hour << ":"
                  << std::setw(2) << dt.minute << ":"
                  << std::setw(2) << dt.second << "."
                  << std::setw(9) << dt.nanosecond << "\n";
        std::cout << "  Day of week: " << dt.day_of_week << " (0=Sunday)\n";
        std::cout << "  UTC offset: " << dt.utc_offset << " seconds\n";
        
        // Date utilities
        std::cout << "\nDate information:\n";
        auto days = dt.days_in_month();
        std::cout << "  Days in month: " << days << "\n";
        if (auto day = dt.day_of_year()) {
            std::cout << "  Day of year: " << *day << "\n";
        }
        std::cout << "  Is leap year: " << std::boolalpha 
                  << sdlpp::is_leap_year(dt.year) << "\n";
        
        // Format the date/time
        std::cout << "\nFormatted output:\n";
        std::cout << "  Default: " << sdlpp::format_date_time(dt) << "\n";
        std::cout << "  ISO 8601: " << sdlpp::format_date_time(dt, "%Y-%m-%dT%H:%M:%S") << "\n";
        std::cout << "  US format: " << sdlpp::format_date_time(dt, "%m/%d/%Y %I:%M %p") << "\n";
        std::cout << "  With ns: " << sdlpp::format_date_time(dt, "%H:%M:%S.%N") << "\n";
    } else {
        std::cerr << "Failed to convert time: " << dt_result.error() << "\n";
    }
}

void demonstrate_clock_conversions() {
    std::cout << "\n=== Clock Conversions ===\n\n";
    
    // Get time from different clocks
    auto sys_now = std::chrono::system_clock::now();
    auto sdl_now = sdlpp::get_current_time();
    
    // Convert between clocks
    [[maybe_unused]] auto sdl_from_sys = sdlpp::from_system_clock(sys_now);
    auto sys_from_sdl = sdlpp::to_system_clock(sdl_now);
    
    std::cout << "System clock time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                    sys_now.time_since_epoch()).count() << " ms\n";
    std::cout << "SDL clock time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                    sdl_now.time_since_epoch()).count() << " ms\n";
    
    // Show the conversions work
    std::cout << "\nConversion test:\n";
    std::cout << "  SDL -> System -> SDL: ";
    auto round_trip = sdlpp::from_system_clock(sys_from_sdl);
    auto diff = std::abs((sdl_now - round_trip).count());
    std::cout << (diff < 1000 ? "OK" : "FAILED") << " (diff: " << diff << " ns)\n";
}

void demonstrate_time_calculations() {
    std::cout << "\n=== Time Calculations ===\n\n";
    
    // Create specific date/time
    sdlpp::date_time event_dt;
    event_dt.year = 2024;
    event_dt.month = 12;
    event_dt.day = 25;
    event_dt.hour = 12;
    event_dt.minute = 0;
    event_dt.second = 0;
    
    std::cout << "Event date: " << sdlpp::format_date_time(event_dt) << "\n";
    
    // Convert to time point
    auto event_time_result = sdlpp::date_time_to_time(event_dt);
    if (event_time_result) {
        auto event_time = *event_time_result;
        auto now = sdlpp::get_current_time();
        
        if (event_time > now) {
            auto duration = event_time - now;
            auto days = static_cast<long long>(std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24);
            auto hours = static_cast<long long>(std::chrono::duration_cast<std::chrono::hours>(duration).count() % 24);
            
            std::cout << "Time until event: " << days << " days, " << hours << " hours\n";
        } else {
            auto duration = now - event_time;
            auto days = static_cast<long long>(std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24);
            
            std::cout << "Event was " << days << " days ago\n";
        }
    }
}

void demonstrate_date_utilities() {
    std::cout << "\n=== Date Utilities ===\n\n";
    
    // Days in each month for 2024
    std::cout << "Days in each month of 2024:\n";
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    
    for (int m = 1; m <= 12; ++m) {
        std::cout << "  " << months[static_cast<size_t>(m-1)] << ": " 
                  << sdlpp::get_days_in_month(2024, m) << " days\n";
    }
    
    // Day of week for specific dates
    std::cout << "\nDay of week examples:\n";
    std::vector<std::tuple<int, int, int, const char*>> dates = {
        {2024, 1, 1, "New Year 2024"},
        {2024, 7, 4, "Independence Day 2024"},
        {2024, 12, 25, "Christmas 2024"},
        {2000, 1, 1, "Y2K"}
    };
    
    const char* day_names[] = {"Sunday", "Monday", "Tuesday", "Wednesday",
                               "Thursday", "Friday", "Saturday"};
    
    for (const auto& [year, month, day, name] : dates) {
        auto dow = sdlpp::get_day_of_week(year, month, day);
        std::cout << "  " << name << " (" << year << "-" << month << "-" << day 
                  << "): " << day_names[static_cast<size_t>(dow)] << "\n";
    }
    
    // Leap years
    std::cout << "\nLeap year check:\n";
    for (int year : {2020, 2021, 2022, 2023, 2024, 2000, 1900, 2100}) {
        std::cout << "  " << year << ": " 
                  << (sdlpp::is_leap_year(year) ? "Leap year" : "Not leap year") << "\n";
    }
}

void demonstrate_locale_preferences() {
    std::cout << "\n=== Date/Time Locale Preferences ===\n\n";
    
    auto prefs = sdlpp::get_date_time_locale_preferences();
    if (prefs) {
        std::cout << "User's date/time format preferences:\n";
        
        // Date format
        std::cout << "  Date format: ";
        switch (prefs->date_fmt) {
            case sdlpp::date_format::yyyy_mm_dd:
                std::cout << "Year/Month/Day (YYYY-MM-DD)\n";
                break;
            case sdlpp::date_format::dd_mm_yyyy:
                std::cout << "Day/Month/Year (DD/MM/YYYY)\n";
                break;
            case sdlpp::date_format::mm_dd_yyyy:
                std::cout << "Month/Day/Year (MM/DD/YYYY)\n";
                break;
        }
        
        // Time format
        std::cout << "  Time format: ";
        if (prefs->is_24_hour()) {
            std::cout << "24-hour (14:00)\n";
        } else {
            std::cout << "12-hour with AM/PM (2:00 PM)\n";
        }
        
        // Show formatted examples based on preferences
        auto now = sdlpp::get_current_time();
        auto dt = sdlpp::time_to_date_time(now);
        if (dt) {
            std::cout << "\nFormatted according to preferences:\n";
            
            // Use the preference-based format strings
            std::cout << "  Date: " << sdlpp::format_date_time(*dt, prefs->get_date_format_string()) << "\n";
            std::cout << "  Time: " << sdlpp::format_date_time(*dt, prefs->get_time_format_string()) << "\n";
            std::cout << "  Combined: " << sdlpp::format_date_time(*dt, 
                prefs->get_date_format_string() + " " + prefs->get_time_format_string()) << "\n";
        }
    } else {
        std::cout << "Could not get locale preferences: " << prefs.error() << "\n";
    }
}

void demonstrate_chrono_utils() {
    std::cout << "\n=== Chrono Conversion Utilities ===\n\n";
    
    using namespace sdlpp::chrono_utils;
    
    std::cout << "Duration conversions:\n";
    
    // Seconds
    auto ns_from_sec = seconds_to_ns(5);
    std::cout << "  5 seconds = " << ns_from_sec.count() << " nanoseconds\n";
    std::cout << "  " << ns_from_sec.count() << " ns = " 
              << ns_to_seconds(ns_from_sec) << " seconds\n";
    
    // Milliseconds
    auto ns_from_ms = ms_to_ns(1500);
    std::cout << "  1500 ms = " << ns_from_ms.count() << " nanoseconds\n";
    std::cout << "  " << ns_from_ms.count() << " ns = " 
              << ns_to_ms(ns_from_ms) << " milliseconds\n";
    
    // Microseconds
    auto ns_from_us = us_to_ns(2500);
    std::cout << "  2500 μs = " << ns_from_us.count() << " nanoseconds\n";
    std::cout << "  " << ns_from_us.count() << " ns = " 
              << ns_to_us(ns_from_us) << " microseconds\n";
}

void demonstrate_performance_timing() {
    std::cout << "\n=== Performance Timing ===\n\n";
    
    // Compare millisecond vs nanosecond precision
    std::cout << "Timing precision comparison:\n";
    
    // Millisecond timing
    auto start_ms = sdlpp::timer::elapsed();
    std::this_thread::sleep_for(std::chrono::microseconds(500)); // 0.5ms
    auto end_ms = sdlpp::timer::elapsed();
    auto diff_ms = end_ms - start_ms;
    
    // Nanosecond timing
    auto start_ns = sdlpp::timer::elapsed_ns();
    std::this_thread::sleep_for(std::chrono::microseconds(500)); // 0.5ms
    auto end_ns = sdlpp::timer::elapsed_ns();
    auto diff_ns = end_ns - start_ns;
    
    std::cout << "  500μs sleep measured with:\n";
    std::cout << "    Millisecond precision: " << diff_ms.count() << " ms\n";
    std::cout << "    Nanosecond precision: " << diff_ns.count() << " ns ("
              << (static_cast<double>(diff_ns.count()) / 1000.0) << " μs)\n";
    
    // Performance counter for highest precision
    std::cout << "\nHigh-precision timing example:\n";
    sdlpp::timer::performance_counter counter;
    
    // Time some operations
    std::vector<int> data(10000);
    for (int& v : data) {
        v = rand() % 1000;
    }
    
    counter.reset();
    std::sort(data.begin(), data.end());
    auto sort_time = counter.elapsed<std::chrono::nanoseconds>();
    
    std::cout << "  Sorting 10,000 integers took: " << sort_time.count() << " ns ("
              << (static_cast<double>(sort_time.count()) / 1000.0) << " μs)\n";
}

int main() {
    try {
        // Initialize SDL - time functions don't require specific subsystems
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "SDL++ Time Example\n";
        std::cout << "==================\n\n";
        
        demonstrate_date_time();
        demonstrate_clock_conversions();
        demonstrate_time_calculations();
        demonstrate_date_utilities();
        demonstrate_locale_preferences();
        demonstrate_chrono_utils();
        demonstrate_performance_timing();
        
        std::cout << "\n=== Summary ===\n";
        std::cout << "SDL++ provides comprehensive time functionality:\n";
        std::cout << "- Calendar date/time with nanosecond precision\n";
        std::cout << "- Seamless std::chrono integration\n";
        std::cout << "- Date utilities (leap year, day of week, etc.)\n";
        std::cout << "- Locale-aware formatting preferences\n";
        std::cout << "- High-precision timing with performance counters\n";
        std::cout << "- Conversion utilities for all time units\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}