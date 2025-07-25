#pragma once

/**
 * @file keyboard_codes.hh
 * @brief Keyboard key codes and scancode definitions
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>

namespace sdlpp {
    /**
     * @brief Keyboard scancode enumeration
     * Note: This is a direct mapping to SDL_Scancode
     */
    enum class scancode : int {
        unknown = SDL_SCANCODE_UNKNOWN,

        // Letters
        a = SDL_SCANCODE_A,
        b = SDL_SCANCODE_B,
        c = SDL_SCANCODE_C,
        d = SDL_SCANCODE_D,
        e = SDL_SCANCODE_E,
        f = SDL_SCANCODE_F,
        g = SDL_SCANCODE_G,
        h = SDL_SCANCODE_H,
        i = SDL_SCANCODE_I,
        j = SDL_SCANCODE_J,
        k = SDL_SCANCODE_K,
        l = SDL_SCANCODE_L,
        m = SDL_SCANCODE_M,
        n = SDL_SCANCODE_N,
        o = SDL_SCANCODE_O,
        p = SDL_SCANCODE_P,
        q = SDL_SCANCODE_Q,
        r = SDL_SCANCODE_R,
        s = SDL_SCANCODE_S,
        t = SDL_SCANCODE_T,
        u = SDL_SCANCODE_U,
        v = SDL_SCANCODE_V,
        w = SDL_SCANCODE_W,
        x = SDL_SCANCODE_X,
        y = SDL_SCANCODE_Y,
        z = SDL_SCANCODE_Z,

        // Numbers
        num_1 = SDL_SCANCODE_1,
        num_2 = SDL_SCANCODE_2,
        num_3 = SDL_SCANCODE_3,
        num_4 = SDL_SCANCODE_4,
        num_5 = SDL_SCANCODE_5,
        num_6 = SDL_SCANCODE_6,
        num_7 = SDL_SCANCODE_7,
        num_8 = SDL_SCANCODE_8,
        num_9 = SDL_SCANCODE_9,
        num_0 = SDL_SCANCODE_0,

        // Function keys
        f1 = SDL_SCANCODE_F1,
        f2 = SDL_SCANCODE_F2,
        f3 = SDL_SCANCODE_F3,
        f4 = SDL_SCANCODE_F4,
        f5 = SDL_SCANCODE_F5,
        f6 = SDL_SCANCODE_F6,
        f7 = SDL_SCANCODE_F7,
        f8 = SDL_SCANCODE_F8,
        f9 = SDL_SCANCODE_F9,
        f10 = SDL_SCANCODE_F10,
        f11 = SDL_SCANCODE_F11,
        f12 = SDL_SCANCODE_F12,
        f13 = SDL_SCANCODE_F13,
        f14 = SDL_SCANCODE_F14,
        f15 = SDL_SCANCODE_F15,
        f16 = SDL_SCANCODE_F16,
        f17 = SDL_SCANCODE_F17,
        f18 = SDL_SCANCODE_F18,
        f19 = SDL_SCANCODE_F19,
        f20 = SDL_SCANCODE_F20,
        f21 = SDL_SCANCODE_F21,
        f22 = SDL_SCANCODE_F22,
        f23 = SDL_SCANCODE_F23,
        f24 = SDL_SCANCODE_F24,

        // Control keys
        return_key = SDL_SCANCODE_RETURN,
        escape = SDL_SCANCODE_ESCAPE,
        backspace = SDL_SCANCODE_BACKSPACE,
        tab = SDL_SCANCODE_TAB,
        space = SDL_SCANCODE_SPACE,

        // Punctuation
        minus = SDL_SCANCODE_MINUS,
        equals = SDL_SCANCODE_EQUALS,
        leftbracket = SDL_SCANCODE_LEFTBRACKET,
        rightbracket = SDL_SCANCODE_RIGHTBRACKET,
        backslash = SDL_SCANCODE_BACKSLASH,
        nonushash = SDL_SCANCODE_NONUSHASH,
        semicolon = SDL_SCANCODE_SEMICOLON,
        apostrophe = SDL_SCANCODE_APOSTROPHE,
        grave = SDL_SCANCODE_GRAVE,
        comma = SDL_SCANCODE_COMMA,
        period = SDL_SCANCODE_PERIOD,
        slash = SDL_SCANCODE_SLASH,

        // Lock keys
        capslock = SDL_SCANCODE_CAPSLOCK,
        scrolllock = SDL_SCANCODE_SCROLLLOCK,
        numlockclear = SDL_SCANCODE_NUMLOCKCLEAR,

        // Navigation
        printscreen = SDL_SCANCODE_PRINTSCREEN,
        pause = SDL_SCANCODE_PAUSE,
        insert = SDL_SCANCODE_INSERT,
        home = SDL_SCANCODE_HOME,
        pageup = SDL_SCANCODE_PAGEUP,
        delete_key = SDL_SCANCODE_DELETE,
        end = SDL_SCANCODE_END,
        pagedown = SDL_SCANCODE_PAGEDOWN,
        right = SDL_SCANCODE_RIGHT,
        left = SDL_SCANCODE_LEFT,
        down = SDL_SCANCODE_DOWN,
        up = SDL_SCANCODE_UP,

        // Keypad
        kp_divide = SDL_SCANCODE_KP_DIVIDE,
        kp_multiply = SDL_SCANCODE_KP_MULTIPLY,
        kp_minus = SDL_SCANCODE_KP_MINUS,
        kp_plus = SDL_SCANCODE_KP_PLUS,
        kp_enter = SDL_SCANCODE_KP_ENTER,
        kp_1 = SDL_SCANCODE_KP_1,
        kp_2 = SDL_SCANCODE_KP_2,
        kp_3 = SDL_SCANCODE_KP_3,
        kp_4 = SDL_SCANCODE_KP_4,
        kp_5 = SDL_SCANCODE_KP_5,
        kp_6 = SDL_SCANCODE_KP_6,
        kp_7 = SDL_SCANCODE_KP_7,
        kp_8 = SDL_SCANCODE_KP_8,
        kp_9 = SDL_SCANCODE_KP_9,
        kp_0 = SDL_SCANCODE_KP_0,
        kp_period = SDL_SCANCODE_KP_PERIOD,

        // Modifiers
        lctrl = SDL_SCANCODE_LCTRL,
        lshift = SDL_SCANCODE_LSHIFT,
        lalt = SDL_SCANCODE_LALT,
        lgui = SDL_SCANCODE_LGUI,
        rctrl = SDL_SCANCODE_RCTRL,
        rshift = SDL_SCANCODE_RSHIFT,
        ralt = SDL_SCANCODE_RALT,
        rgui = SDL_SCANCODE_RGUI,

        // Special keys
        application = SDL_SCANCODE_APPLICATION,
        power = SDL_SCANCODE_POWER,
        execute = SDL_SCANCODE_EXECUTE,
        help = SDL_SCANCODE_HELP,
        menu = SDL_SCANCODE_MENU,
        select = SDL_SCANCODE_SELECT,
        stop = SDL_SCANCODE_STOP,
        again = SDL_SCANCODE_AGAIN,
        undo = SDL_SCANCODE_UNDO,
        cut = SDL_SCANCODE_CUT,
        copy = SDL_SCANCODE_COPY,
        paste = SDL_SCANCODE_PASTE,
        find = SDL_SCANCODE_FIND,
        mute = SDL_SCANCODE_MUTE,
        volumeup = SDL_SCANCODE_VOLUMEUP,
        volumedown = SDL_SCANCODE_VOLUMEDOWN,

        // Total count
        count = SDL_SCANCODE_COUNT
    };

    /**
     * @brief Virtual key code (layout-dependent)
     * Note: Using Uint32 instead of enum due to the large range and special values
     */
    using keycode = Uint32;

    // Common keycodes as constants
    namespace keycodes {
        constexpr keycode unknown = SDLK_UNKNOWN;
        constexpr keycode return_key = SDLK_RETURN;
        constexpr keycode escape = SDLK_ESCAPE;
        constexpr keycode backspace = SDLK_BACKSPACE;
        constexpr keycode tab = SDLK_TAB;
        constexpr keycode space = SDLK_SPACE;
        constexpr keycode exclaim = SDLK_EXCLAIM;
        constexpr keycode dblapostrophe = SDLK_DBLAPOSTROPHE;
        constexpr keycode hash = SDLK_HASH;
        constexpr keycode dollar = SDLK_DOLLAR;
        constexpr keycode percent = SDLK_PERCENT;
        constexpr keycode ampersand = SDLK_AMPERSAND;
        constexpr keycode apostrophe = SDLK_APOSTROPHE;
        constexpr keycode leftparen = SDLK_LEFTPAREN;
        constexpr keycode rightparen = SDLK_RIGHTPAREN;
        constexpr keycode asterisk = SDLK_ASTERISK;
        constexpr keycode plus = SDLK_PLUS;
        constexpr keycode comma = SDLK_COMMA;
        constexpr keycode minus = SDLK_MINUS;
        constexpr keycode period = SDLK_PERIOD;
        constexpr keycode slash = SDLK_SLASH;

        // Numbers
        constexpr keycode num_0 = SDLK_0;
        constexpr keycode num_1 = SDLK_1;
        constexpr keycode num_2 = SDLK_2;
        constexpr keycode num_3 = SDLK_3;
        constexpr keycode num_4 = SDLK_4;
        constexpr keycode num_5 = SDLK_5;
        constexpr keycode num_6 = SDLK_6;
        constexpr keycode num_7 = SDLK_7;
        constexpr keycode num_8 = SDLK_8;
        constexpr keycode num_9 = SDLK_9;

        // Letters
        constexpr keycode a = SDLK_A;
        constexpr keycode b = SDLK_B;
        constexpr keycode c = SDLK_C;
        constexpr keycode d = SDLK_D;
        constexpr keycode e = SDLK_E;
        constexpr keycode f = SDLK_F;
        constexpr keycode g = SDLK_G;
        constexpr keycode h = SDLK_H;
        constexpr keycode i = SDLK_I;
        constexpr keycode j = SDLK_J;
        constexpr keycode k = SDLK_K;
        constexpr keycode l = SDLK_L;
        constexpr keycode m = SDLK_M;
        constexpr keycode n = SDLK_N;
        constexpr keycode o = SDLK_O;
        constexpr keycode p = SDLK_P;
        constexpr keycode q = SDLK_Q;
        constexpr keycode r = SDLK_R;
        constexpr keycode s = SDLK_S;
        constexpr keycode t = SDLK_T;
        constexpr keycode u = SDLK_U;
        constexpr keycode v = SDLK_V;
        constexpr keycode w = SDLK_W;
        constexpr keycode x = SDLK_X;
        constexpr keycode y = SDLK_Y;
        constexpr keycode z = SDLK_Z;

        // Function keys
        constexpr keycode f1 = SDLK_F1;
        constexpr keycode f2 = SDLK_F2;
        constexpr keycode f3 = SDLK_F3;
        constexpr keycode f4 = SDLK_F4;
        constexpr keycode f5 = SDLK_F5;
        constexpr keycode f6 = SDLK_F6;
        constexpr keycode f7 = SDLK_F7;
        constexpr keycode f8 = SDLK_F8;
        constexpr keycode f9 = SDLK_F9;
        constexpr keycode f10 = SDLK_F10;
        constexpr keycode f11 = SDLK_F11;
        constexpr keycode f12 = SDLK_F12;

        // Navigation
        constexpr keycode right = SDLK_RIGHT;
        constexpr keycode left = SDLK_LEFT;
        constexpr keycode down = SDLK_DOWN;
        constexpr keycode up = SDLK_UP;
        constexpr keycode home = SDLK_HOME;
        constexpr keycode end = SDLK_END;
        constexpr keycode pageup = SDLK_PAGEUP;
        constexpr keycode pagedown = SDLK_PAGEDOWN;
        constexpr keycode insert = SDLK_INSERT;
        constexpr keycode delete_key = SDLK_DELETE;

        // Modifiers
        constexpr keycode lshift = SDLK_LSHIFT;
        constexpr keycode rshift = SDLK_RSHIFT;
        constexpr keycode lctrl = SDLK_LCTRL;
        constexpr keycode rctrl = SDLK_RCTRL;
        constexpr keycode lalt = SDLK_LALT;
        constexpr keycode ralt = SDLK_RALT;
        constexpr keycode lgui = SDLK_LGUI;
        constexpr keycode rgui = SDLK_RGUI;
    }

    /**
     * @brief Keyboard modifier flags
     */
    enum class keymod : Uint16 {
        none = SDL_KMOD_NONE,
        lshift = SDL_KMOD_LSHIFT,
        rshift = SDL_KMOD_RSHIFT,
        lctrl = SDL_KMOD_LCTRL,
        rctrl = SDL_KMOD_RCTRL,
        lalt = SDL_KMOD_LALT,
        ralt = SDL_KMOD_RALT,
        lgui = SDL_KMOD_LGUI,
        rgui = SDL_KMOD_RGUI,
        num = SDL_KMOD_NUM,
        caps = SDL_KMOD_CAPS,
        mode = SDL_KMOD_MODE,
        scroll = SDL_KMOD_SCROLL,

        // Combined flags
        ctrl = SDL_KMOD_CTRL,
        shift = SDL_KMOD_SHIFT,
        alt = SDL_KMOD_ALT,
        gui = SDL_KMOD_GUI
    };

    /**
     * @brief Bitwise OR operator for keymod
     */
    [[nodiscard]] inline keymod operator|(keymod lhs, keymod rhs) noexcept {
        return static_cast <keymod>(static_cast <Uint16>(lhs) | static_cast <Uint16>(rhs));
    }

    /**
     * @brief Bitwise AND operator for keymod
     */
    [[nodiscard]] inline keymod operator&(keymod lhs, keymod rhs) noexcept {
        return static_cast <keymod>(static_cast <Uint16>(lhs) & static_cast <Uint16>(rhs));
    }

    /**
     * @brief Bitwise XOR operator for keymod
     */
    [[nodiscard]] inline keymod operator^(keymod lhs, keymod rhs) noexcept {
        return static_cast <keymod>(static_cast <Uint16>(lhs) ^ static_cast <Uint16>(rhs));
    }

    /**
     * @brief Bitwise NOT operator for keymod
     */
    [[nodiscard]] inline keymod operator~(keymod mod) noexcept {
        return static_cast <keymod>(~static_cast <Uint16>(mod));
    }

    /**
     * @brief Check if modifier is set
     */
    [[nodiscard]] inline bool has_keymod(keymod mods, keymod check) noexcept {
        return (mods & check) != keymod::none;
    }

    /**
     * @brief Convert scancode to keycode
     */
    [[nodiscard]] inline keycode scancode_to_keycode(scancode code) noexcept {
        return SDL_GetKeyFromScancode(static_cast <SDL_Scancode>(code), SDL_KMOD_NONE, false);
    }

    /**
     * @brief Convert keycode to scancode
     */
    [[nodiscard]] inline scancode keycode_to_scancode(keycode code) noexcept {
        return static_cast <scancode>(SDL_GetScancodeFromKey(code, nullptr));
    }

    /**
     * @brief Get scancode name
     */
    [[nodiscard]] inline const char* get_scancode_name(scancode code) noexcept {
        return SDL_GetScancodeName(static_cast <SDL_Scancode>(code));
    }

    /**
     * @brief Get keycode name
     */
    [[nodiscard]] inline const char* get_keycode_name(keycode code) noexcept {
        return SDL_GetKeyName(code);
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for scancode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, scancode value);

/**
 * @brief Stream input operator for scancode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, scancode& value);

/**
 * @brief Stream output operator for keymod
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, keymod value);

/**
 * @brief Stream input operator for keymod
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, keymod& value);

}