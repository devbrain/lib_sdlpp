#include <sdlpp/system/clipboard.hh>
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/events/events.hh>
#include <../include/sdlpp/core/log.hh>

#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <chrono>
#include <ctime>

using namespace sdlpp;

// Helper to display clipboard contents
void display_clipboard_info() {
    std::cout << "\n=== Current Clipboard Contents ===\n";
    
    // Get available MIME types
    auto types = clipboard::get_mime_types();
    std::cout << "Available formats: " << types.size() << "\n";
    for (const auto& type : types) {
        std::cout << "  - " << type;
        
        // Show size of data
        auto data = clipboard::get_data(type.c_str());
        std::cout << " (" << data.size() << " bytes)";
        
        // Show preview for text types
        if (type.find("text/") == 0 && !data.empty()) {
            std::string preview(data.begin(), data.end());
            if (preview.length() > 50) {
                preview = preview.substr(0, 47) + "...";
            }
            // Escape newlines for display
            size_t pos = 0;
            while ((pos = preview.find('\n', pos)) != std::string::npos) {
                preview.replace(pos, 1, "\\n");
                pos += 2;
            }
            std::cout << " \"" << preview << "\"";
        }
        
        std::cout << "\n";
    }
    
    if (clipboard::has_text()) {
        std::cout << "\nText content:\n";
        std::string text = clipboard::get_text();
        std::cout << "\"" << text << "\"\n";
    }
    
    if (clipboard::has_primary_selection_text()) {
        std::cout << "\nPrimary selection (X11):\n";
        std::string text = clipboard::get_primary_selection_text();
        std::cout << "\"" << text << "\"\n";
    }
}

// Example of custom data format
struct custom_data {
    int32_t version = 1;
    int32_t x = 100;
    int32_t y = 200;
    float scale = 1.5f;
    char name[32] = "SDL++ Object";
};

