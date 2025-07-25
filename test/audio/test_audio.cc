#include <doctest/doctest.h>
#include <sdlpp/audio/audio.hh>
#include <sdlpp/core/core.hh>

#include <vector>
#include <cmath>
#include <numbers>

// Workaround for doctest Approx compatibility
namespace {
    struct approx_float {
        float value;
        float epsilon = 1e-5f;
        
        explicit approx_float(float v) : value(v) {}
        
        bool operator==(float other) const {
            return std::abs(value - other) < epsilon;
        }
    };
    
    bool operator==(float lhs, const approx_float& rhs) {
        return rhs == lhs;
    }
}

TEST_SUITE("audio") {
    TEST_CASE("audio_format") {
        SUBCASE("bit size") {
            CHECK(sdlpp::audio_bit_size(sdlpp::audio_format::u8) == 8);
            CHECK(sdlpp::audio_bit_size(sdlpp::audio_format::s16) == 16);
            CHECK(sdlpp::audio_bit_size(sdlpp::audio_format::s32) == 32);
            CHECK(sdlpp::audio_bit_size(sdlpp::audio_format::f32) == 32);
        }
        
        SUBCASE("byte size") {
            CHECK(sdlpp::audio_byte_size(sdlpp::audio_format::u8) == 1);
            CHECK(sdlpp::audio_byte_size(sdlpp::audio_format::s16) == 2);
            CHECK(sdlpp::audio_byte_size(sdlpp::audio_format::s32) == 4);
            CHECK(sdlpp::audio_byte_size(sdlpp::audio_format::f32) == 4);
        }
        
        SUBCASE("type checks") {
            CHECK(sdlpp::audio_is_float(sdlpp::audio_format::f32) == true);
            CHECK(sdlpp::audio_is_float(sdlpp::audio_format::s16) == false);
            CHECK(sdlpp::audio_is_int(sdlpp::audio_format::s16) == true);
            CHECK(sdlpp::audio_is_int(sdlpp::audio_format::f32) == false);
            
            CHECK(sdlpp::audio_is_signed(sdlpp::audio_format::s16) == true);
            CHECK(sdlpp::audio_is_signed(sdlpp::audio_format::u8) == false);
            CHECK(sdlpp::audio_is_unsigned(sdlpp::audio_format::u8) == true);
            CHECK(sdlpp::audio_is_unsigned(sdlpp::audio_format::s16) == false);
        }
        
        SUBCASE("endianness") {
            CHECK(sdlpp::audio_is_little_endian(sdlpp::audio_format::s16le) == true);
            CHECK(sdlpp::audio_is_big_endian(sdlpp::audio_format::s16le) == false);
            CHECK(sdlpp::audio_is_big_endian(sdlpp::audio_format::s16be) == true);
            CHECK(sdlpp::audio_is_little_endian(sdlpp::audio_format::s16be) == false);
        }
    }
    
    TEST_CASE("audio_spec") {
        sdlpp::audio_spec spec{
            .format = sdlpp::audio_format::f32,
            .channels = 2,
            .freq = 48000
        };
        
        CHECK(spec.frame_size() == 8);  // 2 channels * 4 bytes per sample
        
        auto sdl_spec = spec.to_sdl();
        CHECK(sdl_spec.format == SDL_AUDIO_F32);
        CHECK(sdl_spec.channels == 2);
        CHECK(sdl_spec.freq == 48000);
        
        auto spec2 = sdlpp::audio_spec::from_sdl(sdl_spec);
        CHECK(spec2.format == spec.format);
        CHECK(spec2.channels == spec.channels);
        CHECK(spec2.freq == spec.freq);
    }
    
    TEST_CASE("audio drivers") {
        auto init = sdlpp::init(sdlpp::init_flags::audio);
        REQUIRE(init.was_init(sdlpp::init_flags::audio));
        
        auto num_drivers = sdlpp::get_num_audio_drivers();
        CHECK(num_drivers >= 0);
        
        for (size_t i = 0; i < num_drivers; ++i) {
            auto name = sdlpp::get_audio_driver(i);
            CHECK(!name.empty());
        }
    }
    
    TEST_CASE("audio devices") {
        auto init = sdlpp::init(sdlpp::init_flags::audio);
        REQUIRE(init.was_init(sdlpp::init_flags::audio));
        
        SUBCASE("list devices") {
            auto playback = sdlpp::get_audio_playback_devices();
            CHECK(playback.size() >= 0);
            
            auto recording = sdlpp::get_audio_recording_devices();
            CHECK(recording.size() >= 0);
            
            // Check device properties
            for (auto id : playback) {
                CHECK(sdlpp::is_audio_device_playback(id) == true);
                CHECK(sdlpp::is_audio_device_physical(id) == true);
                
                auto name = sdlpp::get_audio_device_name(id);
                CHECK(!name.empty());
            }
        }
    }
    
    TEST_CASE("audio_stream") {
        auto init = sdlpp::init(sdlpp::init_flags::audio);
        REQUIRE(init.was_init(sdlpp::init_flags::audio));
        
        SUBCASE("create stream") {
            sdlpp::audio_spec src_spec{
                .format = sdlpp::audio_format::s16,
                .channels = 2,
                .freq = 44100
            };
            
            sdlpp::audio_spec dst_spec{
                .format = sdlpp::audio_format::f32,
                .channels = 2,
                .freq = 48000
            };
            
            auto stream = sdlpp::audio_stream::create(src_spec, dst_spec);
            CHECK(stream.has_value());
            CHECK(stream->is_valid());
            
            // Check formats
            auto input_fmt = stream->get_input_format();
            CHECK(input_fmt.has_value());
            CHECK(input_fmt->format == src_spec.format);
            CHECK(input_fmt->channels == src_spec.channels);
            CHECK(input_fmt->freq == src_spec.freq);
            
            auto output_fmt = stream->get_output_format();
            CHECK(output_fmt.has_value());
            CHECK(output_fmt->format == dst_spec.format);
            CHECK(output_fmt->channels == dst_spec.channels);
            CHECK(output_fmt->freq == dst_spec.freq);
        }
        
        SUBCASE("stream data flow") {
            sdlpp::audio_spec spec{
                .format = sdlpp::audio_format::f32,
                .channels = 1,
                .freq = 48000
            };
            
            auto stream = sdlpp::audio_stream::create(spec, spec);
            REQUIRE(stream.has_value());
            
            // Generate a simple sine wave
            constexpr int sample_count = 480;   // 10ms at 48kHz
            std::vector<float> input_data(static_cast<size_t>(sample_count));
            
            for (size_t i = 0; i < input_data.size(); ++i) {
                constexpr float frequency = 440.0f;
                float t = static_cast<float>(i) / static_cast<float>(spec.freq);
                input_data[i] = std::sin(2.0f * std::numbers::pi_v<float> * frequency * t);
            }
            
            // Put data into stream
            auto put_result = stream->put_data(std::span<const float>{input_data});
            CHECK(put_result.has_value());
            
            // Check available data
            auto available_result = stream->get_available();
            CHECK(available_result.has_value());
            CHECK(*available_result == sample_count * sizeof(float));
            
            // Get data from stream
            std::vector<float> output_data(static_cast<size_t>(sample_count));
            auto get_result = stream->get_data(std::span<float>{output_data});
            CHECK(get_result.has_value());
            CHECK(get_result.value() == static_cast<size_t>(sample_count));
            
            // Verify data (should be identical since no conversion)
            for (size_t i = 0; i < static_cast<size_t>(sample_count); ++i) {
                CHECK(output_data[i] == approx_float(input_data[i]));
            }
        }
        
        SUBCASE("stream properties") {
            sdlpp::audio_spec spec{
                .format = sdlpp::audio_format::f32,
                .channels = 2,
                .freq = 48000
            };
            
            auto stream = sdlpp::audio_stream::create(spec, spec);
            REQUIRE(stream.has_value());
            
            // Test gain
            CHECK(stream->get_gain() == 1.0f);
            CHECK(stream->set_gain(0.5f).has_value());
            CHECK(stream->get_gain() == 0.5f);
            
            // Test frequency ratio
            CHECK(stream->get_frequency_ratio() == 1.0f);
            CHECK(stream->set_frequency_ratio(2.0f).has_value());
            CHECK(stream->get_frequency_ratio() == 2.0f);
        }
        
        SUBCASE("channel mapping") {
            sdlpp::audio_spec spec{
                .format = sdlpp::audio_format::f32,
                .channels = 2,
                .freq = 48000
            };
            
            auto stream = sdlpp::audio_stream::create(spec, spec);
            REQUIRE(stream.has_value());
            
            // Swap left and right channels
            std::vector<int> channel_map = {1, 0};
            CHECK(stream->set_output_channel_map(channel_map).has_value());
        }
    }
    
    TEST_CASE("audio_device") {
        auto init = sdlpp::init(sdlpp::init_flags::audio);
        REQUIRE(init.was_init(sdlpp::init_flags::audio));
        
        SUBCASE("open device") {
            sdlpp::audio_spec spec{
                .format = sdlpp::audio_format::f32,
                .channels = 2,
                .freq = 48000
            };
            
            auto device = sdlpp::audio_device::open(
                sdlpp::default_playback_device(), &spec);
            CHECK(device.has_value());
            CHECK(device->is_valid());
            
            // Check format
            auto fmt = device->get_format();
            CHECK(fmt.has_value());
            // Device may choose different format than requested
            CHECK(fmt->channels > 0);
            CHECK(fmt->freq > 0);
        }
        
        SUBCASE("device control") {
            auto device = sdlpp::audio_device::open(
                sdlpp::default_playback_device(), nullptr);
            REQUIRE(device.has_value());
            
            // Test pause/resume
            CHECK(device->is_paused() == false);
            CHECK(device->pause().has_value());
            CHECK(device->is_paused() == true);
            CHECK(device->resume().has_value());
            CHECK(device->is_paused() == false);
            
            // Test gain
            CHECK(device->get_gain() == 1.0f);
            CHECK(device->set_gain(0.75f).has_value());
            CHECK(device->get_gain() == 0.75f);
        }
    }
    
    TEST_CASE("simplified audio interface") {
        auto init = sdlpp::init(sdlpp::init_flags::audio);
        REQUIRE(init.was_init(sdlpp::init_flags::audio));
        
        sdlpp::audio_spec spec{
            .format = sdlpp::audio_format::f32,
            .channels = 2,
            .freq = 48000
        };
        
        SUBCASE("without callback") {
            auto stream = sdlpp::open_audio_device_stream(
                sdlpp::default_playback_device(), spec);
            CHECK(stream.has_value());
            CHECK(stream->is_valid());
            
            // Device starts paused with this interface
            CHECK(stream->is_device_paused() == true);
            
            // Can control device through stream
            CHECK(stream->resume_device().has_value());
            CHECK(stream->is_device_paused() == false);
        }
        
        SUBCASE("with callback") {
            struct callback_data {
                int callback_count = 0;
                int total_requested = 0;
            } data;
            
            auto stream = sdlpp::open_audio_device_stream(
                sdlpp::default_playback_device(), spec,
                [](void* userdata, sdlpp::audio_stream_ref /*stream*/, int additional, int /*total*/) {
                    auto* cb_data = static_cast<callback_data*>(userdata);
                    cb_data->callback_count++;
                    cb_data->total_requested += additional;
                    
                    // Could generate audio data here
                    // For test, just count calls
                }, &data);
                
            CHECK(stream.has_value());
            CHECK(stream->is_valid());
        }
    }
    
    TEST_CASE("load_wav") {
        SUBCASE("load from filesystem path") {
            std::filesystem::path wav_path = "test_audio.wav";
            auto result = sdlpp::load_wav(wav_path);
            // Will fail in test environment but verifies compilation
            CHECK(!result.has_value());
        }
        
        SUBCASE("load from string path") {
            std::string wav_path = "test_audio.wav";
            auto result = sdlpp::load_wav(std::string_view(wav_path));
            // Will fail in test environment but verifies compilation
            CHECK(!result.has_value());
        }
        
        SUBCASE("load from iostream") {
            // In a real test, we'd create a valid iostream
            // For now, just verify the API compiles
            auto stream_result = sdlpp::open_file(std::string("test_audio.wav"), sdlpp::file_mode::read_binary);
            if (stream_result) {
                auto wav_result = sdlpp::load_wav(*stream_result, true);
                // Will fail but verifies API
                CHECK(!wav_result.has_value());
            }
        }
    }
}