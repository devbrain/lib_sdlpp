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
        // X11 colors from /usr/lib/X11/rgb.txt, exposed as lower_snake_case identifiers.
        inline constexpr color snow{255, 250, 250};
        inline constexpr color ghost_white{248, 248, 255};
        inline constexpr color white_smoke{245, 245, 245};
        inline constexpr color gainsboro{220, 220, 220};
        inline constexpr color floral_white{255, 250, 240};
        inline constexpr color old_lace{253, 245, 230};
        inline constexpr color linen{250, 240, 230};
        inline constexpr color antique_white{250, 235, 215};
        inline constexpr color papaya_whip{255, 239, 213};
        inline constexpr color blanched_almond{255, 235, 205};
        inline constexpr color bisque{255, 228, 196};
        inline constexpr color peach_puff{255, 218, 185};
        inline constexpr color navajo_white{255, 222, 173};
        inline constexpr color moccasin{255, 228, 181};
        inline constexpr color cornsilk{255, 248, 220};
        inline constexpr color ivory{255, 255, 240};
        inline constexpr color lemon_chiffon{255, 250, 205};
        inline constexpr color seashell{255, 245, 238};
        inline constexpr color honeydew{240, 255, 240};
        inline constexpr color mint_cream{245, 255, 250};
        inline constexpr color azure{240, 255, 255};
        inline constexpr color alice_blue{240, 248, 255};
        inline constexpr color lavender{230, 230, 250};
        inline constexpr color lavender_blush{255, 240, 245};
        inline constexpr color misty_rose{255, 228, 225};
        inline constexpr color white{255, 255, 255};
        inline constexpr color black{0, 0, 0};
        inline constexpr color dark_slate_gray{47, 79, 79};
        inline constexpr color dark_slate_grey{47, 79, 79};
        inline constexpr color dim_gray{105, 105, 105};
        inline constexpr color dim_grey{105, 105, 105};
        inline constexpr color slate_gray{112, 128, 144};
        inline constexpr color slate_grey{112, 128, 144};
        inline constexpr color light_slate_gray{119, 136, 153};
        inline constexpr color light_slate_grey{119, 136, 153};
        inline constexpr color gray{190, 190, 190};
        inline constexpr color grey{190, 190, 190};
        inline constexpr color light_grey{211, 211, 211};
        inline constexpr color light_gray{211, 211, 211};
        inline constexpr color midnight_blue{25, 25, 112};
        inline constexpr color navy{0, 0, 128};
        inline constexpr color navy_blue{0, 0, 128};
        inline constexpr color cornflower_blue{100, 149, 237};
        inline constexpr color dark_slate_blue{72, 61, 139};
        inline constexpr color slate_blue{106, 90, 205};
        inline constexpr color medium_slate_blue{123, 104, 238};
        inline constexpr color light_slate_blue{132, 112, 255};
        inline constexpr color medium_blue{0, 0, 205};
        inline constexpr color royal_blue{65, 105, 225};
        inline constexpr color blue{0, 0, 255};
        inline constexpr color dodger_blue{30, 144, 255};
        inline constexpr color deep_sky_blue{0, 191, 255};
        inline constexpr color sky_blue{135, 206, 235};
        inline constexpr color light_sky_blue{135, 206, 250};
        inline constexpr color steel_blue{70, 130, 180};
        inline constexpr color light_steel_blue{176, 196, 222};
        inline constexpr color light_blue{173, 216, 230};
        inline constexpr color powder_blue{176, 224, 230};
        inline constexpr color pale_turquoise{175, 238, 238};
        inline constexpr color dark_turquoise{0, 206, 209};
        inline constexpr color medium_turquoise{72, 209, 204};
        inline constexpr color turquoise{64, 224, 208};
        inline constexpr color cyan{0, 255, 255};
        inline constexpr color light_cyan{224, 255, 255};
        inline constexpr color cadet_blue{95, 158, 160};
        inline constexpr color medium_aquamarine{102, 205, 170};
        inline constexpr color aquamarine{127, 255, 212};
        inline constexpr color dark_green{0, 100, 0};
        inline constexpr color dark_olive_green{85, 107, 47};
        inline constexpr color dark_sea_green{143, 188, 143};
        inline constexpr color sea_green{46, 139, 87};
        inline constexpr color medium_sea_green{60, 179, 113};
        inline constexpr color light_sea_green{32, 178, 170};
        inline constexpr color pale_green{152, 251, 152};
        inline constexpr color spring_green{0, 255, 127};
        inline constexpr color lawn_green{124, 252, 0};
        inline constexpr color green{0, 255, 0};
        inline constexpr color chartreuse{127, 255, 0};
        inline constexpr color medium_spring_green{0, 250, 154};
        inline constexpr color green_yellow{173, 255, 47};
        inline constexpr color lime_green{50, 205, 50};
        inline constexpr color yellow_green{154, 205, 50};
        inline constexpr color forest_green{34, 139, 34};
        inline constexpr color olive_drab{107, 142, 35};
        inline constexpr color dark_khaki{189, 183, 107};
        inline constexpr color khaki{240, 230, 140};
        inline constexpr color pale_goldenrod{238, 232, 170};
        inline constexpr color light_goldenrod_yellow{250, 250, 210};
        inline constexpr color light_yellow{255, 255, 224};
        inline constexpr color yellow{255, 255, 0};
        inline constexpr color gold{255, 215, 0};
        inline constexpr color light_goldenrod{238, 221, 130};
        inline constexpr color goldenrod{218, 165, 32};
        inline constexpr color dark_goldenrod{184, 134, 11};
        inline constexpr color rosy_brown{188, 143, 143};
        inline constexpr color indian_red{205, 92, 92};
        inline constexpr color saddle_brown{139, 69, 19};
        inline constexpr color sienna{160, 82, 45};
        inline constexpr color peru{205, 133, 63};
        inline constexpr color burlywood{222, 184, 135};
        inline constexpr color beige{245, 245, 220};
        inline constexpr color wheat{245, 222, 179};
        inline constexpr color sandy_brown{244, 164, 96};
        inline constexpr color tan{210, 180, 140};
        inline constexpr color chocolate{210, 105, 30};
        inline constexpr color firebrick{178, 34, 34};
        inline constexpr color brown{165, 42, 42};
        inline constexpr color dark_salmon{233, 150, 122};
        inline constexpr color salmon{250, 128, 114};
        inline constexpr color light_salmon{255, 160, 122};
        inline constexpr color orange{255, 165, 0};
        inline constexpr color dark_orange{255, 140, 0};
        inline constexpr color coral{255, 127, 80};
        inline constexpr color light_coral{240, 128, 128};
        inline constexpr color tomato{255, 99, 71};
        inline constexpr color orange_red{255, 69, 0};
        inline constexpr color red{255, 0, 0};
        inline constexpr color hot_pink{255, 105, 180};
        inline constexpr color deep_pink{255, 20, 147};
        inline constexpr color pink{255, 192, 203};
        inline constexpr color light_pink{255, 182, 193};
        inline constexpr color pale_violet_red{219, 112, 147};
        inline constexpr color maroon{176, 48, 96};
        inline constexpr color medium_violet_red{199, 21, 133};
        inline constexpr color violet_red{208, 32, 144};
        inline constexpr color magenta{255, 0, 255};
        inline constexpr color violet{238, 130, 238};
        inline constexpr color plum{221, 160, 221};
        inline constexpr color orchid{218, 112, 214};
        inline constexpr color medium_orchid{186, 85, 211};
        inline constexpr color dark_orchid{153, 50, 204};
        inline constexpr color dark_violet{148, 0, 211};
        inline constexpr color blue_violet{138, 43, 226};
        inline constexpr color purple{160, 32, 240};
        inline constexpr color medium_purple{147, 112, 219};
        inline constexpr color thistle{216, 191, 216};
        inline constexpr color snow_1{255, 250, 250};
        inline constexpr color snow_2{238, 233, 233};
        inline constexpr color snow_3{205, 201, 201};
        inline constexpr color snow_4{139, 137, 137};
        inline constexpr color seashell_1{255, 245, 238};
        inline constexpr color seashell_2{238, 229, 222};
        inline constexpr color seashell_3{205, 197, 191};
        inline constexpr color seashell_4{139, 134, 130};
        inline constexpr color antique_white_1{255, 239, 219};
        inline constexpr color antique_white_2{238, 223, 204};
        inline constexpr color antique_white_3{205, 192, 176};
        inline constexpr color antique_white_4{139, 131, 120};
        inline constexpr color bisque_1{255, 228, 196};
        inline constexpr color bisque_2{238, 213, 183};
        inline constexpr color bisque_3{205, 183, 158};
        inline constexpr color bisque_4{139, 125, 107};
        inline constexpr color peach_puff_1{255, 218, 185};
        inline constexpr color peach_puff_2{238, 203, 173};
        inline constexpr color peach_puff_3{205, 175, 149};
        inline constexpr color peach_puff_4{139, 119, 101};
        inline constexpr color navajo_white_1{255, 222, 173};
        inline constexpr color navajo_white_2{238, 207, 161};
        inline constexpr color navajo_white_3{205, 179, 139};
        inline constexpr color navajo_white_4{139, 121, 94};
        inline constexpr color lemon_chiffon_1{255, 250, 205};
        inline constexpr color lemon_chiffon_2{238, 233, 191};
        inline constexpr color lemon_chiffon_3{205, 201, 165};
        inline constexpr color lemon_chiffon_4{139, 137, 112};
        inline constexpr color cornsilk_1{255, 248, 220};
        inline constexpr color cornsilk_2{238, 232, 205};
        inline constexpr color cornsilk_3{205, 200, 177};
        inline constexpr color cornsilk_4{139, 136, 120};
        inline constexpr color ivory_1{255, 255, 240};
        inline constexpr color ivory_2{238, 238, 224};
        inline constexpr color ivory_3{205, 205, 193};
        inline constexpr color ivory_4{139, 139, 131};
        inline constexpr color honeydew_1{240, 255, 240};
        inline constexpr color honeydew_2{224, 238, 224};
        inline constexpr color honeydew_3{193, 205, 193};
        inline constexpr color honeydew_4{131, 139, 131};
        inline constexpr color lavender_blush_1{255, 240, 245};
        inline constexpr color lavender_blush_2{238, 224, 229};
        inline constexpr color lavender_blush_3{205, 193, 197};
        inline constexpr color lavender_blush_4{139, 131, 134};
        inline constexpr color misty_rose_1{255, 228, 225};
        inline constexpr color misty_rose_2{238, 213, 210};
        inline constexpr color misty_rose_3{205, 183, 181};
        inline constexpr color misty_rose_4{139, 125, 123};
        inline constexpr color azure_1{240, 255, 255};
        inline constexpr color azure_2{224, 238, 238};
        inline constexpr color azure_3{193, 205, 205};
        inline constexpr color azure_4{131, 139, 139};
        inline constexpr color slate_blue_1{131, 111, 255};
        inline constexpr color slate_blue_2{122, 103, 238};
        inline constexpr color slate_blue_3{105, 89, 205};
        inline constexpr color slate_blue_4{71, 60, 139};
        inline constexpr color royal_blue_1{72, 118, 255};
        inline constexpr color royal_blue_2{67, 110, 238};
        inline constexpr color royal_blue_3{58, 95, 205};
        inline constexpr color royal_blue_4{39, 64, 139};
        inline constexpr color blue_1{0, 0, 255};
        inline constexpr color blue_2{0, 0, 238};
        inline constexpr color blue_3{0, 0, 205};
        inline constexpr color blue_4{0, 0, 139};
        inline constexpr color dodger_blue_1{30, 144, 255};
        inline constexpr color dodger_blue_2{28, 134, 238};
        inline constexpr color dodger_blue_3{24, 116, 205};
        inline constexpr color dodger_blue_4{16, 78, 139};
        inline constexpr color steel_blue_1{99, 184, 255};
        inline constexpr color steel_blue_2{92, 172, 238};
        inline constexpr color steel_blue_3{79, 148, 205};
        inline constexpr color steel_blue_4{54, 100, 139};
        inline constexpr color deep_sky_blue_1{0, 191, 255};
        inline constexpr color deep_sky_blue_2{0, 178, 238};
        inline constexpr color deep_sky_blue_3{0, 154, 205};
        inline constexpr color deep_sky_blue_4{0, 104, 139};
        inline constexpr color sky_blue_1{135, 206, 255};
        inline constexpr color sky_blue_2{126, 192, 238};
        inline constexpr color sky_blue_3{108, 166, 205};
        inline constexpr color sky_blue_4{74, 112, 139};
        inline constexpr color light_sky_blue_1{176, 226, 255};
        inline constexpr color light_sky_blue_2{164, 211, 238};
        inline constexpr color light_sky_blue_3{141, 182, 205};
        inline constexpr color light_sky_blue_4{96, 123, 139};
        inline constexpr color slate_gray_1{198, 226, 255};
        inline constexpr color slate_gray_2{185, 211, 238};
        inline constexpr color slate_gray_3{159, 182, 205};
        inline constexpr color slate_gray_4{108, 123, 139};
        inline constexpr color light_steel_blue_1{202, 225, 255};
        inline constexpr color light_steel_blue_2{188, 210, 238};
        inline constexpr color light_steel_blue_3{162, 181, 205};
        inline constexpr color light_steel_blue_4{110, 123, 139};
        inline constexpr color light_blue_1{191, 239, 255};
        inline constexpr color light_blue_2{178, 223, 238};
        inline constexpr color light_blue_3{154, 192, 205};
        inline constexpr color light_blue_4{104, 131, 139};
        inline constexpr color light_cyan_1{224, 255, 255};
        inline constexpr color light_cyan_2{209, 238, 238};
        inline constexpr color light_cyan_3{180, 205, 205};
        inline constexpr color light_cyan_4{122, 139, 139};
        inline constexpr color pale_turquoise_1{187, 255, 255};
        inline constexpr color pale_turquoise_2{174, 238, 238};
        inline constexpr color pale_turquoise_3{150, 205, 205};
        inline constexpr color pale_turquoise_4{102, 139, 139};
        inline constexpr color cadet_blue_1{152, 245, 255};
        inline constexpr color cadet_blue_2{142, 229, 238};
        inline constexpr color cadet_blue_3{122, 197, 205};
        inline constexpr color cadet_blue_4{83, 134, 139};
        inline constexpr color turquoise_1{0, 245, 255};
        inline constexpr color turquoise_2{0, 229, 238};
        inline constexpr color turquoise_3{0, 197, 205};
        inline constexpr color turquoise_4{0, 134, 139};
        inline constexpr color cyan_1{0, 255, 255};
        inline constexpr color cyan_2{0, 238, 238};
        inline constexpr color cyan_3{0, 205, 205};
        inline constexpr color cyan_4{0, 139, 139};
        inline constexpr color dark_slate_gray_1{151, 255, 255};
        inline constexpr color dark_slate_gray_2{141, 238, 238};
        inline constexpr color dark_slate_gray_3{121, 205, 205};
        inline constexpr color dark_slate_gray_4{82, 139, 139};
        inline constexpr color aquamarine_1{127, 255, 212};
        inline constexpr color aquamarine_2{118, 238, 198};
        inline constexpr color aquamarine_3{102, 205, 170};
        inline constexpr color aquamarine_4{69, 139, 116};
        inline constexpr color dark_sea_green_1{193, 255, 193};
        inline constexpr color dark_sea_green_2{180, 238, 180};
        inline constexpr color dark_sea_green_3{155, 205, 155};
        inline constexpr color dark_sea_green_4{105, 139, 105};
        inline constexpr color sea_green_1{84, 255, 159};
        inline constexpr color sea_green_2{78, 238, 148};
        inline constexpr color sea_green_3{67, 205, 128};
        inline constexpr color sea_green_4{46, 139, 87};
        inline constexpr color pale_green_1{154, 255, 154};
        inline constexpr color pale_green_2{144, 238, 144};
        inline constexpr color pale_green_3{124, 205, 124};
        inline constexpr color pale_green_4{84, 139, 84};
        inline constexpr color spring_green_1{0, 255, 127};
        inline constexpr color spring_green_2{0, 238, 118};
        inline constexpr color spring_green_3{0, 205, 102};
        inline constexpr color spring_green_4{0, 139, 69};
        inline constexpr color green_1{0, 255, 0};
        inline constexpr color green_2{0, 238, 0};
        inline constexpr color green_3{0, 205, 0};
        inline constexpr color green_4{0, 139, 0};
        inline constexpr color chartreuse_1{127, 255, 0};
        inline constexpr color chartreuse_2{118, 238, 0};
        inline constexpr color chartreuse_3{102, 205, 0};
        inline constexpr color chartreuse_4{69, 139, 0};
        inline constexpr color olive_drab_1{192, 255, 62};
        inline constexpr color olive_drab_2{179, 238, 58};
        inline constexpr color olive_drab_3{154, 205, 50};
        inline constexpr color olive_drab_4{105, 139, 34};
        inline constexpr color dark_olive_green_1{202, 255, 112};
        inline constexpr color dark_olive_green_2{188, 238, 104};
        inline constexpr color dark_olive_green_3{162, 205, 90};
        inline constexpr color dark_olive_green_4{110, 139, 61};
        inline constexpr color khaki_1{255, 246, 143};
        inline constexpr color khaki_2{238, 230, 133};
        inline constexpr color khaki_3{205, 198, 115};
        inline constexpr color khaki_4{139, 134, 78};
        inline constexpr color light_goldenrod_1{255, 236, 139};
        inline constexpr color light_goldenrod_2{238, 220, 130};
        inline constexpr color light_goldenrod_3{205, 190, 112};
        inline constexpr color light_goldenrod_4{139, 129, 76};
        inline constexpr color light_yellow_1{255, 255, 224};
        inline constexpr color light_yellow_2{238, 238, 209};
        inline constexpr color light_yellow_3{205, 205, 180};
        inline constexpr color light_yellow_4{139, 139, 122};
        inline constexpr color yellow_1{255, 255, 0};
        inline constexpr color yellow_2{238, 238, 0};
        inline constexpr color yellow_3{205, 205, 0};
        inline constexpr color yellow_4{139, 139, 0};
        inline constexpr color gold_1{255, 215, 0};
        inline constexpr color gold_2{238, 201, 0};
        inline constexpr color gold_3{205, 173, 0};
        inline constexpr color gold_4{139, 117, 0};
        inline constexpr color goldenrod_1{255, 193, 37};
        inline constexpr color goldenrod_2{238, 180, 34};
        inline constexpr color goldenrod_3{205, 155, 29};
        inline constexpr color goldenrod_4{139, 105, 20};
        inline constexpr color dark_goldenrod_1{255, 185, 15};
        inline constexpr color dark_goldenrod_2{238, 173, 14};
        inline constexpr color dark_goldenrod_3{205, 149, 12};
        inline constexpr color dark_goldenrod_4{139, 101, 8};
        inline constexpr color rosy_brown_1{255, 193, 193};
        inline constexpr color rosy_brown_2{238, 180, 180};
        inline constexpr color rosy_brown_3{205, 155, 155};
        inline constexpr color rosy_brown_4{139, 105, 105};
        inline constexpr color indian_red_1{255, 106, 106};
        inline constexpr color indian_red_2{238, 99, 99};
        inline constexpr color indian_red_3{205, 85, 85};
        inline constexpr color indian_red_4{139, 58, 58};
        inline constexpr color sienna_1{255, 130, 71};
        inline constexpr color sienna_2{238, 121, 66};
        inline constexpr color sienna_3{205, 104, 57};
        inline constexpr color sienna_4{139, 71, 38};
        inline constexpr color burlywood_1{255, 211, 155};
        inline constexpr color burlywood_2{238, 197, 145};
        inline constexpr color burlywood_3{205, 170, 125};
        inline constexpr color burlywood_4{139, 115, 85};
        inline constexpr color wheat_1{255, 231, 186};
        inline constexpr color wheat_2{238, 216, 174};
        inline constexpr color wheat_3{205, 186, 150};
        inline constexpr color wheat_4{139, 126, 102};
        inline constexpr color tan_1{255, 165, 79};
        inline constexpr color tan_2{238, 154, 73};
        inline constexpr color tan_3{205, 133, 63};
        inline constexpr color tan_4{139, 90, 43};
        inline constexpr color chocolate_1{255, 127, 36};
        inline constexpr color chocolate_2{238, 118, 33};
        inline constexpr color chocolate_3{205, 102, 29};
        inline constexpr color chocolate_4{139, 69, 19};
        inline constexpr color firebrick_1{255, 48, 48};
        inline constexpr color firebrick_2{238, 44, 44};
        inline constexpr color firebrick_3{205, 38, 38};
        inline constexpr color firebrick_4{139, 26, 26};
        inline constexpr color brown_1{255, 64, 64};
        inline constexpr color brown_2{238, 59, 59};
        inline constexpr color brown_3{205, 51, 51};
        inline constexpr color brown_4{139, 35, 35};
        inline constexpr color salmon_1{255, 140, 105};
        inline constexpr color salmon_2{238, 130, 98};
        inline constexpr color salmon_3{205, 112, 84};
        inline constexpr color salmon_4{139, 76, 57};
        inline constexpr color light_salmon_1{255, 160, 122};
        inline constexpr color light_salmon_2{238, 149, 114};
        inline constexpr color light_salmon_3{205, 129, 98};
        inline constexpr color light_salmon_4{139, 87, 66};
        inline constexpr color orange_1{255, 165, 0};
        inline constexpr color orange_2{238, 154, 0};
        inline constexpr color orange_3{205, 133, 0};
        inline constexpr color orange_4{139, 90, 0};
        inline constexpr color dark_orange_1{255, 127, 0};
        inline constexpr color dark_orange_2{238, 118, 0};
        inline constexpr color dark_orange_3{205, 102, 0};
        inline constexpr color dark_orange_4{139, 69, 0};
        inline constexpr color coral_1{255, 114, 86};
        inline constexpr color coral_2{238, 106, 80};
        inline constexpr color coral_3{205, 91, 69};
        inline constexpr color coral_4{139, 62, 47};
        inline constexpr color tomato_1{255, 99, 71};
        inline constexpr color tomato_2{238, 92, 66};
        inline constexpr color tomato_3{205, 79, 57};
        inline constexpr color tomato_4{139, 54, 38};
        inline constexpr color orange_red_1{255, 69, 0};
        inline constexpr color orange_red_2{238, 64, 0};
        inline constexpr color orange_red_3{205, 55, 0};
        inline constexpr color orange_red_4{139, 37, 0};
        inline constexpr color red_1{255, 0, 0};
        inline constexpr color red_2{238, 0, 0};
        inline constexpr color red_3{205, 0, 0};
        inline constexpr color red_4{139, 0, 0};
        inline constexpr color deep_pink_1{255, 20, 147};
        inline constexpr color deep_pink_2{238, 18, 137};
        inline constexpr color deep_pink_3{205, 16, 118};
        inline constexpr color deep_pink_4{139, 10, 80};
        inline constexpr color hot_pink_1{255, 110, 180};
        inline constexpr color hot_pink_2{238, 106, 167};
        inline constexpr color hot_pink_3{205, 96, 144};
        inline constexpr color hot_pink_4{139, 58, 98};
        inline constexpr color pink_1{255, 181, 197};
        inline constexpr color pink_2{238, 169, 184};
        inline constexpr color pink_3{205, 145, 158};
        inline constexpr color pink_4{139, 99, 108};
        inline constexpr color light_pink_1{255, 174, 185};
        inline constexpr color light_pink_2{238, 162, 173};
        inline constexpr color light_pink_3{205, 140, 149};
        inline constexpr color light_pink_4{139, 95, 101};
        inline constexpr color pale_violet_red_1{255, 130, 171};
        inline constexpr color pale_violet_red_2{238, 121, 159};
        inline constexpr color pale_violet_red_3{205, 104, 137};
        inline constexpr color pale_violet_red_4{139, 71, 93};
        inline constexpr color maroon_1{255, 52, 179};
        inline constexpr color maroon_2{238, 48, 167};
        inline constexpr color maroon_3{205, 41, 144};
        inline constexpr color maroon_4{139, 28, 98};
        inline constexpr color violet_red_1{255, 62, 150};
        inline constexpr color violet_red_2{238, 58, 140};
        inline constexpr color violet_red_3{205, 50, 120};
        inline constexpr color violet_red_4{139, 34, 82};
        inline constexpr color magenta_1{255, 0, 255};
        inline constexpr color magenta_2{238, 0, 238};
        inline constexpr color magenta_3{205, 0, 205};
        inline constexpr color magenta_4{139, 0, 139};
        inline constexpr color orchid_1{255, 131, 250};
        inline constexpr color orchid_2{238, 122, 233};
        inline constexpr color orchid_3{205, 105, 201};
        inline constexpr color orchid_4{139, 71, 137};
        inline constexpr color plum_1{255, 187, 255};
        inline constexpr color plum_2{238, 174, 238};
        inline constexpr color plum_3{205, 150, 205};
        inline constexpr color plum_4{139, 102, 139};
        inline constexpr color medium_orchid_1{224, 102, 255};
        inline constexpr color medium_orchid_2{209, 95, 238};
        inline constexpr color medium_orchid_3{180, 82, 205};
        inline constexpr color medium_orchid_4{122, 55, 139};
        inline constexpr color dark_orchid_1{191, 62, 255};
        inline constexpr color dark_orchid_2{178, 58, 238};
        inline constexpr color dark_orchid_3{154, 50, 205};
        inline constexpr color dark_orchid_4{104, 34, 139};
        inline constexpr color purple_1{155, 48, 255};
        inline constexpr color purple_2{145, 44, 238};
        inline constexpr color purple_3{125, 38, 205};
        inline constexpr color purple_4{85, 26, 139};
        inline constexpr color medium_purple_1{171, 130, 255};
        inline constexpr color medium_purple_2{159, 121, 238};
        inline constexpr color medium_purple_3{137, 104, 205};
        inline constexpr color medium_purple_4{93, 71, 139};
        inline constexpr color thistle_1{255, 225, 255};
        inline constexpr color thistle_2{238, 210, 238};
        inline constexpr color thistle_3{205, 181, 205};
        inline constexpr color thistle_4{139, 123, 139};
        inline constexpr color gray_0{0, 0, 0};
        inline constexpr color grey_0{0, 0, 0};
        inline constexpr color gray_1{3, 3, 3};
        inline constexpr color grey_1{3, 3, 3};
        inline constexpr color gray_2{5, 5, 5};
        inline constexpr color grey_2{5, 5, 5};
        inline constexpr color gray_3{8, 8, 8};
        inline constexpr color grey_3{8, 8, 8};
        inline constexpr color gray_4{10, 10, 10};
        inline constexpr color grey_4{10, 10, 10};
        inline constexpr color gray_5{13, 13, 13};
        inline constexpr color grey_5{13, 13, 13};
        inline constexpr color gray_6{15, 15, 15};
        inline constexpr color grey_6{15, 15, 15};
        inline constexpr color gray_7{18, 18, 18};
        inline constexpr color grey_7{18, 18, 18};
        inline constexpr color gray_8{20, 20, 20};
        inline constexpr color grey_8{20, 20, 20};
        inline constexpr color gray_9{23, 23, 23};
        inline constexpr color grey_9{23, 23, 23};
        inline constexpr color gray_10{26, 26, 26};
        inline constexpr color grey_10{26, 26, 26};
        inline constexpr color gray_11{28, 28, 28};
        inline constexpr color grey_11{28, 28, 28};
        inline constexpr color gray_12{31, 31, 31};
        inline constexpr color grey_12{31, 31, 31};
        inline constexpr color gray_13{33, 33, 33};
        inline constexpr color grey_13{33, 33, 33};
        inline constexpr color gray_14{36, 36, 36};
        inline constexpr color grey_14{36, 36, 36};
        inline constexpr color gray_15{38, 38, 38};
        inline constexpr color grey_15{38, 38, 38};
        inline constexpr color gray_16{41, 41, 41};
        inline constexpr color grey_16{41, 41, 41};
        inline constexpr color gray_17{43, 43, 43};
        inline constexpr color grey_17{43, 43, 43};
        inline constexpr color gray_18{46, 46, 46};
        inline constexpr color grey_18{46, 46, 46};
        inline constexpr color gray_19{48, 48, 48};
        inline constexpr color grey_19{48, 48, 48};
        inline constexpr color gray_20{51, 51, 51};
        inline constexpr color grey_20{51, 51, 51};
        inline constexpr color gray_21{54, 54, 54};
        inline constexpr color grey_21{54, 54, 54};
        inline constexpr color gray_22{56, 56, 56};
        inline constexpr color grey_22{56, 56, 56};
        inline constexpr color gray_23{59, 59, 59};
        inline constexpr color grey_23{59, 59, 59};
        inline constexpr color gray_24{61, 61, 61};
        inline constexpr color grey_24{61, 61, 61};
        inline constexpr color gray_25{64, 64, 64};
        inline constexpr color grey_25{64, 64, 64};
        inline constexpr color gray_26{66, 66, 66};
        inline constexpr color grey_26{66, 66, 66};
        inline constexpr color gray_27{69, 69, 69};
        inline constexpr color grey_27{69, 69, 69};
        inline constexpr color gray_28{71, 71, 71};
        inline constexpr color grey_28{71, 71, 71};
        inline constexpr color gray_29{74, 74, 74};
        inline constexpr color grey_29{74, 74, 74};
        inline constexpr color gray_30{77, 77, 77};
        inline constexpr color grey_30{77, 77, 77};
        inline constexpr color gray_31{79, 79, 79};
        inline constexpr color grey_31{79, 79, 79};
        inline constexpr color gray_32{82, 82, 82};
        inline constexpr color grey_32{82, 82, 82};
        inline constexpr color gray_33{84, 84, 84};
        inline constexpr color grey_33{84, 84, 84};
        inline constexpr color gray_34{87, 87, 87};
        inline constexpr color grey_34{87, 87, 87};
        inline constexpr color gray_35{89, 89, 89};
        inline constexpr color grey_35{89, 89, 89};
        inline constexpr color gray_36{92, 92, 92};
        inline constexpr color grey_36{92, 92, 92};
        inline constexpr color gray_37{94, 94, 94};
        inline constexpr color grey_37{94, 94, 94};
        inline constexpr color gray_38{97, 97, 97};
        inline constexpr color grey_38{97, 97, 97};
        inline constexpr color gray_39{99, 99, 99};
        inline constexpr color grey_39{99, 99, 99};
        inline constexpr color gray_40{102, 102, 102};
        inline constexpr color grey_40{102, 102, 102};
        inline constexpr color gray_41{105, 105, 105};
        inline constexpr color grey_41{105, 105, 105};
        inline constexpr color gray_42{107, 107, 107};
        inline constexpr color grey_42{107, 107, 107};
        inline constexpr color gray_43{110, 110, 110};
        inline constexpr color grey_43{110, 110, 110};
        inline constexpr color gray_44{112, 112, 112};
        inline constexpr color grey_44{112, 112, 112};
        inline constexpr color gray_45{115, 115, 115};
        inline constexpr color grey_45{115, 115, 115};
        inline constexpr color gray_46{117, 117, 117};
        inline constexpr color grey_46{117, 117, 117};
        inline constexpr color gray_47{120, 120, 120};
        inline constexpr color grey_47{120, 120, 120};
        inline constexpr color gray_48{122, 122, 122};
        inline constexpr color grey_48{122, 122, 122};
        inline constexpr color gray_49{125, 125, 125};
        inline constexpr color grey_49{125, 125, 125};
        inline constexpr color gray_50{127, 127, 127};
        inline constexpr color grey_50{127, 127, 127};
        inline constexpr color gray_51{130, 130, 130};
        inline constexpr color grey_51{130, 130, 130};
        inline constexpr color gray_52{133, 133, 133};
        inline constexpr color grey_52{133, 133, 133};
        inline constexpr color gray_53{135, 135, 135};
        inline constexpr color grey_53{135, 135, 135};
        inline constexpr color gray_54{138, 138, 138};
        inline constexpr color grey_54{138, 138, 138};
        inline constexpr color gray_55{140, 140, 140};
        inline constexpr color grey_55{140, 140, 140};
        inline constexpr color gray_56{143, 143, 143};
        inline constexpr color grey_56{143, 143, 143};
        inline constexpr color gray_57{145, 145, 145};
        inline constexpr color grey_57{145, 145, 145};
        inline constexpr color gray_58{148, 148, 148};
        inline constexpr color grey_58{148, 148, 148};
        inline constexpr color gray_59{150, 150, 150};
        inline constexpr color grey_59{150, 150, 150};
        inline constexpr color gray_60{153, 153, 153};
        inline constexpr color grey_60{153, 153, 153};
        inline constexpr color gray_61{156, 156, 156};
        inline constexpr color grey_61{156, 156, 156};
        inline constexpr color gray_62{158, 158, 158};
        inline constexpr color grey_62{158, 158, 158};
        inline constexpr color gray_63{161, 161, 161};
        inline constexpr color grey_63{161, 161, 161};
        inline constexpr color gray_64{163, 163, 163};
        inline constexpr color grey_64{163, 163, 163};
        inline constexpr color gray_65{166, 166, 166};
        inline constexpr color grey_65{166, 166, 166};
        inline constexpr color gray_66{168, 168, 168};
        inline constexpr color grey_66{168, 168, 168};
        inline constexpr color gray_67{171, 171, 171};
        inline constexpr color grey_67{171, 171, 171};
        inline constexpr color gray_68{173, 173, 173};
        inline constexpr color grey_68{173, 173, 173};
        inline constexpr color gray_69{176, 176, 176};
        inline constexpr color grey_69{176, 176, 176};
        inline constexpr color gray_70{179, 179, 179};
        inline constexpr color grey_70{179, 179, 179};
        inline constexpr color gray_71{181, 181, 181};
        inline constexpr color grey_71{181, 181, 181};
        inline constexpr color gray_72{184, 184, 184};
        inline constexpr color grey_72{184, 184, 184};
        inline constexpr color gray_73{186, 186, 186};
        inline constexpr color grey_73{186, 186, 186};
        inline constexpr color gray_74{189, 189, 189};
        inline constexpr color grey_74{189, 189, 189};
        inline constexpr color gray_75{191, 191, 191};
        inline constexpr color grey_75{191, 191, 191};
        inline constexpr color gray_76{194, 194, 194};
        inline constexpr color grey_76{194, 194, 194};
        inline constexpr color gray_77{196, 196, 196};
        inline constexpr color grey_77{196, 196, 196};
        inline constexpr color gray_78{199, 199, 199};
        inline constexpr color grey_78{199, 199, 199};
        inline constexpr color gray_79{201, 201, 201};
        inline constexpr color grey_79{201, 201, 201};
        inline constexpr color gray_80{204, 204, 204};
        inline constexpr color grey_80{204, 204, 204};
        inline constexpr color gray_81{207, 207, 207};
        inline constexpr color grey_81{207, 207, 207};
        inline constexpr color gray_82{209, 209, 209};
        inline constexpr color grey_82{209, 209, 209};
        inline constexpr color gray_83{212, 212, 212};
        inline constexpr color grey_83{212, 212, 212};
        inline constexpr color gray_84{214, 214, 214};
        inline constexpr color grey_84{214, 214, 214};
        inline constexpr color gray_85{217, 217, 217};
        inline constexpr color grey_85{217, 217, 217};
        inline constexpr color gray_86{219, 219, 219};
        inline constexpr color grey_86{219, 219, 219};
        inline constexpr color gray_87{222, 222, 222};
        inline constexpr color grey_87{222, 222, 222};
        inline constexpr color gray_88{224, 224, 224};
        inline constexpr color grey_88{224, 224, 224};
        inline constexpr color gray_89{227, 227, 227};
        inline constexpr color grey_89{227, 227, 227};
        inline constexpr color gray_90{229, 229, 229};
        inline constexpr color grey_90{229, 229, 229};
        inline constexpr color gray_91{232, 232, 232};
        inline constexpr color grey_91{232, 232, 232};
        inline constexpr color gray_92{235, 235, 235};
        inline constexpr color grey_92{235, 235, 235};
        inline constexpr color gray_93{237, 237, 237};
        inline constexpr color grey_93{237, 237, 237};
        inline constexpr color gray_94{240, 240, 240};
        inline constexpr color grey_94{240, 240, 240};
        inline constexpr color gray_95{242, 242, 242};
        inline constexpr color grey_95{242, 242, 242};
        inline constexpr color gray_96{245, 245, 245};
        inline constexpr color grey_96{245, 245, 245};
        inline constexpr color gray_97{247, 247, 247};
        inline constexpr color grey_97{247, 247, 247};
        inline constexpr color gray_98{250, 250, 250};
        inline constexpr color grey_98{250, 250, 250};
        inline constexpr color gray_99{252, 252, 252};
        inline constexpr color grey_99{252, 252, 252};
        inline constexpr color gray_100{255, 255, 255};
        inline constexpr color grey_100{255, 255, 255};
        inline constexpr color dark_grey{169, 169, 169};
        inline constexpr color dark_gray{169, 169, 169};
        inline constexpr color dark_blue{0, 0, 139};
        inline constexpr color dark_cyan{0, 139, 139};
        inline constexpr color dark_magenta{139, 0, 139};
        inline constexpr color dark_red{139, 0, 0};
        inline constexpr color light_green{144, 238, 144};
        inline constexpr color sgi_gray_0{0, 0, 0};
        inline constexpr color sgi_grey_0{0, 0, 0};
        inline constexpr color sgi_gray_4{10, 10, 10};
        inline constexpr color sgi_grey_4{10, 10, 10};
        inline constexpr color sgi_gray_8{20, 20, 20};
        inline constexpr color sgi_grey_8{20, 20, 20};
        inline constexpr color sgi_gray_12{30, 30, 30};
        inline constexpr color sgi_grey_12{30, 30, 30};
        inline constexpr color sgi_gray_16{40, 40, 40};
        inline constexpr color sgi_grey_16{40, 40, 40};
        inline constexpr color sgi_gray_20{51, 51, 51};
        inline constexpr color sgi_grey_20{51, 51, 51};
        inline constexpr color sgi_gray_24{61, 61, 61};
        inline constexpr color sgi_grey_24{61, 61, 61};
        inline constexpr color sgi_gray_28{71, 71, 71};
        inline constexpr color sgi_grey_28{71, 71, 71};
        inline constexpr color sgi_gray_32{81, 81, 81};
        inline constexpr color sgi_grey_32{81, 81, 81};
        inline constexpr color sgi_gray_36{91, 91, 91};
        inline constexpr color sgi_grey_36{91, 91, 91};
        inline constexpr color sgi_gray_40{102, 102, 102};
        inline constexpr color sgi_grey_40{102, 102, 102};
        inline constexpr color sgi_gray_44{112, 112, 112};
        inline constexpr color sgi_grey_44{112, 112, 112};
        inline constexpr color sgi_gray_48{122, 122, 122};
        inline constexpr color sgi_grey_48{122, 122, 122};
        inline constexpr color sgi_gray_52{132, 132, 132};
        inline constexpr color sgi_grey_52{132, 132, 132};
        inline constexpr color sgi_gray_56{142, 142, 142};
        inline constexpr color sgi_grey_56{142, 142, 142};
        inline constexpr color sgi_gray_60{153, 153, 153};
        inline constexpr color sgi_grey_60{153, 153, 153};
        inline constexpr color sgi_gray_64{163, 163, 163};
        inline constexpr color sgi_grey_64{163, 163, 163};
        inline constexpr color sgi_gray_68{173, 173, 173};
        inline constexpr color sgi_grey_68{173, 173, 173};
        inline constexpr color sgi_gray_72{183, 183, 183};
        inline constexpr color sgi_grey_72{183, 183, 183};
        inline constexpr color sgi_gray_76{193, 193, 193};
        inline constexpr color sgi_grey_76{193, 193, 193};
        inline constexpr color sgi_gray_80{204, 204, 204};
        inline constexpr color sgi_grey_80{204, 204, 204};
        inline constexpr color sgi_gray_84{214, 214, 214};
        inline constexpr color sgi_grey_84{214, 214, 214};
        inline constexpr color sgi_gray_88{224, 224, 224};
        inline constexpr color sgi_grey_88{224, 224, 224};
        inline constexpr color sgi_gray_92{234, 234, 234};
        inline constexpr color sgi_grey_92{234, 234, 234};
        inline constexpr color sgi_gray_96{244, 244, 244};
        inline constexpr color sgi_grey_96{244, 244, 244};
        inline constexpr color sgi_gray_100{255, 255, 255};
        inline constexpr color sgi_grey_100{255, 255, 255};
        inline constexpr color sgi_light_blue{125, 158, 192};
        inline constexpr color sgi_dark_gray{85, 85, 85};
        inline constexpr color sgi_dark_grey{85, 85, 85};
        inline constexpr color sgi_salmon{198, 113, 113};
        inline constexpr color sgi_chartreuse{113, 198, 113};
        inline constexpr color sgi_olive_drab{142, 142, 56};
        inline constexpr color sgi_slate_blue{113, 113, 198};
        inline constexpr color sgi_beet{142, 56, 142};
        inline constexpr color sgi_teal{56, 142, 142};
        inline constexpr color sgi_light_gray{170, 170, 170};
        inline constexpr color sgi_light_grey{170, 170, 170};
        inline constexpr color sgi_very_light_gray{214, 214, 214};
        inline constexpr color sgi_very_light_grey{214, 214, 214};
        inline constexpr color sgi_medium_gray{132, 132, 132};
        inline constexpr color sgi_medium_grey{132, 132, 132};
        inline constexpr color sgi_very_dark_gray{40, 40, 40};
        inline constexpr color sgi_very_dark_grey{40, 40, 40};
        inline constexpr color sgi_bright_gray{197, 193, 170};
        inline constexpr color sgi_bright_grey{197, 193, 170};
        inline constexpr color indigo{75, 0, 130};
        inline constexpr color indigo_2{33, 136, 104};
        inline constexpr color crimson{220, 20, 60};

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
