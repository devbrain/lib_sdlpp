//
// Created by igor on 7/15/25.
//

/**
 * @file sdl.hh
 * @brief Core SDL include wrapper
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
#elif defined(SDLPP_COMPILER_WASM)
# pragma clang diagnostic push
#endif



#include <SDL3/SDL.h>
#if defined(SDL_MAIN_USE_CALLBACKS)
#include <SDL3/SDL_main.h>
#endif

#if defined(SDLPP_COMPILER_MSVC)
#pragma warning( pop )
#elif defined(SDLPP_COMPILER_GCC)
# pragma GCC diagnostic pop
#elif defined(SDLPP_COMPILER_CLANG)
# pragma clang diagnostic pop
#elif defined(SDLPP_COMPILER_WASM)
# pragma clang diagnostic pop
#endif
