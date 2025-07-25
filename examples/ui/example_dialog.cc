#include <iostream>
#include <thread>
#include <chrono>
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/ui/dialog.hh>
#include <sdlpp/events/events.hh>

void simple_file_dialogs() {
    std::cout << "\n=== Simple File Dialogs ===\n";
    
    // Open file dialog
    std::cout << "Showing open file dialog...\n";
    auto open_result = sdlpp::show_open_file_dialog(
        [](const sdlpp::dialog_result& result) {
            if (result.accepted) {
                std::cout << "Selected file: " << result.paths[0] << "\n";
            } else {
                std::cout << "Open file dialog cancelled\n";
            }
        }
    );
    
    if (!open_result) {
        std::cerr << "Failed to show open dialog: " << open_result.error() << "\n";
    }
    
    // Give some time for the dialog
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Save file dialog
    std::cout << "\nShowing save file dialog...\n";
    auto save_result = sdlpp::show_save_file_dialog(
        [](const sdlpp::dialog_result& result) {
            if (result.accepted) {
                std::cout << "Save to: " << result.paths[0] << "\n";
            } else {
                std::cout << "Save file dialog cancelled\n";
            }
        },
        nullptr,  // no parent window
        {},       // no filters
        "untitled.txt"  // default name
    );
    
    if (!save_result) {
        std::cerr << "Failed to show save dialog: " << save_result.error() << "\n";
    }
    
    // Open folder dialog
    std::cout << "\nShowing open folder dialog...\n";
    auto folder_result = sdlpp::show_open_folder_dialog(
        [](const sdlpp::dialog_result& result) {
            if (result.accepted) {
                std::cout << "Selected folder: " << result.paths[0] << "\n";
            } else {
                std::cout << "Open folder dialog cancelled\n";
            }
        }
    );
    
    if (!folder_result) {
        std::cerr << "Failed to show folder dialog: " << folder_result.error() << "\n";
    }
}

void filtered_file_dialog() {
    std::cout << "\n=== Filtered File Dialog ===\n";
    
    // Create filters for different file types
    std::vector<sdlpp::dialog_file_filter> image_filters = {
        {"Image files", "*.png;*.jpg;*.jpeg;*.gif;*.bmp"},
        {"PNG files", "*.png"},
        {"JPEG files", "*.jpg;*.jpeg"},
        {"All files", "*.*"}
    };
    
    std::cout << "Showing filtered file dialog...\n";
    auto dlg_result = sdlpp::show_open_file_dialog(
        [](const sdlpp::dialog_result& result) {
            if (result.accepted) {
                std::cout << "Selected image: " << result.paths[0] << "\n";
            } else {
                std::cout << "Image selection cancelled\n";
            }
        },
        nullptr,  // no parent window
        image_filters
    );
    
    if (!dlg_result) {
        std::cerr << "Failed to show filtered dialog: " << dlg_result.error() << "\n";
    }
}

void multiple_file_selection() {
    std::cout << "\n=== Multiple File Selection ===\n";
    
    std::cout << "Showing multi-select file dialog...\n";
    auto dlg_result = sdlpp::show_open_file_dialog(
        [](const sdlpp::dialog_result& result) {
            if (result.accepted) {
                std::cout << "Selected " << result.paths.size() << " files:\n";
                for (const auto& path : result.paths) {
                    std::cout << "  - " << path << "\n";
                }
            } else {
                std::cout << "Multi-select cancelled\n";
            }
        },
        nullptr,  // no parent window
        {},       // no filters
        true      // allow multiple selection
    );
    
    if (!dlg_result) {
        std::cerr << "Failed to show multi-select dialog: " << dlg_result.error() << "\n";
    }
}

void dialog_builder_example() {
    std::cout << "\n=== Dialog Builder Example ===\n";
    
    // Create a customized save dialog for a specific file type
    auto dlg_result = sdlpp::file_dialog_builder()
        .set_type(sdlpp::file_dialog_type::save_file)
        .set_title("Export Document")
        .set_accept_label("Export")
        .set_cancel_label("Don't Export")
        .add_filter("Markdown files", "*.md")
        .add_filter("Text files", "*.txt")
        .add_filter("All files", "*.*")
        .set_default_name("document.md")
        .set_default_location(std::filesystem::current_path())
        .show([](const sdlpp::dialog_result& result) {
            if (result.accepted) {
                std::cout << "Export to: " << result.paths[0] << "\n";
                
                // Check the extension
                auto path = result.paths[0];
                if (path.extension() == ".md") {
                    std::cout << "Exporting as Markdown...\n";
                } else if (path.extension() == ".txt") {
                    std::cout << "Exporting as plain text...\n";
                } else {
                    std::cout << "Exporting in default format...\n";
                }
            } else {
                std::cout << "Export cancelled\n";
            }
        });
    
    if (!dlg_result) {
        std::cerr << "Failed to show export dialog: " << dlg_result.error() << "\n";
    }
}

void parent_window_dialog() {
    std::cout << "\n=== Parent Window Dialog ===\n";
    
    // Create a window to act as parent
    auto window_result = sdlpp::window::create("Main Window", 800, 600);
    if (!window_result) {
        std::cerr << "Failed to create window: " << window_result.error() << "\n";
        return;
    }
    
    auto& window = window_result.value();
    bool dialog_shown = false;
    bool running = true;
    
    std::cout << "Click on the window to show a file dialog...\n";
    
    // Event loop
    auto& event_queue = sdlpp::get_event_queue();
    while (running) {
        while (auto event = event_queue.poll()) {
            if (event->type() == sdlpp::event_type::quit) {
                running = false;
            } else if (event->type() == sdlpp::event_type::mouse_button_down && !dialog_shown) {
                dialog_shown = true;
                
                // Show dialog with parent window
                auto dlg_result = sdlpp::file_dialog_builder()
                    .set_type(sdlpp::file_dialog_type::open_file)
                    .set_title("Select Configuration File")
                    .set_parent(window)
                    .add_filter("Config files", "*.conf;*.cfg;*.ini")
                    .add_filter("JSON files", "*.json")
                    .add_filter("All files", "*.*")
                    .show([&running](const sdlpp::dialog_result& result) {
                        if (result.accepted) {
                            std::cout << "Loading config from: " << result.paths[0] << "\n";
                        } else {
                            std::cout << "Config selection cancelled\n";
                        }
                        running = false;  // Close after dialog
                    });
                
                if (!dlg_result) {
                    std::cerr << "Failed to show config dialog: " << dlg_result.error() << "\n";
                    running = false;
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

int main() {
    try {
        // Initialize SDL
        sdlpp::init sdl_init(sdlpp::init_flags::video | sdlpp::init_flags::events);
        
        std::cout << "SDL File Dialog Example\n";
        std::cout << "======================\n";
        std::cout << "Note: File dialogs are non-blocking and use callbacks.\n";
        std::cout << "The program will wait briefly after each dialog.\n";
        
        simple_file_dialogs();
        
        // Wait for dialogs to complete
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        filtered_file_dialog();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        multiple_file_selection();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        dialog_builder_example();
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        parent_window_dialog();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}