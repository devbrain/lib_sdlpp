//
// Created by igor on 7/13/25.
//

/**
 * @file pointer.hh
 * @brief Smart pointer utilities for SDL resources
 */
#pragma once
#include <memory>

namespace sdlpp {
    template<typename T, auto DestroyFunc>
    struct sdl_deleter {
        void operator()(T* ptr) const noexcept {
            if (ptr) {
                DestroyFunc(ptr);
            }
        }
    };

    template<typename T, auto DestroyFunc>
    using pointer = std::unique_ptr <T, sdl_deleter <T, DestroyFunc>>;
}
