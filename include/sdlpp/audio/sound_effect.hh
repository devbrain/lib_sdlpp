//
// Created by igor on 6/13/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_AUDIO_SOUND_EFFECT_HH_
#define SDLPP_INCLUDE_SDLPP_AUDIO_SOUND_EFFECT_HH_

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/call.hh>

namespace neutrino::sdl {
	class sound_effect : public object<Mix_Chunk> {
	 public:
		sound_effect() = default;

		explicit sound_effect(object<SDL_RWops>& rwops);
		explicit sound_effect (object<Mix_Chunk>&& other);
		sound_effect& operator= (object<Mix_Chunk>&& other) noexcept;
		sound_effect& operator= (sound_effect&& other) noexcept;

		sound_effect& operator= (const sound_effect& other) = delete;
		sound_effect (const sound_effect& other) = delete;

		void set_volume(unsigned v);
		[[nodiscard]] unsigned get_volume() const;
	};

	inline
	sound_effect::sound_effect (object<Mix_Chunk>&& other)
		: object<Mix_Chunk> (std::move(other)) {

	}

	inline
	sound_effect& sound_effect::operator= (object<Mix_Chunk>&& other) noexcept {
		object<Mix_Chunk>::operator= (std::move (other));
		return *this;;
	}

	inline
	sound_effect& sound_effect::operator= (sound_effect&& other) noexcept {
		object<Mix_Chunk>::operator= (std::move (other));
		return *this;
	}

	inline
	void sound_effect::set_volume (unsigned int v) {
		SAFE_SDL_CALL(Mix_VolumeChunk, handle(), static_cast<int>(v));
	}

	inline
	unsigned sound_effect::get_volume () const {
		return static_cast<unsigned >(SAFE_SDL_CALL(Mix_VolumeChunk, const_handle(), -1));
	}

	sound_effect::sound_effect (object<SDL_RWops>& rwops)
	: object<Mix_Chunk>(SAFE_SDL_CALL(Mix_LoadWAV_RW, rwops.handle(), 0), true)
	{

	}

}

#endif //SDLPP_INCLUDE_SDLPP_AUDIO_SOUND_EFFECT_HH_
