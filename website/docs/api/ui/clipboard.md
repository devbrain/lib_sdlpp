---
title: Clipboard
sidebar_label: Clipboard
---

# Clipboard Operations

SDL++ provides cross-platform clipboard functionality for text and data transfer between applications.

## Text Clipboard

### Setting Text

```cpp
#include <sdlpp/system/clipboard.hh>

// Set clipboard text
auto result = sdlpp::set_clipboard_text("Hello, Clipboard!");
if (!result) {
    std::cerr << "Failed to set clipboard: " << result.error() << std::endl;
}

// Set multi-line text
std::string text = "Line 1\nLine 2\nLine 3";
sdlpp::set_clipboard_text(text);
```

### Getting Text

```cpp
// Get clipboard text
std::string clipboard_content = sdlpp::get_clipboard_text();
if (!clipboard_content.empty()) {
    std::cout << "Clipboard contains: " << clipboard_content << std::endl;
} else {
    std::cout << "Clipboard is empty" << std::endl;
}

// Check if clipboard has text
if (sdlpp::has_clipboard_text()) {
    auto text = sdlpp::get_clipboard_text();
    process_text(text);
}
```

## Primary Selection (Linux/X11)

On Linux/X11, there's also a primary selection (middle-click paste):

```cpp
// Set primary selection
auto result = sdlpp::set_primary_selection_text("Selected text");

// Get primary selection
if (sdlpp::has_primary_selection_text()) {
    auto text = sdlpp::get_primary_selection_text();
    std::cout << "Primary selection: " << text << std::endl;
}
```

## Common Patterns

### Copy/Paste Manager

```cpp
class clipboard_manager {
    std::string last_clipboard_text;
    
public:
    bool copy(const std::string& text) {
        auto result = sdlpp::set_clipboard_text(text);
        if (result) {
            last_clipboard_text = text;
        }
        return result.has_value();
    }
    
    std::string paste() {
        return sdlpp::get_clipboard_text();
    }
    
    bool has_content() {
        return sdlpp::has_clipboard_text();
    }
    
    void clear() {
        sdlpp::set_clipboard_text("");
    }
    
    bool clipboard_changed() {
        auto current = sdlpp::get_clipboard_text();
        return current != last_clipboard_text;
    }
};
```

### Text Editor Integration

```cpp
class text_editor {
    std::string content;
    std::string selected_text;
    
public:
    void handle_event(const sdlpp::event& e) {
        e.visit([this](auto&& event) {
            using T = std::decay_t<decltype(event)>;
            
            if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
                if (event.is_pressed() && !event.repeat) {
                    handle_keyboard(event);
                }
            }
        });
    }
    
private:
    void handle_keyboard(const sdlpp::keyboard_event& e) {
        // Copy
        if (e.mods.ctrl() && e.scan == sdlpp::scancode::c) {
            if (!selected_text.empty()) {
                sdlpp::set_clipboard_text(selected_text);
            }
        }
        // Cut
        else if (e.mods.ctrl() && e.scan == sdlpp::scancode::x) {
            if (!selected_text.empty()) {
                sdlpp::set_clipboard_text(selected_text);
                delete_selection();
            }
        }
        // Paste
        else if (e.mods.ctrl() && e.scan == sdlpp::scancode::v) {
            if (sdlpp::has_clipboard_text()) {
                insert_text(sdlpp::get_clipboard_text());
            }
        }
        // Select All
        else if (e.mods.ctrl() && e.scan == sdlpp::scancode::a) {
            select_all();
        }
    }
    
    void insert_text(const std::string& text) {
        // Insert at cursor position
        content.insert(cursor_position, text);
        cursor_position += text.length();
    }
    
    void delete_selection() {
        // Remove selected text
        content.erase(selection_start, selection_end - selection_start);
        selected_text.clear();
    }
};
```

### Clipboard History

```cpp
class clipboard_history {
    std::deque<std::string> history;
    size_t max_entries = 10;
    std::string last_clipboard;
    
public:
    void update() {
        if (sdlpp::has_clipboard_text()) {
            auto current = sdlpp::get_clipboard_text();
            
            if (current != last_clipboard && !current.empty()) {
                // Add to history
                history.push_front(current);
                
                // Remove duplicates
                history.erase(
                    std::remove(history.begin() + 1, history.end(), current),
                    history.end()
                );
                
                // Limit size
                if (history.size() > max_entries) {
                    history.resize(max_entries);
                }
                
                last_clipboard = current;
            }
        }
    }
    
    const std::deque<std::string>& get_history() const {
        return history;
    }
    
    void paste_from_history(size_t index) {
        if (index < history.size()) {
            sdlpp::set_clipboard_text(history[index]);
        }
    }
    
    void clear_history() {
        history.clear();
    }
};
```

### Rich Text Support

While SDL only supports plain text, you can implement rich text copying:

