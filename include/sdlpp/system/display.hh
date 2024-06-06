//
// Created by igor on 6/6/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_DISPLAY_HH_
#define SDLPP_INCLUDE_SDLPP_DISPLAY_HH_

#include <cstddef>
#include <string>
#include <tuple>
#include <optional>
#include "sdlpp/detail/sdl2.hh"
#include "sdlpp/detail/call.hh"
#include "sdlpp/video/geometry.hh"
#include "sdlpp/video/pixel_format.hh"
#include "bsw/errors.hh"

namespace neutrino::sdl {
	class display {
	 public:
		enum class orientation : int {
			UNKNOWN = SDL_ORIENTATION_UNKNOWN,            /**< The display orientation can't be determined */
			LANDSCAPE = SDL_ORIENTATION_LANDSCAPE,          /**< The display is in landscape mode, with the right side up, relative to portrait mode */
			LANDSCAPE_FLIPPED = SDL_ORIENTATION_LANDSCAPE_FLIPPED,  /**< The display is in landscape mode, with the left side up, relative to portrait mode */
			PORTRAIT = SDL_ORIENTATION_PORTRAIT,           /**< The display is in portrait mode */
			PORTRAIT_FLIPPED = SDL_ORIENTATION_PORTRAIT_FLIPPED    /**< The display is in portrait mode, upside down */
		};
	 public:
		class mode {
			friend class display;

		 public:
			mode (const display& d, std::size_t mode_index);

			[[nodiscard]] pixel_format get_pixel_format () const noexcept;

			[[nodiscard]] std::optional<int> get_refresh_rate () const noexcept;

			[[nodiscard]] area_type get_bounds () const noexcept;

		 private:
			explicit mode (const SDL_DisplayMode& dm);

		 private:
			uint32_t m_format;
			std::optional<int> m_refresh_rate;
			area_type m_area;
		};

		static std::size_t count () {
			auto rc = SAFE_SDL_CALL(SDL_GetNumVideoDisplays);
			return static_cast<std::size_t>(rc);
		}

		static std::optional<std::string> video_driver () {
			const char* drv = SDL_GetCurrentVideoDriver ();
			if (drv) {
				return drv;
			}
			return std::nullopt;
		}

		static std::size_t count_video_drivers () {
			return SAFE_SDL_CALL(SDL_GetNumVideoDrivers);
		}

		static std::optional<std::string> video_driver (std::size_t index) {
			const char* drv = SDL_GetVideoDriver (static_cast<int>(index));
			if (drv) {
				return drv;
			}
			return std::nullopt;
		}

		static bool screen_saver_enabled () {
			return SDL_IsScreenSaverEnabled () == SDL_TRUE;
		}

		explicit display (std::size_t index);

		[[nodiscard]] std::string get_name () const;

		// ddpi, hdpi, vdpi>
		[[nodiscard]] std::tuple<float, float, float> get_dpi () const;

		[[nodiscard]] orientation get_orientation () const noexcept;

		[[nodiscard]] std::size_t get_index () const noexcept;

		[[nodiscard]] area_type get_bounds () const noexcept;

		[[nodiscard]] area_type get_desktop_bounds () const noexcept;
		[[nodiscard]] std::size_t get_modes () const noexcept;

		[[nodiscard]] mode get_mode () const;
		[[nodiscard]] mode get_desktop_mode () const;

		[[nodiscard]] mode get_mode (std::size_t idx) const;
		[[nodiscard]] std::optional<mode> find_closest_mode (const area_type& area) const;

	 private:
		std::size_t m_index;
		area_type m_area;
		std::size_t m_num_of_modes;

	};

	inline
	display::mode::mode (const display& d, std::size_t mode_index) {
		SDL_DisplayMode dm;
		SAFE_SDL_CALL(SDL_GetDisplayMode, static_cast<int>(d.get_index ()), static_cast<int>(mode_index), &dm);
		m_format = dm.format;
		if (dm.refresh_rate > 0) {
			m_refresh_rate = dm.refresh_rate;
		}
		m_area.w = dm.w;
		m_area.h = dm.h;
	}

	inline
	display::mode::mode (const SDL_DisplayMode& dm)
		: m_format (dm.format),
		  m_area (dm.w, dm.h) {
		if (dm.refresh_rate > 0) {
			m_refresh_rate = dm.refresh_rate;
		}
	}

	inline
	pixel_format display::mode::get_pixel_format () const noexcept {
		return pixel_format (m_format);
	}

	inline
	std::optional<int> display::mode::get_refresh_rate () const noexcept {
		return m_refresh_rate;
	}

	inline
	area_type display::mode::get_bounds () const noexcept {
		return m_area;
	}

	inline
	display::display (std::size_t index)
		: m_index (index) {
		rect r;
		SAFE_SDL_CALL(SDL_GetDisplayBounds, static_cast<int>(index), &r);
		m_area = r.area ();

		auto display_modes = SAFE_SDL_CALL(SDL_GetNumDisplayModes, static_cast<int>(index));
		m_num_of_modes = static_cast<std::size_t > (display_modes);
	}

	inline
	std::string display::get_name () const {
		return SAFE_SDL_CALL(SDL_GetDisplayName, static_cast<int>(m_index));
	}

	// ddpi, hdpi, vdpi>
	inline
	std::tuple<float, float, float> display::get_dpi () const {
		float ddpi = 0, hdpi = 0, vdpi = 0;
		SAFE_SDL_CALL(SDL_GetDisplayDPI, static_cast<int>(m_index), &ddpi, &hdpi, &vdpi);
		return {ddpi, hdpi, vdpi};
	}

	inline
	display::orientation display::get_orientation () const noexcept {
		return static_cast<orientation>(SDL_GetDisplayOrientation (static_cast<int>(m_index)));
	}

	inline
	std::size_t display::get_index () const noexcept {
		return m_index;
	}

	inline
	area_type display::get_bounds () const noexcept {
		return m_area;
	}

	inline
	area_type display::get_desktop_bounds () const noexcept {
		rect r;
		SAFE_SDL_CALL(SDL_GetDisplayUsableBounds, static_cast<int>(m_index), &r);
		return r.area ();
	}

	inline
	std::size_t display::get_modes () const noexcept {
		return m_num_of_modes;
	}

	inline
	display::mode display::get_mode () const {
		SDL_DisplayMode dm;
		SAFE_SDL_CALL(SDL_GetCurrentDisplayMode, static_cast<int>(m_index), &dm);
		return mode (dm);
	}

	inline
	display::mode display::get_desktop_mode () const {
		SDL_DisplayMode dm;
		SAFE_SDL_CALL(SDL_GetDesktopDisplayMode, static_cast<int>(m_index), &dm);
		return mode (dm);
	}

	inline
	display::mode display::get_mode (std::size_t idx) const {
		return {*this, idx};
	}

	inline
	std::optional<display::mode> display::find_closest_mode (const area_type& area) const {
		SDL_DisplayMode desired;
		desired.refresh_rate = 0;
		desired.w = static_cast<int>(area.w);
		desired.h = static_cast<int>(area.h);
		SDL_DisplayMode closest{};
		if (SDL_GetClosestDisplayMode (static_cast<int>(m_index), &desired, &closest)) {
			return mode (closest);
		}
		return std::nullopt;
	}
}

#endif //SDLPP_INCLUDE_SDLPP_DISPLAY_HH_
