#pragma once

/**
 * @file power_state.hh
 * @brief Power state enumeration
 * 
 * This header defines the power state enum used by battery/power management.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>

namespace sdlpp {
    /**
     * @brief Power states for battery-powered devices
     */
    enum class power_state {
        error = SDL_POWERSTATE_ERROR, ///< Error determining power status
        unknown = SDL_POWERSTATE_UNKNOWN, ///< Cannot determine power status
        on_battery = SDL_POWERSTATE_ON_BATTERY, ///< Not plugged in, running on battery
        no_battery = SDL_POWERSTATE_NO_BATTERY, ///< Plugged in, no battery available
        charging = SDL_POWERSTATE_CHARGING, ///< Plugged in, battery charging
        charged = SDL_POWERSTATE_CHARGED ///< Plugged in, battery fully charged
    };
} // namespace sdlpp

// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
    /**
     * @brief Stream output operator for power_state
     */
    SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, power_state value);

    /**
     * @brief Stream input operator for power_state
     */
    SDLPP_EXPORT std::istream& operator>>(std::istream& is, power_state& value);
}
