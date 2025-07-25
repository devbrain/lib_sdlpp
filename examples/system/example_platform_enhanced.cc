#include <iostream>
#include <iomanip>
#include <sdlpp/core/core.hh>
#include <sdlpp/system/platform.hh>

void device_type_info() {
    std::cout << "\n=== Device Type Information ===\n";
    
    std::cout << "Device type detection:\n";
    std::cout << "  Tablet: " << std::boolalpha << sdlpp::platform::is_tablet() << "\n";
    std::cout << "  TV: " << sdlpp::platform::is_tv() << "\n";
    std::cout << "  Chromebook: " << sdlpp::platform::is_chromebook() << "\n";
    std::cout << "  Samsung DeX: " << sdlpp::platform::is_dex_mode() << "\n";
}

void android_specific_example() {
    std::cout << "\n=== Android-Specific Features ===\n";
    
    if (!sdlpp::platform::is_android()) {
        std::cout << "Not running on Android - functions return default values\n";
    }
    
    // SDK Version
    int sdk_version = sdlpp::android::get_sdk_version();
    std::cout << "Android SDK version: " << sdk_version;
    if (sdk_version > 0) {
        if (sdk_version >= 33) std::cout << " (Android 13+)";
        else if (sdk_version >= 31) std::cout << " (Android 12)";
        else if (sdk_version >= 30) std::cout << " (Android 11)";
        else if (sdk_version >= 29) std::cout << " (Android 10)";
    }
    std::cout << "\n";
    
    // Storage paths
    auto internal_path = sdlpp::android::get_internal_storage_path();
    auto external_path = sdlpp::android::get_external_storage_path();
    auto cache_path = sdlpp::android::get_cache_path();
    
    std::cout << "\nStorage paths:\n";
    std::cout << "  Internal: " << (internal_path.empty() ? "<not available>" : internal_path.string()) << "\n";
    std::cout << "  External: " << (external_path.empty() ? "<not available>" : external_path.string()) << "\n";
    std::cout << "  Cache: " << (cache_path.empty() ? "<not available>" : cache_path.string()) << "\n";
    
    // External storage state
    std::cout << "\nExternal storage state:\n";
    std::cout << "  Readable: " << std::boolalpha << sdlpp::android::is_external_storage_readable() << "\n";
    std::cout << "  Writable: " << sdlpp::android::is_external_storage_writable() << "\n";
    
    // Example of requesting permission (on Android)
    if (sdlpp::platform::is_android()) {
        std::cout << "\nRequesting camera permission...\n";
        bool requested = sdlpp::android::request_permission("android.permission.CAMERA");
        std::cout << "Permission request initiated: " << requested << "\n";
        
        // Show a toast message
        std::cout << "Showing toast message...\n";
        bool toast_shown = sdlpp::android::show_toast("Hello from SDL++!", 0);
        std::cout << "Toast shown: " << toast_shown << "\n";
    }
    
    // JNI access (for advanced users)
    void* activity = sdlpp::android::get_activity();
    void* jni_env = sdlpp::android::get_jni_env();
    std::cout << "\nJNI access:\n";
    std::cout << "  Activity handle: " << activity << "\n";
    std::cout << "  JNI environment: " << jni_env << "\n";
}

void ios_specific_example() {
    std::cout << "\n=== iOS-Specific Features ===\n";
    
    if (!sdlpp::platform::is_ios()) {
        std::cout << "Not running on iOS - functions return default values\n";
    }
    
    // Event pump control
    std::cout << "iOS event pump can be controlled with set_event_pump()\n";
    
    // Animation callback info
    std::cout << "Animation callbacks can be set for smooth animations\n";
    std::cout << "Use set_animation_callback() with a window handle\n";
}

void linux_specific_example() {
    std::cout << "\n=== Linux-Specific Features ===\n";
    
    if (!sdlpp::platform::is_linux() || sdlpp::platform::is_android()) {
        std::cout << "Not running on Linux - functions return default values\n";
        return;
    }
    
    std::cout << "Thread priority management available:\n";
    std::cout << "  set_thread_priority() - Set thread priority\n";
    std::cout << "  set_thread_priority_and_policy() - Set priority and scheduling policy\n";
    
    // Example of setting thread priority (would need actual thread ID)
    Sint64 fake_thread_id = 0;
    bool result = sdlpp::linux_platform::set_thread_priority(fake_thread_id, 10);
    std::cout << "\nExample priority setting result: " << std::boolalpha << result << "\n";
    
    // Scheduling policies
    std::cout << "\nAvailable scheduling policies:\n";
    std::cout << "  SCHED_OTHER (0) - Default\n";
    std::cout << "  SCHED_FIFO (1) - First-in-first-out\n";
    std::cout << "  SCHED_RR (2) - Round-robin\n";
}

