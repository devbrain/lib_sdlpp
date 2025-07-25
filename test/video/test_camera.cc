#include <doctest/doctest.h>
#include <sdlpp/video/camera.hh>
#include <sdlpp/core/core.hh>
#include <iostream>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

TEST_SUITE("camera") {
    TEST_CASE("camera device enumeration") {
        // Initialize SDL with camera support
        auto init = sdlpp::init(sdlpp::init_flags::camera);
        if (init) {
            SUBCASE("get camera devices") {
                auto cameras = sdlpp::get_cameras();
                // May be empty if no cameras
                CHECK(cameras.size() >= 0);
                
                std::cout << "Found " << cameras.size() << " camera(s):\n";
                
                for (auto id : cameras) {
                    auto name = sdlpp::get_camera_name(id);
                    auto position = sdlpp::get_camera_position(id);
                    
                    std::cout << "  Camera " << id << ": " << name;
                    
                    switch (position) {
                        case sdlpp::camera_position::front_facing:
                            std::cout << " (front-facing)";
                            break;
                        case sdlpp::camera_position::back_facing:
                            std::cout << " (back-facing)";
                            break;
                        default:
                            std::cout << " (unknown position)";
                            break;
                    }
                    std::cout << "\n";
                    
                    CHECK(!name.empty());
                }
            }
            
            SUBCASE("get supported formats") {
                auto cameras = sdlpp::get_cameras();
                if (!cameras.empty()) {
                    auto formats = sdlpp::get_camera_supported_formats(cameras[0]);
                    
                    std::cout << "Camera " << cameras[0] << " supports " 
                             << formats.size() << " format(s):\n";
                    
                    for (const auto& fmt : formats) {
                        std::cout << "  " << fmt.width << "x" << fmt.height 
                                 << " @ " << fmt.get_framerate() << " FPS"
                                 << " (format: " << static_cast<uint32_t>(fmt.format) << ")\n";
                    }
                    
                    // Most cameras should support at least one format
                    CHECK(!formats.empty());
                }
            }
        }
    }
    
    TEST_CASE("camera operations") {
        auto init = sdlpp::init(sdlpp::init_flags::camera);
        if (init) {
            auto cameras = sdlpp::get_cameras();
            
            SUBCASE("open non-existent camera") {
                auto camera = sdlpp::camera::open(0xFFFFFFFF);
                CHECK(!camera.has_value());
            }
            
            SUBCASE("open camera with default format") {
                if (!cameras.empty()) {
                    auto camera = sdlpp::camera::open(cameras[0]);
                    
                    // Camera may fail to open due to permissions
                    if (camera) {
                        CHECK(camera->is_valid());
                        CHECK(camera->get_id() == cameras[0]);
                        
                        auto name = camera->get_name();
                        CHECK(!name.empty());
                        std::cout << "Opened camera: " << name << "\n";
                        
                        // Check current format
                        auto format = camera->get_format();
                        if (format) {
                            std::cout << "Current format: " 
                                     << format->width << "x" << format->height
                                     << " @ " << format->get_framerate() << " FPS\n";
                            
                            CHECK(format->width > 0);
                            CHECK(format->height > 0);
                        }
                        
                        // Check supported formats
                        auto supported = camera->get_supported_formats();
                        CHECK(!supported.empty());
                        
                        // Permission handling is now at system level in SDL3
                    } else {
                        std::cout << "Failed to open camera: " << camera.error() << "\n";
                    }
                }
            }
            
            SUBCASE("open camera with specific format") {
                if (!cameras.empty()) {
                    auto formats = sdlpp::get_camera_supported_formats(cameras[0]);
                    if (!formats.empty()) {
                        // Try to open with first supported format
                        auto camera = sdlpp::camera::open(cameras[0], &formats[0]);
                        
                        if (camera) {
                            CHECK(camera->is_valid());
                            
                            // Verify format was applied
                            auto current = camera->get_format();
                            if (current) {
                                // Format might not match exactly due to driver limitations
                                std::cout << "Requested: " << formats[0].width << "x" << formats[0].height << "\n";
                                std::cout << "Got: " << current->width << "x" << current->height << "\n";
                            }
                        }
                    }
                }
            }
            
            SUBCASE("format support check") {
                if (!cameras.empty()) {
                    auto camera = sdlpp::camera::open(cameras[0]);
                    if (camera) {
                        auto formats = camera->get_supported_formats();
                        if (!formats.empty()) {
                            // First format should be supported
                            CHECK(camera->is_format_supported(formats[0]));
                            
                            // Made-up format should not be supported
                            sdlpp::camera_spec fake_spec = {
                                .format = sdlpp::pixel_format_enum::ARGB8888,
                                .width = 12345,
                                .height = 67890,
                                .framerate_numerator = 30,
                                .framerate_denominator = 1
                            };
                            CHECK(!camera->is_format_supported(fake_spec));
                        }
                    }
                }
            }
        }
    }
    
    TEST_CASE("camera frame acquisition") {
        auto init = sdlpp::init(sdlpp::init_flags::camera);
        if (init) {
            auto cameras = sdlpp::get_cameras();
            
            SUBCASE("manual frame acquisition") {
                if (!cameras.empty()) {
                    auto camera = sdlpp::camera::open(cameras[0]);
                    if (camera) {
                        // Try to acquire a few frames
                        std::cout << "Attempting to acquire frames...\n";
                        
                        int frames_acquired = 0;
                        for (int i = 0; i < 10; ++i) {
                            Uint64 timestamp;
                            SDL_Surface* frame = camera->acquire_frame(&timestamp);
                            
                            if (frame) {
                                frames_acquired++;
                                std::cout << "  Frame " << i << ": " 
                                         << frame->w << "x" << frame->h
                                         << " timestamp: " << timestamp << "ns\n";
                                
                                CHECK(frame->w > 0);
                                CHECK(frame->h > 0);
                                CHECK(timestamp > 0);
                                
                                // Must release frame
                                CHECK(camera->release_frame(frame));
                            }
                            
                            // Small delay between frames
                            std::this_thread::sleep_for(100ms);
                        }
                        
                        std::cout << "Acquired " << frames_acquired << " frames\n";
                    }
                }
            }
            
            SUBCASE("RAII frame acquisition") {
                if (!cameras.empty()) {
                    auto camera = sdlpp::camera::open(cameras[0]);
                    if (camera) {
                        // Use RAII frame helper
                        {
                            sdlpp::camera_frame frame(*camera);
                            if (frame) {
                                std::cout << "RAII Frame: " 
                                         << frame->w << "x" << frame->h << "\n";
                                
                                CHECK(frame.is_valid());
                                CHECK(frame.get() != nullptr);
                                CHECK(frame.get_timestamp_ns() > 0);
                                
                                // Can get timestamp as chrono duration
                                auto timestamp = frame.get_timestamp();
                                CHECK(timestamp.count() > 0);
                            }
                            // Frame automatically released here
                        }
                        
                        // Multiple frames
                        for (int i = 0; i < 3; ++i) {
                            sdlpp::camera_frame frame(*camera);
                            if (frame) {
                                std::cout << "Frame " << i << " timestamp: " 
                                         << frame.get_timestamp_ns() << "ns\n";
                            }
                            std::this_thread::sleep_for(100ms);
                        }
                    }
                }
            }
        }
    }
    
    TEST_CASE("camera spec") {
        SUBCASE("framerate calculation") {
            sdlpp::camera_spec spec;
            spec.framerate_numerator = 30;
            spec.framerate_denominator = 1;
            CHECK(spec.get_framerate() == doctest::Approx(30.0f));
            
            spec.framerate_numerator = 60000;
            spec.framerate_denominator = 1001;  // NTSC framerate
            CHECK(spec.get_framerate() == doctest::Approx(59.94f).epsilon(0.01));
            
            // Invalid denominator
            spec.framerate_denominator = 0;
            CHECK(spec.get_framerate() == 0.0f);
        }
        
        SUBCASE("SDL conversion") {
            sdlpp::camera_spec spec = {
                .format = sdlpp::pixel_format_enum::RGB24,
                .width = 1920,
                .height = 1080,
                .framerate_numerator = 25,
                .framerate_denominator = 1
            };
            
            auto sdl_spec = spec.to_sdl();
            CHECK(sdl_spec.format == SDL_PIXELFORMAT_RGB24);
            CHECK(sdl_spec.width == 1920);
            CHECK(sdl_spec.height == 1080);
            CHECK(sdl_spec.framerate_numerator == 25);
            CHECK(sdl_spec.framerate_denominator == 1);
            
            // Convert back
            auto spec2 = sdlpp::camera_spec::from_sdl(sdl_spec);
            CHECK(spec2.format == spec.format);
            CHECK(spec2.width == spec.width);
            CHECK(spec2.height == spec.height);
            CHECK(spec2.framerate_numerator == spec.framerate_numerator);
            CHECK(spec2.framerate_denominator == spec.framerate_denominator);
        }
    }
}