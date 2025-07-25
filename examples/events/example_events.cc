#include <iostream>
#include <iomanip>
#include <sdlpp/core/core.hh>
#include <sdlpp/events/events.hh>
#include <sdlpp/video/window.hh>

// Helper to print event info
void print_event_info(const sdlpp::event& event) {
    std::cout << "Event: " << static_cast<Uint32>(event.type()) 
              << " at " << event.timestamp() << "ms" << std::endl;
}

int main() {
    // Initialize SDL
    auto init = sdlpp::init(sdlpp::init_flags::video | sdlpp::init_flags::events);
    if (!init.was_init(sdlpp::init_flags::video | sdlpp::init_flags::events)) {
        std::cerr << "Failed to initialize SDL" << std::endl;
        return 1;
    }
    
    // Create a window for receiving events
    auto window_result = sdlpp::window::create("Event Example", 800, 600);
    if (!window_result) {
        std::cerr << "Failed to create window: " << window_result.error() << std::endl;
        return 1;
    }
    [[maybe_unused]] auto& window = window_result.value();
    
    std::cout << "Event handling examples. Press ESC to quit." << std::endl;
    std::cout << "Try: keyboard input, mouse clicks/motion, window resize, etc." << std::endl;
    std::cout << std::endl;
    
    auto& queue = sdlpp::get_event_queue();
    bool running = true;
    
    // Example of event filter
    auto filter = sdlpp::event_filter([](void* userdata, SDL_Event* event) -> bool {
        // Filter out mouse motion events during processing
        if (event->type == static_cast<Uint32>(sdlpp::event_type::mouse_motion)) {
            auto* verbose = static_cast<bool*>(userdata);
            return !(*verbose); // Return false to filter out
        }
        return true; // Allow all other events
    }, &running);
    
    // Main event loop demonstrating different access patterns
    while (running) {
        queue.pump();
        
        // Poll for events
        while (auto event = queue.poll()) {
            // Method 1: Switch on event type (fastest)
            switch (event->type()) {
                case sdlpp::event_type::quit:
                    std::cout << "Quit requested" << std::endl;
                    running = false;
                    break;
                    
                case sdlpp::event_type::key_down:
                    {
                        auto& key = event->key();
                        std::cout << "Key pressed: " 
                                  << sdlpp::get_keycode_name(static_cast<sdlpp::keycode>(key.key)) 
                                  << " (scancode: " << sdlpp::get_scancode_name(static_cast<sdlpp::scancode>(key.scancode)) << ")";
                        if (key.repeat) std::cout << " [repeat]";
                        std::cout << std::endl;
                        
                        if (key.scancode == static_cast<Uint16>(sdlpp::scancode::escape)) {
                            running = false;
                        }
                    }
                    break;
                    
                case sdlpp::event_type::key_up:
                    {
                        auto& key = event->key();
                        std::cout << "Key released: " 
                                  << sdlpp::get_keycode_name(key.key) << std::endl;
                    }
                    break;
                    
                case sdlpp::event_type::text_input:
                    {
                        auto& text = event->text();
                        std::cout << "Text input: \"" << text.text << "\"" << std::endl;
                    }
                    break;
                    
                case sdlpp::event_type::mouse_button_down:
                case sdlpp::event_type::mouse_button_up:
                    {
                        auto& button = event->button();
                        std::cout << "Mouse button " 
                                  << (button.down ? "pressed" : "released")
                                  << ": button " << static_cast<int>(button.button)
                                  << " at (" << button.x << ", " << button.y << ")";
                        if (button.clicks == 2) std::cout << " [double-click]";
                        std::cout << std::endl;
                    }
                    break;
                    
                case sdlpp::event_type::mouse_wheel:
                    {
                        auto& wheel = event->wheel();
                        std::cout << "Mouse wheel: x=" << wheel.x 
                                  << ", y=" << wheel.y << std::endl;
                    }
                    break;
                    
                case sdlpp::event_type::window_resized:
                    {
                        auto& win = event->window();
                        std::cout << "Window resized to " 
                                  << win.data1 << "x" << win.data2 << std::endl;
                    }
                    break;
                    
                default:
                    // Skip other events
                    break;
            }
            
            // Method 2: Type-safe casting
            if (event->is<sdlpp::window_event>()) {
                auto* win = event->as<sdlpp::window_event>();
                if (win->is_close_requested()) {
                    std::cout << "Window close requested" << std::endl;
                    running = false;
                }
            }
            
            // Method 3: Functional handle
            event->handle<sdlpp::joystick_device_event>([](const sdlpp::joystick_device_event& e) {
                if (e.is_added()) {
                    std::cout << "Joystick connected: ID " << e.which << std::endl;
                } else if (e.is_removed()) {
                    std::cout << "Joystick disconnected: ID " << e.which << std::endl;
                }
            });
            
            // Method 4: Visitor pattern (for demonstration)
            if (event->type() == sdlpp::event_type::drop_file) {
                event->visit([](const auto& e) {
                    using T = std::decay_t<decltype(e)>;
                    if constexpr (std::is_same_v<T, sdlpp::drop_event>) {
                        if (e.is_file()) {
                            std::cout << "File dropped: " << e.get_data() << std::endl;
                        }
                    }
                });
            }
        }
    }
    
    // Demonstrate custom events
    std::cout << "\nRegistering custom events..." << std::endl;
    auto custom_events = sdlpp::event_registry::register_events(2);
    if (custom_events) {
        std::cout << "Registered custom events starting at ID: " << *custom_events << std::endl;
        
        // Push a custom event
        SDL_Event custom{};
        custom.type = *custom_events;
        custom.user.code = 42;
        custom.user.data1 = reinterpret_cast<void*>(0x1234);
        
        auto push_result = queue.push(sdlpp::event(custom));
        if (!push_result) {
            std::cerr << "Failed to push custom event: " << push_result.error() << std::endl;
        }
        
        // Poll and check for it
        if (auto event = queue.poll()) {
            if (sdlpp::event_registry::is_custom(event->type())) {
                auto* user = event->as<sdlpp::user_event>();
                if (user) {
                    std::cout << "Received custom event with code: " << user->code << std::endl;
                }
            }
        }
    }
    
    return 0;
}