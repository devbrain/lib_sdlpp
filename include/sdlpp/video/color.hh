//
// Created by igor on 01/06/2020.
//

#ifndef NEUTRINO_SDL_COLOR_HH
#define NEUTRINO_SDL_COLOR_HH

#include <algorithm>
#include <cstdint>
#include <tuple>
#include <cmath>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	/**
	 * @brief The color struct represents a color in RGBA format.
	 *
	 * The color struct provides methods for creating colors, converting between color spaces,
	 * and accessing the individual color component values.
	 */
	struct color : public SDL_Color {
		color ();
		constexpr color (uint8_t r, uint8_t g, uint8_t b);
		constexpr color (uint8_t r, uint8_t g, uint8_t b, uint8_t a);
		explicit color (const SDL_Color& c);
		color& operator= (const SDL_Color& c);

		/**
		 * @brief Creates a color object from the given HSL values.
		 *
		 * This static method takes three parameters representing the Hue, Saturation, and Lightness values of the desired color.
		 * It then calculates the corresponding RGB values and creates a color object with those values.
		 *
		 * @param h The Hue value of the color (0-255).
		 * @param s The Saturation value of the color (0-255).
		 * @param l The Lightness value of the color (0-255).
		 * @return The color object created from the given HSL values.
		 *
		 * @see color
		 */
		static color from_hsl (uint8_t h, uint8_t s, uint8_t l);
		/**
		 * @brief Creates a color object from the given HSV values.
		 *
		 * This static method takes three parameters representing the Hue, Saturation, and Value (brightness) values of the desired color.
		 * It then calculates the corresponding RGB values and creates a color object with those values.
		 *
		 * @param h The Hue value of the color (0-255).
		 * @param s The Saturation value of the color (0-255).
		 * @param v The Value (brightness) value of the color (0-255).
		 * @return The color object created from the given HSV values.
		 */
		static color from_hsv (uint8_t h, uint8_t s, uint8_t v);

		/**
		 * @brief Converts the color from RGB to HSL color space.
		 *
		 * This method calculates the Hue, Saturation, and Lightness values of the color in the HSL color space.
		 * The resulting values are returned as a tuple.
		 *
		 * @return A tuple containing the Hue, Saturation, and Lightness values of the color in the HSL color space.
		 *
		 * @see color
		 */
		[[nodiscard]] std::tuple<uint8_t, uint8_t, uint8_t> to_hsl () const;
		/**
		 * @brief Converts the color from RGB to HSV color space.
		 *
		 * This method calculates the Hue, Saturation, and Value (brightness) values of the color in the HSV color space.
		 * The resulting values are returned as a tuple.
		 *
		 * @return A tuple containing the Hue, Saturation, and Value values of the color in the HSV color space.
		 *
		 * @see color
		 */
		[[nodiscard]] std::tuple<uint8_t, uint8_t, uint8_t> to_hsv () const;
	};

	inline
	bool operator== (const color& a, const color& b) {
		return (a.r == b.r) && (a.g == b.g) && (a.b && b.b) && (a.a == b.a);
	}

	inline
	bool operator!= (const color& a, const color& b) {
		return !(a == b);
	}

	d_SDLPP_OSTREAM(const color&);

	inline constexpr color::color (uint8_t r, uint8_t g, uint8_t b)
	: SDL_Color {r, g, b, 0xFF} {
	}

	inline constexpr color::color (uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	: SDL_Color{r, g, b, a} {
	}

namespace colors {
	inline constexpr color transparent{0, 0, 0, 0};
	inline constexpr color white{0xFF, 0xFF, 0xFF};
	inline constexpr color black{0, 0, 0};

	inline constexpr color alice_blue{0xF0, 0xF8, 0xFF};
	inline constexpr color antique_white{0xFA, 0xEB, 0xD7};
	inline constexpr color aqua{0, 0xFF, 0xFF};
	inline constexpr color aquamarine{0x7F, 0xFF, 0xD4};
	inline constexpr color azure{0xF0, 0xFF, 0xFF};
	inline constexpr color beige{0xF5, 0xF5, 0xDC};
	inline constexpr color bisque{0xFF, 0xE4, 0xC4};
	inline constexpr color blanched_almond{0xFF, 0xEB, 0xCD};
	inline constexpr color blue{0, 0, 0xFF};
	inline constexpr color blue_violet{0x8A, 0x2B, 0xE2};
	inline constexpr color brown{0xA5, 0x2A, 0x2A};
	inline constexpr color burly_wood{0xDE, 0xB8, 0x87};
	inline constexpr color cadet_blue{0x5F, 0x9E, 0xA0};
	inline constexpr color chartreuse{0x7F, 0xFF, 0};
	inline constexpr color chocolate{0xD2, 0x69, 0x1E};
	inline constexpr color coral{0xFF, 0x7F, 0x50};
	inline constexpr color cornflower_blue{0x64, 0x95, 0xED};
	inline constexpr color cornsilk{0xFF, 0xF8, 0xDC};
	inline constexpr color crimson{0xDC, 0x14, 0x3C};
	inline constexpr color cyan{0, 0xFF, 0xFF};
	inline constexpr color deep_pink{0xFF, 0x14, 0x93};
	inline constexpr color deep_sky_blue{0, 0xBF, 0xFF};
	inline constexpr color dim_gray{0x69, 0x69, 0x69};
	inline constexpr color dim_grey{dim_gray};
	inline constexpr color dodger_blue{0x1E, 0x90, 0xFF};
	inline constexpr color fire_brick{0xB2, 0x22, 0x22};
	inline constexpr color floral_white{0xFF, 0xFA, 0xF0};
	inline constexpr color forest_green{0x22, 0x8B, 0x22};
	inline constexpr color fuchsia{0xFF, 0, 0xFF};
	inline constexpr color gainsboro{0xDC, 0xDC, 0xDC};
	inline constexpr color ghost_white{0xF8, 0xF8, 0xFF};
	inline constexpr color gold{0xFF, 0xD7, 0};
	inline constexpr color golden_rod{0xDA, 0xA5, 0x20};
	inline constexpr color gray{0x80, 0x80, 0x80};
	inline constexpr color grey{gray};
	inline constexpr color green{0, 0x80, 0};
	inline constexpr color green_yellow{0xAD, 0xFF, 0x2F};
	inline constexpr color honey_dew{0xF0, 0xFF, 0xF0};
	inline constexpr color hot_pink{0xFF, 0x69, 0xB4};
	inline constexpr color indian_red{0xCD, 0x5C, 0x5C};
	inline constexpr color indigo{0x4B, 0, 0x82};
	inline constexpr color ivory{0xFF, 0xFF, 0xF0};
	inline constexpr color khaki{0xF0, 0xE6, 0x8C};
	inline constexpr color lavender{0xE6, 0xE6, 0xFA};
	inline constexpr color lavender_blush{0xFF, 0xF0, 0xF5};
	inline constexpr color lawn_green{0x7C, 0xFC, 0};
	inline constexpr color lemon_chiffon{0xFF, 0xFA, 0xCD};
	inline constexpr color lime{0, 0xFF, 0};
	inline constexpr color lime_green{0x32, 0xCD, 0x32};
	inline constexpr color linen{0xFA, 0xF0, 0xE6};
	inline constexpr color magenta{0xFF, 0, 0xFF};
	inline constexpr color maroon{0x80, 0, 0};
	inline constexpr color midnight_blue{0x19, 0x19, 0x70};
	inline constexpr color mint_cream{0xF5, 0xFF, 0xFA};
	inline constexpr color misty_rose{0xFF, 0xE4, 0xE1};
	inline constexpr color moccasin{0xFF, 0xE4, 0xB5};
	inline constexpr color navajo_white{0xFF, 0xDE, 0xAD};
	inline constexpr color navy{0, 0, 0x80};
	inline constexpr color old_lace{0xFD, 0xF5, 0xE6};
	inline constexpr color olive{0x80, 0x80, 0};
	inline constexpr color olive_drab{0x6B, 0x8E, 0x23};
	inline constexpr color orange{0xFF, 0xA5, 0};
	inline constexpr color orange_red{0xFF, 0x45, 0};
	inline constexpr color orchid{0xDA, 0x70, 0xD6};
	inline constexpr color pale_golden_rod{0xEE, 0xE8, 0xAA};
	inline constexpr color pale_green{0x98, 0xFB, 0x98};
	inline constexpr color pale_turquoise{0xAF, 0xEE, 0xEE};
	inline constexpr color pale_violet_red{0xDB, 0x70, 0x93};
	inline constexpr color papaya_whip{0xFF, 0xEF, 0xD5};
	inline constexpr color peach_puff{0xFF, 0xDA, 0xB9};
	inline constexpr color peru{0xCD, 0x85, 0x3F};
	inline constexpr color pink{0xFF, 0xC0, 0xCB};
	inline constexpr color plum{0xDD, 0xA0, 0xDD};
	inline constexpr color powder_blue{0xB0, 0xE0, 0xE6};
	inline constexpr color purple{0x80, 0, 0x80};
	inline constexpr color rebecca_purple{0x66, 0x33, 0x99};
	inline constexpr color red{0xFF, 0, 0};
	inline constexpr color rosy_brown{0xBC, 0x8F, 0x8F};
	inline constexpr color royal_blue{0x41, 0x69, 0xE1};
	inline constexpr color saddle_brown{0x8B, 0x45, 0x13};
	inline constexpr color salmon{0xFA, 0x80, 0x72};
	inline constexpr color sandy_brown{0xF4, 0xA4, 0x60};
	inline constexpr color sea_green{0x2E, 0x8B, 0x57};
	inline constexpr color sea_shell{0xFF, 0xF5, 0xEE};
	inline constexpr color sienna{0xA0, 0x52, 0x2D};
	inline constexpr color silver{0xC0, 0xC0, 0xC0};
	inline constexpr color sky_blue{0x87, 0xCE, 0xEB};
	inline constexpr color slate_blue{0x6A, 0x5A, 0xCD};
	inline constexpr color slate_gray{0x70, 0x80, 0x90};
	inline constexpr color slate_grey{slate_gray};
	inline constexpr color snow{0xFF, 0xFA, 0xFA};
	inline constexpr color spring_green{0, 0xFF, 0x7F};
	inline constexpr color steel_blue{0x46, 0x82, 0xB4};
	inline constexpr color tan{0xD2, 0xB4, 0x8C};
	inline constexpr color teal{0, 0x80, 0x80};
	inline constexpr color thistle{0xD8, 0xBF, 0xD8};
	inline constexpr color tomato{0xFF, 0x63, 0x47};
	inline constexpr color turquoise{0x40, 0xE0, 0xD0};
	inline constexpr color violet{0xEE, 0x82, 0xEE};
	inline constexpr color wheat{0xF5, 0xDE, 0xB3};
	inline constexpr color white_smoke{0xF5, 0xF5, 0xF5};
	inline constexpr color yellow{0xFF, 0xFF, 0};
	inline constexpr color yellow_green{0x9A, 0xCD, 0x32};

	inline constexpr color light_blue{0xAD, 0xD8, 0xE6};
	inline constexpr color light_coral{0xF0, 0x80, 0x80};
	inline constexpr color light_cyan{0xE0, 0xFF, 0xFF};
	inline constexpr color light_golden_rod_yellow{0xFA, 0xFA, 0xD2};
	inline constexpr color light_gray{0xD3, 0xD3, 0xD3};
	inline constexpr color light_grey{light_gray};
	inline constexpr color light_green{0x90, 0xEE, 0x90};
	inline constexpr color light_pink{0xFF, 0xB6, 0xC1};
	inline constexpr color light_salmon{0xFF, 0xA0, 0x7A};
	inline constexpr color light_sea_green{0x20, 0xB2, 0xAA};
	inline constexpr color light_sky_blue{0x87, 0xCE, 0xFA};
	inline constexpr color light_slate_gray{0x77, 0x88, 0x99};
	inline constexpr color light_slate_grey{light_slate_gray};
	inline constexpr color light_steel_blue{0xB0, 0xC4, 0xDE};
	inline constexpr color light_yellow{0xFF, 0xFF, 0xE0};

	inline constexpr color medium_aqua_marine{0x66, 0xCD, 0xAA};
	inline constexpr color medium_blue{0, 0, 0xCD};
	inline constexpr color medium_orchid{0xBA, 0x55, 0xD3};
	inline constexpr color medium_purple{0x93, 0x70, 0xDB};
	inline constexpr color medium_sea_green{0x3C, 0xB3, 0x71};
	inline constexpr color medium_slate_blue{0x7B, 0x68, 0xEE};
	inline constexpr color medium_spring_green{0, 0xFA, 0x9A};
	inline constexpr color medium_turquoise{0x48, 0xD1, 0xCC};
	inline constexpr color medium_violet_red{0xC7, 0x15, 0x85};

	inline constexpr color dark_blue{0, 0, 0x8B};
	inline constexpr color dark_cyan{0, 0x8B, 0x8B};
	inline constexpr color dark_golden_rod{0xB8, 0x86, 0x0B};
	inline constexpr color dark_gray{0xA9, 0xA9, 0xA9};
	inline constexpr color dark_grey{dark_gray};
	inline constexpr color dark_green{0, 0x64, 0};
	inline constexpr color dark_khaki{0xBD, 0xB7, 0x6B};
	inline constexpr color dark_magenta{0x8B, 0, 0x8B};
	inline constexpr color dark_olive_green{0x55, 0x6B, 0x2F};
	inline constexpr color dark_orange{0xFF, 0x8C, 0};
	inline constexpr color dark_orchid{0x99, 0x32, 0xCC};
	inline constexpr color dark_red{0x8B, 0, 0};
	inline constexpr color dark_salmon{0xE9, 0x96, 0x7A};
	inline constexpr color dark_sea_green{0x8F, 0xBC, 0x8F};
	inline constexpr color dark_slate_blue{0x48, 0x3D, 0x8B};
	inline constexpr color dark_slate_gray{0x2F, 0x4F, 0x4F};
	inline constexpr color dark_slate_grey{dark_slate_gray};
	inline constexpr color dark_turquoise{0, 0xCE, 0xD1};
	inline constexpr color dark_violet{0x94, 0, 0xD3};

}  // namespace colors
} // ns sdl

