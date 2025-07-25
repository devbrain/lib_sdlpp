//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <sdlpp/video/gl.hh>
#include <sdlpp/core/version.hh>
#include <sdlpp/core/sdl.hh>

TEST_CASE("OpenGL context management") {
    using namespace sdlpp;
    
    // Skip tests if running in CI or without display
    if (std::getenv("CI") || !std::getenv("DISPLAY")) {
        //WARN("Skipping OpenGL tests - no display available");
        return;
    }
    
    // Initialize SDL video subsystem
    if (!SDL_Init(SDL_INIT_VIDEO)) {
       // WARN("Failed to initialize SDL video");
        return;
    }
    
    SUBCASE("GL attribute configuration") {
        // Reset attributes to known state
        gl::reset_attributes();
        
        // Test setting individual attributes
        CHECK(gl::set_attribute(gl_attr::red_size, 8));
        CHECK(gl::set_attribute(gl_attr::green_size, 8));
        CHECK(gl::set_attribute(gl_attr::blue_size, 8));
        CHECK(gl::set_attribute(gl_attr::alpha_size, 8));
        CHECK(gl::set_attribute(gl_attr::doublebuffer, 1));
        CHECK(gl::set_attribute(gl_attr::depth_size, 24));
        
        // Test getting attributes (may fail before context creation)
        auto red = gl::get_attribute(gl_attr::red_size);
        if (red.has_value()) {
            CHECK(red.value() == 8);
        }
    }
    
    SUBCASE("Attribute config builder") {
        gl::reset_attributes();
        
        // Test core profile configuration
        auto config = gl::attribute_config::core_profile(3, 3);
        CHECK(config.major_version.value() == 3);
        CHECK(config.minor_version.value() == 3);
        CHECK(config.profile.value() == gl_profile::core);
        CHECK(config.doublebuffer.value() == true);
        CHECK(config.depth_size.value() == 24);
        
        // Apply configuration
        CHECK(config.apply());
    }
    
    SUBCASE("ES profile configuration") {
        gl::reset_attributes();
        
        auto config = gl::attribute_config::es_profile(3, 0);
        CHECK(config.major_version.value() == 3);
        CHECK(config.minor_version.value() == 0);
        CHECK(config.profile.value() == gl_profile::es);
        
        CHECK(config.apply());
    }
    
    SUBCASE("Custom attribute configuration") {
        gl::reset_attributes();
        
        gl::attribute_config config;
        config.red_size = 5;
        config.green_size = 6;
        config.blue_size = 5;
        config.depth_size = 16;
        config.stencil_size = 8;
        config.multisamplesamples = 4;
        config.context_flags = static_cast<Uint32>(gl_context_flag::debug);
        
        CHECK(config.apply());
    }
    
    SDL_Quit();
}

TEST_CASE("OpenGL library loading") {
    using namespace sdlpp;
    
    SUBCASE("RAII library loader") {
        {
            gl_library lib;
            // Library should load by default
            // Can't reliably test this without a display
        }
        // Library should be unloaded when out of scope
    }
    
    SUBCASE("Manual library management") {
        // These may fail without proper GL support
        [[maybe_unused]] bool loaded = gl::load_library();
        gl::unload_library();
    }
}

TEST_CASE("OpenGL enums") {
    using namespace sdlpp;
    
    SUBCASE("Profile enum values") {
        CHECK(static_cast<Uint32>(gl_profile::core) == SDL_GL_CONTEXT_PROFILE_CORE);
        CHECK(static_cast<Uint32>(gl_profile::compatibility) == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        CHECK(static_cast<Uint32>(gl_profile::es) == SDL_GL_CONTEXT_PROFILE_ES);
    }
    
    SUBCASE("Context flag enum values") {
        CHECK(static_cast<Uint32>(gl_context_flag::debug) == SDL_GL_CONTEXT_DEBUG_FLAG);
        CHECK(static_cast<Uint32>(gl_context_flag::forward_compatible) == SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
        CHECK(static_cast<Uint32>(gl_context_flag::robust_access) == SDL_GL_CONTEXT_ROBUST_ACCESS_FLAG);
        CHECK(static_cast<Uint32>(gl_context_flag::reset_isolation) == SDL_GL_CONTEXT_RESET_ISOLATION_FLAG);
    }
    
    SUBCASE("Release behavior enum values") {
        CHECK(static_cast<Uint32>(gl_release_behavior::none) == SDL_GL_CONTEXT_RELEASE_BEHAVIOR_NONE);
        CHECK(static_cast<Uint32>(gl_release_behavior::flush) == SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH);
    }
    
    SUBCASE("Reset notification enum values") {
        CHECK(static_cast<Uint32>(gl_reset_notification::no_notification) == SDL_GL_CONTEXT_RESET_NO_NOTIFICATION);
        CHECK(static_cast<Uint32>(gl_reset_notification::lose_context) == SDL_GL_CONTEXT_RESET_LOSE_CONTEXT);
    }
}

TEST_CASE("OpenGL context class") {
    using namespace sdlpp;
    
    SUBCASE("Default construction") {
        gl_context ctx;
        CHECK(!ctx.is_valid());
        CHECK(!ctx);
        CHECK(ctx.get() == nullptr);
    }
    
    SUBCASE("Move semantics") {
        gl_context ctx1;
        gl_context ctx2(std::move(ctx1));
        CHECK(!ctx1.is_valid());
        
        gl_context ctx3;
        ctx3 = std::move(ctx2);
        CHECK(!ctx2.is_valid());
    }
    
    SUBCASE("Release ownership") {
        gl_context ctx(reinterpret_cast<SDL_GLContext>(0x1234)); // Fake handle
        SDL_GLContext handle = ctx.release();
        CHECK(handle == reinterpret_cast<SDL_GLContext>(0x1234));
        CHECK(!ctx.is_valid());
    }
}

TEST_CASE("EGL utilities") {
    using namespace sdlpp;
    
    SUBCASE("EGL attribute callbacks") {
        egl::attribute_callbacks callbacks;
        
        // Test fluent interface
        callbacks
            .set_platform_callback(nullptr)
            .set_surface_callback(nullptr)
            .set_context_callback(nullptr)
            .set_userdata(nullptr);
            
        // Apply would set the callbacks in SDL
        // callbacks.apply();
    }
}

TEST_CASE("OpenGL version detection") {
    using namespace sdlpp;
    
    // Check if we're running SDL 3.x
    auto runtime_ver = version_info::runtime();
    if (runtime_ver.major() >= 3) {
        INFO("Running with SDL " << runtime_ver);
        // All GL features should be available in SDL3
    }
}
