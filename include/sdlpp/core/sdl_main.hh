//
// Created by igor on 04/03/2026.
//

/**
 * @file sdl_main.hh
 * @brief SDL_main include wrapper with warning suppression
 */
#pragma once

#include <sdlpp/detail/compiler.hh>

#if defined(SDLPP_COMPILER_MSVC)
#pragma warning( push )
#pragma warning( disable : 4820)
#elif defined(SDLPP_COMPILER_GCC)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
# pragma GCC diagnostic ignored "-Wuseless-cast"
#elif defined(SDLPP_COMPILER_CLANG)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
# pragma clang diagnostic ignored "-Wundef"
#elif defined(SDLPP_COMPILER_WASM)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wold-style-cast"
# pragma clang diagnostic ignored "-Wundef"
#endif

#include <SDL3/SDL_main.h>

#if defined(SDLPP_COMPILER_MSVC)
#pragma warning( pop )
#elif defined(SDLPP_COMPILER_GCC)
# pragma GCC diagnostic pop
#elif defined(SDLPP_COMPILER_CLANG)
# pragma clang diagnostic pop
#elif defined(SDLPP_COMPILER_WASM)
# pragma clang diagnostic pop
#endif
