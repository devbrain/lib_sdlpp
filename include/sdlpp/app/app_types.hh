//
// SDL++
// Copyright (C) 2024 Igor Molchanov
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
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

#ifndef SDLPP_APP_APP_TYPES_HH
#define SDLPP_APP_APP_TYPES_HH

#include <SDL3/SDL.h>

namespace sdlpp {
    
    /**
     * @brief Wrapper for SDL_AppResult
     * 
     * SDL3 uses SDL_AppResult for app callbacks return values
     */
    enum class app_result {
        continue_ = SDL_APP_CONTINUE,   // Continue iterating
        success = SDL_APP_SUCCESS,      // Terminate with success  
        failure = SDL_APP_FAILURE       // Terminate with error
    };
    
    /**
     * @brief Convert bool to SDL_AppResult
     */
    inline SDL_AppResult to_sdl_result(bool success) {
        return success ? SDL_APP_CONTINUE : SDL_APP_FAILURE;
    }
    
    /**
     * @brief Convert app_result to SDL_AppResult
     */
    inline SDL_AppResult to_sdl_result(app_result res) {
        return static_cast<SDL_AppResult>(res);
    }
    
} // namespace sdlpp

#endif // SDLPP_APP_APP_TYPES_HH