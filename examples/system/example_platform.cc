#include <iostream>
#include <iomanip>
#include <sdlpp/system/platform.hh>
#include <sdlpp/core/core.hh>

void print_platform_info() {
    std::cout << "\n=== Platform Information ===\n";
    
    auto info = sdlpp::platform::get_platform_info();
    
    std::cout << "Platform: " << info.name << "\n";
    std::cout << "Category: ";
    switch (info.category) {
        case sdlpp::platform::platform_category::desktop:
            std::cout << "Desktop\n";
            break;
        case sdlpp::platform::platform_category::mobile:
            std::cout << "Mobile\n";
            break;
        case sdlpp::platform::platform_category::web:
            std::cout << "Web\n";
            break;
        case sdlpp::platform::platform_category::console:
            std::cout << "Console\n";
            break;
        case sdlpp::platform::platform_category::embedded:
            std::cout << "Embedded\n";
            break;
        case sdlpp::platform::platform_category::unknown:
            std::cout << "Unknown\n";
            break;
    }
    
    std::cout << "Architecture: " << (info.is_64bit ? "64-bit" : "32-bit") << "\n";
    std::cout << "Byte order: " << (info.is_big_endian ? "Big Endian" : "Little Endian") << "\n";
    
    // Platform checks
    std::cout << "\nPlatform checks:\n";
    std::cout << "  Windows: " << (sdlpp::platform::is_windows() ? "Yes" : "No") << "\n";
    std::cout << "  macOS: " << (sdlpp::platform::is_macos() ? "Yes" : "No") << "\n";
    std::cout << "  Linux: " << (sdlpp::platform::is_linux() ? "Yes" : "No") << "\n";
    std::cout << "  Android: " << (sdlpp::platform::is_android() ? "Yes" : "No") << "\n";
    std::cout << "  iOS: " << (sdlpp::platform::is_ios() ? "Yes" : "No") << "\n";
    std::cout << "  tvOS: " << (sdlpp::platform::is_tvos() ? "Yes" : "No") << "\n";
    std::cout << "  Apple platform: " << (sdlpp::platform::is_apple() ? "Yes" : "No") << "\n";
    std::cout << "  BSD: " << (sdlpp::platform::is_bsd() ? "Yes" : "No") << "\n";
    std::cout << "  Unix-like: " << (sdlpp::platform::is_unix() ? "Yes" : "No") << "\n";
    std::cout << "  Emscripten: " << (sdlpp::platform::is_emscripten() ? "Yes" : "No") << "\n";
}

void print_power_info() {
    std::cout << "\n=== Power Information ===\n";
    
    auto power = sdlpp::power::get_power_info();
    
    std::cout << "Power state: ";
    switch (power.state) {
        case sdlpp::power_state::unknown:
            std::cout << "Unknown\n";
            break;
        case sdlpp::power_state::on_battery:
            std::cout << "On Battery\n";
            break;
        case sdlpp::power_state::no_battery:
            std::cout << "No Battery (Plugged In)\n";
            break;
        case sdlpp::power_state::charging:
            std::cout << "Charging\n";
            break;
        case sdlpp::power_state::charged:
            std::cout << "Fully Charged\n";
            break;
        case sdlpp::power_state::error:
            std::cout << "Error\n";
            break;
    }
    
    if (power.has_battery()) {
        if (power.percent_left >= 0) {
            std::cout << "Battery level: " << power.percent_left << "%\n";
        }
        if (power.seconds_left >= 0) {
            int hours = power.seconds_left / 3600;
            int minutes = (power.seconds_left % 3600) / 60;
            std::cout << "Time remaining: " << hours << "h " << minutes << "m\n";
        }
    }
    
    std::cout << "Plugged in: " << (power.is_plugged_in() ? "Yes" : "No") << "\n";
    std::cout << "Has battery: " << (power.has_battery() ? "Yes" : "No") << "\n";
}

void print_directories() {
    std::cout << "\n=== System Directories ===\n";
    
    auto base = sdlpp::directories::get_base_path();
    std::cout << "Base path: " << base.string() << "\n";
    
    // Get preferences path
    auto pref_path = sdlpp::directories::get_pref_path("ExampleOrg", "PlatformExample");
    std::cout << "Preferences path: " << pref_path.string() << "\n";
    
    // User folders
    std::cout << "\nUser folders:\n";
    auto home = sdlpp::directories::get_home_folder();
    if (!home.empty()) std::cout << "  Home: " << home.string() << "\n";
    
    auto desktop = sdlpp::directories::get_desktop_folder();
    if (!desktop.empty()) std::cout << "  Desktop: " << desktop.string() << "\n";
    
    auto documents = sdlpp::directories::get_documents_folder();
    if (!documents.empty()) std::cout << "  Documents: " << documents.string() << "\n";
    
    auto downloads = sdlpp::directories::get_downloads_folder();
    if (!downloads.empty()) std::cout << "  Downloads: " << downloads.string() << "\n";
    
    auto music = sdlpp::directories::get_music_folder();
    if (!music.empty()) std::cout << "  Music: " << music.string() << "\n";
    
    auto pictures = sdlpp::directories::get_pictures_folder();
    if (!pictures.empty()) std::cout << "  Pictures: " << pictures.string() << "\n";
    
    auto videos = sdlpp::directories::get_videos_folder();
    if (!videos.empty()) std::cout << "  Videos: " << videos.string() << "\n";
    
    auto screenshots = sdlpp::directories::get_screenshots_folder();
    if (!screenshots.empty()) std::cout << "  Screenshots: " << screenshots.string() << "\n";
    
    auto saved_games = sdlpp::directories::get_saved_games_folder();
    if (!saved_games.empty()) std::cout << "  Saved Games: " << saved_games.string() << "\n";
    
    // Additional folders
    auto publicshare = sdlpp::directories::get_publicshare_folder();
    if (!publicshare.empty()) std::cout << "  Public Share: " << publicshare.string() << "\n";
    
    auto templates = sdlpp::directories::get_templates_folder();
    if (!templates.empty()) std::cout << "  Templates: " << templates.string() << "\n";
}

void environment_example() {
    std::cout << "\n=== Environment Variables ===\n";
    
    // Get some common environment variables
    auto path = sdlpp::environment::get_env("PATH");
    if (!path.empty()) {
        std::cout << "PATH length: " << path.length() << " characters\n";
    }
    
    auto home = sdlpp::environment::get_env("HOME");
    if (!home.empty()) {
        std::cout << "HOME: " << home << "\n";
    }
    
    // Set a custom environment variable
    const std::string test_var = "SDLPP_TEST_VAR";
    const std::string test_value = "Hello from SDL++";
    
    if (sdlpp::environment::set_env(test_var, test_value)) {
        std::cout << "\nSet " << test_var << " = " << test_value << "\n";
        
        // Read it back
        auto read_value = sdlpp::environment::get_env(test_var);
        std::cout << "Read back: " << read_value << "\n";
        
        // Unset it
        if (sdlpp::environment::unset_env(test_var)) {
            std::cout << "Unset " << test_var << "\n";
            
            // Verify it's gone
            auto gone = sdlpp::environment::get_env(test_var);
            std::cout << "After unset: " << (gone.empty() ? "empty" : gone) << "\n";
        }
    }
}

int main() {
    try {
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "SDL Platform Information Example\n";
        std::cout << "================================\n";
        
        print_platform_info();
        print_power_info();
        print_directories();
        environment_example();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}