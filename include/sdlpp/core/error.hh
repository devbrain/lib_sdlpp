/**
 * @file error.hh
 * @brief Error handling utilities
 */
#pragma once

#include <sdlpp/core/sdl.hh>

#include <failsafe/detail/string_utils.hh>

#include <string>

namespace sdlpp {
    
    // Note: string building utilities are now in detail/string_utils.hh
    // They provide enhanced type support including filesystem::path, chrono, optional, variant
    
    /**
     * @brief Get the last error message
     * 
     * Retrieves a human-readable message for the last error that occurred on the
     * calling thread. This function is thread-safe as SDL maintains per-thread error state.
     * 
     * @return The error message, or an empty string if no error is set
     */
    [[nodiscard]] inline std::string get_error() noexcept {
        const char* error = SDL_GetError();
        return error ? std::string{error} : std::string{};
    }
    
    /**
     * @brief Clear the current error state
     * 
     * Clears any error message set on the calling thread.
     */
    inline void clear_error() noexcept {
        SDL_ClearError();
    }
    
    /**
     * @brief Set an error message using modern C++ string building
     * 
     * Sets a custom error message by concatenating all arguments with spaces.
     * This is a type-safe alternative to SDL's printf-style formatting.
     * 
     * @tparam Args Variadic template parameter pack
     * @param args Arguments to concatenate into the error message
     * @return false (following SDL convention for error returns)
     * 
     * @example
     * ```cpp
     * set_error("Failed to load texture:", filename, "with size", width, "x", height);
     * ```
     */
    template<typename... Args>
    [[nodiscard]] inline bool set_error(Args&&... args) noexcept {
        std::string message = failsafe::detail::build_message(std::forward<Args>(args)...);
        SDL_SetError("%s", message.c_str());
        return false;
    }
    
    /**
     * @brief Set an out of memory error
     * 
     * Convenience function that sets a standard out-of-memory error message.
     * 
     * @return false (following SDL convention for error returns)
     */
    [[nodiscard]] inline bool set_out_of_memory_error() noexcept {
        SDL_OutOfMemory();
        return false;
    }
    
    /**
     * @brief Set an unsupported operation error
     * 
     * Convenience function that sets a standard unsupported operation error message.
     * 
     * @return false (following SDL convention for error returns)
     */
    [[nodiscard]] inline bool set_unsupported_error() noexcept {
        SDL_Unsupported();
        return false;
    }
    
    /**
     * @brief Set an invalid parameter error
     * 
     * Sets an error message indicating that an invalid parameter was passed.
     * 
     * @param param The name of the invalid parameter
     * @return false (following SDL convention for error returns)
     */
    [[nodiscard]] inline bool set_invalid_param_error(const std::string& param) noexcept {
        SDL_InvalidParamError(param.c_str());
        return false;
    }
    
    /**
     * @brief RAII guard to preserve error state
     * 
     * This class saves the current error state on construction and restores it
     * on destruction. Useful when you need to perform operations that might
     * change the error state but want to preserve the original error.
     */
    class error_guard {
    public:
        error_guard() noexcept : saved_error_{get_error()} {
            clear_error();
        }
        
        ~error_guard() noexcept {
            if (!saved_error_.empty()) {
                SDL_SetError("%s", saved_error_.c_str());
            } else {
                // If there was no error initially, clear any error that was set
                SDL_ClearError();
            }
        }
        
        error_guard(const error_guard&) = delete;
        error_guard& operator=(const error_guard&) = delete;
        error_guard(error_guard&&) = delete;
        error_guard& operator=(error_guard&&) = delete;
        
        /**
         * @brief Get the saved error message
         * @return The error message that was saved on construction
         */
        [[nodiscard]] const std::string& saved_error() const noexcept {
            return saved_error_;
        }
        
    private:
        std::string saved_error_;
    };
    
    /**
     * @brief Scoped error clearer
     * 
     * Clears the error state on construction and optionally on destruction.
     * Useful for ensuring a clean error state within a specific scope.
     */
    class error_scope {
    public:
        explicit error_scope(bool clear_on_exit = true) noexcept 
            : clear_on_exit_{clear_on_exit} {
            clear_error();
        }
        
        ~error_scope() noexcept {
            if (clear_on_exit_) {
                clear_error();
            }
        }
        
        error_scope(const error_scope&) = delete;
        error_scope& operator=(const error_scope&) = delete;
        error_scope(error_scope&&) = delete;
        error_scope& operator=(error_scope&&) = delete;
        
    private:
        bool clear_on_exit_;
    };
    
} // namespace sdlpp