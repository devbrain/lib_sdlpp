#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/ui/tray.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/video/pixels.hh>

// Simple function to create a colored icon
sdlpp::surface create_icon(Uint8 r, Uint8 g, Uint8 b) {
    // Create a 32x32 icon
    auto surface_result = sdlpp::surface::create_rgb(32, 32, sdlpp::pixel_format_enum::RGBA8888);
    if (!surface_result) {
        throw std::runtime_error("Failed to create icon surface");
    }
    
    auto& surf = surface_result.value();
    
    // Fill with color
    sdlpp::color fill_color{r, g, b, 255};
    [[maybe_unused]] auto fill_result = surf.fill(fill_color);
    
    // Add a simple border
    sdlpp::rect_i border{0, 0, 32, 32};
    sdlpp::color black{0, 0, 0, 255};
    for (int i = 0; i < 2; ++i) {
        [[maybe_unused]] auto border_result = surf.fill_rect(border, black);
        border.x++; border.y++;
        border.w -= 2; border.h -= 2;
    }
    
    return std::move(surf);
}

void basic_tray_example() {
    std::cout << "\n=== Basic Tray Example ===\n";
    
    // Create a simple icon
    auto icon = create_icon(255, 128, 0);  // Orange icon
    
    // Create system tray
    auto tray_result = sdlpp::tray::create(icon, "SDL++ Tray Example");
    if (!tray_result) {
        std::cerr << "Failed to create tray: " << tray_result.error() << "\n";
        return;
    }
    
    auto& tray = tray_result.value();
    auto& menu = tray.get_menu();
    
    std::atomic<bool> running{true};
    std::atomic<int> click_count{0};
    
    // Add menu items
    [[maybe_unused]] auto hello_result = menu.add_item("Hello World", []([[maybe_unused]] auto& entry) {
        std::cout << "Hello World clicked!\n";
    });
    
    [[maybe_unused]] auto sep1_result = menu.add_separator();
    
    // Add a checkable item
    auto check_result = menu.add_item("Enable Feature", 
        [](auto& entry) {
            bool checked = entry.is_checked();
            [[maybe_unused]] auto check_set_result = entry.set_checked(!checked);
            std::cout << "Feature " << (!checked ? "enabled" : "disabled") << "\n";
        });
    
    if (!check_result) {
        std::cerr << "Failed to add checkable item\n";
    }
    
    // Add a counter item
    auto counter_result = menu.add_item("Click Counter: 0", 
        [&click_count](auto& entry) {
            click_count++;
            std::string label = "Click Counter: " + std::to_string(click_count);
            [[maybe_unused]] auto label_result = entry.set_label(label);
            std::cout << "Counter clicked: " << click_count << "\n";
        });
    
    if (!counter_result) {
        std::cerr << "Failed to add counter item\n";
    }
    
    [[maybe_unused]] auto sep2_result = menu.add_separator();
    
    // Add quit item
    [[maybe_unused]] auto quit_result = menu.add_item("Quit", [&running]([[maybe_unused]] auto& entry) {
        std::cout << "Quit selected\n";
        running = false;
    });
    
    // Run for a short time
    std::cout << "Tray icon created. Running for 10 seconds...\n";
    std::cout << "Right-click the tray icon to see the menu.\n";
    
    auto start = std::chrono::steady_clock::now();
    while (running && std::chrono::steady_clock::now() - start < std::chrono::seconds(10)) {
        sdlpp::update_trays();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void submenu_example() {
    std::cout << "\n=== Submenu Example ===\n";
    
    auto icon = create_icon(0, 255, 128);  // Green icon
    
    auto tray_result = sdlpp::tray::create(icon, "Submenu Example");
    if (!tray_result) {
        std::cerr << "Failed to create tray: " << tray_result.error() << "\n";
        return;
    }
    
    auto& tray = tray_result.value();
    auto& menu = tray.get_menu();
    
    std::atomic<bool> running{true};
    
    // Add main menu items
    [[maybe_unused]] auto main_result = menu.add_item("Main Action", []([[maybe_unused]] auto&) {
        std::cout << "Main action executed\n";
    });
    
    [[maybe_unused]] auto sep1_result = menu.add_separator();
    
    // Add submenu for settings
    auto settings_result = menu.add_submenu("Settings");
    if (settings_result) {
        auto& submenu = settings_result.value();
        
        [[maybe_unused]] auto audio_result = submenu.add_item("Audio Settings", []([[maybe_unused]] auto&) {
            std::cout << "Audio settings selected\n";
        });
        
        [[maybe_unused]] auto video_result = submenu.add_item("Video Settings", []([[maybe_unused]] auto&) {
            std::cout << "Video settings selected\n";
        });
        
        [[maybe_unused]] auto input_result = submenu.add_item("Input Settings", []([[maybe_unused]] auto&) {
            std::cout << "Input settings selected\n";
        });
        
        [[maybe_unused]] auto sub_sep_result = submenu.add_separator();
        
        [[maybe_unused]] auto reset_result = submenu.add_item("Reset to Defaults", []([[maybe_unused]] auto&) {
            std::cout << "Settings reset to defaults\n";
        });
    }
    
    // Add submenu for recent files
    auto recent_result = menu.add_submenu("Recent Files");
    if (recent_result) {
        auto& submenu = recent_result.value();
        
        for (int i = 1; i <= 5; ++i) {
            std::string filename = "document" + std::to_string(i) + ".txt";
            [[maybe_unused]] auto file_result = submenu.add_item(filename, [filename]([[maybe_unused]] auto& entry) {
                std::cout << "Opening recent file: " << filename << "\n";
            });
        }
        
        [[maybe_unused]] auto recent_sep_result = submenu.add_separator();
        
        [[maybe_unused]] auto clear_result = submenu.add_item("Clear Recent", []([[maybe_unused]] auto&) {
            std::cout << "Clearing recent files\n";
            // Note: Can't disable parent menu from here in current API
        });
    }
    
    [[maybe_unused]] auto sep2_result = menu.add_separator();
    
    [[maybe_unused]] auto exit_result = menu.add_item("Exit", [&running]([[maybe_unused]] auto& entry) {
        running = false;
    });
    
    // Run for a short time
    std::cout << "Tray with submenus created. Running for 10 seconds...\n";
    
    auto start = std::chrono::steady_clock::now();
    while (running && std::chrono::steady_clock::now() - start < std::chrono::seconds(10)) {
        sdlpp::update_trays();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void dynamic_tray_example() {
    std::cout << "\n=== Dynamic Tray Example ===\n";
    
    auto icon_red = create_icon(255, 0, 0);
    auto icon_green = create_icon(0, 255, 0);
    auto icon_blue = create_icon(0, 0, 255);
    
    auto tray_result = sdlpp::tray::create(icon_red, "Dynamic Tray");
    if (!tray_result) {
        std::cerr << "Failed to create tray: " << tray_result.error() << "\n";
        return;
    }
    
    auto& tray = tray_result.value();
    auto& menu = tray.get_menu();
    
    std::atomic<bool> running{true};
    [[maybe_unused]] int color_index = 0;
    
    // Add color changing items
    [[maybe_unused]] auto red_result = menu.add_item("Red Icon", [&tray, &icon_red]([[maybe_unused]] auto& entry) {
        [[maybe_unused]] auto icon_result = tray.set_icon(icon_red);
        [[maybe_unused]] auto tooltip_result = tray.set_tooltip("Red Icon Active");
        std::cout << "Changed to red icon\n";
    });
    
    [[maybe_unused]] auto green_result = menu.add_item("Green Icon", [&tray, &icon_green]([[maybe_unused]] auto& entry) {
        [[maybe_unused]] auto icon_result = tray.set_icon(icon_green);
        [[maybe_unused]] auto tooltip_result = tray.set_tooltip("Green Icon Active");
        std::cout << "Changed to green icon\n";
    });
    
    [[maybe_unused]] auto blue_result = menu.add_item("Blue Icon", [&tray, &icon_blue]([[maybe_unused]] auto& entry) {
        [[maybe_unused]] auto icon_result = tray.set_icon(icon_blue);
        [[maybe_unused]] auto tooltip_result = tray.set_tooltip("Blue Icon Active");
        std::cout << "Changed to blue icon\n";
    });
    
    [[maybe_unused]] auto sep1_result = menu.add_separator();
    
    // Add dynamic menu item
    sdlpp::tray_entry dynamic_entry;
    auto dynamic_result = menu.add_item("Dynamic Item", [](auto& entry) {
        static int state = 0;
        state = (state + 1) % 3;
        
        switch (state) {
            case 0: {
                [[maybe_unused]] auto label_result = entry.set_label("State: Ready");
                [[maybe_unused]] auto enable_result = entry.set_enabled(true);
                break;
            }
            case 1: {
                [[maybe_unused]] auto label_result = entry.set_label("State: Busy");
                [[maybe_unused]] auto enable_result = entry.set_enabled(false);
                break;
            }
            case 2: {
                [[maybe_unused]] auto label_result = entry.set_label("State: Complete");
                [[maybe_unused]] auto enable_result = entry.set_enabled(true);
                break;
            }
        }
        
        std::cout << "Dynamic item state changed to: " << state << "\n";
    });
    
    if (dynamic_result) {
        dynamic_entry = std::move(dynamic_result.value());
    }
    
    [[maybe_unused]] auto sep2_result = menu.add_separator();
    
    // Add programmatic click example
    [[maybe_unused]] auto auto_result = menu.add_item("Auto-click Dynamic Item", [&dynamic_entry]([[maybe_unused]] auto& entry) {
        if (dynamic_entry.is_valid()) {
            std::cout << "Programmatically clicking dynamic item...\n";
            [[maybe_unused]] auto click_result = dynamic_entry.click();
        }
    });
    
    [[maybe_unused]] auto sep4_result = menu.add_separator();
    
    [[maybe_unused]] auto quit2_result = menu.add_item("Quit", [&running]([[maybe_unused]] auto& entry) {
        running = false;
    });
    
    // Run for a short time
    std::cout << "Dynamic tray created. Running for 15 seconds...\n";
    std::cout << "Try changing the icon color and clicking the dynamic item.\n";
    
    auto start = std::chrono::steady_clock::now();
    while (running && std::chrono::steady_clock::now() - start < std::chrono::seconds(15)) {
        sdlpp::update_trays();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void integrated_example() {
    std::cout << "\n=== Integrated Window + Tray Example ===\n";
    
    // Create a window
    auto window_result = sdlpp::window::create("Tray Application", 400, 300);
    if (!window_result) {
        std::cerr << "Failed to create window: " << window_result.error() << "\n";
        return;
    }
    
    auto& window = window_result.value();
    auto icon = create_icon(128, 128, 255);  // Light blue icon
    
    // Create tray
    auto tray_result = sdlpp::tray::create(icon, "Window Controller");
    if (!tray_result) {
        std::cerr << "Failed to create tray: " << tray_result.error() << "\n";
        return;
    }
    
    auto& tray = tray_result.value();
    auto& menu = tray.get_menu();
    
    std::atomic<bool> running{true};
    bool window_visible = true;
    
    // Add window control items
    [[maybe_unused]] auto show_hide_result = menu.add_item("Show/Hide Window", [&window, &window_visible](auto& entry) {
        window_visible = !window_visible;
        if (window_visible) {
            [[maybe_unused]] auto show_result = window.show();
            [[maybe_unused]] auto label_result = entry.set_label("Hide Window");
        } else {
            [[maybe_unused]] auto hide_result = window.hide();
            [[maybe_unused]] auto label_result = entry.set_label("Show Window");
        }
        std::cout << "Window " << (window_visible ? "shown" : "hidden") << "\n";
    });
    
    [[maybe_unused]] auto minimize_result = menu.add_item("Minimize to Tray", [&window]([[maybe_unused]] auto& entry) {
        [[maybe_unused]] auto min_result = window.minimize();
        std::cout << "Window minimized\n";
    });
    
    [[maybe_unused]] auto sep5_result = menu.add_separator();
    
    [[maybe_unused]] auto about_result = menu.add_item("About", []([[maybe_unused]] auto& entry) {
        std::cout << "SDL++ Tray Example v1.0\n";
    });
    
    [[maybe_unused]] auto quit3_result = menu.add_item("Quit", [&running]([[maybe_unused]] auto& entry) {
        running = false;
    });
    
    // Event loop
    std::cout << "Window with tray icon created.\n";
    std::cout << "Close the window or select Quit from tray to exit.\n";
    
    auto& event_queue = sdlpp::get_event_queue();
    while (running) {
        while (auto event = event_queue.poll()) {
            if (event->type() == sdlpp::event_type::quit) {
                running = false;
            } else if (event->type() == sdlpp::event_type::window_close_requested) {
                // Hide window instead of closing
                [[maybe_unused]] auto hide_window_result = window.hide();
                window_visible = false;
                std::cout << "Window hidden (use tray to show again)\n";
            }
        }
        
        sdlpp::update_trays();
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

int main() {
    try {
        // Initialize SDL
        sdlpp::init sdl_init(sdlpp::init_flags::video | sdlpp::init_flags::events);
        
        std::cout << "SDL System Tray Example\n";
        std::cout << "======================\n";
        std::cout << "Note: System tray support may vary by platform.\n";
        
        basic_tray_example();
        submenu_example();
        dynamic_tray_example();
        integrated_example();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}