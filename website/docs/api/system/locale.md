---
title: Locale Support
sidebar_label: Locale
---

# Locale and Localization

SDL++ provides comprehensive locale detection and localization support for creating multilingual applications.

## Getting User Locales

### Preferred Locales

```cpp
#include <sdlpp/system/locale.hh>

// Get user's preferred locales (ordered by preference)
auto locales = sdlpp::get_preferred_locales();

for (const auto& locale : locales) {
    std::cout << "Locale: " << locale.to_string() << std::endl;
    std::cout << "  Language: " << locale.language << std::endl;
    if (locale.country) {
        std::cout << "  Country: " << *locale.country << std::endl;
    }
}

// Example output:
// Locale: en_US
//   Language: en
//   Country: US
// Locale: en
//   Language: en
// Locale: fr_FR
//   Language: fr
//   Country: FR
```

### Locale Structure

```cpp
struct locale {
    std::string language;              // ISO 639-1 code (e.g., "en", "fr")
    std::optional<std::string> country; // ISO 3166-1 code (e.g., "US", "GB")
    
    // Convert to string format
    std::string to_string() const {
        if (country) {
            return language + "_" + *country;
        }
        return language;
    }
    
    // Convert to BCP 47 format
    std::string to_bcp47() const {
        if (country) {
            return language + "-" + *country;
        }
        return language;
    }
};
```

## Locale Matching

### Finding Best Match

```cpp
// Your app's supported locales
std::vector<sdlpp::locale> supported_locales = {
    {"en", "US"},  // English (US)
    {"en", "GB"},  // English (UK)
    {"fr", "FR"},  // French (France)
    {"fr", "CA"},  // French (Canada)
    {"de", "DE"},  // German (Germany)
    {"es", "ES"},  // Spanish (Spain)
    {"ja", std::nullopt},  // Japanese
};

// Find best match for user
auto best_locale = sdlpp::find_best_locale(supported_locales);
if (best_locale) {
    std::cout << "Selected locale: " << best_locale->to_string() << std::endl;
    load_translations(best_locale->to_string());
} else {
    std::cout << "No matching locale found, using default" << std::endl;
    load_translations("en_US");
}
```

### Custom Matching Logic

```cpp
// Find locale with fallback
sdlpp::locale find_locale_with_fallback(
    const std::vector<sdlpp::locale>& supported) {
    
    auto user_locales = sdlpp::get_preferred_locales();
    
    // First pass: exact match
    for (const auto& user : user_locales) {
        for (const auto& supported : supported) {
            if (user.language == supported.language &&
                user.country == supported.country) {
                return supported;
            }
        }
    }
    
    // Second pass: language-only match
    for (const auto& user : user_locales) {
        for (const auto& supported : supported) {
            if (user.language == supported.language) {
                return supported;
            }
        }
    }
    
    // Default fallback
    return {"en", "US"};
}
```

## Localization System

### Translation Manager

```cpp
class translation_manager {
    std::unordered_map<std::string, 
        std::unordered_map<std::string, std::string>> translations;
    std::string current_locale = "en_US";
    
public:
    bool load_locale(const std::string& locale_code) {
        std::string path = "locales/" + locale_code + ".json";
        
        auto file_result = sdlpp::io::read_file(path);
        if (!file_result) {
            return false;
        }
        
        // Parse JSON (using your preferred JSON library)
        auto translation_map = parse_json(file_result.value());
        translations[locale_code] = translation_map;
        current_locale = locale_code;
        
        return true;
    }
    
    std::string translate(const std::string& key) const {
        auto locale_it = translations.find(current_locale);
        if (locale_it != translations.end()) {
            auto trans_it = locale_it->second.find(key);
            if (trans_it != locale_it->second.end()) {
                return trans_it->second;
            }
        }
        
        // Fallback to key
        return key;
    }
    
    // Shorthand operator
    std::string operator()(const std::string& key) const {
        return translate(key);
    }
    
    // Format with parameters
    template<typename... Args>
    std::string format(const std::string& key, Args&&... args) const {
        std::string base = translate(key);
        return std::vformat(base, std::make_format_args(args...));
    }
};

// Global instance
translation_manager tr;

// Usage
std::cout << tr("menu.file") << std::endl;  // "File"
std::cout << tr("menu.edit") << std::endl;  // "Edit"
std::cout << tr.format("welcome.message", "John") << std::endl;  // "Welcome, John!"
```