// =========================================================================
// Implementation
// =========================================================================

namespace neutrino::sdl {
	inline color::color ()
		: SDL_Color{0, 0, 0, 0} {

	}

	inline color::color (const SDL_Color& c)
		: SDL_Color{c.r, c.g, c.b, c.a} {

	}

	inline color& color::operator= (const SDL_Color& c) {
		r = c.r;
		g = c.g;
		b = c.b;
		a = c.a;
		return *this;
	}

	namespace detail {
		static constexpr double EPSILON = 0.001;
	}

	inline color color::from_hsl (uint8_t
	h_,
	uint8_t s_, uint8_t
	l_) {
	double r, g, b, h, s, l; //this function works with floats between 0 and 1
	double temp1, temp2, tempr, tempg, tempb;
	h = h_ / 256.0;
	s = s_ / 256.0;
	l = l_ / 256.0;

	//If saturation is 0, the color is a shade of gray
	if (
	std::abs (s)
	< detail::EPSILON) {
	r = g = b = l;
}
//If saturation > 0, more complex calculations are needed
else {
//Set the temporary values
if (l < 0.5) {
temp2 = l * (1 + s);
} else {
temp2 = (l + s) - (l * s);
}
temp1 = 2 * l - temp2;
tempr = h + 1.0 / 3.0;
if (tempr > 1) {
tempr--;
}
tempg = h;
tempb = h - 1.0 / 3.0;
if (tempb < 0) {
tempb++;
}

//Red
if (tempr < 1.0 / 6.0) {
r = temp1 + (temp2 - temp1) * 6.0 * tempr;
} else {
if (tempr < 0.5) {
r = temp2;
} else {
if (tempr < 2.0 / 3.0) {
r = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempr) * 6.0;
} else {
r = temp1;
}
}
}

//Green
if (tempg < 1.0 / 6.0) {
g = temp1 + (temp2 - temp1) * 6.0 * tempg;
} else {
if (tempg < 0.5) {
g = temp2;
} else {
if (tempg < 2.0 / 3.0) {
g = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempg) * 6.0;
} else {
g = temp1;
}
}
}

//Blue
if (tempb < 1.0 / 6.0) {
b = temp1 + (temp2 - temp1) * 6.0 * tempb;
} else {
if (tempb < 0.5) {
b = temp2;
} else {
if (tempb < 2.0 / 3.0) {
b = temp1 + (temp2 - temp1) * ((2.0 / 3.0) - tempb) * 6.0;
} else {
b = temp1;
}
}
}
}
return color (static_cast
<uint8_t>(r
* 256), static_cast
<uint8_t>(g
* 256), static_cast
<uint8_t>(b
* 256));
}

