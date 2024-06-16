//
// Created by igor on 6/14/24.
//

#include <algorithm>
#include <iterator>
#include <sstream>
#include <sdlpp/system/audio.hh>
#include "utils/switch_ostream.hh"
#include <bsw/strings/string_tokenizer.hh>

namespace neutrino::sdl {
	void audio::open (unsigned channels, std::size_t chunk_size) {
		SAFE_SDL_CALL(Mix_OpenAudio, MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, static_cast<int>(channels), static_cast<int>(chunk_size));
	}

	void audio::open (unsigned freq, format f, unsigned channels, std::size_t chunk_size) {
		SAFE_SDL_CALL(Mix_OpenAudio, freq, static_cast<int>(f), static_cast<int>(channels), static_cast<int>(chunk_size));
	}

	void audio::close () {
		Mix_CloseAudio ();
	}

	std::set<std::string> audio::get_music_decoders () {
		std::set<std::string> out;
		for (int i = 0; i < Mix_GetNumMusicDecoders (); i++) {
			if (const auto* name = Mix_GetMusicDecoder (i)) {
				out.insert (name);
			}
		}
		return out;
	}

	std::set<std::string> audio::get_sound_effect_decoders () {
		std::set<std::string> out;
		for (int i = 0; i < Mix_GetNumChunkDecoders (); i++) {
			if (const auto* name = Mix_GetChunkDecoder (i)) {
				out.insert (name);
			}
		}
		return out;
	}

	unsigned audio::get_volume () {
		return static_cast<unsigned >(Mix_MasterVolume (-1));
	}

	void audio::set_volume (unsigned v) {
		Mix_MasterVolume (clamp(static_cast<int>(v), 0, MIX_MAX_VOLUME));
	}

	audio_channel_id_t audio::get_channels_count () {
		return audio_channel_id_t{static_cast<std::size_t> (Mix_AllocateChannels (-1))};
	}

	void audio::allocate_channels (std::size_t num) {
		auto rc = Mix_AllocateChannels (static_cast<int>(num));
		if (rc != num) {
			RAISE_SDL_EX("Failed to allocate ", num, " channels");
		}
	}

	void audio::group_channels (audio_channel_id_t from, audio_channel_id_t to, audio_group_id_t tag) {
		SAFE_SDL_CALL(Mix_GroupChannels, static_cast<int>(from.value_of ()), static_cast<int>(to
			.value_of ()), static_cast<int>(tag.value_of ()));
	}

	void audio::group_channels (audio_channel_id_t chan, audio_group_id_t tag) {
		SAFE_SDL_CALL(Mix_GroupChannel, static_cast<int>(chan.value_of ()), static_cast<int>(tag.value_of ()));
	}

	void audio::ungroup_channels (audio_channel_id_t from, audio_channel_id_t to) {
		SAFE_SDL_CALL(Mix_GroupChannels, static_cast<int>(from.value_of ()), static_cast<int>(to.value_of ()), -1);
	}

	audio_channel_id_t audio::get_channels_count (audio_channel_id_t tag) {
		return audio_channel_id_t{
			static_cast<std::size_t>(SAFE_SDL_CALL(Mix_GroupCount, static_cast<int>(tag.value_of ())))};
	}

	std::optional<audio_channel_id_t> audio::find_most_recent_playing_channel (audio_channel_id_t tag) {
		auto rc = Mix_GroupNewer (static_cast<int>(tag.value_of ()));
		if (rc == -1) {
			return std::nullopt;
		}
		return audio_channel_id_t{static_cast<std::size_t>(rc)};
	}

	std::optional<audio_channel_id_t> audio::find_oldest_playing_channel (audio_channel_id_t tag) {
		auto rc = Mix_GroupOldest (static_cast<int>(tag.value_of ()));
		if (rc == -1) {
			return std::nullopt;
		}
		return audio_channel_id_t{static_cast<std::size_t>(rc)};
	}

	void audio::halt (audio_channel_id_t chan) {
		SAFE_SDL_CALL(Mix_HaltChannel, static_cast<int>(chan.value_of ()));
	}

	void audio::halt () {
		SAFE_SDL_CALL(Mix_HaltChannel, -1);
	}

	void audio::halt (audio_group_id_t grp) {
		Mix_HaltGroup (static_cast<int>(grp.value_of ()));
	}

