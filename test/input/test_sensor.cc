#include <doctest/doctest.h>
#include <sdlpp/input/sensor.hh>
#include <sdlpp/core/core.hh>
#include <iostream>
#include <iomanip>

TEST_SUITE("sensor") {
    TEST_CASE("sensor enumeration") {
        // Initialize SDL with sensor support
        auto init = sdlpp::init(sdlpp::init_flags::sensor);
        if (init) {
            SUBCASE("get sensors") {
                auto sensors = sdlpp::get_sensors();
                // May be empty if no sensors available
                CHECK(sensors.size() >= 0);
                
                std::cout << "Found " << sensors.size() << " sensor(s):\n";
                
                for (auto id : sensors) {
                    // Test sensor info before opening
                    auto name = sdlpp::get_sensor_name_for_id(id);
                    auto type = sdlpp::get_sensor_type_for_id(id);
                    auto non_portable = sdlpp::get_sensor_non_portable_type_for_id(id);
                    
                    std::cout << "Sensor " << id << ":\n";
                    std::cout << "  Name: " << name << "\n";
                    std::cout << "  Type: ";
                    
                    switch (type) {
                        case sdlpp::sensor_type::accel:
                            std::cout << "Accelerometer";
                            break;
                        case sdlpp::sensor_type::gyro:
                            std::cout << "Gyroscope";
                            break;
                        case sdlpp::sensor_type::accel_l:
                            std::cout << "Left Accelerometer";
                            break;
                        case sdlpp::sensor_type::gyro_l:
                            std::cout << "Left Gyroscope";
                            break;
                        case sdlpp::sensor_type::accel_r:
                            std::cout << "Right Accelerometer";
                            break;
                        case sdlpp::sensor_type::gyro_r:
                            std::cout << "Right Gyroscope";
                            break;
                        case sdlpp::sensor_type::unknown:
                            std::cout << "Unknown";
                            break;
                        default:
                            std::cout << "Invalid";
                            break;
                    }
                    std::cout << "\n";
                    std::cout << "  Non-portable type: " << non_portable << "\n";
                    
                    CHECK(!name.empty());
                    CHECK(type != sdlpp::sensor_type::invalid);
                }
            }
            
            SUBCASE("open sensor") {
                auto sensors = sdlpp::get_sensors();
                if (!sensors.empty()) {
                    auto id = sensors[0];
                    
                    auto sensor_result = sdlpp::sensor::open(id);
                    CHECK(sensor_result.has_value());
                    
                    if (sensor_result) {
                        auto& sensor = sensor_result.value();
                        
                        // Test sensor properties
                        CHECK(sensor.is_valid());
                        CHECK(sensor.get() != nullptr);
                        CHECK(sensor.get_id() == id);
                        
                        auto name = sensor.get_name();
                        CHECK(!name.empty());
                        
                        auto type = sensor.get_type();
                        CHECK(type != sdlpp::sensor_type::invalid);
                        
                        auto non_portable = sensor.get_non_portable_type();
                        CHECK(non_portable >= -1);
                        
                        auto properties = sensor.get_properties();
                        CHECK(properties >= 0);
                        
                        // Test type checking methods
                        bool is_accel = sensor.is_accelerometer();
                        bool is_gyro = sensor.is_gyroscope();
                        CHECK((is_accel || is_gyro || type == sdlpp::sensor_type::unknown));
                        
                        // Test data reading
                        if (is_accel || is_gyro) {
                            auto data3 = sensor.get_data_3();
                            CHECK(data3.has_value());
                            
                            if (data3) {
                                std::cout << "  Data (3 values): ";
                                for (float v : data3.value()) {
                                    std::cout << std::fixed << std::setprecision(3) << v << " ";
                                }
                                std::cout << "\n";
                            }
                        }
                        
                        // Test 6-value read (may fail for standard sensors)
                        auto data6 = sensor.get_data_6();
                        if (data6) {
                            std::cout << "  Data (6 values): ";
                            for (float v : data6.value()) {
                                std::cout << std::fixed << std::setprecision(3) << v << " ";
                            }
                            std::cout << "\n";
                        }
                    }
                }
            }
            
            SUBCASE("sensor from ID") {
                auto sensors = sdlpp::get_sensors();
                if (!sensors.empty()) {
                    auto id = sensors[0];
                    
                    // Open sensor
                    auto sensor_result = sdlpp::sensor::open(id);
                    if (sensor_result) {
                        // Try to get it by ID
                        auto* found = sdlpp::get_sensor_from_id(id);
                        CHECK(found != nullptr);
                        CHECK(found == sensor_result.value().get());
                    }
                }
            }
            
            SUBCASE("sensor manager") {
                sdlpp::sensor_manager manager;
                
                // Test open all
                auto opened = manager.open_all();
                CHECK(opened >= 0);
                CHECK(manager.get_sensors().size() == static_cast<size_t>(opened));
                
                // Test find by type
                if (auto* accel = manager.find_by_type(sdlpp::sensor_type::accel)) {
                    CHECK(accel->get_type() == sdlpp::sensor_type::accel);
                }
                
                // Test close all
                manager.close_all();
                CHECK(manager.get_sensors().empty());
                
                // Test open specific type
                auto accel_count = manager.open_all_of_type(sdlpp::sensor_type::accel);
                CHECK(accel_count >= 0);
            }
            
            SUBCASE("accelerometer data helper") {
                // Test with sample data
                std::array<float, 3> sample_data = {0.0f, 9.8f, 0.0f};
                sdlpp::accelerometer_data accel(sample_data);
                
                CHECK(accel.x() == 0.0f);
                CHECK(accel.y() == 9.8f);
                CHECK(accel.z() == 0.0f);
                
                // Test magnitude (should be close to standard gravity)
                float mag = accel.magnitude();
                CHECK(mag > 9.0f);
                CHECK(mag < 10.0f);
                
                // Test at rest detection
                CHECK(accel.is_at_rest(1.0f));
                
                // Test with motion data
                std::array<float, 3> motion_data = {5.0f, 9.8f, 3.0f};
                sdlpp::accelerometer_data moving(motion_data);
                CHECK(!moving.is_at_rest(1.0f));
            }
            
            SUBCASE("gyroscope data helper") {
                // Test with sample data
                std::array<float, 3> sample_data = {0.01f, -0.02f, 0.005f};
                sdlpp::gyroscope_data gyro(sample_data);
                
                CHECK(gyro.pitch() == 0.01f);
                CHECK(gyro.yaw() == -0.02f);
                CHECK(gyro.roll() == 0.005f);
                
                // Test magnitude
                float mag = gyro.magnitude();
                CHECK(mag > 0.0f);
                
                // Test stationary detection
                CHECK(!gyro.is_stationary(0.001f));
                CHECK(gyro.is_stationary(0.1f));
                
                // Test with no rotation
                std::array<float, 3> still_data = {0.0f, 0.0f, 0.0f};
                sdlpp::gyroscope_data still(still_data);
                CHECK(still.is_stationary());
            }
            
            // Call update (won't do anything without events enabled)
            sdlpp::update_sensors();
        }
    }
    
    TEST_CASE("sensor constants") {
        // Test standard gravity constant
        CHECK(sdlpp::standard_gravity == SDL_STANDARD_GRAVITY);
        CHECK(sdlpp::standard_gravity > 9.0f);
        CHECK(sdlpp::standard_gravity < 10.0f);
    }
}