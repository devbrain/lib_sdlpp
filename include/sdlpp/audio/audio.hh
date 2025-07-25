#pragma once

/**
 * @file audio.hh
 * @brief Audio functionality wrapper for SDL3
 * 
 * This header provides C++ wrappers around SDL3's audio API, offering
 * RAII-managed audio devices and streams for playback and recording.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/type_utils.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/io/iostream.hh>

#include <cstdint>
#include <string>
#include <vector>
#include <span>
#include <memory>
#include <functional>
#include <filesystem>

namespace sdlpp {
    /**
     * @brief Audio format enumeration
     */
    enum class audio_format : Uint16 {
        unknown = SDL_AUDIO_UNKNOWN,
        u8 = SDL_AUDIO_U8, ///< Unsigned 8-bit samples
        s8 = SDL_AUDIO_S8, ///< Signed 8-bit samples
        s16le = SDL_AUDIO_S16LE, ///< Signed 16-bit samples, little-endian
        s16be = SDL_AUDIO_S16BE, ///< Signed 16-bit samples, big-endian
        s32le = SDL_AUDIO_S32LE, ///< 32-bit integer samples, little-endian
        s32be = SDL_AUDIO_S32BE, ///< 32-bit integer samples, big-endian
        f32le = SDL_AUDIO_F32LE, ///< 32-bit floating point samples, little-endian
        f32be = SDL_AUDIO_F32BE, ///< 32-bit floating point samples, big-endian

        // System byte order aliases
        s16 = SDL_AUDIO_S16, ///< Signed 16-bit samples, native byte order
        s32 = SDL_AUDIO_S32, ///< 32-bit integer samples, native byte order
        f32 = SDL_AUDIO_F32 ///< 32-bit floating point samples, native byte order
    };

    /**
     * @brief Get bit size of audio format
     * @param format Audio format
     * @return Bit size
     */
    [[nodiscard]] inline constexpr size_t audio_bit_size(audio_format format) noexcept {
        return static_cast<size_t>(SDL_AUDIO_BITSIZE(static_cast<SDL_AudioFormat>(format)));
    }

    /**
     * @brief Get byte size of audio format
     * @param format Audio format
     * @return Byte size
     */
    [[nodiscard]] inline constexpr size_t audio_byte_size(audio_format format) noexcept {
        return static_cast<size_t>(SDL_AUDIO_BYTESIZE(static_cast<SDL_AudioFormat>(format)));
    }

    /**
     * @brief Check if audio format is floating point
     * @param format Audio format
     * @return true if floating point
     */
    [[nodiscard]] inline constexpr bool audio_is_float(audio_format format) noexcept {
        return SDL_AUDIO_ISFLOAT(static_cast<SDL_AudioFormat>(format));
    }

    /**
     * @brief Check if audio format is integer
     * @param format Audio format
     * @return true if integer
     */
    [[nodiscard]] inline constexpr bool audio_is_int(audio_format format) noexcept {
        return SDL_AUDIO_ISINT(static_cast<SDL_AudioFormat>(format));
    }

    /**
     * @brief Check if audio format is big-endian
     * @param format Audio format
     * @return true if big-endian
     */
    [[nodiscard]] inline constexpr bool audio_is_big_endian(audio_format format) noexcept {
        return SDL_AUDIO_ISBIGENDIAN(static_cast<SDL_AudioFormat>(format));
    }

    /**
     * @brief Check if audio format is little-endian
     * @param format Audio format
     * @return true if little-endian
     */
    [[nodiscard]] inline constexpr bool audio_is_little_endian(audio_format format) noexcept {
        return SDL_AUDIO_ISLITTLEENDIAN(static_cast<SDL_AudioFormat>(format));
    }

    /**
     * @brief Check if audio format is signed
     * @param format Audio format
     * @return true if signed
     */
    [[nodiscard]] inline constexpr bool audio_is_signed(audio_format format) noexcept {
        return SDL_AUDIO_ISSIGNED(static_cast<SDL_AudioFormat>(format));
    }

    /**
     * @brief Check if audio format is unsigned
     * @param format Audio format
     * @return true if unsigned
     */
    [[nodiscard]] inline constexpr bool audio_is_unsigned(audio_format format) noexcept {
        return SDL_AUDIO_ISUNSIGNED(static_cast<SDL_AudioFormat>(format));
    }

    // Forward declarations
    class audio_device_id;
    class audio_device;
    class audio_stream;
    struct audio_spec;
    using audio_stream_callback = std::function<void(void*, class audio_stream_ref, size_t, size_t)>;
    
    /**
     * @brief Opaque audio device ID type
     */
    class audio_device_id {
    private:
        SDL_AudioDeviceID id_{0};
        
        // Private constructor from SDL type - only accessible by friend functions
        explicit audio_device_id(SDL_AudioDeviceID id) : id_(id) {}
        
        // Private getter for SDL type - only accessible by friend functions
        [[nodiscard]] SDL_AudioDeviceID get_sdl_id() const noexcept { return id_; }
        
        // Friend declarations for internal SDL++ functions that need access
        friend class audio_device;
        friend class audio_stream;
        friend expected <audio_stream, std::string> open_audio_device_stream(audio_device_id, const audio_spec&, const audio_stream_callback&, void*);
        friend std::vector <audio_device_id> get_audio_playback_devices();
        friend std::vector <audio_device_id> get_audio_recording_devices();
        friend inline std::string get_audio_device_name(audio_device_id);
        friend inline expected<audio_spec, std::string> get_audio_device_format(audio_device_id);
        friend inline bool is_audio_device_physical(audio_device_id);
        friend inline bool is_audio_device_playback(audio_device_id);
        
        // Friend functions for creating default devices
        friend audio_device_id default_playback_device() noexcept;
        friend audio_device_id default_recording_device() noexcept;
        
    public:
        audio_device_id() = default;
        
        [[nodiscard]] bool is_valid() const noexcept { return id_ != 0; }
        [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }
        
        friend bool operator==(audio_device_id a, audio_device_id b) noexcept {
            return a.id_ == b.id_;
        }
        friend bool operator!=(audio_device_id a, audio_device_id b) noexcept {
            return a.id_ != b.id_;
        }
    };
