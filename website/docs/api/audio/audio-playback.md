---
title: Audio Playback
sidebar_label: Audio Playback
---

# Audio Playback

SDL++ provides a modern C++ interface for audio playback, supporting both simple sound effects and advanced audio streaming.

## Audio Device Management

### Opening Audio Devices

```cpp
#include <sdlpp/audio/audio.hh>

// List available audio devices
auto playback_devices = sdlpp::audio::get_playback_devices();
for (const auto& device : playback_devices) {
    std::cout << "Device: " << device.name 
              << " (ID: " << device.id << ")" << std::endl;
}

// Open default audio device
auto device_result = sdlpp::audio_device::open_playback(
    sdlpp::audio_spec{
        .freq = 48000,
        .format = sdlpp::audio_format::f32,
        .channels = 2
    }
);

if (!device_result) {
    std::cerr << "Failed to open audio device: " 
              << device_result.error() << std::endl;
    return;
}

auto& device = device_result.value();
```

### Audio Specifications

```cpp
// Audio format options
enum class audio_format : SDL_AudioFormat {
    u8     = AUDIO_U8,     // Unsigned 8-bit
    s8     = AUDIO_S8,     // Signed 8-bit
    s16lsb = AUDIO_S16LSB, // Signed 16-bit little-endian
    s16msb = AUDIO_S16MSB, // Signed 16-bit big-endian
    s16    = AUDIO_S16,    // Native 16-bit
    s32lsb = AUDIO_S32LSB, // Signed 32-bit little-endian
    s32msb = AUDIO_S32MSB, // Signed 32-bit big-endian
    s32    = AUDIO_S32,    // Native 32-bit
    f32lsb = AUDIO_F32LSB, // 32-bit float little-endian
    f32msb = AUDIO_F32MSB, // 32-bit float big-endian
    f32    = AUDIO_F32     // Native float (recommended)
};

// Audio specification
struct audio_spec {
    int freq;              // Sample rate (e.g., 48000)
    audio_format format;   // Sample format
    Uint8 channels;        // Number of channels (1=mono, 2=stereo)
    Uint16 samples = 4096; // Buffer size in samples
};
```

## Audio Streams

SDL++ uses audio streams for flexible audio playback:

```cpp
// Create an audio stream
auto stream_result = sdlpp::audio_stream::create(
    sdlpp::audio_spec{
        .freq = 44100,
        .format = sdlpp::audio_format::s16,
        .channels = 2
    },
    device.get_spec()  // Convert to device format
);

if (!stream_result) {
    std::cerr << "Failed to create stream: " 
              << stream_result.error() << std::endl;
    return;
}

auto& stream = stream_result.value();

// Queue audio data
std::vector<int16_t> audio_data = load_wav_file("sound.wav");
auto result = stream.put(audio_data.data(), 
                        audio_data.size() * sizeof(int16_t));

// Bind stream to device
device.bind_streams(stream);

// Start playback
device.resume();
```

## Loading Audio Files

### WAV Files

```cpp
// Load WAV file
auto wav_result = sdlpp::load_wav("sound.wav");
if (!wav_result) {
    std::cerr << "Failed to load WAV: " << wav_result.error() << std::endl;
    return;
}

auto& [spec, data] = wav_result.value();

// Create stream from WAV
auto stream = sdlpp::audio_stream::create(spec, device.get_spec());
if (stream) {
    stream->put(data.data(), data.size());
}
```

### Custom Audio Loading

```cpp
class audio_loader {
public:
    struct audio_data {
        sdlpp::audio_spec spec;
        std::vector<float> samples;
    };
    
    // Load and convert to float
    static tl::expected<audio_data, std::string> 
    load_audio(const std::string& path) {
        auto wav_result = sdlpp::load_wav(path);
        if (!wav_result) {
            return tl::unexpected(wav_result.error());
        }
        
        auto& [spec, data] = wav_result.value();
        audio_data result;
        result.spec = spec;
        result.spec.format = sdlpp::audio_format::f32;
        
        // Convert to float
        result.samples = convert_to_float(spec.format, data);
        
        return result;
    }
};
```

## Sound Playback Patterns

### Simple Sound Effect Player