### Locale-Aware Formatting

```cpp
class locale_formatter {
    sdlpp::locale current_locale;
    
public:
    explicit locale_formatter(const sdlpp::locale& loc) 
        : current_locale(loc) {}
    
    // Number formatting
    std::string format_number(double value, int decimals = 2) {
        std::stringstream ss;
        ss.imbue(std::locale(current_locale.to_string()));
        ss << std::fixed << std::setprecision(decimals) << value;
        return ss.str();
    }
    
    // Currency formatting
    std::string format_currency(double amount) {
        // Simplified example - real implementation would use ICU or similar
        if (current_locale.language == "en") {
            if (current_locale.country == "US") {
                return "$" + format_number(amount);
            } else if (current_locale.country == "GB") {
                return "£" + format_number(amount);
            }
        } else if (current_locale.language == "fr") {
            return format_number(amount) + " €";
        } else if (current_locale.language == "ja") {
            return "¥" + format_number(amount, 0);
        }
        
        return format_number(amount);
    }
    
    // Date formatting
    std::string format_date(const std::chrono::system_clock::time_point& time) {
        auto tt = std::chrono::system_clock::to_time_t(time);
        std::stringstream ss;
        
        if (current_locale.language == "en") {
            ss << std::put_time(std::localtime(&tt), "%m/%d/%Y");
        } else if (current_locale.language == "fr" || 
                   current_locale.language == "de") {
            ss << std::put_time(std::localtime(&tt), "%d/%m/%Y");
        } else if (current_locale.language == "ja") {
            ss << std::put_time(std::localtime(&tt), "%Y年%m月%d日");
        }
        
        return ss.str();
    }
};
```

## Locale Detection Patterns

### Application Startup

```cpp
class localized_app {
    translation_manager translations;
    sdlpp::locale current_locale;
    
public:
    void initialize() {
        // Get supported locales from resources
        auto supported = get_supported_locales();
        
        // Find best match
        auto best = sdlpp::find_best_locale(supported);
        if (best) {
            current_locale = *best;
        } else {
            // Fallback
            current_locale = {"en", "US"};
        }
        
        // Load translations
        if (!translations.load_locale(current_locale.to_string())) {
            // Try language-only
            if (!translations.load_locale(current_locale.language)) {
                // Final fallback
                translations.load_locale("en");
            }
        }
        
        std::cout << "Application initialized with locale: " 
                  << current_locale.to_string() << std::endl;
    }
    
private:
    std::vector<sdlpp::locale> get_supported_locales() {
        std::vector<sdlpp::locale> locales;
        
        // Scan locale directory
        std::filesystem::path locale_dir = "locales";
        for (const auto& entry : std::filesystem::directory_iterator(locale_dir)) {
            if (entry.path().extension() == ".json") {
                auto locale_str = entry.path().stem().string();
                
                // Parse locale string
                size_t underscore = locale_str.find('_');
                if (underscore != std::string::npos) {
                    locales.push_back({
                        locale_str.substr(0, underscore),
                        locale_str.substr(underscore + 1)
                    });
                } else {
                    locales.push_back({locale_str, std::nullopt});
                }
            }
        }
        
        return locales;
    }
};
```

### Dynamic Locale Switching

