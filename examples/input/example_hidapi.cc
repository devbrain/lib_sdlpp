#include <sdlpp/input/hidapi.hh>
#include <sdlpp/core/version.hh>
#include <../include/sdlpp/core/log.hh>
#include <../include/sdlpp/core/timer.hh>

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <chrono>
#include <thread>
#include <cstring>

using namespace sdlpp;

// Helper to convert wide string to narrow string (basic ASCII only)
std::string wstring_to_string(const std::wstring& wstr) {
    std::string result;
    for (wchar_t ch : wstr) {
        if (ch < 128) {
            result += static_cast<char>(ch);
        } else {
            result += '?';
        }
    }
    return result;
}

// Helper to print hex dump
void print_hex_dump(const uint8_t* data, size_t size) {
    for (size_t i = 0; i < size; i += 16) {
        std::cout << "  " << std::hex << std::setw(4) << std::setfill('0') << i << ": ";
        
        // Hex bytes
        for (size_t j = 0; j < 16 && i + j < size; ++j) {
            std::cout << std::setw(2) << std::setfill('0') 
                     << static_cast<unsigned>(data[i + j]) << " ";
        }
        
        // Padding
        for (size_t j = size - i; j < 16; ++j) {
            std::cout << "   ";
        }
        
        std::cout << " ";
        
        // ASCII representation
        for (size_t j = 0; j < 16 && i + j < size; ++j) {
            char ch = static_cast<char>(data[i + j]);
            if (ch >= 32 && ch < 127) {
                std::cout << ch;
            } else {
                std::cout << '.';
            }
        }
        
        std::cout << "\n";
    }
    std::cout << std::dec;
}

// Interactive device monitor
void monitor_device(hid_device& device) {
    std::cout << "\n=== Device Monitor Mode ===\n";
    std::cout << "Commands:\n";
    std::cout << "  r - Read data (blocking)\n";
    std::cout << "  n - Read data (non-blocking)\n";
    std::cout << "  f - Get feature report\n";
    std::cout << "  i - Get device info\n";
    std::cout << "  s - Get device strings\n";
    std::cout << "  q - Quit monitor\n\n";
    
    // Set to non-blocking mode initially
    auto nb_result = device.set_nonblocking(true);
    if (!nb_result) {
        std::cout << "Warning: Failed to set non-blocking mode: " << nb_result.error() << "\n";
    }
    
    bool running = true;
    while (running) {
        std::cout << "> ";
        std::cout.flush();
        
        char cmd;
        std::cin >> cmd;
        std::cin.ignore(); // Clear newline
        
        switch (cmd) {
            case 'r': {
                // Blocking read
                std::cout << "Setting blocking mode...\n";
                if (auto res = device.set_nonblocking(false); !res) {
                    std::cerr << "Failed to set blocking mode: " << res.error() << "\n";
                    return;
                }
                
                std::cout << "Waiting for data (press Ctrl+C to interrupt)...\n";
                std::array<uint8_t, 256> buffer{};
                auto read_result = device.read(buffer);
                
                if (read_result) {
                    if (*read_result > 0) {
                        std::cout << "Read " << *read_result << " bytes:\n";
                        print_hex_dump(buffer.data(), static_cast<size_t>(*read_result));
                    } else {
                        std::cout << "No data available\n";
                    }
                } else {
                    std::cout << "Read error: " << read_result.error() << "\n";
                }
                
                [[maybe_unused]] auto nb_res = device.set_nonblocking(true);
                break;
            }
            
            case 'n': {
                // Non-blocking read
                std::cout << "Performing non-blocking read...\n";
                std::array<uint8_t, 256> buffer{};
                auto read_result = device.read(buffer);
                
                if (read_result) {
                    if (*read_result > 0) {
                        std::cout << "Read " << *read_result << " bytes:\n";
                        print_hex_dump(buffer.data(), static_cast<size_t>(*read_result));
                    } else {
                        std::cout << "No data available\n";
                    }
                } else {
                    std::cout << "Read error: " << read_result.error() << "\n";
                }
                break;
            }
            
            case 'f': {
                // Get feature report
                std::cout << "Enter report ID (0 for devices with single report): ";
                int report_id;
                std::cin >> report_id;
                std::cin.ignore();
                
                std::array<uint8_t, 256> buffer{};
                buffer[0] = static_cast<uint8_t>(report_id);
                
                auto result = device.get_feature_report(buffer);
                if (result) {
                    std::cout << "Got feature report, " << *result << " bytes:\n";
                    print_hex_dump(buffer.data(), static_cast<size_t>(*result));
                } else {
                    std::cout << "Failed to get feature report: " << result.error() << "\n";
                }
                break;
            }
            
            case 'i': {
                // Get device info
                auto info_result = device.get_device_info();
                if (info_result) {
                    const auto& info = *info_result;
                    std::cout << "\nDevice Information:\n";
                    std::cout << "  Path: " << info.path << "\n";
                    std::cout << "  VID: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                             << info.vendor_id << "\n";
                    std::cout << "  PID: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                             << info.product_id << "\n";
                    std::cout << std::dec;
                    std::cout << "  Release: " << info.release_number << "\n";
                    std::cout << "  Interface: " << info.interface_number << "\n";
                    std::cout << "  Usage Page: 0x" << std::hex << info.usage_page << "\n";
                    std::cout << "  Usage: 0x" << std::hex << info.usage << "\n";
                    std::cout << std::dec;
                } else {
                    std::cout << "Failed to get device info: " << info_result.error() << "\n";
                }
                break;
            }
            
            case 's': {
                // Get device strings
                auto mfr = device.get_manufacturer_string();
                if (mfr) {
                    std::cout << "Manufacturer: " << *mfr << "\n";
                }
                
                auto prod = device.get_product_string();
                if (prod) {
                    std::cout << "Product: " << *prod << "\n";
                }
                
                auto serial = device.get_serial_number_string();
                if (serial) {
                    std::cout << "Serial: " << *serial << "\n";
                }
                break;
            }
            
            case 'q':
                running = false;
                break;
                
            default:
                std::cout << "Unknown command\n";
                break;
        }
    }
}

