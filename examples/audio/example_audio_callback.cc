#include <iostream>
#include <vector>
#include <cmath>
#include <numbers>
#include <thread>
#include <chrono>
#include <atomic>

#include <sdlpp/core/core.hh>
#include <sdlpp/audio/audio.hh>

// Example demonstrating audio callbacks
int main() {
    // Initialize SDL audio subsystem
    auto init = sdlpp::init(sdlpp::init_flags::audio);
    if (!init.was_init(sdlpp::init_flags::audio)) {
        std::cerr << "Failed to initialize SDL audio" << std::endl;
        return 1;
    }

    // Audio format specification
    sdlpp::audio_spec spec{
        .format = sdlpp::audio_format::f32,
        .channels = 2,      // Stereo
        .freq = 48000       // 48 kHz sample rate
    };

    // Callback data structure
    struct tone_generator {
        float frequency{440.0f};
        float amplitude{0.25f};
        float phase{0.0f};
        int sample_rate{48000};
        std::atomic<bool> playing{true};
    };
    
    tone_generator tone_gen;
    tone_gen.sample_rate = spec.freq;

    // Create audio stream with callback
    auto stream_result = sdlpp::open_audio_device_stream(
        sdlpp::default_playback_device(), spec,
        [](void* userdata, sdlpp::audio_stream_ref stream, int additional, int /*total*/) {
            auto* gen = static_cast<tone_generator*>(userdata);
            
            if (!gen->playing.load()) {
                return;
            }
            
            // Calculate how many frames we need
            int frames_needed = additional / static_cast<int>(2 * sizeof(float)); // stereo float
            
            // Generate audio data
            std::vector<float> buffer(static_cast<size_t>(frames_needed * 2));
            
            for (int i = 0; i < frames_needed; ++i) {
                float sample = gen->amplitude * std::sin(gen->phase);
                
                // Write to both channels
                buffer[static_cast<size_t>(i * 2)] = sample;
                buffer[static_cast<size_t>(i * 2 + 1)] = sample;
                
                // Advance phase
                gen->phase += 2.0f * std::numbers::pi_v<float> * gen->frequency / static_cast<float>(gen->sample_rate);
                
                // Wrap phase
                if (gen->phase > 2.0f * std::numbers::pi_v<float>) {
                    gen->phase -= 2.0f * std::numbers::pi_v<float>;
                }
            }
            
            // Send data to stream
            auto result = stream.put_data(std::span<const float>{buffer});
            if (!result) {
                std::cerr << "Failed to put audio data: " << result.error() << std::endl;
            }
        }, &tone_gen);
    
    if (!stream_result) {
        std::cerr << "Failed to open audio stream: " << stream_result.error() << std::endl;
        return 1;
    }
    
    auto& stream = stream_result.value();
    
    // Start audio playback
    if (!stream.resume_device()) {
        std::cerr << "Failed to resume audio device" << std::endl;
        return 1;
    }
    
    std::cout << "Playing 440Hz tone for 2 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Change frequency
    std::cout << "Changing to 880Hz..." << std::endl;
    tone_gen.frequency = 880.0f;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Demonstrate gain control
    std::cout << "Fading out..." << std::endl;
    for (int i = 10; i >= 0; --i) {
        auto gain_result = stream.set_gain(static_cast<float>(i) / 10.0f);
        if (!gain_result) {
            std::cerr << "Failed to set gain: " << gain_result.error() << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Stop generating audio
    tone_gen.playing.store(false);
    
    // Wait a bit for any remaining audio to play
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    std::cout << "Audio playback complete!" << std::endl;
    
    return 0;
}