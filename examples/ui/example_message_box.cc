#include <iostream>
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>
#include <sdlpp/ui/message_box.hh>

void simple_message_boxes() {
    std::cout << "\n=== Simple Message Boxes ===\n";
    
    // Information box
    auto info_result = sdlpp::show_info_box("Information", 
                                           "This is an informational message.");
    if (!info_result) {
        std::cerr << "Failed to show info box: " << info_result.error() << "\n";
    }
    
    // Warning box
    auto warn_result = sdlpp::show_warning_box("Warning", 
                                              "This is a warning message!");
    if (!warn_result) {
        std::cerr << "Failed to show warning box: " << warn_result.error() << "\n";
    }
    
    // Error box
    auto error_result = sdlpp::show_error_box("Error", 
                                             "This is an error message!");
    if (!error_result) {
        std::cerr << "Failed to show error box: " << error_result.error() << "\n";
    }
}

void custom_message_boxes() {
    std::cout << "\n=== Custom Message Boxes ===\n";
    
    // Yes/No dialog
    auto yes_no_result = sdlpp::message_box_builder()
        .set_title("Confirm Action")
        .set_message("Do you want to proceed with this action?")
        .set_type(sdlpp::message_box_flags::warning)
        .add_button(1, "Yes", true)    // Return key default
        .add_button(0, "No", false, true)  // Escape key default
        .show();
    
    if (yes_no_result) {
        std::cout << "User selected: " 
                  << (yes_no_result.value() == 1 ? "Yes" : "No") << "\n";
    } else {
        std::cerr << "Failed to show dialog: " << yes_no_result.error() << "\n";
    }
    
    // Multiple choice dialog
    auto choice_result = sdlpp::message_box_builder()
        .set_title("Save Changes?")
        .set_message("You have unsaved changes. What would you like to do?")
        .set_type(sdlpp::message_box_flags::warning)
        .add_button(2, "Save", true)       // Return key default
        .add_button(1, "Don't Save")
        .add_button(0, "Cancel", false, true)  // Escape key default
        .show();
    
    if (choice_result) {
        switch (choice_result.value()) {
            case 2:
                std::cout << "User chose to save\n";
                break;
            case 1:
                std::cout << "User chose not to save\n";
                break;
            case 0:
                std::cout << "User cancelled\n";
                break;
        }
    }
}

void color_scheme_example() {
    std::cout << "\n=== Custom Color Scheme ===\n";
    
    // Create a dark theme color scheme
    sdlpp::message_box_color_scheme dark_theme;
    dark_theme.set_color(sdlpp::message_box_color_type::background, {32, 32, 32})
              .set_color(sdlpp::message_box_color_type::text, {200, 200, 200})
              .set_color(sdlpp::message_box_color_type::button_border, {64, 64, 64})
              .set_color(sdlpp::message_box_color_type::button_background, {48, 48, 48})
              .set_color(sdlpp::message_box_color_type::button_selected, {64, 128, 255});
    
    auto themed_result = sdlpp::message_box_builder()
        .set_title("Dark Theme Dialog")
        .set_message("This dialog uses a custom dark color scheme.")
        .set_type(sdlpp::message_box_flags::information)
        .set_color_scheme(dark_theme)
        .add_button(0, "Cool!", true)
        .show();
    
    if (!themed_result) {
        std::cerr << "Failed to show themed dialog: " << themed_result.error() << "\n";
    }
}

void parent_window_example() {
    std::cout << "\n=== Parent Window Example ===\n";
    
    // Create a window to act as parent
    auto window_result = sdlpp::window::create("Parent Window", 640, 480);
    if (!window_result) {
        std::cerr << "Failed to create window: " << window_result.error() << "\n";
        return;
    }
    
    auto& window = window_result.value();
    
    // Show message box with parent window (will be modal to this window)
    auto modal_result = sdlpp::message_box_builder()
        .set_title("Modal Dialog")
        .set_message("This dialog is modal to the parent window.")
        .set_type(sdlpp::message_box_flags::information)
        .set_parent(window)
        .add_button(0, "OK", true)
        .show();
    
    if (!modal_result) {
        std::cerr << "Failed to show modal dialog: " << modal_result.error() << "\n";
    }
}

int main() {
    try {
        // Initialize SDL
        sdlpp::init sdl_init(sdlpp::init_flags::video);
        
        std::cout << "SDL Message Box Example\n";
        std::cout << "======================\n";
        
        simple_message_boxes();
        custom_message_boxes();
        color_scheme_example();
        parent_window_example();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}