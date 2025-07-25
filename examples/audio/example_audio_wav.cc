#include <sdlpp/core/core.hh>
#include <sdlpp/audio/audio.hh>
#include <sdlpp/io/iostream.hh>
#include <iostream>
#include <filesystem>

int main() {
    // Initialize SDL audio subsystem
    try {
        sdlpp::init init(sdlpp::init_flags::audio);

        // Example 1: Load WAV from filesystem::path
        std::filesystem::path wav_path = "sample.wav";
        auto wav_result = sdlpp::load_wav(wav_path);
        if (!wav_result) {
            std::cerr << "Failed to load WAV from path: " << wav_result.error() << std::endl;
        } else {
            std::cout << "Loaded WAV from filesystem::path" << std::endl;
            std::cout << "  Format: " << static_cast<int>(wav_result->spec.format) << std::endl;
            std::cout << "  Channels: " << wav_result->spec.channels << std::endl;
            std::cout << "  Frequency: " << wav_result->spec.freq << " Hz" << std::endl;
            std::cout << "  Buffer size: " << wav_result->buffer.size() << " bytes" << std::endl;
        }

        // Example 2: Load WAV from string_view
        std::string wav_string = "sample.wav";
        wav_result = sdlpp::load_wav(std::string_view(wav_string));
        if (wav_result) {
            std::cout << "\nLoaded WAV from string path" << std::endl;
        }

        // Example 3: Load WAV from iostream
        auto stream_result = sdlpp::open_file(std::string("sample.wav"), sdlpp::file_mode::read_binary);
        if (stream_result) {
            wav_result = sdlpp::load_wav(*stream_result, true);  // true = close stream after loading
            if (wav_result) {
                std::cout << "\nLoaded WAV from iostream" << std::endl;
            }
        }

        // Example 4: Load WAV from memory buffer
        auto file_data = sdlpp::load_file("sample.wav");
        if (file_data) {
            auto mem_stream = sdlpp::from_memory(file_data->data(), file_data->size());
            if (mem_stream) {
                wav_result = sdlpp::load_wav(*mem_stream, false);
                if (wav_result) {
                    std::cout << "\nLoaded WAV from memory stream" << std::endl;
                }
            }
        }

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize SDL: " << e.what() << std::endl;
        return 1;
    }
}