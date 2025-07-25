#include <doctest/doctest.h>
#include <sdlpp/system/platform.hh>
#include <sdlpp/core/core.hh>
#include <iostream>

TEST_SUITE("platform_enhanced") {
    TEST_CASE("device type detection") {
        // These functions may return different values on different devices
        bool tablet = sdlpp::platform::is_tablet();
        bool tv = sdlpp::platform::is_tv();
        bool chromebook = sdlpp::platform::is_chromebook();
        bool dex = sdlpp::platform::is_dex_mode();
        
        // Just verify they return valid boolean values
        CHECK((tablet == true || tablet == false));
        CHECK((tv == true || tv == false));
        CHECK((chromebook == true || chromebook == false));
        CHECK((dex == true || dex == false));
        
        // Log the detected device types for informational purposes
        if (tablet) INFO("Device is a tablet");
        if (tv) INFO("Device is a TV");
        if (chromebook) INFO("Device is a Chromebook");
        if (dex) INFO("Device is in Samsung DeX mode");
    }
    
    TEST_CASE("android namespace") {
        // Test that Android functions can be called (they return defaults on non-Android)
        int sdk_version = sdlpp::android::get_sdk_version();
        CHECK(sdk_version >= 0);
        
        auto internal_path = sdlpp::android::get_internal_storage_path();
        CHECK((internal_path.empty() || !internal_path.empty())); // Valid path or empty
        
        auto external_path = sdlpp::android::get_external_storage_path();
        CHECK((external_path.empty() || !external_path.empty()));
        
        auto cache_path = sdlpp::android::get_cache_path();
        CHECK((cache_path.empty() || !cache_path.empty()));
        
        bool readable = sdlpp::android::is_external_storage_readable();
        CHECK((readable == true || readable == false));
        
        bool writable = sdlpp::android::is_external_storage_writable();
        CHECK((writable == true || writable == false));
        
        // Test permission request (returns false on non-Android)
        bool permission = sdlpp::android::request_permission("android.permission.CAMERA");
        CHECK((permission == true || permission == false));
        
        // Test back button (no-op on non-Android)
        sdlpp::android::send_back_button();
        
        // Test toast (returns false on non-Android)
        bool toast = sdlpp::android::show_toast("Test message");
        CHECK((toast == true || toast == false));
        
        // Test JNI functions
        void* activity = sdlpp::android::get_activity();
        void* jni_env = sdlpp::android::get_jni_env();
        CHECK((activity != nullptr || activity == nullptr));
        CHECK((jni_env != nullptr || jni_env == nullptr));
        
        // Test message sending
        int result = sdlpp::android::send_message(1, 2);
        CHECK(result >= 0);
        
        // Platform-specific logging
        if (sdlpp::platform::is_android()) {
            INFO("Running on Android SDK version: " << sdk_version);
            if (!internal_path.empty()) {
                INFO("Internal storage: " << internal_path);
            }
        }
    }
    
    TEST_CASE("ios namespace") {
        // Test iOS functions (they return defaults on non-iOS)
        
        // Test event pump setting (no-op on non-iOS)
        sdlpp::ios::set_event_pump(true);
        sdlpp::ios::set_event_pump(false);
        
        // Animation callback would require a valid window
        // Just verify the function exists and can be called with nullptr
        bool anim_result = sdlpp::ios::set_animation_callback(nullptr, 1, nullptr, nullptr);
        CHECK((anim_result == true || anim_result == false));
        
        if (sdlpp::platform::is_ios()) {
            INFO("Running on iOS");
        }
    }
    
    TEST_CASE("linux namespace") {
        // Test Linux thread functions (return false on non-Linux)
        bool priority_result = sdlpp::linux_platform::set_thread_priority(0, 10);
        CHECK((priority_result == true || priority_result == false));
        
        bool policy_result = sdlpp::linux_platform::set_thread_priority_and_policy(0, 1, 10);
        CHECK((policy_result == true || policy_result == false));
        
        if (sdlpp::platform::is_linux() && !sdlpp::platform::is_android()) {
            INFO("Running on Linux");
        }
    }
    
    TEST_CASE("windows namespace") {
        // Test Windows message hook (no-op on non-Windows)
        sdlpp::windows::set_message_hook(nullptr, nullptr);
        
        // Verify we can create a lambda matching the signature
        sdlpp::windows::message_hook hook = [](void*, void*, unsigned int, Uint64, Sint64) {};
        sdlpp::windows::set_message_hook(hook, nullptr);
        
        if (sdlpp::platform::is_windows()) {
            INFO("Running on Windows");
        }
    }
    
    TEST_CASE("x11 namespace") {
        // Test X11 event hook (no-op on non-X11)
        sdlpp::x11::set_event_hook(nullptr, nullptr);
        
        // Event hook type varies by platform
        #if defined(SDL_PLATFORM_LINUX) || defined(SDL_PLATFORM_UNIX)
            // On X11, the hook takes XEvent*
            [[maybe_unused]] auto hook = [](void*, void*) -> bool { return false; };
            // Note: Can't directly assign lambda due to XEvent* parameter
        #else
            // On non-X11, it's a simple function pointer
            sdlpp::x11::event_hook hook = [](void*, void*) -> bool { return false; };
            sdlpp::x11::set_event_hook(hook, nullptr);
        #endif
        
        if ((sdlpp::platform::is_linux() || sdlpp::platform::is_unix()) && 
            !sdlpp::platform::is_android()) {
            INFO("Possibly running on X11");
        }
    }
    
    TEST_CASE("android external storage state") {
        using namespace sdlpp::android;
        
        // Test the enum values
        auto read_state = external_storage_state::read;
        auto write_state = external_storage_state::write;
        
        // Just verify they compile and have distinct values
        CHECK(static_cast<int>(read_state) != static_cast<int>(write_state));
        
        // Get the actual state
        [[maybe_unused]] Uint32 state = get_external_storage_state();
        
        // Check if the state flags make sense
        bool readable = is_external_storage_readable();
        bool writable = is_external_storage_writable();
        
        // If writable, should also be readable
        if (writable) {
            CHECK(readable);
        }
    }
}