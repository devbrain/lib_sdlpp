---
title: Message Boxes
sidebar_label: Message Box
---

# Message Boxes

SDL++ provides native message box dialogs for displaying alerts, confirmations, and choices to users.

## Simple Message Box

Display a basic message box:

```cpp
#include <sdlpp/ui/message_box.hh>

// Simple information box
auto result = sdlpp::show_simple_message_box(
    sdlpp::message_box_flags::information,
    "Success",
    "Operation completed successfully!",
    window  // Parent window (optional)
);

if (!result) {
    std::cerr << "Failed to show message box: " << result.error() << std::endl;
}

// Warning message
sdlpp::show_simple_message_box(
    sdlpp::message_box_flags::warning,
    "Warning",
    "This action cannot be undone!",
    nullptr  // No parent window
);

// Error message
sdlpp::show_simple_message_box(
    sdlpp::message_box_flags::error,
    "Error",
    "Failed to load file: config.json",
    window
);
```

## Message Box Flags

```cpp
enum class message_box_flags : Uint32 {
    error = SDL_MESSAGEBOX_ERROR,           // Error icon
    warning = SDL_MESSAGEBOX_WARNING,       // Warning icon  
    information = SDL_MESSAGEBOX_INFORMATION // Info icon
};
```

## Custom Message Box

Create message boxes with multiple buttons:

```cpp
// Define buttons
std::vector<sdlpp::message_box_button> buttons = {
    {
        .flags = sdlpp::message_box_button_flags::return_key_default,
        .id = 1,
        .text = "Save"
    },
    {
        .flags = sdlpp::message_box_button_flags::none,
        .id = 2,
        .text = "Don't Save"
    },
    {
        .flags = sdlpp::message_box_button_flags::escape_key_default,
        .id = 3,
        .text = "Cancel"
    }
};

// Create message box data
sdlpp::message_box_data data{
    .flags = sdlpp::message_box_flags::warning,
    .window = window,  // Parent window (optional)
    .title = "Unsaved Changes",
    .message = "Do you want to save your changes before closing?",
    .buttons = buttons
};

// Show message box
auto result = sdlpp::show_message_box(data);

if (result) {
    switch (*result) {
        case 1:  // Save button
            save_document();
            close_application();
            break;
        case 2:  // Don't Save
            close_application();
            break;
        case 3:  // Cancel
            // Do nothing
            break;
    }
} else {
    std::cerr << "Message box error: " << result.error() << std::endl;
}
```

## Button Flags

```cpp
enum class message_box_button_flags : Uint32 {
    none = 0,
    return_key_default = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,
    escape_key_default = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT
};
```

## Color Customization

Customize message box colors:

```cpp
// Define color scheme
sdlpp::message_box_color_scheme color_scheme{
    .colors = {
        // Background
        {200, 200, 200, 255},
        // Text
        {0, 0, 0, 255},
        // Button border
        {100, 100, 100, 255},
        // Button background
        {180, 180, 180, 255},
        // Button selected
        {150, 150, 255, 255}
    }
};

// Create message box with custom colors
sdlpp::message_box_data data{
    .flags = sdlpp::message_box_flags::information,
    .window = nullptr,
    .title = "Custom Colors",
    .message = "This message box has custom colors!",
    .buttons = {
        {sdlpp::message_box_button_flags::return_key_default, 1, "OK"}
    },
    .color_scheme = color_scheme
};

auto result = sdlpp::show_message_box(data);
```

## Common Patterns

### Confirmation Dialog

```cpp
class dialogs {
public:
    static bool confirm(const std::string& title, 
                       const std::string& message,
                       sdlpp::window* parent = nullptr) {
        std::vector<sdlpp::message_box_button> buttons = {
            {
                sdlpp::message_box_button_flags::return_key_default,
                1, "Yes"
            },
            {
                sdlpp::message_box_button_flags::escape_key_default,
                0, "No"
            }
        };
        
        sdlpp::message_box_data data{
            .flags = sdlpp::message_box_flags::warning,
            .window = parent,
            .title = title,
            .message = message,
            .buttons = buttons
        };
        
        auto result = sdlpp::show_message_box(data);
        return result && *result == 1;
    }
    
    static void alert(const std::string& title,
                     const std::string& message,
                     sdlpp::window* parent = nullptr) {
        sdlpp::show_simple_message_box(
            sdlpp::message_box_flags::information,
            title, message, parent
        );
    }
    
    static void error(const std::string& title,
                     const std::string& message,
                     sdlpp::window* parent = nullptr) {
        sdlpp::show_simple_message_box(
            sdlpp::message_box_flags::error,
            title, message, parent
        );
    }
};

// Usage
if (dialogs::confirm("Delete File", "Are you sure you want to delete this file?")) {
    delete_file();
}
```

### Choice Dialog

