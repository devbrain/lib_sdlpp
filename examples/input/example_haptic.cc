#include <sdlpp/input/haptic.hh>
#include <sdlpp/input/joystick.hh>
#include <sdlpp/core/core.hh>
#include <../include/sdlpp/core/timer.hh>
#include <../include/sdlpp/core/log.hh>

#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>

using namespace sdlpp;
using namespace std::chrono_literals;

// Helper to display haptic features
void display_haptic_info(const haptic& device) {
    std::cout << "\n=== Haptic Device Information ===\n";
    std::cout << "Name: " << device.get_name() << "\n";
    std::cout << "ID: " << device.get_id() << "\n";
    if (auto max_effects = device.get_max_effects()) {
        std::cout << "Max Effects: " << *max_effects << "\n";
    }
    if (auto max_playing = device.get_max_effects_playing()) {
        std::cout << "Max Playing: " << *max_playing << "\n";
    }
    std::cout << "Axes: " << device.get_num_axes() << "\n";
    
    auto features = device.get_features();
    std::cout << "\nSupported Effects:\n";
    
    if (has_flag(features, haptic_feature::constant))
        std::cout << "  [✓] Constant Force\n";
    if (has_flag(features, haptic_feature::sine))
        std::cout << "  [✓] Sine Wave\n";
    if (has_flag(features, haptic_feature::square))
        std::cout << "  [✓] Square Wave\n";
    if (has_flag(features, haptic_feature::triangle))
        std::cout << "  [✓] Triangle Wave\n";
    if (has_flag(features, haptic_feature::sawtoothup))
        std::cout << "  [✓] Sawtooth Up\n";
    if (has_flag(features, haptic_feature::sawtoothdown))
        std::cout << "  [✓] Sawtooth Down\n";
    if (has_flag(features, haptic_feature::ramp))
        std::cout << "  [✓] Ramp\n";
    if (has_flag(features, haptic_feature::spring))
        std::cout << "  [✓] Spring\n";
    if (has_flag(features, haptic_feature::damper))
        std::cout << "  [✓] Damper\n";
    if (has_flag(features, haptic_feature::inertia))
        std::cout << "  [✓] Inertia\n";
    if (has_flag(features, haptic_feature::friction))
        std::cout << "  [✓] Friction\n";
    if (has_flag(features, haptic_feature::leftright))
        std::cout << "  [✓] Left/Right Motors\n";
    if (has_flag(features, haptic_feature::custom))
        std::cout << "  [✓] Custom\n";
        
    std::cout << "\nCapabilities:\n";
    if (has_flag(features, haptic_feature::gain))
        std::cout << "  [✓] Gain Control\n";
    if (has_flag(features, haptic_feature::autocenter))
        std::cout << "  [✓] Autocenter\n";
    if (has_flag(features, haptic_feature::status))
        std::cout << "  [✓] Effect Status Query\n";
    if (has_flag(features, haptic_feature::pause))
        std::cout << "  [✓] Pause/Resume\n";
        
    bool rumble = device.is_rumble_supported();
    std::cout << "  [" << (rumble ? "✓" : "✗") << "] Simple Rumble\n";
}

