//
// Created by igor on 7/13/25.
//

#pragma once

/**
 * @file color.hh
 * @brief Template-based color types with concepts for SDL3
 * 
 * This header provides modern C++ color types with both integer (0-255)
 * and floating-point (0.0-1.0) representations, designed to work seamlessly
 * with SDL3's color types while providing rich functionality.
 */

#include <sdlpp/core/sdl.hh>
#include <algorithm>
#include <type_traits>
#include <concepts>

namespace sdlpp {
    /**
     * @brief Type traits for mapping C++ types to SDL color types
     */
    template<typename T>
    struct sdl_color_types;

    template<>
    struct sdl_color_types <uint8_t> {
        using color_type = SDL_Color;
        static constexpr uint8_t max_value = 255;
        static constexpr uint8_t default_alpha = 255;
    };

    template<>
    struct sdl_color_types <float> {
        using color_type = SDL_FColor;
        static constexpr float max_value = 1.0f;
        static constexpr float default_alpha = 1.0f;
    };

    /**
     * @brief Concept for numeric types suitable for color components
     */
    template<typename T>
    concept color_component = (std::is_same_v <T, uint8_t> || std::is_same_v <T, float>);

    /**
     * @brief Generic RGBA color with template component type
     * @tparam T Component type (uint8_t for 0-255, float for 0.0-1.0)
     */
    template<color_component T>
    struct basic_color {
        using value_type = T;
        using sdl_type = typename sdl_color_types <T>::color_type;

        T r = 0; ///< Red component
        T g = 0; ///< Green component
        T b = 0; ///< Blue component
        T a = sdl_color_types <T>::default_alpha; ///< Alpha component

        /**
         * @brief Default constructor - creates black color with full opacity
         */
        constexpr basic_color() = default;

        /**
         * @brief Construct a color from RGBA components
         * @param red Red component
         * @param green Green component
         * @param blue Blue component
         * @param alpha Alpha component (default is full opacity)
         */
        constexpr basic_color(T red, T green, T blue, T alpha = sdl_color_types <T>::default_alpha)
            : r(red), g(green), b(blue), a(alpha) {
        }

        /**
         * @brief Convert to SDL color type
         * @return SDL_Color for uint8_t, SDL_FColor for float
         */
        [[nodiscard]] constexpr auto to_sdl() const {
            if constexpr (std::is_same_v <T, uint8_t>) {
                return SDL_Color{r, g, b, a};
            } else {
                return SDL_FColor{r, g, b, a};
            }
        }

        /**
         * @brief Create color from SDL type
         * @param c SDL color structure
         * @return basic_color instance
         */
        static constexpr basic_color from_sdl(const sdl_type& c) {
            return {c.r, c.g, c.b, c.a};
        }

        /**
         * @brief Conversion constructor from different component type
         * @tparam U Source component type
         * @param other Source color
         */
        template<color_component U>
        constexpr explicit basic_color(const basic_color <U>& other) {
            if constexpr (std::is_same_v <T, uint8_t> && std::is_same_v <U, float>) {
                // Float to uint8_t conversion
                r = static_cast <T>(std::clamp(other.r * 255.0f, 0.0f, 255.0f));
                g = static_cast <T>(std::clamp(other.g * 255.0f, 0.0f, 255.0f));
                b = static_cast <T>(std::clamp(other.b * 255.0f, 0.0f, 255.0f));
                a = static_cast <T>(std::clamp(other.a * 255.0f, 0.0f, 255.0f));
            } else if constexpr (std::is_same_v <T, float> && std::is_same_v <U, uint8_t>) {
                // uint8_t to float conversion
                r = static_cast <T>(other.r) / 255.0f;
                g = static_cast <T>(other.g) / 255.0f;
                b = static_cast <T>(other.b) / 255.0f;
                a = static_cast <T>(other.a) / 255.0f;
            }
        }