inline color color::from_hsv (uint8_t h_, uint8_t s_, uint8_t v_) {
	double r, g, b, h, s, v; //this function works with floats between 0 and 1
	h = h_ / 256.0;
	s = s_ / 256.0;
	v = v_ / 256.0;

	//If saturation is 0, the color is a shade of gray
	if (std::abs (s) < detail::EPSILON) {
		r = g = b = v;
	}
		//If saturation > 0, more complex calculations are needed
	else {
		double f, p, q, t;
		int i;
		h *= 6; //to bring hue to a number between 0 and 6, better for the calculations
		i = int (floor (h));  //e.g. 2.7 becomes 2 and 3.01 becomes 3 or 4.9999 becomes 4
		f = h - i;  //the fractional part of h
		p = v * (1 - s);
		q = v * (1 - (s * f));
		t = v * (1 - (s * (1 - f)));
		switch (i) {
			case 0:r = v;
				g = t;
				b = p;
				break;
			case 1:r = q;
				g = v;
				b = p;
				break;
			case 2:r = p;
				g = v;
				b = t;
				break;
			case 3:r = p;
				g = q;
				b = v;
				break;
			case 4:r = t;
				g = p;
				b = v;
				break;
			default:
				/*case 5:*/ r = v;
				g = p;
				b = q;
				break;

		}
	}
	return color (static_cast<uint8_t >(r * 256), static_cast<uint8_t >(g * 256), static_cast<uint8_t >(b * 256));
}