int main() {
    std::cout << "\n=== SDL++ HID API Example ===\n\n";
    
    // Check if HID API is available
    if (!version_info::features::has_hidapi) {
        std::cout << "HID API is not available in this SDL build.\n";
        std::cout << "SDL was compiled with SDL_HIDAPI_DISABLED.\n";
        return 1;
    }
    
    // Initialize HID API
    std::cout << "Initializing HID API...\n";
    hid_context ctx;
    
    // Check device changes
    auto change_count = hid_device_change_count();
    std::cout << "Device change counter: " << change_count << "\n";
    
    // Enumerate all HID devices
    std::cout << "\nEnumerating HID devices...\n";
    auto devices = hid_enumerate();
    
    if (devices.empty()) {
        std::cout << "No HID devices found.\n";
        std::cout << "\nNote: By default SDL only enumerates game controllers.\n";
        std::cout << "Set SDL_HINT_HIDAPI_ENUMERATE_ONLY_CONTROLLERS to \"0\"\n";
        std::cout << "to enumerate all HID devices.\n";
        return 0;
    }
    
    std::cout << "Found " << devices.size() << " device(s):\n\n";
    
    // Display device list
    for (size_t i = 0; i < devices.size(); ++i) {
        const auto& dev = devices[i];
        std::cout << "[" << i << "] ";
        
        // Try to display product name if available
        if (!dev.product_string.empty()) {
            std::cout << dev.product_string;
        } else {
            std::cout << "Unknown Device";
        }
        
        std::cout << " (VID: 0x" << std::hex << std::setw(4) << std::setfill('0') 
                 << dev.vendor_id;
        std::cout << ", PID: 0x" << std::setw(4) << std::setfill('0') 
                 << dev.product_id << ")" << std::dec;
        
        // Show bus type
        std::cout << " [";
        switch (dev.bus_type) {
            case hid_bus_type::usb:
                std::cout << "USB";
                break;
            case hid_bus_type::bluetooth:
                std::cout << "Bluetooth";
                break;
            case hid_bus_type::i2c:
                std::cout << "I2C";
                break;
            case hid_bus_type::spi:
                std::cout << "SPI";
                break;
            default:
                std::cout << "Unknown";
                break;
        }
        std::cout << "]\n";
        
        // Show additional details
        if (!dev.manufacturer_string.empty()) {
            std::cout << "    Manufacturer: " << dev.manufacturer_string << "\n";
        }
        if (!dev.serial_number.empty()) {
            std::cout << "    Serial: " << dev.serial_number << "\n";
        }
        if (dev.usage_page != 0 || dev.usage != 0) {
            std::cout << "    Usage: 0x" << std::hex << dev.usage_page 
                     << "/0x" << dev.usage << std::dec << "\n";
        }
        std::cout << "    Path: " << dev.path << "\n";
        std::cout << "\n";
    }
    
    // Ask user to select a device
    std::cout << "Enter device number to open (or -1 to quit): ";
    int selection;
    std::cin >> selection;
    
    if (selection < 0 || selection >= static_cast<int>(devices.size())) {
        std::cout << "Exiting.\n";
        return 0;
    }
    
    // Open the selected device
    const auto& selected = devices[static_cast<size_t>(selection)];
    std::cout << "\nOpening device...\n";
    
    auto device = hid_device::open_path(selected.path);
    if (!device) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to open device: ", device.error());
        return 1;
    }
    
    std::cout << "Device opened successfully!\n";
    
    // Get and display device information
    auto info = device->get_device_info();
    if (info) {
        std::cout << "\nDevice Details:\n";
        
        auto mfr = device->get_manufacturer_string();
        if (mfr) {
            std::cout << "  Manufacturer: " << *mfr << "\n";
        }
        
        auto prod = device->get_product_string();
        if (prod) {
            std::cout << "  Product: " << *prod << "\n";
        }
        
        auto serial = device->get_serial_number_string();
        if (serial) {
            std::cout << "  Serial Number: " << *serial << "\n";
        }
    }
    
    // Try to get report descriptor
    std::cout << "\nAttempting to get report descriptor...\n";
    std::array<uint8_t, 4096> descriptor{};
    auto desc_result = device->get_report_descriptor(descriptor);
    if (desc_result && *desc_result > 0) {
        std::cout << "Report descriptor (" << *desc_result << " bytes):\n";
        print_hex_dump(descriptor.data(), static_cast<size_t>(*desc_result));
    } else {
        std::cout << "Could not get report descriptor.\n";
    }
    
    // Enter interactive mode
    monitor_device(*device);
    
    std::cout << "\nClosing device...\n";
    std::cout << "Goodbye!\n";
    
    return 0;
}