        /**
         * @brief Mix two colors with linear interpolation
         * @param other Target color
         * @param t Interpolation factor (0-1)
         * @return Interpolated color
         */
        [[nodiscard]] constexpr basic_color mix(const basic_color& other, float t) const {
            if constexpr (std::is_same_v <T, uint8_t>) {
                return {
                    static_cast <T>(r + (other.r - r) * t),
                    static_cast <T>(g + (other.g - g) * t),
                    static_cast <T>(b + (other.b - b) * t),
                    static_cast <T>(a + (other.a - a) * t)
                };
            } else {
                return {
                    r + (other.r - r) * t,
                    g + (other.g - g) * t,
                    b + (other.b - b) * t,
                    a + (other.a - a) * t
                };
            }
        }

        /**
         * @brief Premultiply RGB by alpha
         * @return Color with premultiplied alpha
         */
        [[nodiscard]] constexpr basic_color premultiply() const {
            if constexpr (std::is_same_v <T, uint8_t>) {
                float af = a / 255.0f;
                return {
                    static_cast <T>(r * af),
                    static_cast <T>(g * af),
                    static_cast <T>(b * af),
                    a
                };
            } else {
                return {r * a, g * a, b * a, a};
            }
        }

        /**
         * @brief Calculate luminance (perceived brightness)
         * @return Luminance value in component range
         */
        [[nodiscard]] constexpr T luminance() const {
            if constexpr (std::is_same_v <T, uint8_t>) {
                // ITU-R BT.709 luma coefficients
                return static_cast <T>(0.2126f * r + 0.7152f * g + 0.0722f * b);
            } else {
                return 0.2126f * r + 0.7152f * g + 0.0722f * b;
            }
        }

        /**
         * @brief Convert to grayscale
         * @return Grayscale color
         */
        [[nodiscard]] constexpr basic_color to_grayscale() const {
            T lum = luminance();
            return {lum, lum, lum, a};
        }

        /**
         * @brief Adjust brightness
         * @param factor Brightness factor (1.0 = no change, >1.0 = brighter, <1.0 = darker)
         * @return Adjusted color
         */
        [[nodiscard]] constexpr basic_color adjust_brightness(float factor) const {
            if constexpr (std::is_same_v <T, uint8_t>) {
                return {
                    static_cast <T>(std::clamp(r * factor, 0.0f, 255.0f)),
                    static_cast <T>(std::clamp(g * factor, 0.0f, 255.0f)),
                    static_cast <T>(std::clamp(b * factor, 0.0f, 255.0f)),
                    a
                };
            } else {
                return {
                    std::clamp(r * factor, 0.0f, 1.0f),
                    std::clamp(g * factor, 0.0f, 1.0f),
                    std::clamp(b * factor, 0.0f, 1.0f),
                    a
                };
            }
        }

        // Comparison operators
        [[nodiscard]] constexpr bool operator==(const basic_color&) const = default;

        // Arithmetic operators for color blending
        [[nodiscard]] constexpr basic_color operator+(const basic_color& other) const {
            if constexpr (std::is_same_v <T, uint8_t>) {
                return {
                    static_cast <T>(std::min(static_cast <int>(r) + other.r, 255)),
                    static_cast <T>(std::min(static_cast <int>(g) + other.g, 255)),
                    static_cast <T>(std::min(static_cast <int>(b) + other.b, 255)),
                    static_cast <T>(std::min(static_cast <int>(a) + other.a, 255))
                };
            } else {
                return {
                    std::min(r + other.r, 1.0f),
                    std::min(g + other.g, 1.0f),
                    std::min(b + other.b, 1.0f),
                    std::min(a + other.a, 1.0f)
                };
            }
        }

        [[nodiscard]] constexpr basic_color operator*(float factor) const {
            return adjust_brightness(factor);
        }
    };

    // Type aliases for common use cases
    using color = basic_color <uint8_t>; ///< Integer RGBA color (0-255)
    using fcolor = basic_color <float>; ///< Floating-point RGBA color (0.0-1.0)

    /**
     * @brief Concept for color-like types
     */
    template<typename T>
    concept color_like = requires(T t)
    {
        { t.r } -> std::convertible_to <typename T::value_type>;
        { t.g } -> std::convertible_to <typename T::value_type>;
        { t.b } -> std::convertible_to <typename T::value_type>;
        { t.a } -> std::convertible_to <typename T::value_type>;
        typename T::value_type;
        requires color_component <typename T::value_type>;
    };