#if defined(SDLPP_COMPILER_MSVC)
#pragma warning( push )
#elif defined(SDLPP_COMPILER_GCC)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wold-style-cast"
# pragma GCC diagnostic ignored "-Wuseless-cast"
#elif defined(SDLPP_COMPILER_CLANG)
# pragma clang diagnostic push
#elif defined(SDLPP_COMPILER_WASM)
# pragma clang diagnostic push
#endif
    /**
     * @brief Get default playback device
     * @return Default playback device ID
     */
    [[nodiscard]] inline audio_device_id default_playback_device() noexcept {
        return audio_device_id{SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK};
    }

    /**
     * @brief Get default recording device
     * @return Default recording device ID
     */
    [[nodiscard]] inline audio_device_id default_recording_device() noexcept {
        return audio_device_id{SDL_AUDIO_DEVICE_DEFAULT_RECORDING};
    }
#if defined(SDLPP_COMPILER_MSVC)
#pragma warning( pop )
#elif defined(SDLPP_COMPILER_GCC)
# pragma GCC diagnostic pop
#elif defined(SDLPP_COMPILER_CLANG)
# pragma clang diagnostic pop
#elif defined(SDLPP_COMPILER_WASM)
# pragma clang diagnostic pop
#endif
    /**
     * @brief Audio specification
     */
    struct audio_spec {
        audio_format format{audio_format::f32}; ///< Audio data format
        int channels{2}; ///< Number of channels
        int freq{48000}; ///< Sample rate (Hz)

        /**
         * @brief Calculate frame size in bytes
         * @return Frame size
         */
        [[nodiscard]] constexpr size_t frame_size() const noexcept {
            return audio_byte_size(format) * static_cast<size_t>(channels);
        }

        /**
         * @brief Convert to SDL_AudioSpec
         * @return SDL audio spec
         */
        [[nodiscard]] SDL_AudioSpec to_sdl() const noexcept {
            return SDL_AudioSpec{
                .format = static_cast <SDL_AudioFormat>(format),
                .channels = channels,
                .freq = freq
            };
        }

        /**
         * @brief Create from SDL_AudioSpec
         * @param spec SDL audio spec
         * @return Audio spec
         */
        [[nodiscard]] static audio_spec from_sdl(const SDL_AudioSpec& spec) noexcept {
            return audio_spec{
                .format = static_cast <audio_format>(spec.format),
                .channels = spec.channels,
                .freq = spec.freq
            };
        }
    };

    // Forward declarations
    class audio_stream;
    class audio_stream_ref;

    /**
     * @brief Audio stream callback function type
     * @param userdata User data pointer
     * @param stream Audio stream reference (non-owning)
     * @param additional_amount Additional bytes needed/provided
     * @param total_amount Total bytes available
     */
    using audio_stream_callback = std::function <void(void* userdata, audio_stream_ref stream, size_t additional_amount,
                                                      size_t total_amount)>;

    /**
     * @brief Audio stream wrapper
     *
     * Central component for audio in SDL3. Handles format conversion,
     * resampling, and buffering for both playback and recording.
     */
    class audio_stream {
        public:
            audio_stream() = default;

            audio_stream(const audio_stream&) = delete;
            audio_stream& operator=(const audio_stream&) = delete;

            audio_stream(audio_stream&& other) noexcept
                : stream_(std::exchange(other.stream_, nullptr)),
                  owned_device_(std::exchange(other.owned_device_, audio_device_id{})) {
            }

            audio_stream& operator=(audio_stream&& other) noexcept {
                if (this != &other) {
                    reset();
                    stream_ = std::exchange(other.stream_, nullptr);
                    owned_device_ = std::exchange(other.owned_device_, audio_device_id{});
                }
                return *this;
            }

            ~audio_stream() {
                reset();
            }

            /**
             * @brief Create an audio stream
             * @param src_spec Input format specification
             * @param dst_spec Output format specification
             * @return Audio stream or error
             */
            [[nodiscard]] static sdlpp::expected <audio_stream, std::string> create(
                const audio_spec& src_spec,
                const audio_spec& dst_spec) {
                auto src_sdl = src_spec.to_sdl();
                auto dst_sdl = dst_spec.to_sdl();

                SDL_AudioStream* stream = SDL_CreateAudioStream(&src_sdl, &dst_sdl);
                if (!stream) {
                    return sdlpp::unexpected(get_error());
                }

                return audio_stream(stream);
            }

            /**
             * @brief Put audio data into the stream
             * @param data Audio data
             * @param len Data length in bytes
             * @return Success or error
             */
            [[nodiscard]] expected <void, std::string> put_data(const void* data, size_t len) {
                if (!stream_) {
                    return unexpected("Invalid audio stream");
                }

                auto int_len = detail::size_to_int(len);
                if (!int_len) {
                    return unexpected("Data size too large: " + int_len.error());
                }

                if (!SDL_PutAudioStreamData(stream_, data, *int_len)) {
                    return unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Put audio data into the stream
             * @tparam T Data type
             * @param data Audio data span
             * @return Success or error
             */
            template<typename T>
            [[nodiscard]] expected <void, std::string> put_data(std::span <const T> data) {
                return put_data(data.data(), data.size_bytes());
            }

            /**
             * @brief Get converted audio data from the stream
             * @param data Buffer to fill
             * @param len Maximum bytes to get
             * @return Number of bytes read or error
             */
            [[nodiscard]] sdlpp::expected <size_t, std::string> get_data(void* data, size_t len) {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                auto int_len = detail::size_to_int(len);
                if (!int_len) {
                    return sdlpp::unexpected("Buffer size too large: " + int_len.error());
                }

                int bytes_read = SDL_GetAudioStreamData(stream_, data, *int_len);
                if (bytes_read < 0) {
                    return sdlpp::unexpected(get_error());
                }

                return static_cast<size_t>(bytes_read);
            }

            /**
             * @brief Get converted audio data from the stream
             * @tparam T Data type
             * @param data Buffer span to fill
             * @return Number of elements read or error
             */
            template<typename T>
            [[nodiscard]] sdlpp::expected <size_t, std::string> get_data(std::span <T> data) {
                auto bytes_result = get_data(data.data(), data.size_bytes());
                if (!bytes_result) {
                    return sdlpp::unexpected(bytes_result.error());
                }
                return *bytes_result / sizeof(T);
            }

            /**
             * @brief Get available data amount
             * @return Bytes available
             */
            [[nodiscard]] expected<size_t, std::string> get_available() const {
                if (!stream_) return make_unexpected("Invalid stream");
                int available = SDL_GetAudioStreamAvailable(stream_);
                if (available < 0) {
                    return make_unexpected(get_error());
                }
                return static_cast<size_t>(available);
            }

            /**
             * @brief Get queued data amount
             * @return Bytes queued
             */
            [[nodiscard]] expected<size_t, std::string> get_queued() const {
                if (!stream_) return make_unexpected("Invalid stream");
                int queued = SDL_GetAudioStreamQueued(stream_);
                if (queued < 0) {
                    return make_unexpected(get_error());
                }
                return static_cast<size_t>(queued);
            }

            /**
             * @brief Flush the stream
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> flush() {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_FlushAudioStream(stream_)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Clear the stream
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> clear() {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_ClearAudioStream(stream_)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Lock the stream
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> lock() {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_LockAudioStream(stream_)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Unlock the stream
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> unlock() {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_UnlockAudioStream(stream_)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get input format
             * @return Format or error
             */
            [[nodiscard]] sdlpp::expected <audio_spec, std::string> get_input_format() const {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                SDL_AudioSpec spec;
                if (!SDL_GetAudioStreamFormat(stream_, &spec, nullptr)) {
                    return sdlpp::unexpected(get_error());
                }

                return audio_spec::from_sdl(spec);
            }

            /**
             * @brief Get output format
             * @return Format or error
             */
            [[nodiscard]] sdlpp::expected <audio_spec, std::string> get_output_format() const {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                SDL_AudioSpec spec;
                if (!SDL_GetAudioStreamFormat(stream_, nullptr, &spec)) {
                    return sdlpp::unexpected(get_error());
                }

                return audio_spec::from_sdl(spec);
            }

            /**
             * @brief Set stream format
             * @param src_spec New input format (optional)
             * @param dst_spec New output format (optional)
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> set_format(
                const audio_spec* src_spec = nullptr,
                const audio_spec* dst_spec = nullptr) {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                SDL_AudioSpec src_sdl, dst_sdl;
                const SDL_AudioSpec* src_ptr = nullptr;
                const SDL_AudioSpec* dst_ptr = nullptr;

                if (src_spec) {
                    src_sdl = src_spec->to_sdl();
                    src_ptr = &src_sdl;
                }
                if (dst_spec) {
                    dst_sdl = dst_spec->to_sdl();
                    dst_ptr = &dst_sdl;
                }

                if (!SDL_SetAudioStreamFormat(stream_, src_ptr, dst_ptr)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get frequency ratio
             * @return Ratio
             */
            [[nodiscard]] float get_frequency_ratio() const {
                if (!stream_) return 1.0f;
                return SDL_GetAudioStreamFrequencyRatio(stream_);
            }

            /**
             * @brief Set frequency ratio
             * @param ratio Frequency ratio (0.01 to 100)
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> set_frequency_ratio(float ratio) {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_SetAudioStreamFrequencyRatio(stream_, ratio)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get gain
             * @return Gain value
             */
            [[nodiscard]] float get_gain() const {
                if (!stream_) return 1.0f;
                return SDL_GetAudioStreamGain(stream_);
            }

            /**
             * @brief Set gain
             * @param gain Gain value (0.0 = silence, 1.0 = no change)
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> set_gain(float gain) {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_SetAudioStreamGain(stream_, gain)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get bound device
             * @return Device ID or 0 if not bound
             */
            [[nodiscard]] audio_device_id get_device() const {
                if (!stream_) return audio_device_id{};
                return audio_device_id{SDL_GetAudioStreamDevice(stream_)};
            }

            /**
             * @brief Set input channel map
             * @param channel_map Channel mapping array
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> set_input_channel_map(
                std::span <const int> channel_map) {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_SetAudioStreamInputChannelMap(stream_, channel_map.data(),
                                                       static_cast <int>(channel_map.size()))) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Set output channel map
             * @param channel_map Channel mapping array
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> set_output_channel_map(
                std::span <const int> channel_map) {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_SetAudioStreamOutputChannelMap(stream_, channel_map.data(),
                                                        static_cast <int>(channel_map.size()))) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return stream_ != nullptr;
            }

            [[nodiscard]] SDL_AudioStream* get() const noexcept {
                return stream_;
            }

            /**
             * @brief Pause associated device (for streams from open_device_stream)
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> pause_device() {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_PauseAudioStreamDevice(stream_)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Resume associated device (for streams from open_device_stream)
             * @return Success or error
             */
            [[nodiscard]] sdlpp::expected <void, std::string> resume_device() {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_ResumeAudioStreamDevice(stream_)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Check if associated device is paused
             * @return true if paused
             */
            [[nodiscard]] bool is_device_paused() const {
                if (!stream_) return false;
                return SDL_AudioStreamDevicePaused(stream_);
            }

            void reset() {
                if (stream_) {
                    SDL_DestroyAudioStream(stream_);
                    stream_ = nullptr;
                }
                owned_device_ = audio_device_id{};
            }

        private:
            friend class audio_device;

            explicit audio_stream(SDL_AudioStream* stream) noexcept
                : stream_(stream) {
            }

            audio_stream(SDL_AudioStream* stream, audio_device_id device) noexcept
                : stream_(stream), owned_device_(device) {
            }

            // Allow open_audio_device_stream to create streams and access internals
            friend sdlpp::expected <audio_stream, std::string> open_audio_device_stream(
                audio_device_id device_id,
                const audio_spec& spec,
                const audio_stream_callback& callback,
                void* userdata);

            SDL_AudioStream* stream_{nullptr};
            audio_device_id owned_device_{0}; // For streams that own their device
    };

    /**
     * @brief Non-owning reference to an audio stream
     *
     * This class provides a safe way to reference an audio stream in callbacks
     * without taking ownership. It has the same interface as audio_stream but
     * doesn't manage the lifetime of the underlying SDL_AudioStream.
     */
    class audio_stream_ref {
        public:
            explicit audio_stream_ref(SDL_AudioStream* stream) noexcept
                : stream_(stream) {
            }

            audio_stream_ref(const audio_stream_ref&) = default;
            audio_stream_ref& operator=(const audio_stream_ref&) = default;

            // Provide the same interface as audio_stream
            [[nodiscard]] sdlpp::expected <void, std::string> put_data(const void* data, size_t len) {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                auto int_len = detail::size_to_int(len);
                if (!int_len) {
                    return sdlpp::unexpected("Data size too large: " + int_len.error());
                }

                if (!SDL_PutAudioStreamData(stream_, data, *int_len)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            template<typename T>
            [[nodiscard]] sdlpp::expected <void, std::string> put_data(std::span <const T> data) {
                return put_data(data.data(), data.size_bytes());
            }

            [[nodiscard]] sdlpp::expected <size_t, std::string> get_data(void* data, size_t len) {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                auto int_len = detail::size_to_int(len);
                if (!int_len) {
                    return sdlpp::unexpected("Buffer size too large: " + int_len.error());
                }

                int bytes_read = SDL_GetAudioStreamData(stream_, data, *int_len);
                if (bytes_read < 0) {
                    return sdlpp::unexpected(get_error());
                }

                return static_cast<size_t>(bytes_read);
            }

            template<typename T>
            [[nodiscard]] sdlpp::expected <size_t, std::string> get_data(std::span <T> data) {
                auto bytes_result = get_data(data.data(), data.size_bytes());
                if (!bytes_result) {
                    return sdlpp::unexpected(bytes_result.error());
                }
                return *bytes_result / sizeof(T);
            }

            [[nodiscard]] expected<size_t, std::string> get_available() const {
                if (!stream_) return make_unexpected("Invalid stream");
                int available = SDL_GetAudioStreamAvailable(stream_);
                if (available < 0) {
                    return make_unexpected(get_error());
                }
                return static_cast<size_t>(available);
            }

            [[nodiscard]] expected<size_t, std::string> get_queued() const {
                if (!stream_) return make_unexpected("Invalid stream");
                int queued = SDL_GetAudioStreamQueued(stream_);
                if (queued < 0) {
                    return make_unexpected(get_error());
                }
                return static_cast<size_t>(queued);
            }

            [[nodiscard]] sdlpp::expected <void, std::string> flush() {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_FlushAudioStream(stream_)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            [[nodiscard]] sdlpp::expected <void, std::string> clear() {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_ClearAudioStream(stream_)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            [[nodiscard]] float get_gain() const {
                if (!stream_) return 1.0f;
                return SDL_GetAudioStreamGain(stream_);
            }

            [[nodiscard]] sdlpp::expected <void, std::string> set_gain(float gain) {
                if (!stream_) {
                    return sdlpp::unexpected("Invalid audio stream");
                }

                if (!SDL_SetAudioStreamGain(stream_, gain)) {
                    return sdlpp::unexpected(get_error());
                }

                return {};
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return stream_ != nullptr;
            }

            [[nodiscard]] SDL_AudioStream* get() const noexcept {
                return stream_;
            }

        private:
            SDL_AudioStream* stream_;
    };

    /**
     * @brief Audio device wrapper
     */
    class audio_device {
        public:
            audio_device() = default;

            audio_device(const audio_device&) = delete;
            audio_device& operator=(const audio_device&) = delete;

            audio_device(audio_device&& other) noexcept
                : device_id_(std::exchange(other.device_id_, audio_device_id{})) {
            }

            audio_device& operator=(audio_device&& other) noexcept {
                if (this != &other) {
                    reset();
                    device_id_ = std::exchange(other.device_id_, audio_device_id{});
                }
                return *this;
            }

            ~audio_device() {
                reset();
            }

            /**
             * @brief Open an audio device
             * @param device_id Physical device ID or default constant
             * @param spec Desired format specification (optional)
             * @return Audio device or error
             */
            [[nodiscard]] static sdlpp::expected <audio_device, std::string> open(
                audio_device_id device_id,
                const audio_spec* spec = nullptr) {
                SDL_AudioSpec sdl_spec;
                const SDL_AudioSpec* spec_ptr = nullptr;

                if (spec) {
                    sdl_spec = spec->to_sdl();
                    spec_ptr = &sdl_spec;
                }

                SDL_AudioDeviceID id = SDL_OpenAudioDevice(device_id.get_sdl_id(), spec_ptr);
                if (id == 0) {
                    return sdlpp::unexpected(get_error());
                }

                return audio_device(audio_device_id{id});
            }

            /**
             * @brief Get device format
             * @return Format specification or error
             */
            [[nodiscard]] sdlpp::expected <audio_spec, std::string> get_format() const {
                if (!device_id_) {
                    return sdlpp::unexpected("Invalid audio device");
                }

                SDL_AudioSpec spec;
                int sample_frames;
                if (!SDL_GetAudioDeviceFormat(device_id_.get_sdl_id(), &spec, &sample_frames)) {
                    return sdlpp::unexpected(get_error());
                }

                return audio_spec::from_sdl(spec);
            }

            /**
             * @brief Pause the device
             * @return Success or error
             */
            [[nodiscard]] expected <void, std::string> pause() {
                if (!device_id_) {
                    return unexpected("Invalid audio device");
                }

                if (!SDL_PauseAudioDevice(device_id_.get_sdl_id())) {
                    return unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Resume the device
             * @return Success or error
             */
            [[nodiscard]] expected <void, std::string> resume() {
                if (!device_id_) {
                    return unexpected("Invalid audio device");
                }

                if (!SDL_ResumeAudioDevice(device_id_.get_sdl_id())) {
                    return unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Check if device is paused
             * @return true if paused
             */
            [[nodiscard]] bool is_paused() const {
                if (!device_id_) return false;
                return SDL_AudioDevicePaused(device_id_.get_sdl_id());
            }

            /**
             * @brief Get device gain
             * @return Gain value
             */
            [[nodiscard]] float get_gain() const {
                if (!device_id_) return 1.0f;
                return SDL_GetAudioDeviceGain(device_id_.get_sdl_id());
            }

            /**
             * @brief Set device gain
             * @param gain Gain value
             * @return Success or error
             */
            [[nodiscard]] expected <void, std::string> set_gain(float gain) {
                if (!device_id_) {
                    return unexpected("Invalid audio device");
                }

                if (!SDL_SetAudioDeviceGain(device_id_.get_sdl_id(), gain)) {
                    return unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Bind streams to device
             * @param streams Array of streams to bind
             * @return Success or error
             */
            [[nodiscard]] expected <void, std::string> bind_streams(
                std::span <audio_stream*> streams) {
                if (!device_id_) {
                    return unexpected("Invalid audio device");
                }

                std::vector <SDL_AudioStream*> sdl_streams;
                sdl_streams.reserve(streams.size());

                for (auto* stream : streams) {
                    if (stream && stream->is_valid()) {
                        sdl_streams.push_back(stream->get());
                    }
                }

                auto stream_count = detail::size_to_int(sdl_streams.size());
                if (!stream_count) {
                    return unexpected("Too many streams to bind: " + stream_count.error());
                }

                if (!SDL_BindAudioStreams(device_id_.get_sdl_id(), sdl_streams.data(), *stream_count)) {
                    return unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Bind single stream to device
             * @param stream Stream to bind
             * @return Success or error
             */
            [[nodiscard]] expected <void, std::string> bind_stream(audio_stream& stream) {
                if (!device_id_) {
                    return unexpected("Invalid audio device");
                }

                if (!SDL_BindAudioStream(device_id_.get_sdl_id(), stream.get())) {
                    return unexpected(get_error());
                }

                return {};
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return device_id_.is_valid();
            }

            [[nodiscard]] audio_device_id get_id() const noexcept {
                return device_id_;
            }

            void reset() {
                if (device_id_) {
                    SDL_CloseAudioDevice(device_id_.get_sdl_id());
                    device_id_ = audio_device_id{};
                }
            }

        private:
            explicit audio_device(audio_device_id id) noexcept
                : device_id_(id) {
            }

            audio_device_id device_id_{};
    };

    /**
     * @brief Get number of audio drivers
     * @return Driver count
     */
    [[nodiscard]] inline size_t get_num_audio_drivers() {
        int count = SDL_GetNumAudioDrivers();
        return count >= 0 ? static_cast<size_t>(count) : 0;
    }

    /**
     * @brief Get audio driver name
     * @param index Driver index
     * @return Driver name or empty string if invalid
     */
    [[nodiscard]] inline std::string get_audio_driver(size_t index) {
        auto int_index = detail::size_to_int(index);
        if (!int_index) {
            return {};
        }
        const char* name = SDL_GetAudioDriver(*int_index);
        return name ? name : "";
    }

    /**
     * @brief Get current audio driver name
     * @return Driver name or empty string if none
     */
    [[nodiscard]] inline std::string get_current_audio_driver() {
        const char* name = SDL_GetCurrentAudioDriver();
        return name ? name : "";
    }

    /**
     * @brief Get playback devices
     * @return Vector of device IDs
     */
    [[nodiscard]] inline std::vector <audio_device_id> get_audio_playback_devices() {
        int count = 0;
        SDL_AudioDeviceID* devices = SDL_GetAudioPlaybackDevices(&count);
        if (!devices || count < 0) {
            return {};
        }

        std::vector <audio_device_id> device_list;
        device_list.reserve(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            device_list.push_back(audio_device_id{devices[i]});
        }
        SDL_free(devices);
        return device_list;
    }

    /**
     * @brief Get recording devices
     * @return Vector of device IDs
     */
    [[nodiscard]] inline std::vector <audio_device_id> get_audio_recording_devices() {
        int count = 0;
        SDL_AudioDeviceID* devices = SDL_GetAudioRecordingDevices(&count);
        if (!devices) {
            return {};
        }

        std::vector <audio_device_id> device_list;
        device_list.reserve(static_cast<size_t>(count));
        for (int i = 0; i < count; ++i) {
            device_list.push_back(audio_device_id{devices[i]});
        }
        SDL_free(devices);
        return device_list;
    }

    /**
     * @brief Get device name
     * @param device_id Device ID
     * @return Device name or empty string
     */
    [[nodiscard]] inline std::string get_audio_device_name(audio_device_id device_id) {
        const char* name = SDL_GetAudioDeviceName(device_id.get_sdl_id());
        return name ? name : "";
    }

    /**
     * @brief Get device format
     * @param device_id Device ID
     * @return Format specification or error
     */
    [[nodiscard]] inline expected <audio_spec, std::string> get_audio_device_format(
        audio_device_id device_id) {
        SDL_AudioSpec spec;
        int sample_frames;
        if (!SDL_GetAudioDeviceFormat(device_id.get_sdl_id(), &spec, &sample_frames)) {
            return unexpected(get_error());
        }

        return audio_spec::from_sdl(spec);
    }

    /**
     * @brief Check if device is physical
     * @param device_id Device ID
     * @return true if physical device
     */
    [[nodiscard]] inline bool is_audio_device_physical(audio_device_id device_id) {
        return SDL_IsAudioDevicePhysical(device_id.get_sdl_id());
    }

    /**
     * @brief Check if device is playback
     * @param device_id Device ID
     * @return true if playback device
     */
    [[nodiscard]] inline bool is_audio_device_playback(audio_device_id device_id) {
        return SDL_IsAudioDevicePlayback(device_id.get_sdl_id());
    }

    /**
     * @brief Unbind audio streams
     * @param streams Streams to unbind
     */
    inline void unbind_audio_streams(std::span <audio_stream*> streams) {
        std::vector <SDL_AudioStream*> sdl_streams;
        sdl_streams.reserve(streams.size());

        for (auto* stream : streams) {
            if (stream && stream->is_valid()) {
                sdl_streams.push_back(stream->get());
            }
        }

        if (!sdl_streams.empty()) {
            auto stream_count = detail::size_to_int(sdl_streams.size());
            if (stream_count) {
                SDL_UnbindAudioStreams(sdl_streams.data(), *stream_count);
            }
        }
    }

    /**
     * @brief Unbind single audio stream
     * @param stream Stream to unbind
     */
    inline void unbind_audio_stream(audio_stream& stream) {
        SDL_UnbindAudioStream(stream.get());
    }

    /**
     * @brief Open audio device stream (simplified interface)
     * @param device_id Device ID or default constant
     * @param spec Audio format specification
     * @param callback Optional callback for data
     * @param userdata User data for callback
     * @return Audio stream or error
     */
    [[nodiscard]] inline expected <audio_stream, std::string> open_audio_device_stream(
        audio_device_id device_id,
        const audio_spec& spec,
        const audio_stream_callback& callback = {},
        void* userdata = nullptr) {
        SDL_AudioSpec sdl_spec = spec.to_sdl();

        // Callback wrapper that creates audio_stream_ref
        struct callback_data {
            audio_stream_callback callback;
            void* userdata;

            static void sdl_callback(void* user, SDL_AudioStream* stream, int additional, int total) {
                auto* data = static_cast <callback_data*>(user);
                audio_stream_ref stream_ref(stream);
                // Convert negative values to 0 for safety
                size_t additional_bytes = additional > 0 ? static_cast<size_t>(additional) : 0;
                size_t total_bytes = total > 0 ? static_cast<size_t>(total) : 0;
                data->callback(data->userdata, stream_ref, additional_bytes, total_bytes);
            }
        };

        SDL_AudioStreamCallback sdl_callback = nullptr;
        void* sdl_userdata = nullptr;
        std::unique_ptr <callback_data> cb_data;

        if (callback) {
            cb_data = std::make_unique <callback_data>();
            cb_data->callback = callback;
            cb_data->userdata = userdata;
            sdl_callback = &callback_data::sdl_callback;
            sdl_userdata = cb_data.get();
        }

        SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(device_id.get_sdl_id(), &sdl_spec, sdl_callback, sdl_userdata);
        if (!stream) {
            return unexpected(get_error());
        }

        // Store callback data in properties to keep it alive
        if (cb_data) {
            SDL_PropertiesID props = SDL_GetAudioStreamProperties(stream);
            if (props) {
                SDL_SetPointerPropertyWithCleanup(props, "sdlpp.callback_data", cb_data.release(),
                                                  [](void* /*userdata*/, void* value) {
                                                      delete static_cast <callback_data*>(value);
                                                  }, nullptr);
            }
        }

        return audio_stream(stream, device_id);
    }

    /**
     * @brief WAV file data
     */
    struct wav_data {
        std::vector <uint8_t> buffer; ///< Audio data
        audio_spec spec; ///< Format specification
    };

    /**
     * @brief Load WAV file
     * @param path File path
     * @return WAV data or error
     */
    [[nodiscard]] inline expected <wav_data, std::string> load_wav(std::string_view path) {
        SDL_AudioSpec spec;
        Uint8* audio_buf = nullptr;
        Uint32 audio_len = 0;

        // SDL requires null-terminated string
        std::string path_str(path);
        if (!SDL_LoadWAV(path_str.c_str(), &spec, &audio_buf, &audio_len)) {
            return unexpected(get_error());
        }

        wav_data wav;
        wav.spec = audio_spec::from_sdl(spec);
        wav.buffer.assign(audio_buf, audio_buf + audio_len);

        SDL_free(audio_buf);

        return wav;
    }

    /**
     * @brief Load WAV file from std::filesystem::path
     * @param path File path
     * @return WAV data or error
     */
    [[nodiscard]] inline expected <wav_data, std::string> load_wav(const std::filesystem::path& path) {
        return load_wav(std::string_view(path.string()));
    }


    /**
     * @brief Load WAV file from IOStream
     * @param stream IOStream to read from     * @return WAV data or error
     */
    [[nodiscard]] inline expected <wav_data, std::string> load_wav(iostream& stream, bool close_io = false) {
        if (!stream.is_valid()) {
            return unexpected("Invalid IOStream");
        }

        SDL_AudioSpec spec;
        Uint8* audio_buf = nullptr;
        Uint32 audio_len = 0;

        if (!SDL_LoadWAV_IO(stream.get(), close_io, &spec, &audio_buf, &audio_len)) {
            return unexpected(get_error());
        }

        wav_data wav;
        wav.spec = audio_spec::from_sdl(spec);
        wav.buffer.assign(audio_buf, audio_buf + audio_len);

        SDL_free(audio_buf);

        return wav;
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for audio_format
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, audio_format value);

/**
 * @brief Stream input operator for audio_format
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, audio_format& value);

}