```cpp
class rich_text_clipboard {
    struct rich_text {
        std::string plain_text;
        std::string html;
        std::string rtf;
    };
    
    // Store rich text data in application
    static std::optional<rich_text> rich_data;
    
public:
    static void copy_rich_text(const rich_text& text) {
        // Store rich data internally
        rich_data = text;
        
        // Put plain text version in system clipboard
        sdlpp::set_clipboard_text(text.plain_text);
    }
    
    static std::optional<rich_text> paste_rich_text() {
        // Check if clipboard text matches our stored rich text
        auto clipboard = sdlpp::get_clipboard_text();
        
        if (rich_data && rich_data->plain_text == clipboard) {
            return rich_data;
        }
        
        // Only plain text available
        return rich_text{clipboard, "", ""};
    }
};
```

### Drag and Drop Integration

```cpp
class drag_drop_handler {
    std::string drag_data;
    bool dragging = false;
    
public:
    void start_drag(const std::string& data) {
        drag_data = data;
        dragging = true;
        
        // Also put in clipboard for external drops
        sdlpp::set_clipboard_text(data);
    }
    
    void handle_drop(int x, int y) {
        if (dragging) {
            // Internal drop
            process_drop(drag_data, x, y);
        } else if (sdlpp::has_clipboard_text()) {
            // External drop - try clipboard
            process_drop(sdlpp::get_clipboard_text(), x, y);
        }
        
        dragging = false;
    }
    
private:
    void process_drop(const std::string& data, int x, int y) {
        // Handle the dropped data
    }
};
```

## Platform-Specific Behavior

### Format Conversion

Different platforms handle line endings differently:

```cpp
std::string normalize_line_endings(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\r') {
            if (i + 1 < text.size() && text[i + 1] == '\n') {
                i++;  // Skip \n in \r\n
            }
            result += '\n';
        } else {
            result += text[i];
        }
    }
    
    return result;
}

// When pasting
auto text = sdlpp::get_clipboard_text();
text = normalize_line_endings(text);
```

### Clipboard Monitoring

Monitor clipboard changes:

```cpp
class clipboard_monitor {
    std::thread monitor_thread;
    std::atomic<bool> running{true};
    std::function<void(const std::string&)> callback;
    
public:
    void start(std::function<void(const std::string&)> on_change) {
        callback = on_change;
        
        monitor_thread = std::thread([this]() {
            std::string last_content;
            
            while (running) {
                if (sdlpp::has_clipboard_text()) {
                    auto current = sdlpp::get_clipboard_text();
                    if (current != last_content) {
                        last_content = current;
                        callback(current);
                    }
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
    }
    
    ~clipboard_monitor() {
        running = false;
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }
};
```

## Best Practices

1. **Check Availability**: Always check `has_clipboard_text()` before pasting
2. **Handle Empty**: Gracefully handle empty clipboard
3. **Validate Content**: Validate pasted content before using
4. **User Feedback**: Provide feedback when copy/paste operations occur
5. **Privacy**: Be mindful of sensitive data in clipboard

## Example: Complete Clipboard System

```cpp
class clipboard_system {
    clipboard_manager manager;
    clipboard_history history;
    
    // Clipboard shortcuts
    struct shortcuts {
        sdlpp::scancode copy = sdlpp::scancode::c;
        sdlpp::scancode cut = sdlpp::scancode::x;
        sdlpp::scancode paste = sdlpp::scancode::v;
        sdlpp::key_mod modifier = sdlpp::key_mod::ctrl;
    } shortcuts;
    
public:
    void update() {
        history.update();
    }
    
    void handle_event(const sdlpp::keyboard_event& e) {
        if (!e.is_pressed() || !e.mods.has(shortcuts.modifier)) {
            return;
        }
        
        if (e.scan == shortcuts.copy) {
            handle_copy();
        } else if (e.scan == shortcuts.cut) {
            handle_cut();
        } else if (e.scan == shortcuts.paste) {
            handle_paste();
        }
    }
    
    bool copy_text(const std::string& text) {
        return manager.copy(text);
    }
    
    std::string paste_text() {
        return manager.paste();
    }
    
    void show_history_menu() {
        auto& items = history.get_history();
        
        for (size_t i = 0; i < items.size(); ++i) {
            // Create menu item with preview
            std::string preview = items[i].substr(0, 50);
            if (items[i].size() > 50) {
                preview += "...";
            }
            
            // On selection
            if (menu_item_selected(i)) {
                history.paste_from_history(i);
            }
        }
    }
    
private:
    void handle_copy() {
        if (auto text = get_selected_text()) {
            if (copy_text(*text)) {
                show_notification("Copied to clipboard");
            }
        }
    }
    
    void handle_cut() {
        if (auto text = get_selected_text()) {
            if (copy_text(*text)) {
                delete_selected_text();
                show_notification("Cut to clipboard");
            }
        }
    }
    
    void handle_paste() {
        auto text = paste_text();
        if (!text.empty()) {
            insert_text_at_cursor(text);
            show_notification("Pasted from clipboard");
        }
    }
};
```