    // Predefined color constants
    namespace colors {
        // Basic colors
        inline constexpr color black{0, 0, 0};
        inline constexpr color white{255, 255, 255};
        inline constexpr color red{255, 0, 0};
        inline constexpr color green{0, 255, 0};
        inline constexpr color blue{0, 0, 255};
        inline constexpr color yellow{255, 255, 0};
        inline constexpr color cyan{0, 255, 255};
        inline constexpr color magenta{255, 0, 255};

        // Grays
        inline constexpr color gray{128, 128, 128};
        inline constexpr color light_gray{192, 192, 192};
        inline constexpr color dark_gray{64, 64, 64};

        // Transparent
        inline constexpr color transparent{0, 0, 0, 0};

        // Floating-point versions
        namespace f {
            inline constexpr fcolor black{0.0f, 0.0f, 0.0f};
            inline constexpr fcolor white{1.0f, 1.0f, 1.0f};
            inline constexpr fcolor red{1.0f, 0.0f, 0.0f};
            inline constexpr fcolor green{0.0f, 1.0f, 0.0f};
            inline constexpr fcolor blue{0.0f, 0.0f, 1.0f};
            inline constexpr fcolor yellow{1.0f, 1.0f, 0.0f};
            inline constexpr fcolor cyan{0.0f, 1.0f, 1.0f};
            inline constexpr fcolor magenta{1.0f, 0.0f, 1.0f};
            inline constexpr fcolor gray{0.5f, 0.5f, 0.5f};
            inline constexpr fcolor transparent{0.0f, 0.0f, 0.0f, 0.0f};
        }
    }

    /**
     * @brief Linear interpolation between two colors
     * @tparam C Color type
     * @param a Start color
     * @param b End color
     * @param t Interpolation factor (0-1)
     * @return Interpolated color
     */
    template<color_like C>
    [[nodiscard]] constexpr C lerp(const C& a, const C& b, float t) {
        return a.mix(b, t);
    }

    /**
     * @brief Blend two colors using alpha blending
     * @tparam C Color type
     * @param src Source color (foreground)
     * @param dst Destination color (background)
     * @return Blended color
     */
    template<color_like C>
    [[nodiscard]] constexpr C alpha_blend(const C& src, const C& dst) {
        if constexpr (std::is_same_v <typename C::value_type, uint8_t>) {
            float src_a = src.a / 255.0f;
            float inv_src_a = 1.0f - src_a;

            return C{
                static_cast <uint8_t>(src.r * src_a + dst.r * inv_src_a),
                static_cast <uint8_t>(src.g * src_a + dst.g * inv_src_a),
                static_cast <uint8_t>(src.b * src_a + dst.b * inv_src_a),
                static_cast <uint8_t>(src.a + dst.a * inv_src_a)
            };
        } else {
            float inv_src_a = 1.0f - src.a;
            return C{
                src.r * src.a + dst.r * inv_src_a,
                src.g * src.a + dst.g * inv_src_a,
                src.b * src.a + dst.b * inv_src_a,
                src.a + dst.a * inv_src_a
            };
        }
    }

    /**
     * @brief Convert color to 32-bit RGBA value
     * @param c Color to convert
     * @return 32-bit RGBA value
     */
    [[nodiscard]] inline constexpr uint32_t to_rgba32(const color& c) {
        return (static_cast <uint32_t>(c.r) << 24) |
               (static_cast <uint32_t>(c.g) << 16) |
               (static_cast <uint32_t>(c.b) << 8) |
               static_cast <uint32_t>(c.a);
    }

    /**
     * @brief Create color from 32-bit RGBA value
     * @param rgba 32-bit RGBA value
     * @return Color instance
     */
    [[nodiscard]] inline constexpr color from_rgba32(uint32_t rgba) {
        return color{
            static_cast <uint8_t>((rgba >> 24) & 0xFF),
            static_cast <uint8_t>((rgba >> 16) & 0xFF),
            static_cast <uint8_t>((rgba >> 8) & 0xFF),
            static_cast <uint8_t>(rgba & 0xFF)
        };
    }
} // namespace sdlpp