// Demo different effect types
void demo_effects(haptic& device) {
    std::cout << "\n=== Effect Demonstrations ===\n";
    std::cout << "Press Enter after each effect...\n\n";
    
    // 1. Simple Rumble (if supported)
    if (device.is_rumble_supported()) {
        std::cout << "1. Simple Rumble Test\n";
        auto rumble_init = device.init_rumble();
        if (rumble_init) {
            std::cout << "   Playing light rumble (0.3 strength, 1 second)...\n";
            [[maybe_unused]] auto r1 = device.play_rumble(0.3f, 1000);
            std::this_thread::sleep_for(1200ms);
            
            std::cout << "   Playing strong rumble (0.8 strength, 1 second)...\n";
            [[maybe_unused]] auto r2 = device.play_rumble(0.8f, 1s);
            std::this_thread::sleep_for(1200ms);
            
            std::cout << "   Playing pulsing rumble...\n";
            for (int i = 0; i < 5; ++i) {
                [[maybe_unused]] auto r3 = device.play_rumble(0.6f, 200);
                std::this_thread::sleep_for(400ms);
            }
            
            [[maybe_unused]] auto r4 = device.stop_rumble();
        }
        std::cin.get();
    }
    
    // 2. Constant Force
    {
        haptic_constant effect;
        effect.direction = haptic_direction::polar(0); // North
        effect.length = 2000;
        effect.level = 0x4000; // Half strength
        effect.attack_length = 500;
        effect.attack_level = 0;
        effect.fade_length = 500;
        effect.fade_level = 0;
        
        if (device.is_effect_supported(effect)) {
            std::cout << "\n2. Constant Force (North direction, with envelope)\n";
            auto id = device.create_effect(effect);
            if (id) {
                haptic_effect_handle handle(&device, *id);
                std::cout << "   Playing constant force...\n";
                [[maybe_unused]] auto run_result = handle.run();
                std::cin.get();
                [[maybe_unused]] auto stop_result = handle.stop();
            }
        }
    }
    
    // 3. Sine Wave
    {
        haptic_periodic effect;
        effect.wave_type = haptic_feature::sine;
        effect.direction = haptic_direction::polar(18000); // South
        effect.period = 100; // 100ms period = 10Hz
        effect.magnitude = 0x6000;
        effect.length = 3000;
        effect.attack_length = 1000;
        effect.fade_length = 1000;
        
        if (device.is_effect_supported(effect)) {
            std::cout << "\n3. Sine Wave (10Hz, with attack/fade)\n";
            auto id = device.create_effect(effect);
            if (id) {
                haptic_effect_handle handle(&device, *id);
                std::cout << "   Playing sine wave...\n";
                [[maybe_unused]] auto run_result = handle.run();
                std::cin.get();
                [[maybe_unused]] auto stop_result = handle.stop();
            }
        }
    }
    
    // 4. Square Wave
    {
        haptic_periodic effect;
        effect.wave_type = haptic_feature::square;
        effect.direction = haptic_direction::polar(9000); // East
        effect.period = 200; // 5Hz
        effect.magnitude = 0x4000;
        effect.length = 2000;
        
        if (device.is_effect_supported(effect)) {
            std::cout << "\n4. Square Wave (5Hz)\n";
            auto id = device.create_effect(effect);
            if (id) {
                haptic_effect_handle handle(&device, *id);
                std::cout << "   Playing square wave...\n";
                [[maybe_unused]] auto run_result = handle.run();
                std::cin.get();
                [[maybe_unused]] auto stop_result = handle.stop();
            }
        }
    }
    
    // 5. Ramp Effect
    {
        haptic_ramp effect;
        effect.direction = haptic_direction::polar(27000); // West
        effect.length = 3000;
        effect.start = 0;
        effect.end = 0x7FFF; // Max strength
        
        if (device.is_effect_supported(effect)) {
            std::cout << "\n5. Ramp (increasing strength)\n";
            auto id = device.create_effect(effect);
            if (id) {
                haptic_effect_handle handle(&device, *id);
                std::cout << "   Playing ramp up...\n";
                [[maybe_unused]] auto run_result = handle.run();
                std::cin.get();
                [[maybe_unused]] auto stop_result = handle.stop();
            }
        }
    }
    
    // 6. Left/Right Motors
    {
        haptic_leftright effect;
        effect.length = 2000;
        effect.large_magnitude = 0x7FFF; // Full large motor
        effect.small_magnitude = 0x4000; // Half small motor
        
        if (device.is_effect_supported(effect)) {
            std::cout << "\n6. Left/Right Motors\n";
            auto id = device.create_effect(effect);
            if (id) {
                haptic_effect_handle handle(&device, *id);
                std::cout << "   Playing left/right effect...\n";
                [[maybe_unused]] auto run_result = handle.run();
                std::cin.get();
                [[maybe_unused]] auto stop_result = handle.stop();
            }
        }
    }
    
    // 7. Spring Effect (if supported)
    {
        haptic_condition effect;
        effect.condition_type = haptic_feature::spring;
        effect.length = haptic_infinity; // Infinite duration
        effect.right_coeff = {0x2000, 0x2000, 0x2000};
        effect.left_coeff = {0x2000, 0x2000, 0x2000};
        effect.center = {0, 0, 0};
        
        if (device.is_effect_supported(effect)) {
            std::cout << "\n7. Spring Effect (move your controller)\n";
            auto id = device.create_effect(effect);
            if (id) {
                haptic_effect_handle handle(&device, *id);
                std::cout << "   Spring effect active. Press Enter to stop...\n";
                [[maybe_unused]] auto run_result = handle.run(haptic_infinity);
                std::cin.get();
                [[maybe_unused]] auto stop_result = handle.stop();
            }
        }
    }
}