```cpp
class locale_switcher {
    std::function<void(const sdlpp::locale&)> on_locale_changed;
    sdlpp::locale current_locale;
    
public:
    void set_locale_change_handler(
        std::function<void(const sdlpp::locale&)> handler) {
        on_locale_changed = handler;
    }
    
    void switch_locale(const sdlpp::locale& new_locale) {
        if (current_locale.to_string() != new_locale.to_string()) {
            current_locale = new_locale;
            
            if (on_locale_changed) {
                on_locale_changed(new_locale);
            }
        }
    }
    
    void show_locale_menu(sdlpp::renderer& r) {
        // Render locale selection menu
        std::vector<sdlpp::locale> available = {
            {"en", "US"}, {"en", "GB"}, {"fr", "FR"},
            {"de", "DE"}, {"es", "ES"}, {"ja", std::nullopt}
        };
        
        int y = 100;
        for (const auto& locale : available) {
            std::string display_name = get_locale_display_name(locale);
            
            // Render menu item
            if (render_menu_item(r, display_name, 100, y)) {
                switch_locale(locale);
            }
            
            y += 30;
        }
    }
    
private:
    std::string get_locale_display_name(const sdlpp::locale& loc) {
        static std::unordered_map<std::string, std::string> names = {
            {"en_US", "English (US)"},
            {"en_GB", "English (UK)"},
            {"fr_FR", "Français"},
            {"de_DE", "Deutsch"},
            {"es_ES", "Español"},
            {"ja", "日本語"}
        };
        
        auto it = names.find(loc.to_string());
        return it != names.end() ? it->second : loc.to_string();
    }
};
```

### Pluralization Support

```cpp
class pluralization {
public:
    enum class plural_form {
        zero, one, few, many, other
    };
    
    // Get plural form for a number based on locale rules
    static plural_form get_form(int n, const sdlpp::locale& locale) {
        if (locale.language == "en") {
            return (n == 1) ? plural_form::one : plural_form::other;
        }
        else if (locale.language == "fr") {
            return (n == 0 || n == 1) ? plural_form::one : plural_form::other;
        }
        else if (locale.language == "pl") {
            // Polish rules
            if (n == 1) return plural_form::one;
            if (n % 10 >= 2 && n % 10 <= 4 && 
                (n % 100 < 10 || n % 100 >= 20)) {
                return plural_form::few;
            }
            return plural_form::many;
        }
        else if (locale.language == "ru") {
            // Russian rules
            if (n % 10 == 1 && n % 100 != 11) return plural_form::one;
            if (n % 10 >= 2 && n % 10 <= 4 && 
                (n % 100 < 10 || n % 100 >= 20)) {
                return plural_form::few;
            }
            return plural_form::many;
        }
        
        // Default
        return plural_form::other;
    }
    
    // Format message with correct plural form
    static std::string format_plural(
        const std::string& key,
        int count,
        const sdlpp::locale& locale,
        const translation_manager& tr) {
        
        auto form = get_form(count, locale);
        
        std::string plural_key = key + ".";
        switch (form) {
            case plural_form::zero: plural_key += "zero"; break;
            case plural_form::one: plural_key += "one"; break;
            case plural_form::few: plural_key += "few"; break;
            case plural_form::many: plural_key += "many"; break;
            case plural_form::other: plural_key += "other"; break;
        }
        
        return tr.format(plural_key, count);
    }
};

// Usage
int item_count = 5;
std::string message = pluralization::format_plural(
    "items.count", item_count, current_locale, tr
);
// English: "You have 5 items"
// Polish: "Masz 5 przedmiotów" (uses "few" form)
```

## Best Practices

1. **Support Fallbacks**: Always have English or a default locale
2. **Use Standard Codes**: Follow ISO 639-1 and ISO 3166-1
3. **Test All Locales**: Ensure UI works with longest translations
4. **Handle Missing Translations**: Show keys or fallback gracefully
5. **Consider Right-to-Left**: Plan for RTL languages if needed

## Example: Complete Localization System