inline
std::tuple<uint8_t, uint8_t, uint8_t> color::to_hsl () const {
	double fr, fg, fb, h, s, l; //this function works with floats between 0 and 1
	fr = r / 256.0;
	fg = g / 256.0;
	fb = b / 256.0;
	auto maxColor = std::max (fr, std::max (fg, fb));
	auto minColor = std::min (fr, std::min (fg, fb));
	//R == G == B, so it's a shade of gray
	if (std::abs (minColor - maxColor) < detail::EPSILON) {
		h = 0.0; //it doesn't matter what value it has
		s = 0.0;
		l = fr; //doesn't matter if you pick r, g, or b
	} else {
		l = (minColor + maxColor) / 2;

		if (l < 0.5) {
			s = (maxColor - minColor) / (maxColor + minColor);
		} else {
			s = (maxColor - minColor) / (2.0 - maxColor - minColor);
		}

		if (std::abs (fr - maxColor) < detail::EPSILON) {
			h = (fg - fb) / (maxColor - minColor);
		} else {
			if (std::abs (fg - maxColor) < detail::EPSILON) {
				h = 2.0 + (fb - fr) / (maxColor - minColor);
			} else {
				h = 4.0 + (fr - fg) / (maxColor - minColor);
			}
		}

		h /= 6; //to bring it to a number between 0 and 1
		if (h < 0) {
			h++;
		}
	}
	return {static_cast<uint8_t>(256 * h), static_cast<uint8_t>(256 * s), static_cast<uint8_t>(256 * l)};
}