// Interactive menu
void interactive_menu(haptic& device) {
    std::cout << "\n=== Interactive Haptic Control ===\n";
    
    // Set initial gain to maximum
    [[maybe_unused]] auto gain_result = device.set_gain(100);
    
    while (true) {
        std::cout << "\nMenu:\n";
        std::cout << "1. Play rumble\n";
        std::cout << "2. Adjust gain\n";
        std::cout << "3. Toggle autocenter\n";
        std::cout << "4. Pause/Resume device\n";
        std::cout << "5. Stop all effects\n";
        std::cout << "0. Exit\n";
        std::cout << "\nChoice: ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        switch (choice) {
            case 1: {
                if (device.is_rumble_supported()) {
                    std::cout << "Strength (0.0-1.0): ";
                    float strength;
                    std::cin >> strength;
                    std::cout << "Duration (ms): ";
                    int duration;
                    std::cin >> duration;
                    std::cin.ignore();
                    
                    auto result = device.play_rumble(strength, static_cast<uint32_t>(duration));
                    if (!result) {
                        std::cout << "Error: " << result.error() << "\n";
                    }
                } else {
                    std::cout << "Rumble not supported!\n";
                }
                break;
            }
            
            case 2: {
                std::cout << "Gain (0-100): ";
                int gain;
                std::cin >> gain;
                std::cin.ignore();
                
                auto result = device.set_gain(gain);
                if (result) {
                    std::cout << "Gain set to " << gain << "%\n";
                } else {
                    std::cout << "Error: " << result.error() << "\n";
                }
                break;
            }
            
            case 3: {
                std::cout << "Autocenter (0-100, 0=off): ";
                int autocenter;
                std::cin >> autocenter;
                std::cin.ignore();
                
                auto result = device.set_autocenter(autocenter);
                if (result) {
                    std::cout << "Autocenter set to " << autocenter << "%\n";
                } else {
                    std::cout << "Error: " << result.error() << "\n";
                }
                break;
            }
            
            case 4: {
                static bool paused = false;
                if (!paused) {
                    auto result = device.pause();
                    if (result) {
                        std::cout << "Device paused\n";
                        paused = true;
                    } else {
                        std::cout << "Error: " << result.error() << "\n";
                    }
                } else {
                    auto result = device.resume();
                    if (result) {
                        std::cout << "Device resumed\n";
                        paused = false;
                    } else {
                        std::cout << "Error: " << result.error() << "\n";
                    }
                }
                break;
            }
            
            case 5: {
                auto result = device.stop_all_effects();
                if (result) {
                    std::cout << "All effects stopped\n";
                } else {
                    std::cout << "Error: " << result.error() << "\n";
                }
                break;
            }
            
            case 0:
                return;
                
            default:
                std::cout << "Invalid choice\n";
                break;
        }
    }
}

int main() {
    std::cout << "\n=== SDL++ Haptic Example ===\n\n";
    
    // Initialize SDL with haptic and joystick support
    auto init_result = init(init_flags::haptic | init_flags::joystick);
    if (!init_result) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to initialize SDL");
        return 1;
    }
    
    // List all haptic devices
    std::cout << "Available Haptic Devices:\n";
    auto haptic_devices = get_haptics();
    
    std::vector<std::unique_ptr<haptic>> available_haptics;
    
    // 1. Standalone haptic devices
    for (size_t i = 0; i < haptic_devices.size(); ++i) {
        auto name = get_haptic_name_for_id(haptic_devices[i]);
        std::cout << "  [" << i << "] " << name << " (ID: " << haptic_devices[i] << ")\n";
    }
    
    // 2. Check mouse
    if (is_mouse_haptic()) {
        std::cout << "  [M] Mouse (haptic capable)\n";
    }
    
    // 3. Check joysticks
    auto joysticks = get_joysticks();
    std::vector<std::unique_ptr<joystick>> haptic_joysticks;
    
    for (auto joy_id : joysticks) {
        auto joy = joystick::open(joy_id);
        if (joy && is_joystick_haptic(*joy)) {
            std::cout << "  [J" << haptic_joysticks.size() << "] " 
                     << joy->get_name() << " (Joystick)\n";
            haptic_joysticks.push_back(std::make_unique<joystick>(std::move(*joy)));
        }
    }
    
    if (haptic_devices.empty() && !is_mouse_haptic() && haptic_joysticks.empty()) {
        std::cout << "\nNo haptic devices found!\n";
        std::cout << "Please connect a game controller with force feedback support.\n";
        return 0;
    }
    
    // Select device
    std::cout << "\nSelect device (number, M for mouse, J# for joystick): ";
    std::string selection;
    std::cin >> selection;
    std::cin.ignore();
    
    std::unique_ptr<haptic> device;
    
    if (selection == "M" || selection == "m") {
        auto result = haptic::open_from_mouse();
        if (result) {
            device = std::make_unique<haptic>(std::move(*result));
        }
    } else if (selection[0] == 'J' || selection[0] == 'j') {
        size_t joy_idx = std::stoul(selection.substr(1));
        if (joy_idx < haptic_joysticks.size()) {
            auto result = haptic::open_from_joystick(*haptic_joysticks[joy_idx]);
            if (result) {
                device = std::make_unique<haptic>(std::move(*result));
            }
        }
    } else {
        size_t idx = std::stoul(selection);
        if (idx < haptic_devices.size()) {
            auto result = haptic::open(haptic_devices[idx]);
            if (result) {
                device = std::make_unique<haptic>(std::move(*result));
            }
        }
    }
    
    if (!device || !device->is_valid()) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to open haptic device");
        return 1;
    }
    
    // Display device info
    display_haptic_info(*device);
    
    // Initialize rumble if supported
    if (device->is_rumble_supported()) {
        [[maybe_unused]] auto rumble_init = device->init_rumble();
    }
    
    // Menu
    std::cout << "\n\nWhat would you like to do?\n";
    std::cout << "1. Run effect demonstrations\n";
    std::cout << "2. Interactive control\n";
    std::cout << "0. Exit\n";
    std::cout << "\nChoice: ";
    
    int choice;
    std::cin >> choice;
    std::cin.ignore();
    
    switch (choice) {
        case 1:
            demo_effects(*device);
            break;
        case 2:
            interactive_menu(*device);
            break;
        default:
            break;
    }
    
    std::cout << "\nGoodbye!\n";
    return 0;
}