	void audio::halt_music () {
		Mix_HaltMusic ();
	}

	void audio::pause (audio_channel_id_t chan) {
		Mix_Pause (static_cast<int>(chan.value_of ()));
	}

	void audio::resume (audio_channel_id_t chan) {
		Mix_Resume (static_cast<int>(chan.value_of ()));
	}

	void audio::pause () {
		Mix_PauseAudio (1);
	}

	void audio::resume () {
		Mix_PauseAudio (0);
	}

	bool audio::is_paused (audio_channel_id_t chan) {
		return 1 == Mix_Paused (static_cast<int>(chan.value_of ()));
	}

	std::size_t audio::paused_channels_count () {
		return static_cast<std::size_t >(Mix_Paused (-1));
	}

	bool audio::is_music_paused () {
		return 1 == Mix_PausedMusic ();
	}

	void audio::pause_music () {
		Mix_PauseMusic ();
	}

	void audio::resume_music () {
		Mix_ResumeMusic ();
	}

	bool audio::is_music_playing () {
		return 0 != Mix_PlayingMusic ();
	}

	bool audio::is_playing (audio_channel_id_t chan) {
		return 0 != Mix_Playing (static_cast<int>(chan.value_of ()));
	}

	std::size_t audio::playing_channels_count () {
		return static_cast<std::size_t >(Mix_Playing (-1));
	}