inline
std::tuple<uint8_t, uint8_t, uint8_t> color::to_hsv () const {
	double fr, fg, fb, h, s, v; //this function works with floats between 0 and 1
	fr = r / 256.0;
	fg = g / 256.0;
	fb = b / 256.0;
	auto maxColor = std::max (fr, std::max (fg, fb));
	auto minColor = std::min (fr, std::min (fg, fb));
	v = maxColor;
	if (std::abs (maxColor) < detail::EPSILON) //avoid division by zero when the color is black
	{
		s = 0;
	} else {
		s = (maxColor - minColor) / maxColor;
	}
	if (std::abs (s) < detail::EPSILON) {
		h = 0; //it doesn't matter what value it has
	} else {
		if (std::abs (r - maxColor) < detail::EPSILON) {
			h = (fg - fb) / (maxColor - minColor);
		} else {
			if (std::abs (fg - maxColor) < detail::EPSILON) {
				h = 2.0 + (fb - fr) / (maxColor - minColor);
			} else {
				h = 4.0 + (fr - fg) / (maxColor - minColor);
			}
		}
		h /= 6.0; //to bring it to a number between 0 and 1
		if (h < 0) {
			h++;
		}
	}
	return {static_cast<uint8_t>(256 * h), static_cast<uint8_t>(256 * s), static_cast<uint8_t>(256 * v)};
}

}

#endif