```cpp
enum class save_choice {
    save,
    dont_save,
    cancel
};

save_choice show_save_dialog(sdlpp::window* parent) {
    std::vector<sdlpp::message_box_button> buttons = {
        {sdlpp::message_box_button_flags::none, 0, "Save"},
        {sdlpp::message_box_button_flags::none, 1, "Don't Save"},
        {sdlpp::message_box_button_flags::escape_key_default, 2, "Cancel"}
    };
    
    sdlpp::message_box_data data{
        .flags = sdlpp::message_box_flags::warning,
        .window = parent,
        .title = "Unsaved Changes",
        .message = "You have unsaved changes. What would you like to do?",
        .buttons = buttons
    };
    
    auto result = sdlpp::show_message_box(data);
    
    if (!result) {
        return save_choice::cancel;  // Error treated as cancel
    }
    
    switch (*result) {
        case 0: return save_choice::save;
        case 1: return save_choice::dont_save;
        default: return save_choice::cancel;
    }
}
```

### Error Reporter

```cpp
class error_reporter {
    struct error_info {
        std::string title;
        std::string message;
        std::string details;
    };
    
public:
    static void report_error(const error_info& error, sdlpp::window* parent = nullptr) {
        std::string full_message = error.message;
        
        if (!error.details.empty()) {
            full_message += "\n\nDetails:\n" + error.details;
        }
        
        std::vector<sdlpp::message_box_button> buttons = {
            {sdlpp::message_box_button_flags::none, 1, "Copy Details"},
            {sdlpp::message_box_button_flags::return_key_default, 0, "OK"}
        };
        
        sdlpp::message_box_data data{
            .flags = sdlpp::message_box_flags::error,
            .window = parent,
            .title = error.title,
            .message = full_message,
            .buttons = buttons
        };
        
        auto result = sdlpp::show_message_box(data);
        
        if (result && *result == 1) {
            // Copy to clipboard
            sdlpp::set_clipboard_text(full_message);
        }
    }
    
    static void report_exception(const std::exception& e, sdlpp::window* parent = nullptr) {
        report_error({
            .title = "Application Error",
            .message = "An unexpected error occurred.",
            .details = e.what()
        }, parent);
    }
};
```

## Platform Considerations

### Modal Behavior

Message boxes are modal and will block the calling thread:

```cpp
// This blocks until user dismisses the dialog
auto result = sdlpp::show_message_box(data);

// For non-blocking, use a separate thread
std::thread([data]() {
    auto result = sdlpp::show_message_box(data);
    // Handle result
}).detach();
```

### Parent Window

Providing a parent window ensures proper modal behavior:

```cpp
// With parent - dialog is centered on window
sdlpp::show_simple_message_box(
    sdlpp::message_box_flags::information,
    "Info", "Message", window
);

// Without parent - dialog appears at system default position
sdlpp::show_simple_message_box(
    sdlpp::message_box_flags::information,
    "Info", "Message", nullptr
);
```

## Best Practices

1. **Provide Parent Window**: Always pass parent window for proper modal behavior
2. **Clear Messages**: Keep messages concise and actionable
3. **Appropriate Icons**: Use correct flag (error, warning, info) for context
4. **Keyboard Shortcuts**: Set return/escape defaults for accessibility
5. **Error Handling**: Always check return values

## Example: Application Dialogs

```cpp
class app_dialogs {
    sdlpp::window* main_window;
    
public:
    explicit app_dialogs(sdlpp::window* window) : main_window(window) {}
    
    bool confirm_exit() {
        if (has_unsaved_changes()) {
            auto choice = show_save_dialog(main_window);
            
            switch (choice) {
                case save_choice::save:
                    return save_all_documents();
                case save_choice::dont_save:
                    return true;
                case save_choice::cancel:
                    return false;
            }
        }
        return true;
    }
    
    void show_about() {
        std::string message = 
            "MyApp v1.0.0\n\n"
            "Copyright © 2024 MyCompany\n\n"
            "Built with SDL++ and SDL3";
            
        sdlpp::show_simple_message_box(
            sdlpp::message_box_flags::information,
            "About MyApp",
            message,
            main_window
        );
    }
    
    void show_update_available(const std::string& version) {
        std::vector<sdlpp::message_box_button> buttons = {
            {sdlpp::message_box_button_flags::return_key_default, 1, "Download"},
            {sdlpp::message_box_button_flags::none, 0, "Later"}
        };
        
        sdlpp::message_box_data data{
            .flags = sdlpp::message_box_flags::information,
            .window = main_window,
            .title = "Update Available",
            .message = "Version " + version + " is available. Download now?",
            .buttons = buttons
        };
        
        auto result = sdlpp::show_message_box(data);
        if (result && *result == 1) {
            open_download_page();
        }
    }
};
```