#include <sdlpp/core/core.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/events/events.hh>
#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <algorithm>
#include <vector>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    // Initialize SDL
    auto init = sdlpp::init(sdlpp::init_flags::video | sdlpp::init_flags::events);
    if (!init.was_init(sdlpp::init_flags::video | sdlpp::init_flags::events)) {
        std::cerr << "Failed to initialize SDL" << std::endl;
        return 1;
    }

    // Create window
    auto window_result = sdlpp::window::create("Event Category Demo", 800, 600);
    if (!window_result) {
        std::cerr << "Failed to create window: " << window_result.error() << std::endl;
        return 1;
    }
    [[maybe_unused]] auto& window = window_result.value();

    // Event category statistics
    std::unordered_map<sdlpp::event_category, size_t> category_counts;
    std::unordered_map<sdlpp::event_type, size_t> event_counts;

    std::cout << "Event Category Demo - Press ESC to quit\n";
    std::cout << "Try various inputs to see event categorization\n\n";

    // Get event queue
    auto& queue = sdlpp::get_event_queue();
    
    // Main event loop
    bool running = true;
    while (running) {
        queue.pump();
        
        while (auto event = queue.poll()) {
            // Get event category
            auto category = sdlpp::get_event_category(*event);
            auto type = event->type();
            
            // Update statistics
            category_counts[category]++;
            event_counts[type]++;

            // Log event with category
            std::cout << "[" << std::setw(12) << sdlpp::event_category_to_string(category) << "] ";
            
            // Handle events by type
            switch (type) {
                case sdlpp::event_type::quit:
                    std::cout << "Quit requested";
                    running = false;
                    break;
                    
                case sdlpp::event_type::key_down:
                    {
                        auto& key = event->key();
                        std::cout << "Key down: " << sdlpp::get_keycode_name(static_cast<sdlpp::keycode>(key.key));
                        if (key.repeat) std::cout << " (repeat)";
                        if (key.scancode == static_cast<Uint16>(sdlpp::scancode::escape)) {
                            running = false;
                        }
                    }
                    break;
                    
                case sdlpp::event_type::key_up:
                    {
                        auto& key = event->key();
                        std::cout << "Key up: " << sdlpp::get_keycode_name(static_cast<sdlpp::keycode>(key.key));
                    }
                    break;
                    
                case sdlpp::event_type::text_input:
                    {
                        auto& text = event->text();
                        std::cout << "Text input: \"" << text.text << "\"";
                    }
                    break;
                    
                case sdlpp::event_type::mouse_motion:
                    {
                        auto& mouse = event->motion();
                        std::cout << "Mouse moved to " << mouse.x << "," << mouse.y
                                 << " (rel: " << mouse.xrel << "," << mouse.yrel << ")";
                    }
                    break;
                    
                case sdlpp::event_type::mouse_button_down:
                case sdlpp::event_type::mouse_button_up:
                    {
                        auto& button = event->button();
                        std::cout << "Mouse button " << static_cast<int>(button.button)
                                 << " " << (button.down ? "pressed" : "released")
                                 << " at " << button.x << "," << button.y;
                    }
                    break;
                    
                case sdlpp::event_type::mouse_wheel:
                    {
                        auto& wheel = event->wheel();
                        std::cout << "Mouse wheel: " << wheel.x << "," << wheel.y;
                    }
                    break;
                    
                case sdlpp::event_type::window_shown:
                    std::cout << "Window shown";
                    break;
                    
                case sdlpp::event_type::window_hidden:
                    std::cout << "Window hidden";
                    break;
                    
                case sdlpp::event_type::window_moved:
                    {
                        auto& win = event->window();
                        std::cout << "Window moved to " << win.data1 << "," << win.data2;
                    }
                    break;
                    
                case sdlpp::event_type::window_resized:
                    {
                        auto& win = event->window();
                        std::cout << "Window resized to " << win.data1 << "x" << win.data2;
                    }
                    break;
                    
                case sdlpp::event_type::window_focus_gained:
                    std::cout << "Window focus gained";
                    break;
                    
                case sdlpp::event_type::window_focus_lost:
                    std::cout << "Window focus lost";
                    break;
                    
                case sdlpp::event_type::drop_file:
                    {
                        auto& drop = event->drop();
                        std::cout << "File dropped: " << drop.data;
                    }
                    break;
                    
                case sdlpp::event_type::drop_text:
                    {
                        auto& drop = event->drop();
                        std::cout << "Text dropped: " << drop.data;
                    }
                    break;
                    
                default:
                    std::cout << "Event type: " << static_cast<int>(type);
                    break;
            }
            
            // Additional categorization info
            if (sdlpp::is_input_event(*event)) {
                std::cout << " [INPUT]";
            }
            if (sdlpp::is_device_event(*event)) {
                std::cout << " [DEVICE]";
            }
            
            std::cout << std::endl;
        }
    }

    // Print statistics
    std::cout << "\n=== Event Category Statistics ===\n";
    for (const auto& [category, count] : category_counts) {
        std::cout << std::setw(15) << sdlpp::event_category_to_string(category) 
                  << ": " << count << " events\n";
    }

    std::cout << "\n=== Top Event Types ===\n";
    // Sort by count
    std::vector<std::pair<sdlpp::event_type, size_t>> sorted_events(
        event_counts.begin(), event_counts.end());
    std::sort(sorted_events.begin(), sorted_events.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    // Show top 10
    for (size_t i = 0; i < std::min(size_t(10), sorted_events.size()); ++i) {
        std::cout << std::setw(20) << static_cast<int>(sorted_events[i].first)
                  << ": " << sorted_events[i].second << " events\n";
    }

    return 0;
}