```cpp
class localization_system {
    struct locale_data {
        sdlpp::locale locale;
        std::string display_name;
        std::string native_name;
        bool rtl = false;
    };
    
    translation_manager translations;
    locale_formatter* formatter = nullptr;
    std::vector<locale_data> available_locales;
    locale_data current;
    
public:
    void initialize() {
        // Setup available locales
        available_locales = {
            {{"en", "US"}, "English (US)", "English", false},
            {{"en", "GB"}, "English (UK)", "English", false},
            {{"fr", "FR"}, "French", "Français", false},
            {{"de", "DE"}, "German", "Deutsch", false},
            {{"es", "ES"}, "Spanish", "Español", false},
            {{"ja", std::nullopt}, "Japanese", "日本語", false},
            {{"ar", "SA"}, "Arabic", "العربية", true},
            {{"zh", "CN"}, "Chinese (Simplified)", "简体中文", false}
        };
        
        // Detect user locale
        auto best = sdlpp::find_best_locale(get_locale_list());
        if (best) {
            set_locale(best->to_string());
        } else {
            set_locale("en_US");
        }
    }
    
    bool set_locale(const std::string& locale_str) {
        // Find locale data
        auto it = std::find_if(available_locales.begin(), 
                               available_locales.end(),
            [&](const locale_data& ld) {
                return ld.locale.to_string() == locale_str;
            });
            
        if (it == available_locales.end()) {
            return false;
        }
        
        // Load translations
        if (!translations.load_locale(locale_str)) {
            // Try language only
            if (!translations.load_locale(it->locale.language)) {
                return false;
            }
        }
        
        // Update current locale
        current = *it;
        
        // Create formatter
        delete formatter;
        formatter = new locale_formatter(current.locale);
        
        // Notify application of RTL change if needed
        if (current.rtl) {
            enable_rtl_layout();
        } else {
            disable_rtl_layout();
        }
        
        return true;
    }
    
    // Translation functions
    std::string tr(const std::string& key) const {
        return translations.translate(key);
    }
    
    template<typename... Args>
    std::string tr_format(const std::string& key, Args&&... args) const {
        return translations.format(key, std::forward<Args>(args)...);
    }
    
    std::string tr_plural(const std::string& key, int count) const {
        return pluralization::format_plural(
            key, count, current.locale, translations
        );
    }
    
    // Formatting functions
    std::string format_number(double value, int decimals = 2) const {
        return formatter ? formatter->format_number(value, decimals) : 
                          std::to_string(value);
    }
    
    std::string format_currency(double amount) const {
        return formatter ? formatter->format_currency(amount) : 
                          "$" + std::to_string(amount);
    }
    
    std::string format_date(
        const std::chrono::system_clock::time_point& time) const {
        return formatter ? formatter->format_date(time) : "N/A";
    }
    
    // Getters
    const std::vector<locale_data>& get_available_locales() const {
        return available_locales;
    }
    
    const locale_data& get_current_locale() const {
        return current;
    }
    
    bool is_rtl() const {
        return current.rtl;
    }
    
private:
    std::vector<sdlpp::locale> get_locale_list() const {
        std::vector<sdlpp::locale> list;
        for (const auto& ld : available_locales) {
            list.push_back(ld.locale);
        }
        return list;
    }
    
    void enable_rtl_layout() {
        // Configure UI for RTL
        sdlpp::set_hint("SDL_HINT_TEXT_DIRECTION", "rtl");
    }
    
    void disable_rtl_layout() {
        sdlpp::set_hint("SDL_HINT_TEXT_DIRECTION", "ltr");
    }
};

// Global instance
localization_system i18n;

// Usage in application
class my_app {
    void show_welcome() {
        std::string user_name = "Alice";
        int message_count = 3;
        
        std::cout << i18n.tr("app.title") << std::endl;
        std::cout << i18n.tr_format("welcome.user", user_name) << std::endl;
        std::cout << i18n.tr_plural("messages.unread", message_count) << std::endl;
        
        auto now = std::chrono::system_clock::now();
        std::cout << i18n.tr("date.today") << ": " 
                  << i18n.format_date(now) << std::endl;
    }
};
```