```cpp
class sound_effect_player {
    sdlpp::audio_device device;
    std::unordered_map<std::string, sdlpp::audio_stream> sounds;
    
public:
    bool init() {
        auto device_result = sdlpp::audio_device::open_playback(
            sdlpp::audio_spec{
                .freq = 48000,
                .format = sdlpp::audio_format::f32,
                .channels = 2
            }
        );
        
        if (!device_result) return false;
        device = std::move(device_result.value());
        device.resume();
        return true;
    }
    
    bool load_sound(const std::string& name, const std::string& path) {
        auto wav_result = sdlpp::load_wav(path);
        if (!wav_result) return false;
        
        auto& [spec, data] = wav_result.value();
        auto stream_result = sdlpp::audio_stream::create(
            spec, device.get_spec()
        );
        
        if (!stream_result) return false;
        
        auto& stream = stream_result.value();
        stream.put(data.data(), data.size());
        
        sounds[name] = std::move(stream);
        return true;
    }
    
    void play_sound(const std::string& name) {
        if (auto it = sounds.find(name); it != sounds.end()) {
            device.bind_streams(it->second);
        }
    }
};
```

### Music Player with Streaming

```cpp
class music_player {
    sdlpp::audio_device device;
    sdlpp::audio_stream stream;
    std::thread loader_thread;
    std::atomic<bool> playing{false};
    
    static constexpr size_t CHUNK_SIZE = 4096 * 4;
    
public:
    bool play_music(const std::string& path) {
        stop();
        
        // Open file and read header
        auto file_data = load_music_file(path);
        if (!file_data) return false;
        
        // Create stream
        auto stream_result = sdlpp::audio_stream::create(
            file_data->spec, device.get_spec()
        );
        
        if (!stream_result) return false;
        stream = std::move(stream_result.value());
        
        // Start streaming thread
        playing = true;
        loader_thread = std::thread([this, data = std::move(file_data)]() {
            stream_audio(*data);
        });
        
        device.bind_streams(stream);
        device.resume();
        
        return true;
    }
    
    void stop() {
        playing = false;
        if (loader_thread.joinable()) {
            loader_thread.join();
        }
        stream.clear();
    }
    
private:
    void stream_audio(const music_file_data& data) {
        size_t position = 0;
        
        while (playing && position < data.samples.size()) {
            // Wait if buffer is full
            while (stream.get_available() > CHUNK_SIZE * 4 && playing) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            
            // Queue next chunk
            size_t to_queue = std::min(CHUNK_SIZE, 
                                     data.samples.size() - position);
            stream.put(&data.samples[position], 
                      to_queue * sizeof(float));
            
            position += to_queue;
        }
    }
};
```

## Audio Mixing

### Volume Control

```cpp
class audio_mixer {
    struct channel {
        sdlpp::audio_stream stream;
        float volume = 1.0f;
        bool active = false;
    };
    
    std::array<channel, 8> channels;
    
public:
    void set_channel_volume(int ch, float volume) {
        if (ch >= 0 && ch < channels.size()) {
            channels[ch].volume = std::clamp(volume, 0.0f, 1.0f);
            // Apply volume to stream
            channels[ch].stream.set_gain(channels[ch].volume);
        }
    }
    
    void play_on_channel(int ch, const audio_data& data) {
        if (ch >= 0 && ch < channels.size()) {
            channels[ch].stream.clear();
            channels[ch].stream.put(data.samples.data(), 
                                  data.samples.size() * sizeof(float));
            channels[ch].active = true;
        }
    }
};
```

### Audio Effects

```cpp
// Simple fade effect
class fade_effect {
public:
    static void apply_fade_in(std::vector<float>& samples, 
                            float duration_seconds,
                            int sample_rate) {
        size_t fade_samples = static_cast<size_t>(
            duration_seconds * sample_rate
        );
        fade_samples = std::min(fade_samples, samples.size());
        
        for (size_t i = 0; i < fade_samples; ++i) {
            float factor = static_cast<float>(i) / fade_samples;
            samples[i] *= factor;
        }
    }
    
    static void apply_fade_out(std::vector<float>& samples,
                             float duration_seconds,
                             int sample_rate) {
        size_t fade_samples = static_cast<size_t>(
            duration_seconds * sample_rate
        );
        fade_samples = std::min(fade_samples, samples.size());
        
        size_t start = samples.size() - fade_samples;
        for (size_t i = 0; i < fade_samples; ++i) {
            float factor = 1.0f - (static_cast<float>(i) / fade_samples);
            samples[start + i] *= factor;
        }
    }
};
```

## Audio Callbacks (Advanced)

For low-latency audio or custom processing:

