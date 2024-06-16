//
// Created by igor on 6/14/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_AUDIO_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_AUDIO_HH_

#include <set>
#include <string>
#include <optional>
#include <chrono>
#include <filesystem>
#include <functional>

#include <strong_type/strong_type.hpp>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/clamp.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	using audio_channel_id_t = strong::type<std::size_t, struct _channel_index_t_, strong::bicrementable, strong::ordered, strong::ostreamable>;
	using audio_group_id_t = strong::type<unsigned, struct _channel_index_t_, strong::bicrementable, strong::ordered, strong::ostreamable>;

	using music_hook_function_t = std::function<void(uint8_t* stream, std::size_t len)>;
	using post_mix_function_t = std::function<void(uint8_t* stream, std::size_t len)>;

	class SDLPP_EXPORT sound_effect_callback {
	 public:
		virtual ~sound_effect_callback() =default;
		virtual void before_sound (audio_channel_id_t chan, void* stream, std::size_t len) = 0;
		virtual void after_sound (audio_channel_id_t chan) = 0;
	};

	class SDLPP_EXPORT audio {
	 public:
		enum class format : int {
			U8 = AUDIO_U8,
			S8 = AUDIO_S8,
 			U16_LSB = AUDIO_U16LSB,
			S16_LSB = AUDIO_S16LSB,
			U16_MSB = AUDIO_U16MSB,
			S16_MSB = AUDIO_S16MSB,
			S32_LSB = AUDIO_S32LSB,
			S32_MSB = AUDIO_S32MSB,
			F32_LSB = AUDIO_F32LSB,
			F32_MSB = AUDIO_F32MSB
		};

		enum class fading_status : int {
			NONE = MIX_NO_FADING,
			OUT = MIX_FADING_OUT,
			IN = MIX_FADING_IN
		};

		static constexpr unsigned MAX_VOLUME = MIX_MAX_VOLUME;

		static void open(unsigned channels, std::size_t chunk_size = 2048);
		static void open(unsigned freq, format f, unsigned channels, std::size_t chunk_size);
		static void close();

		static std::set<std::string > get_music_decoders();
		static std::set<std::string > get_sound_effect_decoders();

		static unsigned get_volume();
		static void set_volume(unsigned v);
		static audio_channel_id_t get_channels_count();
		static void allocate_channels(std::size_t num);

		static void group_channels(audio_channel_id_t from, audio_channel_id_t to, audio_group_id_t tag);
		static void group_channels(audio_channel_id_t chan, audio_group_id_t tag);
		static void ungroup_channels(audio_channel_id_t from, audio_channel_id_t to);
		static audio_channel_id_t get_channels_count(audio_channel_id_t tag);

		static std::optional<audio_channel_id_t> find_most_recent_playing_channel(audio_channel_id_t tag);
		static std::optional<audio_channel_id_t> find_oldest_playing_channel(audio_channel_id_t tag);

		static void halt(audio_channel_id_t chan);
		static void halt (audio_group_id_t grp);
		static void halt();
		static void halt_music();

		static void pause(audio_channel_id_t chan);
		static void pause();
		static bool is_paused(audio_channel_id_t chan);
		static std::size_t paused_channels_count();
		static bool is_music_paused();
		static void pause_music();

		static bool is_music_playing();
		static bool is_playing(audio_channel_id_t chan);
		static std::size_t playing_channels_count();


		static void resume(audio_channel_id_t chan);
		static void resume();
		static void resume_music();

		static void set_expiration(audio_channel_id_t chan, std::chrono::milliseconds ticks);
		static void unset_expiration(audio_channel_id_t chan);

		static void play(audio_channel_id_t chan, unsigned loops, const object<Mix_Chunk>& effect);
		static void play(audio_channel_id_t chan, const object<Mix_Chunk>& effect);
		static void play_inf(audio_channel_id_t chan, const object<Mix_Chunk>& effect);
		static bool play_on_first_available_channel(unsigned loops, const object<Mix_Chunk>& effect);
		static bool play_on_first_available_channel(const object<Mix_Chunk>& effect);
		static bool play_on_first_available_channel_inf(const object<Mix_Chunk>& effect);

		static void play(audio_channel_id_t chan, unsigned loops, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static void play(audio_channel_id_t chan, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static void play_inf(audio_channel_id_t chan, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static bool play_on_first_available_channel(unsigned loops, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static bool play_on_first_available_channel(std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static bool play_on_first_available_channel_inf(std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);

		static void fade_in(audio_channel_id_t chan, unsigned loops, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static void fade_in(audio_channel_id_t chan, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static void fade_in_inf(audio_channel_id_t chan, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static bool fade_in_on_first_available_channel(unsigned loops, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static bool fade_in_on_first_available_channel(std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static bool fade_in_on_first_available_channel_inf(std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);

		static void fade_in(audio_channel_id_t chan, unsigned loops, std::chrono::milliseconds effect_duration, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static void fade_in(audio_channel_id_t chan, std::chrono::milliseconds effect_duration,std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static void fade_in_inf(audio_channel_id_t chan, std::chrono::milliseconds effect_duration,std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static bool fade_in_on_first_available_channel(unsigned loops, std::chrono::milliseconds effect_duration, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static bool fade_in_on_first_available_channel(std::chrono::milliseconds effect_duration, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);
		static bool fade_in_on_first_available_channel_inf(std::chrono::milliseconds effect_duration, std::chrono::milliseconds ticks, const object<Mix_Chunk>& effect);

		static void fade_in(unsigned loops, std::chrono::milliseconds ticks, const object<Mix_Music>& mus);
		static void fade_in(std::chrono::milliseconds ticks, const object<Mix_Music>& mus);
		static void fade_in_inf(std::chrono::milliseconds ticks, const object<Mix_Music>& mus);

		static void fade_in(unsigned loops, std::chrono::milliseconds ticks, std::chrono::milliseconds pos, const object<Mix_Music>& mus);
		static void fade_in(unsigned loops, std::chrono::milliseconds ticks, std::chrono::seconds pos, const object<Mix_Music>& mus);
		static void fade_in(std::chrono::milliseconds ticks, std::chrono::milliseconds pos, const object<Mix_Music>& mus);
		static void fade_in(std::chrono::milliseconds ticks, std::chrono::seconds pos, const object<Mix_Music>& mus);
		static void fade_in_inf(std::chrono::milliseconds ticks, std::chrono::milliseconds pos, const object<Mix_Music>& mus);
		static void fade_in_inf(std::chrono::milliseconds ticks, std::chrono::seconds pos, const object<Mix_Music>& mus);

		static void fade_out(audio_channel_id_t chan, std::chrono::milliseconds ticks);
		static void fade_out(audio_group_id_t tag, std::chrono::milliseconds ticks);
		static void fade_out_music(std::chrono::milliseconds ticks);

		static fading_status get_fading_status(audio_channel_id_t chan);
		static fading_status get_fading_status_music();
		static object<Mix_Chunk> get_sound_effect(audio_channel_id_t chan);

		static void play(unsigned loops, const object<Mix_Music>& mus);
		static void play(const object<Mix_Music>& mus);
		static bool play(const object<Mix_Music>& mus, unsigned track);

		static void rewind_music();
		static void rewind_music(std::chrono::milliseconds pos);
		static void rewind_music(std::chrono::seconds pos);

		static void set_distance(audio_channel_id_t chan, uint8_t distance);
		static void set_panning(audio_channel_id_t chan, uint8_t left, uint8_t right);
		static void set_position(audio_channel_id_t chan, int16_t angle, uint8_t distance);
		static void set_reverse_stereo(audio_channel_id_t chan, bool enable);

		static void register_effect(audio_channel_id_t chan, sound_effect_callback& cbk);
		static void unregister_effect(audio_channel_id_t chan);
		static void register_music_hook(music_hook_function_t& f);
		static void unregister_music_hook();
		static std::optional<music_hook_function_t> get_music_hook();
		static void register_post_mix_hook(post_mix_function_t& f);
		static void unregister_post_mix_hook();

		static std::vector<std::string> get_sound_fonts();
		static void set_sound_fonts(const std::vector<std::string>& pathes);
		static void clear_sound_fonts();

		static void set_timidity_config(const std::filesystem::path& pth);
		static std::optional<std::filesystem::path> get_timidity_config();
	};

	d_SDLPP_OSTREAM(audio::format);
	d_SDLPP_OSTREAM(audio::fading_status);
}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_AUDIO_HH_