	void audio::play (audio_channel_id_t chan, unsigned int loops, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_PlayChannel, static_cast<int>(chan.value_of ()), effect
			.const_handle (), static_cast<int>(loops));
	}

	void audio::play (audio_channel_id_t chan, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_PlayChannel, static_cast<int>(chan.value_of ()), effect.const_handle (), 0);
	}

	void audio::play_inf (audio_channel_id_t chan, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_PlayChannel, static_cast<int>(chan.value_of ()), effect.const_handle (), -1);
	}

	bool audio::play_on_first_available_channel (unsigned int loops, const object<Mix_Chunk>& effect) {
		return -1 != Mix_PlayChannel (-1, effect.const_handle (), static_cast<int>(loops));
	}

	bool audio::play_on_first_available_channel (const object<Mix_Chunk>& effect) {
		return -1 != Mix_PlayChannel (-1, effect.const_handle (), 0);
	}
	bool audio::play_on_first_available_channel_inf (const object<Mix_Chunk>& effect) {
		return -1 != Mix_PlayChannel (-1, effect.const_handle (), -1);
	}

	void
	audio::play (audio_channel_id_t chan, unsigned int loops, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_PlayChannelTimed, static_cast<int>(chan.value_of ()),
					  effect.const_handle (), static_cast<int>(loops), static_cast<int>(ticks.count ()));
	}

	void audio::play (audio_channel_id_t chan, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_PlayChannelTimed, static_cast<int>(chan.value_of ()),
					  effect.const_handle (), 0, static_cast<int>(ticks.count ()));
	}

	void audio::play_inf (audio_channel_id_t chan, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_PlayChannelTimed, static_cast<int>(chan.value_of ()),
					  effect.const_handle (), -1, static_cast<int>(ticks.count ()));
	}

	bool
	audio::play_on_first_available_channel (unsigned int loops, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		return -1 != Mix_PlayChannelTimed(-1, effect.const_handle (), static_cast<int>(loops), static_cast<int>(ticks.count()));
	}

	bool audio::play_on_first_available_channel(std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		return -1 != Mix_PlayChannelTimed(-1, effect.const_handle (), 0, static_cast<int>(ticks.count()));
	}

	bool audio::play_on_first_available_channel_inf (std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		return -1 != Mix_PlayChannelTimed(-1, effect.const_handle (), -1, static_cast<int>(ticks.count()));
	}

	void audio::play (unsigned int loops, const object<Mix_Music>& mus) {
		SAFE_SDL_CALL(Mix_PlayMusic, mus.const_handle(), static_cast<int>(loops));
	}

	void audio::play (const object<Mix_Music>& mus) {
		SAFE_SDL_CALL(Mix_PlayMusic, mus.const_handle(), 0);
	}

	void audio::rewind_music () {
		Mix_RewindMusic();
	}

	void audio::rewind_music (std::chrono::milliseconds pos) {
		using namespace std::chrono_literals;
		double fp_seconds = static_cast<double>(pos/1.0s);
		SAFE_SDL_CALL(Mix_SetMusicPosition, fp_seconds);
	}

	void audio::rewind_music(std::chrono::seconds pos) {
		SAFE_SDL_CALL(Mix_SetMusicPosition, static_cast<double >(pos.count()));
	}

	void audio::set_distance (audio_channel_id_t chan, uint8_t distance) {
		if (0 == Mix_SetDistance (static_cast<int>(chan.value_of()), distance)) {
			RAISE_SDL_EX();
		}
	}

	void audio::set_panning (audio_channel_id_t chan, uint8_t left, uint8_t right) {
		if (0 == Mix_SetPanning (static_cast<int>(chan.value_of()), left, right)) {
			RAISE_SDL_EX();
		}
	}

	void audio::set_position (audio_channel_id_t chan, int16_t angle, uint8_t distance) {
		if (0 == Mix_SetPosition (static_cast<int>(chan.value_of()), angle, distance)) {
			RAISE_SDL_EX();
		}
	}

	void audio::set_reverse_stereo (audio_channel_id_t chan, bool enable) {
		if (0 == Mix_SetReverseStereo (static_cast<int>(chan.value_of()), enable ? 1 : 0)) {
			RAISE_SDL_EX();
		}
	}

	void audio::set_expiration (audio_channel_id_t chan, std::chrono::milliseconds ticks) {
		Mix_ExpireChannel (static_cast<int>(chan.value_of()), static_cast<int>(ticks.count()));
	}

	void audio::unset_expiration (audio_channel_id_t chan) {
		Mix_ExpireChannel (static_cast<int>(chan.value_of()), -1);
	}


	void
	audio::fade_in (audio_channel_id_t chan, unsigned int loops, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_FadeInChannel, static_cast<int>(chan.value_of ()),
					  effect.const_handle (), static_cast<int>(loops), static_cast<int>(ticks.count ()));
	}

	void audio::fade_in (audio_channel_id_t chan, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_FadeInChannel, static_cast<int>(chan.value_of ()),
					  effect.const_handle (), 0, static_cast<int>(ticks.count ()));
	}

	void audio::fade_in_inf (audio_channel_id_t chan, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_FadeInChannel, static_cast<int>(chan.value_of ()),
					  effect.const_handle (), -1, static_cast<int>(ticks.count ()));
	}

	bool
	audio::fade_in_on_first_available_channel (unsigned int loops, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		return -1 != Mix_FadeInChannel(-1, effect.const_handle (), static_cast<int>(loops), static_cast<int>(ticks.count()));
	}

	bool audio::fade_in_on_first_available_channel(std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		return -1 != Mix_FadeInChannel(-1, effect.const_handle (), 0, static_cast<int>(ticks.count()));
	}

	bool audio::fade_in_on_first_available_channel_inf (std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		return -1 != Mix_FadeInChannel(-1, effect.const_handle (), -1, static_cast<int>(ticks.count()));
	}

	void audio::fade_out (audio_channel_id_t chan, std::chrono::milliseconds ticks) {
		Mix_FadeOutChannel (static_cast<int>(chan.value_of()), static_cast<int>(ticks.count()));
	}

	void audio::fade_in (unsigned int loops, std::chrono::milliseconds ticks, const object<Mix_Music>& mus) {
		SAFE_SDL_CALL(Mix_FadeInMusic, mus.const_handle(), static_cast<int>(loops), static_cast<int>(ticks.count()));
	}

	void audio::fade_in (std::chrono::milliseconds ticks, const object<Mix_Music>& mus) {
		SAFE_SDL_CALL(Mix_FadeInMusic, mus.const_handle(), 0, static_cast<int>(ticks.count()));
	}

	void audio::fade_in_inf (std::chrono::milliseconds ticks, const object<Mix_Music>& mus) {
		SAFE_SDL_CALL(Mix_FadeInMusic, mus.const_handle(), -1, static_cast<int>(ticks.count()));
	}

	void
	audio::fade_in (unsigned int loops, std::chrono::milliseconds ticks, std::chrono::milliseconds pos, const object<
		Mix_Music>& mus) {
		using namespace std::chrono_literals;
		double fp_seconds = static_cast<double>(pos/1.0s);
		SAFE_SDL_CALL(Mix_FadeInMusicPos, mus.const_handle(), static_cast<int>(loops), static_cast<int>(ticks.count()), fp_seconds);
	}

	void audio::fade_in(unsigned loops, std::chrono::milliseconds ticks, std::chrono::seconds pos, const object<Mix_Music>& mus) {
		SAFE_SDL_CALL(Mix_FadeInMusicPos, mus.const_handle(), static_cast<int>(loops), static_cast<int>(ticks.count()), static_cast<double>(pos.count()));
	}

	void audio::fade_in(std::chrono::milliseconds ticks, std::chrono::milliseconds pos, const object<Mix_Music>& mus) {
		fade_in (0, ticks, pos, mus);
	}
	void audio::fade_in(std::chrono::milliseconds ticks, std::chrono::seconds pos, const object<Mix_Music>& mus) {
		fade_in (0, ticks, pos, mus);
	}
	void audio::fade_in_inf(std::chrono::milliseconds ticks, std::chrono::milliseconds pos, const object<Mix_Music>& mus) {
		using namespace std::chrono_literals;
		double fp_seconds = static_cast<double>(pos/1.0s);
		SAFE_SDL_CALL(Mix_FadeInMusicPos, mus.const_handle(), -1, static_cast<int>(ticks.count()), fp_seconds);
	}
	void audio::fade_in_inf(std::chrono::milliseconds ticks, std::chrono::seconds pos, const object<Mix_Music>& mus) {
		SAFE_SDL_CALL(Mix_FadeInMusicPos, mus.const_handle(), -1, static_cast<int>(ticks.count()), static_cast<double>(pos.count()));
	}

	void audio::fade_out (audio_group_id_t tag, std::chrono::milliseconds ticks) {
		Mix_FadeOutGroup (static_cast<int>(tag.value_of()), static_cast<int>(ticks.count()));
	}

	void audio::fade_out_music (std::chrono::milliseconds ticks) {
		Mix_FadeOutMusic (static_cast<int>(ticks.count()));
	}

	void audio::fade_in(audio_channel_id_t chan, unsigned loops, std::chrono::milliseconds effect_duration, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_FadeInChannelTimed, static_cast<int>(chan.value_of()), effect.const_handle(), static_cast<int>(loops), static_cast<int>(ticks.count()), static_cast<int>(effect_duration.count()));
	}
	void audio::fade_in(audio_channel_id_t chan, std::chrono::milliseconds effect_duration,std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_FadeInChannelTimed, static_cast<int>(chan.value_of()), effect.const_handle(), 0, static_cast<int>(ticks.count()), static_cast<int>(effect_duration.count()));
	}
	void audio::fade_in_inf(audio_channel_id_t chan, std::chrono::milliseconds effect_duration,std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		SAFE_SDL_CALL(Mix_FadeInChannelTimed, static_cast<int>(chan.value_of()), effect.const_handle(), -1, static_cast<int>(ticks.count()), static_cast<int>(effect_duration.count()));
	}
	bool audio::fade_in_on_first_available_channel(unsigned loops, std::chrono::milliseconds effect_duration, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		return -1 != Mix_FadeInChannelTimed (-1, effect.const_handle(), static_cast<int>(loops), static_cast<int>(ticks.count()), static_cast<int>(effect_duration.count()));
	}
	bool audio::fade_in_on_first_available_channel(std::chrono::milliseconds effect_duration, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		return -1 != Mix_FadeInChannelTimed (-1, effect.const_handle(), 0, static_cast<int>(ticks.count()), static_cast<int>(effect_duration.count()));
	}
	bool audio::fade_in_on_first_available_channel_inf(std::chrono::milliseconds effect_duration, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect) {
		return -1 != Mix_FadeInChannelTimed (-1, effect.const_handle(), -1, static_cast<int>(ticks.count()), static_cast<int>(effect_duration.count()));
	}

	audio::fading_status audio::get_fading_status (audio_channel_id_t chan) {
		return static_cast<fading_status>(Mix_FadingChannel (static_cast<int>(chan.value_of())));
	}

	audio::fading_status audio::get_fading_status_music () {
		return static_cast<fading_status>(Mix_FadingMusic());
	}

	object<Mix_Chunk> audio::get_sound_effect (audio_channel_id_t chan) {
		return {Mix_GetChunk (static_cast<int>(chan.value_of())), false};
	}

	static void SDLCALL pre_effect(int chan, void *stream, int len, void *udata) {
		auto* func = reinterpret_cast<sound_effect_callback*>(udata);
		func->before_sound(audio_channel_id_t{static_cast<std::size_t>(chan)}, stream, static_cast<std::size_t>(len));
	}

	static void SDLCALL post_effect(int chan, void* udata) {
		auto* func = reinterpret_cast<sound_effect_callback*>(udata);
		func->after_sound(audio_channel_id_t{static_cast<std::size_t>(chan)});
	}

	void
	audio::register_effect (audio_channel_id_t chan, sound_effect_callback& cbk) {
		if (0 == Mix_RegisterEffect (static_cast<int>(chan.value_of()), pre_effect, post_effect, &cbk)) {
			RAISE_SDL_EX();
		}
	}

	void audio::unregister_effect (audio_channel_id_t chan) {
		Mix_UnregisterAllEffects (static_cast<int >(chan.value_of()));
	}

	bool audio::play (const object<Mix_Music>& mus, unsigned int track) {
		return -1 != Mix_StartTrack (mus.const_handle(), static_cast<int>(track));
	}

	std::vector<std::string> audio::get_sound_fonts () {
		const char* fonts = Mix_GetSoundFonts();
		if (!fonts) {
			return {};
		}

		bsw::string_tokenizer tokenizer(fonts, ";", bsw::string_tokenizer::TOK_IGNORE_EMPTY | bsw::string_tokenizer::TOK_TRIM);
		std::vector<std::string> out;
		std::copy(tokenizer.begin(), tokenizer.end(), std::back_inserter (out));
		return out;
	}

	void audio::set_sound_fonts (const std::vector<std::string>& pathes) {
		std::ostringstream oss;
		std::copy(pathes.begin(), pathes.end(), std::ostream_iterator<std::string>(oss, ";"));
		if (0 == Mix_SetSoundFonts (oss.str().c_str())) {
			RAISE_SDL_EX();
		}
	}

	void audio::clear_sound_fonts () {
		Mix_SetSoundFonts (nullptr);
	}

	void audio::set_timidity_config (const std::filesystem::path& pth) {
		if (0 == Mix_SetTimidityCfg (pth.u8string().c_str())) {
			RAISE_SDL_EX();
		}
	}

	std::optional<std::filesystem::path> audio::get_timidity_config () {
		const char* p = Mix_GetTimidityCfg();
		if (!p) {
			return std::nullopt;
		}
		return p;
	}

	static void SDLCALL mix_func(void *udata, Uint8 *stream, int len) {
		auto* f = reinterpret_cast<music_hook_function_t*>(udata);
		(*f)(stream, len);
	}

	void audio::register_music_hook (music_hook_function_t& f) {
		Mix_HookMusic (mix_func, &f);
	}

	void audio::unregister_music_hook () {
		Mix_HookMusic (nullptr, nullptr);
	}

	std::optional<music_hook_function_t> audio::get_music_hook () {
		void* f = Mix_GetMusicHookData();
		if (!f) {
			return std::nullopt;
		}
		auto* func = reinterpret_cast<music_hook_function_t *>(f);
		return *func;
	}

	void audio::register_post_mix_hook (post_mix_function_t& f) {
		Mix_SetPostMix (mix_func, &f);
	}

	void audio::unregister_post_mix_hook () {
		Mix_SetPostMix (nullptr, nullptr);
	}

	BEGIN_IMPL_OUTPUT(audio::format)
			case audio::format::U8: return "U8";
			case audio::format::S8: return "S8";
			case audio::format::U16_LSB: return "U16_LSB";
			case audio::format::S16_LSB: return "S16_LSB";
			case audio::format::U16_MSB: return "U16_MSB";
			case audio::format::S16_MSB: return "S16_MSB";
			case audio::format::S32_LSB: return "S32_LSB";
			case audio::format::S32_MSB: return "S32_MSB";
			case audio::format::F32_LSB: return "F32_LSB";
			case audio::format::F32_MSB: return "F32_MSB";
	END_IMPL_OUTPUT(audio::format)

	BEGIN_IMPL_OUTPUT(audio::fading_status)
			case audio::fading_status::NONE: return "NONE";
			case audio::fading_status::IN: return "IN";
			case audio::fading_status::OUT: return "OUT";
	END_IMPL_OUTPUT(audio::fading_status)
}