```cpp
class audio_callback_device {
    sdlpp::audio_device device;
    
    static void audio_callback(void* userdata, Uint8* stream, int len) {
        auto* self = static_cast<audio_callback_device*>(userdata);
        self->process_audio(reinterpret_cast<float*>(stream), 
                          len / sizeof(float));
    }
    
    void process_audio(float* buffer, size_t samples) {
        // Generate or process audio
        for (size_t i = 0; i < samples; i += 2) {
            // Stereo sine wave
            float value = std::sin(phase) * 0.25f;
            buffer[i] = value;      // Left
            buffer[i + 1] = value;  // Right
            
            phase += (440.0f * 2.0f * M_PI) / 48000.0f;
            if (phase > 2.0f * M_PI) phase -= 2.0f * M_PI;
        }
    }
    
    float phase = 0.0f;
    
public:
    bool init() {
        auto device_result = sdlpp::audio_device::open_playback(
            sdlpp::audio_spec{
                .freq = 48000,
                .format = sdlpp::audio_format::f32,
                .channels = 2,
                .samples = 256  // Low latency
            },
            audio_callback,
            this
        );
        
        if (!device_result) return false;
        
        device = std::move(device_result.value());
        device.resume();
        return true;
    }
};
```

## Best Practices

1. **Use Float Format**: F32 provides best quality and processing flexibility
2. **Stream Large Files**: Don't load entire music files into memory
3. **Reuse Streams**: Clear and reuse streams instead of creating new ones
4. **Handle Device Changes**: Listen for device events on mobile/USB audio
5. **Volume Control**: Use stream gains instead of modifying samples

## Example: Complete Audio System

```cpp
class audio_system {
    sdlpp::audio_device device;
    
    // Sound effects
    std::unordered_map<std::string, audio_buffer> sound_effects;
    std::vector<sdlpp::audio_stream> effect_streams;
    
    // Music
    sdlpp::audio_stream music_stream;
    std::thread music_thread;
    std::atomic<bool> music_playing{false};
    
    // Settings
    float master_volume = 1.0f;
    float effects_volume = 1.0f;
    float music_volume = 0.7f;
    
public:
    bool init() {
        // Open audio device
        auto device_result = sdlpp::audio_device::open_playback(
            sdlpp::audio_spec{
                .freq = 48000,
                .format = sdlpp::audio_format::f32,
                .channels = 2,
                .samples = 2048
            }
        );
        
        if (!device_result) {
            std::cerr << "Failed to open audio: " 
                      << device_result.error() << std::endl;
            return false;
        }
        
        device = std::move(device_result.value());
        
        // Pre-create effect streams
        effect_streams.reserve(8);
        for (int i = 0; i < 8; ++i) {
            auto stream = sdlpp::audio_stream::create(
                device.get_spec(), device.get_spec()
            );
            if (stream) {
                effect_streams.push_back(std::move(stream.value()));
            }
        }
        
        // Create music stream
        auto music_result = sdlpp::audio_stream::create(
            device.get_spec(), device.get_spec()
        );
        if (music_result) {
            music_stream = std::move(music_result.value());
        }
        
        // Bind all streams
        update_device_bindings();
        
        // Start audio
        device.resume();
        
        return true;
    }
    
    bool load_sound(const std::string& name, const std::string& path) {
        auto result = load_wav(path);
        if (!result) return false;
        
        sound_effects[name] = std::move(result.value());
        return true;
    }
    
    void play_sound(const std::string& name, float volume = 1.0f) {
        auto it = sound_effects.find(name);
        if (it == sound_effects.end()) return;
        
        // Find free stream
        for (auto& stream : effect_streams) {
            if (stream.get_available() == 0) {
                stream.clear();
                stream.put(it->second.data.data(), 
                         it->second.data.size());
                stream.set_gain(volume * effects_volume * master_volume);
                break;
            }
        }
    }
    
    void set_master_volume(float volume) {
        master_volume = std::clamp(volume, 0.0f, 1.0f);
        update_volumes();
    }
    
private:
    void update_device_bindings() {
        std::vector<std::reference_wrapper<sdlpp::audio_stream>> streams;
        
        for (auto& stream : effect_streams) {
            streams.push_back(stream);
        }
        streams.push_back(music_stream);
        
        device.bind_streams(streams);
    }
    
    void update_volumes() {
        for (auto& stream : effect_streams) {
            stream.set_gain(effects_volume * master_volume);
        }
        music_stream.set_gain(music_volume * master_volume);
    }
};
```