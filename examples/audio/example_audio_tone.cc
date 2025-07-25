#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <thread>
#include <chrono>

#include <sdlpp/core/core.hh>
#include <sdlpp/audio/audio.hh>

// Simple tone generator example
int main() {
    // Initialize SDL audio subsystem
    auto init = sdlpp::init(sdlpp::init_flags::audio);
    if (!init.was_init(sdlpp::init_flags::audio)) {
        std::cerr << "Failed to initialize SDL audio" << std::endl;
        return 1;
    }

    // List available audio devices
    std::cout << "Available playback devices:" << std::endl;
    auto devices = sdlpp::get_audio_playback_devices();
    for (auto id : devices) {
        std::cout << "  - " << sdlpp::get_audio_device_name(id) << std::endl;
    }
    std::cout << std::endl;

    // Audio format specification
    sdlpp::audio_spec spec{
        .format = sdlpp::audio_format::f32,
        .channels = 2,      // Stereo
        .freq = 48000       // 48 kHz sample rate
    };

    // Create audio stream using simplified interface
    auto stream_result = sdlpp::open_audio_device_stream(
        sdlpp::default_playback_device(), spec);
    
    if (!stream_result) {
        std::cerr << "Failed to open audio stream: " << stream_result.error() << std::endl;
        return 1;
    }
    
    auto& stream = stream_result.value();
    
    // Generate sine wave tones
    struct ToneGenerator {
        float frequency;
        float amplitude;
        float phase = 0.0f;
        
        void generate(float* buffer, int frames, int channels, int sample_rate) {
            for (int i = 0; i < frames; ++i) {
                float sample = amplitude * std::sin(phase);
                
                // Write to all channels
                for (int ch = 0; ch < channels; ++ch) {
                    buffer[i * channels + ch] = sample;
                }
                
                // Advance phase
                phase += 2.0f * std::numbers::pi_v<float> * frequency / static_cast<float>(sample_rate);
                
                // Wrap phase to prevent precision loss
                if (phase > 2.0f * std::numbers::pi_v<float>) {
                    phase -= 2.0f * std::numbers::pi_v<float>;
                }
            }
        }
    };
    
    // Musical notes (A major scale)
    const std::vector<std::pair<float, const char*>> notes = {
        {440.00f, "A4"},
        {493.88f, "B4"},
        {554.37f, "C#5"},
        {587.33f, "D5"},
        {659.25f, "E5"},
        {739.99f, "F#5"},
        {830.61f, "G#5"},
        {880.00f, "A5"}
    };
    
    std::cout << "Playing A major scale..." << std::endl;
    
    // Buffer for one second of audio
    const int frames_per_buffer = spec.freq / 10;  // 100ms buffer
    std::vector<float> buffer(static_cast<size_t>(frames_per_buffer * spec.channels));
    
    // Start audio playback
    if (!stream.resume_device()) {
        std::cerr << "Failed to resume audio device" << std::endl;
        return 1;
    }
    
    ToneGenerator tone_gen;
    tone_gen.amplitude = 0.25f;  // 25% volume
    
    // Play each note for 500ms
    for (const auto& [freq, name] : notes) {
        std::cout << "Playing " << name << " (" << freq << " Hz)" << std::endl;
        
        tone_gen.frequency = freq;
        tone_gen.phase = 0.0f;  // Reset phase for each note
        
        // Generate and play 500ms of the tone
        for (int i = 0; i < 5; ++i) {  // 5 * 100ms = 500ms
            tone_gen.generate(buffer.data(), frames_per_buffer, spec.channels, spec.freq);
            
            auto result = stream.put_data(std::span<const float>{buffer});
            if (!result) {
                std::cerr << "Failed to put audio data: " << result.error() << std::endl;
            }
        }
        
        // Small gap between notes
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    // Wait for audio to finish playing
    while (auto queued = stream.get_queued()) {
        if (*queued == 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "\nPlaying chord (C#5 + E5 + G#5)..." << std::endl;
    
    // Play a chord
    struct {
        ToneGenerator generators[3];
    } chord;
    
    chord.generators[0].frequency = 554.37f;  // C#5
    chord.generators[0].amplitude = 0.15f;
    chord.generators[1].frequency = 659.25f;  // E5
    chord.generators[1].amplitude = 0.15f;
    chord.generators[2].frequency = 830.61f;  // G#5
    chord.generators[2].amplitude = 0.15f;
    
    // Generate chord (mix multiple tones)
    for (int b = 0; b < 20; ++b) {  // 2 seconds
        // Clear buffer
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        
        // Generate each tone into temporary buffer and mix
        std::vector<float> temp_buffer(buffer.size());
        for (auto& gen : chord.generators) {
            gen.generate(temp_buffer.data(), frames_per_buffer, spec.channels, spec.freq);
            
            // Mix into main buffer
            for (size_t i = 0; i < buffer.size(); ++i) {
                buffer[i] += temp_buffer[i];
            }
        }
        
        auto result = stream.put_data(std::span<const float>{buffer});
        if (!result) {
            std::cerr << "Failed to put audio data: " << result.error() << std::endl;
        }
    }
    
    // Wait for audio to finish
    while (auto queued = stream.get_queued()) {
        if (*queued == 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "\nDemonstrating real-time effects..." << std::endl;
    
    // Demonstrate gain changes
    tone_gen.frequency = 440.0f;  // A4
    tone_gen.amplitude = 0.3f;
    tone_gen.phase = 0.0f;
    
    std::cout << "Fading in and out..." << std::endl;
    
    // For fading to work properly, we need to apply the gain to the samples directly
    // or ensure small buffer sizes so gain changes take effect quickly
    for (int i = 0; i < 40; ++i) {  // 4 seconds total
        // Generate tone
        tone_gen.generate(buffer.data(), frames_per_buffer, spec.channels, spec.freq);
        
        // Apply fade in/out directly to samples
        float t = static_cast<float>(i) / 40.0f;
        float gain = (t < 0.5f) ? (t * 2.0f) : (2.0f - t * 2.0f);
        
        // Apply gain to buffer
        for (auto& sample : buffer) {
            sample *= gain;
        }
        
        auto result = stream.put_data(std::span<const float>{buffer});
        if (!result) {
            std::cerr << "Failed to put audio data: " << result.error() << std::endl;
        }
        
        // Small delay to ensure smooth playback
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Wait for audio to finish
    while (auto queued = stream.get_queued()) {
        if (*queued == 0) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    std::cout << "\nAudio playback complete!" << std::endl;
    
    return 0;
}