void windows_specific_example() {
    std::cout << "\n=== Windows-Specific Features ===\n";
    
    if (!sdlpp::platform::is_windows()) {
        std::cout << "Not running on Windows - functions return default values\n";
        return;
    }
    
    std::cout << "Windows message hook available for intercepting messages\n";
    std::cout << "Use set_message_hook() to install a custom message handler\n";
    
    // Example message hook
    auto message_handler = [](void* userdata, void* hwnd, unsigned int message, 
                            Uint64 wparam, Sint64 lparam) {
        (void)userdata;
        (void)hwnd;
        (void)lparam;
        // Log interesting messages
        if (message == 0x0100) { // WM_KEYDOWN
            std::cout << "Key down: wparam=" << wparam << "\n";
        }
    };
    
    std::cout << "\nInstalling example message hook...\n";
    sdlpp::windows::set_message_hook(message_handler, nullptr);
}

void x11_specific_example() {
    std::cout << "\n=== X11-Specific Features ===\n";
    
    if ((!sdlpp::platform::is_linux() && !sdlpp::platform::is_unix()) || 
        sdlpp::platform::is_android()) {
        std::cout << "Not running on X11 - functions return default values\n";
        return;
    }
    
    std::cout << "X11 event hook available for intercepting X11 events\n";
    std::cout << "Use set_event_hook() to install a custom event handler\n";
    
    // Example event hook
    std::cout << "\nX11 event hook can be installed with proper XEvent* handling\n";
    
    // Note: On actual X11 systems, the hook would take XEvent* as second parameter
    // For demonstration, we just show how to install a null hook
    std::cout << "Installing null X11 event hook...\n";
    sdlpp::x11::set_event_hook(nullptr, nullptr);
}

void platform_specific_paths() {
    std::cout << "\n=== Platform-Specific Paths ===\n";
    
    if (sdlpp::platform::is_android()) {
        std::cout << "\nAndroid storage hierarchy:\n";
        std::cout << "  Internal storage: App-private files\n";
        std::cout << "  External storage: Shared files (requires permission)\n";
        std::cout << "  Cache: Temporary files (may be deleted by system)\n";
    } else if (sdlpp::platform::is_ios()) {
        std::cout << "\niOS storage hierarchy:\n";
        std::cout << "  Documents: User-visible files (backed up)\n";
        std::cout << "  Library: App support files (backed up)\n";
        std::cout << "  Caches: Temporary files (not backed up)\n";
        std::cout << "  tmp: Temporary files (deleted on reboot)\n";
    } else {
        std::cout << "\nDesktop storage locations:\n";
        auto base = sdlpp::directories::get_base_path();
        auto pref = sdlpp::directories::get_pref_path("MyCompany", "MyApp");
        std::cout << "  Base path: " << base << "\n";
        std::cout << "  Preferences: " << pref << "\n";
    }
}

int main() {
    try {
        // Initialize SDL
        sdlpp::init sdl_init(sdlpp::init_flags::video);
        
        std::cout << "Enhanced Platform Information Example\n";
        std::cout << "====================================\n";
        
        // Basic platform info
        auto info = sdlpp::platform::get_platform_info();
        std::cout << "\nPlatform: " << info.name << "\n";
        std::cout << "Category: ";
        switch (info.category) {
            case sdlpp::platform::platform_category::desktop:
                std::cout << "Desktop";
                break;
            case sdlpp::platform::platform_category::mobile:
                std::cout << "Mobile";
                break;
            case sdlpp::platform::platform_category::web:
                std::cout << "Web";
                break;
            case sdlpp::platform::platform_category::console:
                std::cout << "Console";
                break;
            case sdlpp::platform::platform_category::embedded:
                std::cout << "Embedded";
                break;
            default:
                std::cout << "Unknown";
        }
        std::cout << "\n";
        
        // Device type information
        device_type_info();
        
        // Platform-specific examples
        android_specific_example();
        ios_specific_example();
        linux_specific_example();
        windows_specific_example();
        x11_specific_example();
        
        // Platform-specific paths
        platform_specific_paths();
        
        std::cout << "\n=== Summary ===\n";
        std::cout << "This example demonstrates platform-specific features.\n";
        std::cout << "Most functions are only functional on their target platforms.\n";
        std::cout << "Cross-platform apps should check platform before using these features.\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}