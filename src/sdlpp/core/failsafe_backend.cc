//
// Created by igor on 7/20/25.
//

/**
 * @file failsafe_backend.cc
 * @brief Implementation file for failsafe backend
 * 
 * This file is intentionally minimal as most of the failsafe_backend
 * implementation is header-only for performance reasons.
 */

#include <sdlpp/core/failsafe_backend.hh>

namespace sdlpp {
    // All implementation is currently in the header file
    // This file exists to:
    // 1. Provide a compilation unit for the backend
    // 2. Allow for future non-inline implementations if needed
    // 3. Ensure proper linking when the backend is used across multiple translation units
} // namespace sdlpp