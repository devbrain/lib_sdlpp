//
// Created by igor on 6/16/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_AUDIO_MUSIC_HH_
#define SDLPP_INCLUDE_SDLPP_AUDIO_MUSIC_HH_

#include <optional>
#include <chrono>
#include <string>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	class music : public object<Mix_Music> {
	 public:
		enum class format {
			NONE = MUS_NONE,
			CMD = MUS_CMD,
			WAV = MUS_WAV,
			MOD = MUS_MOD,
			MID = MUS_MID,
			OGG = MUS_OGG,
			MP3 = MUS_MP3,
			MP3_MAD_UNUSED = MUS_MP3_MAD_UNUSED,
			FLAC = MUS_FLAC,
			MODPLUG_UNUSED = MUS_MODPLUG_UNUSED,
			OPUS = MUS_OPUS,
			WAVPACK = MUS_WAVPACK,
			GME = MUS_GME
		};
	 public:
		music() = default;

		explicit music(object<SDL_RWops>& rwops);
		explicit music (object<Mix_Music>&& other);
		music& operator= (object<Mix_Music>&& other) noexcept;
		music& operator= (music&& other) noexcept;

		music& operator= (const music& other) = delete;
		music (const music& other) = delete;

		[[nodiscard]] unsigned get_volume() const;
		[[nodiscard]] std::optional<std::string> get_album() const;
		[[nodiscard]] std::optional<std::string> get_artist() const;
		[[nodiscard]] std::optional<std::string> get_copyright() const;
		[[nodiscard]] std::optional<std::string> get_title() const;

		[[nodiscard]] std::chrono::milliseconds get_loop_end_time() const;
		[[nodiscard]] std::chrono::milliseconds get_loop_start_time() const;
		[[nodiscard]] std::chrono::milliseconds get_loop_length_time() const;
		[[nodiscard]] std::chrono::milliseconds get_position() const;

		[[nodiscard]] format get_format() const;
	};

	d_SDLPP_OSTREAM(music::format);

	inline
	music::music (object<Mix_Music>&& other)
		: object<Mix_Music> (std::move(other)) {

	}

	inline
	music& music::operator= (object<Mix_Music>&& other) noexcept {
		object<Mix_Music>::operator= (std::move (other));
		return *this;;
	}

	inline
	music& music::operator= (music&& other) noexcept {
		object<Mix_Music>::operator= (std::move (other));
		return *this;
	}



	inline
	unsigned music::get_volume () const {
		return static_cast<unsigned >(SAFE_SDL_CALL(Mix_GetMusicVolume, const_handle()));
	}

	namespace detail {
		inline
		std::optional<std::string> opt_string(const char* p) {
			return !p ? std::nullopt : std::optional{std::string (p)};
		}
	}

	std::optional<std::string> music::get_album () const {
		return detail::opt_string (Mix_GetMusicAlbumTag (const_handle()));
	}

	std::optional<std::string> music::get_artist () const {
		return detail::opt_string (Mix_GetMusicArtistTag (const_handle()));
	}

	std::optional<std::string> music::get_copyright () const {
		return detail::opt_string (Mix_GetMusicCopyrightTag (const_handle()));
	}

	std::optional<std::string> music::get_title () const {
		const char* p = Mix_GetMusicTitleTag (const_handle());
		if (p) {
			return p;
		}
		return detail::opt_string (Mix_GetMusicTitle (const_handle()));
	}

	std::chrono::milliseconds music::get_loop_end_time () const {
		return std::chrono::milliseconds(static_cast<unsigned long>(1000* SAFE_SDL_CALL(Mix_GetMusicLoopEndTime ,const_handle())));
	}

	std::chrono::milliseconds music::get_loop_length_time () const {
		return std::chrono::milliseconds(static_cast<unsigned long>(1000* SAFE_SDL_CALL(Mix_GetMusicLoopLengthTime,const_handle())));
	}

	std::chrono::milliseconds music::get_loop_start_time () const {
		return std::chrono::milliseconds(static_cast<unsigned long>(1000* SAFE_SDL_CALL(Mix_GetMusicLoopStartTime,const_handle())));
	}

	std::chrono::milliseconds music::get_position () const {
		return std::chrono::milliseconds(static_cast<unsigned long>(1000* SAFE_SDL_CALL(Mix_GetMusicPosition,const_handle())));
	}

	music::format music::get_format () const {
		return static_cast<format>(Mix_GetMusicType (const_handle()));
	}

	music::music (object<SDL_RWops>& rwops)
	: object<Mix_Music>(SAFE_SDL_CALL(Mix_LoadMUS_RW, rwops.handle(), 0), true)
	{

	}

}

#endif //SDLPP_INCLUDE_SDLPP_AUDIO_MUSIC_HH_