void interactive_menu() {
    while (true) {
        std::cout << "\n=== Clipboard Operations Menu ===\n";
        std::cout << "1. Display clipboard contents\n";
        std::cout << "2. Set plain text\n";
        std::cout << "3. Set multiple formats (HTML + plain text)\n";
        std::cout << "4. Set custom binary data\n";
        std::cout << "5. Set clipboard with timestamp provider\n";
        std::cout << "6. Clear clipboard\n";
        std::cout << "7. Copy from/to primary selection (X11)\n";
        std::cout << "8. Test unicode text\n";
        std::cout << "0. Exit\n";
        std::cout << "\nChoice: ";
        
        int choice;
        std::cin >> choice;
        std::cin.ignore();
        
        switch (choice) {
            case 1: {
                display_clipboard_info();
                break;
            }
            
            case 2: {
                std::cout << "Enter text to copy: ";
                std::string text;
                std::getline(std::cin, text);
                
                auto result = clipboard::set_text(text);
                if (result) {
                    std::cout << "Text copied to clipboard!\n";
                } else {
                    std::cout << "Error: " << result.error() << "\n";
                }
                break;
            }
            
            case 3: {
                std::cout << "Enter text: ";
                std::string text;
                std::getline(std::cin, text);
                
                // Create HTML version
                std::stringstream html;
                html << "<html><body><p style=\"color: blue; font-family: Arial;\">";
                html << text;
                html << "</p></body></html>";
                
                // Set both formats
                const char* mime_types[] = {"text/html", "text/plain"};
                std::string html_str = html.str();
                
                std::span<const uint8_t> data_spans[] = {
                    std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(html_str.data()), html_str.size()},
                    std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(text.data()), text.size()}
                };
                
                auto result = clipboard::set_data(mime_types, data_spans, 2);
                if (result) {
                    std::cout << "Multiple formats copied to clipboard!\n";
                    std::cout << "Try pasting in a rich text editor to see HTML formatting.\n";
                } else {
                    std::cout << "Error: " << result.error() << "\n";
                }
                break;
            }
            
            case 4: {
                // Create custom binary data
                custom_data data;
                std::cout << "Enter object name: ";
                std::string name;
                std::getline(std::cin, name);
                
                // Copy name to fixed buffer
                name.copy(data.name, sizeof(data.name) - 1);
                data.name[sizeof(data.name) - 1] = '\0';
                
                std::cout << "Enter X coordinate: ";
                std::cin >> data.x;
                std::cout << "Enter Y coordinate: ";
                std::cin >> data.y;
                std::cout << "Enter scale: ";
                std::cin >> data.scale;
                std::cin.ignore();
                
                // Also provide text representation
                std::stringstream text_repr;
                text_repr << "Object '" << data.name << "' at (" 
                         << data.x << ", " << data.y << ") scale " << data.scale;
                std::string text_str = text_repr.str();
                
                // Set both binary and text formats
                const char* mime_types[] = {"application/x-sdlpp-object", "text/plain"};
                std::span<const uint8_t> data_spans[] = {
                    std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(&data), sizeof(data)},
                    std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(text_str.data()), text_str.size()}
                };
                
                auto result = clipboard::set_data(mime_types, data_spans, 2);
                if (result) {
                    std::cout << "Custom data copied to clipboard!\n";
                } else {
                    std::cout << "Error: " << result.error() << "\n";
                }
                break;
            }
            
            case 5: {
                // Data provider that generates timestamp on request
                static std::string timestamp_data;
                auto data_callback = []([[maybe_unused]] void* userdata, const char* mime_type, size_t* size) -> const void* {
                    if (std::string(mime_type) == "text/plain") {
                        // Generate current timestamp
                        auto now = std::chrono::system_clock::now();
                        auto time_point = std::chrono::system_clock::to_time_t(now);
                        
                        std::stringstream ss;
                        ss << "Generated at: " << std::put_time(std::localtime(&time_point), "%Y-%m-%d %H:%M:%S");
                        timestamp_data = ss.str();
                        
                        *size = timestamp_data.size();
                        return timestamp_data.data();
                    }
                    return nullptr;
                };
                
                std::vector<std::string> types = {"text/plain"};
                auto result = clipboard::data_provider::set(types, data_callback);
                if (result) {
                    std::cout << "Timestamp provider set! Paste to see current time.\n";
                } else {
                    std::cout << "Error: " << result.error() << "\n";
                }
                break;
            }
            
            case 6: {
                auto result = clipboard::clear();
                if (result) {
                    std::cout << "Clipboard cleared!\n";
                } else {
                    std::cout << "Error: " << result.error() << "\n";
                }
                break;
            }
            
            case 7: {
                std::cout << "1. Copy clipboard to primary selection\n";
                std::cout << "2. Copy primary selection to clipboard\n";
                std::cout << "Choice: ";
                
                int subchoice;
                std::cin >> subchoice;
                std::cin.ignore();
                
                if (subchoice == 1) {
                    std::string text = clipboard::get_text();
                    if (!text.empty()) {
                        auto result = clipboard::set_primary_selection_text(text);
                        if (result) {
                            std::cout << "Copied to primary selection!\n";
                        } else {
                            std::cout << "Error (may not be supported on this platform): " 
                                     << result.error() << "\n";
                        }
                    } else {
                        std::cout << "Clipboard is empty!\n";
                    }
                } else if (subchoice == 2) {
                    std::string text = clipboard::get_primary_selection_text();
                    if (!text.empty()) {
                        auto result = clipboard::set_text(text);
                        if (result) {
                            std::cout << "Copied to clipboard!\n";
                        } else {
                            std::cout << "Error: " << result.error() << "\n";
                        }
                    } else {
                        std::cout << "Primary selection is empty!\n";
                    }
                }
                break;
            }
            
            case 8: {
                // Test various unicode strings
                std::vector<std::string> test_strings = {
                    reinterpret_cast<const char*>(u8"Hello, 世界! 🌍"),
                    reinterpret_cast<const char*>(u8"Émojis: 😀 🎉 🚀 ❤️"),
                    reinterpret_cast<const char*>(u8"Math: ∑ ∏ ∫ √ ∞"),
                    reinterpret_cast<const char*>(u8"Currencies: $ € £ ¥ ₹"),
                    reinterpret_cast<const char*>(u8"Languages: Ελληνικά Русский العربية")
                };
                
                std::cout << "Unicode test strings:\n";
                for (size_t i = 0; i < test_strings.size(); ++i) {
                    std::cout << i + 1 << ". " << test_strings[i] << "\n";
                }
                std::cout << "Select string to copy (1-" << test_strings.size() << "): ";
                
                size_t idx;
                std::cin >> idx;
                std::cin.ignore();
                
                if (idx > 0 && idx <= test_strings.size()) {
                    auto result = clipboard::set_text(test_strings[idx - 1]);
                    if (result) {
                        std::cout << "Unicode text copied!\n";
                        
                        // Verify it reads back correctly
                        std::string retrieved = clipboard::get_text();
                        if (retrieved == test_strings[idx - 1]) {
                            std::cout << "Verified: Text reads back correctly!\n";
                        } else {
                            std::cout << "Warning: Retrieved text doesn't match!\n";
                        }
                    } else {
                        std::cout << "Error: " << result.error() << "\n";
                    }
                }
                break;
            }
            
            case 0:
                return;
                
            default:
                std::cout << "Invalid choice!\n";
                break;
        }
    }
}


int main() {
    std::cout << "=== SDL++ Clipboard Example ===\n\n";
    
    // Initialize SDL with video (required for clipboard)
    auto init_result = init(init_flags::video);
    if (!init_result) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to initialize SDL");
        return 1;
    }
    
    // Create a hidden window (required on some platforms for clipboard)
    auto window_result = window::create("SDL++ Clipboard Example", 1, 1);
    if (!window_result) {
        logger::error(log_category::application, std::source_location::current(), 
                     "Failed to create window");
        return 1;
    }
    
    auto& win = *window_result;
    [[maybe_unused]] auto hide_result = win.hide();  // We don't need to show the window
    
    std::cout << "Note: A hidden window has been created (required for clipboard on some platforms)\n\n";
    
    // Display initial clipboard state
    display_clipboard_info();
    
    // Run interactive menu
    interactive_menu();
    
    std::cout << "\nGoodbye!\n";
    return 0;
}