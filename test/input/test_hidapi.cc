#include <doctest/doctest.h>
#include <sdlpp/input/hidapi.hh>
#include <sdlpp/core/version.hh>
#include <iostream>
#include <iomanip>
#include <array>
#include <string>
#include <exception>

TEST_SUITE("hidapi") {
    TEST_CASE("HID API availability") {
        std::cout << "HID API available: " 
                  << (sdlpp::version_info::features::has_hidapi ? "YES" : "NO") << "\n";
        
#ifndef SDL_HIDAPI_DISABLED
        CHECK(sdlpp::version_info::features::has_hidapi == true);
#else
        CHECK(sdlpp::version_info::features::has_hidapi == false);
#endif
    }
    
#ifndef SDL_HIDAPI_DISABLED
    TEST_CASE("HID initialization") {
        SUBCASE("init and exit") {
            auto init_result = sdlpp::hid_init();
            if (!init_result) {
                std::string message = "HID init failed: " + init_result.error();
                MESSAGE(message);
                return;
            }

            auto exit_result = sdlpp::hid_exit();
            CHECK(exit_result.has_value());
        }
        
        SUBCASE("RAII context") {
            try {
                sdlpp::hid_context ctx;
                CHECK(true);
            } catch (const std::exception& ex) {
                MESSAGE("Skipping HID test: ", ex.what());
                return;
            }
        }
        
        SUBCASE("device change count") {
            auto init_result = sdlpp::hid_init();
            if (!init_result) {
                std::string message = "HID init failed: " + init_result.error();
                MESSAGE(message);
                return;
            }
            auto count = sdlpp::hid_device_change_count();
            // Just check it doesn't crash - value depends on platform
            CHECK(count >= 0);
            auto exit_result = sdlpp::hid_exit();
            CHECK(exit_result.has_value());
        }
    }
    
    TEST_CASE("HID enumeration") {
        try {
            // Initialize HID API for this test
            sdlpp::hid_context ctx;
        
            SUBCASE("enumerate all devices") {
                auto devices = sdlpp::hid_enumerate();
            
                std::cout << "\nFound " << devices.size() << " HID device(s):\n";
            
                for (const auto& dev : devices) {
                    std::cout << "\nDevice:\n";
                    std::cout << "  Path: " << dev.path << "\n";
                    std::cout << "  VID: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                             << dev.vendor_id << "\n";
                    std::cout << "  PID: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                             << dev.product_id << "\n";
                    std::cout << std::dec;
                
                    // Now these are UTF-8 strings
                    std::cout << "  Serial: " << (dev.serial_number.empty() ? "(none)" : dev.serial_number) << "\n";
                    std::cout << "  Manufacturer: " << (dev.manufacturer_string.empty() ? "(none)" : dev.manufacturer_string) << "\n";
                    std::cout << "  Product: " << (dev.product_string.empty() ? "(none)" : dev.product_string) << "\n";
                
                    std::cout << "  Release: " << dev.release_number << "\n";
                    std::cout << "  Usage Page: 0x" << std::hex << dev.usage_page << "\n";
                    std::cout << "  Usage: 0x" << std::hex << dev.usage << "\n";
                    std::cout << std::dec;
                    std::cout << "  Interface: " << dev.interface_number << "\n";
                
                    std::cout << "  Bus Type: ";
                    switch (dev.bus_type) {
                        case sdlpp::hid_bus_type::unknown:
                            std::cout << "Unknown";
                            break;
                        case sdlpp::hid_bus_type::usb:
                            std::cout << "USB";
                            break;
                        case sdlpp::hid_bus_type::bluetooth:
                            std::cout << "Bluetooth";
                            break;
                        case sdlpp::hid_bus_type::i2c:
                            std::cout << "I2C";
                            break;
                        case sdlpp::hid_bus_type::spi:
                            std::cout << "SPI";
                            break;
                    }
                    std::cout << "\n";
                }
            
                // Just check it doesn't crash - device count varies
                CHECK(devices.size() >= 0);
            }
        
            SUBCASE("enumerate specific vendor") {
                // Try to enumerate devices from a common vendor (e.g., Microsoft)
                auto devices = sdlpp::hid_enumerate(0x045E, 0);
            
                // Check all returned devices have the correct vendor ID
                for (const auto& dev : devices) {
                    CHECK(dev.vendor_id == 0x045E);
                }
            }
        } catch (const std::exception& ex) {
            MESSAGE("Skipping HID test: ", ex.what());
            return;
        }
    }
    
    TEST_CASE("HID device operations") {
        try {
            sdlpp::hid_context ctx;
        
            SUBCASE("open non-existent device") {
                // Try to open a device that probably doesn't exist
                auto device = sdlpp::hid_device::open(0xFFFF, 0xFFFF);
                CHECK(!device.has_value());
            }
        
            SUBCASE("open by invalid path") {
                auto device = sdlpp::hid_device::open_path("/invalid/path/to/device");
                CHECK(!device.has_value());
            }
        
            SUBCASE("invalid device operations") {
                sdlpp::hid_device device; // Empty device
                CHECK(!device.is_valid());
                CHECK(!device);
            
                // Test operations on invalid device
                std::array<uint8_t, 64> buffer{};
            
                auto write_result = device.write(buffer);
                CHECK(!write_result.has_value());
            
                auto read_result = device.read(buffer);
                CHECK(!read_result.has_value());
            
                auto nonblock_result = device.set_nonblocking(true);
                CHECK(!nonblock_result.has_value());
            
                auto feature_result = device.send_feature_report(buffer);
                CHECK(!feature_result.has_value());
            
                auto get_feature_result = device.get_feature_report(buffer);
                CHECK(!get_feature_result.has_value());
            
                auto info_result = device.get_device_info();
                CHECK(!info_result.has_value());
            }
        
            SUBCASE("device string operations") {
                sdlpp::hid_device device; // Empty device
            
                auto mfr_result = device.get_manufacturer_string();
                CHECK(!mfr_result.has_value());
            
                auto prod_result = device.get_product_string();
                CHECK(!prod_result.has_value());
            
                auto serial_result = device.get_serial_number_string();
                CHECK(!serial_result.has_value());
            
                auto indexed_result = device.get_indexed_string(1);
                CHECK(!indexed_result.has_value());
            }
        
            // If we have any real devices, we could test opening them
            // But we can't assume any specific device is present
            SUBCASE("open first available device") {
                auto devices = sdlpp::hid_enumerate();
                if (!devices.empty()) {
                    std::cout << "\nTrying to open first device...\n";
                
                    const auto& first = devices[0];
                    auto device = sdlpp::hid_device::open_path(first.path);
                
                    if (device) {
                        CHECK(device->is_valid());
                    
                        // Try to get device info
                        auto info_result = device->get_device_info();
                        if (info_result) {
                            CHECK(info_result->vendor_id == first.vendor_id);
                            CHECK(info_result->product_id == first.product_id);
                        }
                    
                        // Try to get strings
                        auto mfr_result = device->get_manufacturer_string();
                        if (mfr_result) {
                            std::cout << "  Manufacturer string retrieved\n";
                        }
                    
                        auto prod_result = device->get_product_string();
                        if (prod_result) {
                            std::cout << "  Product string retrieved\n";
                        }
                    
                        // Try to set non-blocking mode
                        auto nonblock_result = device->set_nonblocking(true);
                        CHECK(nonblock_result.has_value());
                    
                        // Try a non-blocking read (should return 0 if no data)
                        std::array<uint8_t, 64> buffer{};
                        auto read_result = device->read(buffer);
                        if (read_result) {
                            std::cout << "  Read returned " << *read_result << " bytes\n";
                        }
                    } else {
                        std::cout << "  Failed to open device: " << device.error() << "\n";
                    }
                }
            }
        } catch (const std::exception& ex) {
            MESSAGE("Skipping HID test: ", ex.what());
            return;
        }
    }
    
    TEST_CASE("HID BLE scan") {
        // This is iOS/tvOS specific, but we can test that it doesn't crash
        sdlpp::hid_ble_scan(true);
        sdlpp::hid_ble_scan(false);
        CHECK(true); // Just verify no crash
    }
    
#else // SDL_HIDAPI_DISABLED
    
    TEST_CASE("HID API disabled behavior") {
        SUBCASE("init returns error") {
            auto result = sdlpp::hid_init();
            CHECK(!result.has_value());
            CHECK(result.error() == "HID API is disabled in this SDL build");
        }
        
        SUBCASE("enumerate returns empty") {
            auto devices = sdlpp::hid_enumerate();
            CHECK(devices.empty());
        }
        
        SUBCASE("device is invalid") {
            sdlpp::hid_device device;
            CHECK(!device.is_valid());
        }
    }
    
#endif // SDL_HIDAPI_DISABLED
}
