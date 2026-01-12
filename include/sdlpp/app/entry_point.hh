//
// SDL++
// Copyright (C) 2024 Igor Molchanov
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial abstract_applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef SDLPP_ENTRY_POINT_HH
#define SDLPP_ENTRY_POINT_HH

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <sdlpp/app/app.hh>

namespace sdlpp::detail {

    inline abstract_application* g_app = nullptr;

    inline SDL_AppResult app_init(abstract_application* app, int argc, char* argv[]) {
        try {
            app->init_sdl_();
            app->on_init(argc, argv);
            return SDL_APP_CONTINUE;
        } catch (const std::exception& e) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Initialization failed: %s", e.what());
            return SDL_APP_FAILURE;
        } catch (...) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Initialization failed: unknown exception");
            return SDL_APP_FAILURE;
        }
    }

    inline SDL_AppResult app_iterate(abstract_application* app) {
        if (!app->is_running()) {
            return SDL_APP_SUCCESS;
        }

        try {
            app->on_iterate();
            return app->is_running() ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
        } catch (const std::exception& e) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Error in iterate: %s", e.what());
            return SDL_APP_FAILURE;
        } catch (...) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Error in iterate: unknown exception");
            return SDL_APP_FAILURE;
        }
    }

    inline SDL_AppResult app_handle_event(abstract_application* app, const SDL_Event* sdl_event) {
        if (!app->is_running()) {
            return SDL_APP_SUCCESS;
        }

        try {
            event e(*sdl_event);
            app->on_event(e);
            return app->is_running() ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
        } catch (const std::exception& ex) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Error in event handler: %s", ex.what());
            return SDL_APP_FAILURE;
        } catch (...) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "Error in event handler: unknown exception");
            return SDL_APP_FAILURE;
        }
    }

    inline void app_quit(abstract_application* app) {
        try {
            app->on_quit();
        } catch (...) {
            // on_quit must not throw, but swallow anyway
        }
        app->shutdown_sdl_();
    }

} // namespace sdlpp::detail

/**
 * @brief Main macro for SDL3 abstract_application
 *
 * Defines SDL3 app callbacks and sets up the abstract_application instance.
 * Must be used in exactly one source file.
 *
 * @param AppClass The abstract_application class (must derive from sdlpp::abstract_application)
 */
#define SDLPP_MAIN(AppClass) \
    static AppClass g_app_instance; \
    \
    extern "C" SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[]) { \
        sdlpp::detail::g_app = &g_app_instance; \
        *appstate = sdlpp::detail::g_app; \
        return sdlpp::detail::app_init(sdlpp::detail::g_app, argc, argv); \
    } \
    \
    extern "C" SDL_AppResult SDL_AppIterate(void* appstate) { \
        return sdlpp::detail::app_iterate( \
            static_cast<sdlpp::abstract_application*>(appstate)); \
    } \
    \
    extern "C" SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event) { \
        return sdlpp::detail::app_handle_event( \
            static_cast<sdlpp::abstract_application*>(appstate), event); \
    } \
    \
    extern "C" void SDL_AppQuit(void* appstate, SDL_AppResult) { \
        sdlpp::detail::app_quit( \
            static_cast<sdlpp::abstract_application*>(appstate)); \
    }

#endif // SDLPP_ENTRY_POINT_HH
