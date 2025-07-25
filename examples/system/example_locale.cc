#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <sdlpp/core/core.hh>
#include <sdlpp/system/locale.hh>

// Example translations database
struct translations {
    std::map<std::string, std::map<std::string, std::string>> strings = {
        {"welcome", {
            {"en", "Welcome!"},
            {"en-US", "Welcome!"},
            {"en-GB", "Welcome!"},
            {"fr", "Bienvenue!"},
            {"fr-FR", "Bienvenue!"},
            {"fr-CA", "Bienvenue!"},
            {"es", "¡Bienvenido!"},
            {"de", "Willkommen!"},
            {"ja", "ようこそ！"},
            {"zh", "欢迎！"},
            {"zh-CN", "欢迎！"},
            {"zh-TW", "歡迎！"}
        }},
        {"goodbye", {
            {"en", "Goodbye!"},
            {"en-US", "Goodbye!"},
            {"en-GB", "Cheerio!"},
            {"fr", "Au revoir!"},
            {"fr-FR", "Au revoir!"},
            {"fr-CA", "À la prochaine!"},
            {"es", "¡Adiós!"},
            {"de", "Auf Wiedersehen!"},
            {"ja", "さようなら！"},
            {"zh", "再见！"},
            {"zh-CN", "再见！"},
            {"zh-TW", "再見！"}
        }},
        {"color", {
            {"en", "color"},
            {"en-US", "color"},
            {"en-GB", "colour"},
            {"fr", "couleur"},
            {"es", "color"},
            {"de", "Farbe"},
            {"ja", "色"},
            {"zh", "颜色"}
        }}
    };
    
    std::string get(const std::string& key, const sdlpp::locale& loc) {
        auto key_it = strings.find(key);
        if (key_it == strings.end()) {
            return key; // Return key if not found
        }
        
        const auto& translation_map = key_it->second;
        
        // Try exact match first
        std::string exact_key = loc.to_string();
        auto it = translation_map.find(exact_key);
        if (it != translation_map.end()) {
            return it->second;
        }
        
        // Try language-only fallback
        it = translation_map.find(loc.language);
        if (it != translation_map.end()) {
            return it->second;
        }
        
        // Default to English if available
        it = translation_map.find("en");
        if (it != translation_map.end()) {
            return it->second;
        }
        
        return key; // Return key as last resort
    }
};

void print_locale_info() {
    std::cout << "=== System Locale Information ===\n\n";
    
    // Get preferred locales
    auto locales = sdlpp::get_preferred_locales();
    
    if (locales.empty()) {
        std::cout << "No locale information available from system.\n";
        return;
    }
    
    std::cout << "Preferred locales (in order of preference):\n";
    for (size_t i = 0; i < locales.size(); ++i) {
        const auto& loc = locales[i];
        std::cout << "  " << std::setw(2) << (i + 1) << ". ";
        std::cout << std::setw(10) << std::left << loc.to_string();
        std::cout << " (language: " << loc.language;
        if (loc.country) {
            std::cout << ", country: " << *loc.country;
        }
        std::cout << ")\n";
        
        // Show different formats
        if (i == 0) {
            std::cout << "      POSIX format: " << loc.to_posix_string() << "\n";
            std::cout << "      BCP 47 format: " << loc.to_bcp47_string() << "\n";
        }
    }
    
    // Get primary locale
    auto primary = sdlpp::get_primary_locale();
    if (primary) {
        std::cout << "\nPrimary locale: " << primary->to_string() << "\n";
    }
}

void demonstrate_locale_matching() {
    std::cout << "\n=== Locale Matching Demo ===\n\n";
    
    // Define what our application supports
    std::vector<sdlpp::locale> supported = {
        sdlpp::locales::en_US(),
        sdlpp::locales::en_GB(),
        sdlpp::locales::fr_FR(),
        sdlpp::locales::fr_CA(),
        {"es"},  // Generic Spanish
        {"de"},  // Generic German
        sdlpp::locales::ja_JP(),
        sdlpp::locales::zh_CN(),
        sdlpp::locales::zh_TW()
    };
    
    std::cout << "Application supports these locales:\n";
    for (const auto& loc : supported) {
        std::cout << "  - " << loc.to_string() << "\n";
    }
    
    // Find best match
    auto best = sdlpp::find_best_locale(supported);
    if (best) {
        std::cout << "\nBest matching locale: " << best->to_string() << "\n";
    } else {
        std::cout << "\nNo matching locale found.\n";
    }
    
    // Find all matches
    auto all_matches = sdlpp::find_all_matching_locales(supported);
    if (!all_matches.empty()) {
        std::cout << "\nAll matching locales (ordered by preference):\n";
        for (const auto& match : all_matches) {
            std::cout << "  - " << match.matched.to_string();
            std::cout << " (preference #" << (match.preference_index + 1);
            std::cout << ", " << (match.exact_match ? "exact" : "language-only") << " match)\n";
        }
    }
}

void demonstrate_translations() {
    std::cout << "\n=== Translation Demo ===\n\n";
    
    translations trans;
    
    // Get system's preferred locale
    auto preferred = sdlpp::get_primary_locale();
    if (!preferred) {
        std::cout << "No system locale available, using en-US\n";
        preferred = sdlpp::locales::en_US();
    }
    
    std::cout << "Using locale: " << preferred->to_string() << "\n\n";
    
    // Show translations
    std::cout << "Translations:\n";
    std::cout << "  welcome: " << trans.get("welcome", *preferred) << "\n";
    std::cout << "  goodbye: " << trans.get("goodbye", *preferred) << "\n";
    std::cout << "  color/colour: " << trans.get("color", *preferred) << "\n";
    
    // Show how different locales get different translations
    std::cout << "\n'goodbye' in different locales:\n";
    std::vector<sdlpp::locale> test_locales = {
        {"en", "US"},
        {"en", "GB"},
        {"fr", "FR"},
        {"fr", "CA"},
        {"es"},
        {"de"},
        {"ja"},
        {"zh", "CN"},
        {"zh", "TW"}
    };
    
    for (const auto& loc : test_locales) {
        std::cout << "  " << std::setw(8) << std::left << loc.to_string() 
                  << " : " << trans.get("goodbye", loc) << "\n";
    }
}

void demonstrate_locale_comparison() {
    std::cout << "\n=== Locale Comparison Demo ===\n\n";
    
    sdlpp::locale en_us("en", "US");
    sdlpp::locale en_gb("en", "GB");
    sdlpp::locale en("en");
    sdlpp::locale fr_ca("fr", "CA");
    
    std::cout << "Locale comparisons:\n";
    std::cout << "  en-US == en-US : " << std::boolalpha << (en_us == en_us) << "\n";
    std::cout << "  en-US == en-GB : " << (en_us == en_gb) << "\n";
    std::cout << "  en-US == en    : " << (en_us == en) << "\n";
    std::cout << "  en-US == fr-CA : " << (en_us == fr_ca) << "\n";
    
    std::cout << "\nLocale matching (with language fallback):\n";
    std::cout << "  en-US matches en-GB : " << en_us.matches(en_gb, true) << "\n";
    std::cout << "  en-US matches en    : " << en_us.matches(en, true) << "\n";
    std::cout << "  en    matches en-US : " << en.matches(en_us, true) << "\n";
    std::cout << "  en-US matches fr-CA : " << en_us.matches(fr_ca, true) << "\n";
    
    std::cout << "\nLocale matching (exact only):\n";
    std::cout << "  en-US matches en-GB : " << en_us.matches(en_gb, false) << "\n";
    std::cout << "  en-US matches en    : " << en_us.matches(en, false) << "\n";
    
    // Sorting demonstration
    std::cout << "\nSorting locales:\n";
    std::vector<sdlpp::locale> to_sort = {
        {"zh", "CN"}, {"en", "US"}, {"en"}, {"fr", "FR"},
        {"en", "GB"}, {"de"}, {"fr", "CA"}
    };
    
    std::cout << "Before: ";
    for (const auto& loc : to_sort) {
        std::cout << loc.to_string() << " ";
    }
    std::cout << "\n";
    
    std::sort(to_sort.begin(), to_sort.end());
    
    std::cout << "After:  ";
    for (const auto& loc : to_sort) {
        std::cout << loc.to_string() << " ";
    }
    std::cout << "\n";
}

int main() {
    try {
        // Initialize SDL
        sdlpp::init sdl_init(sdlpp::init_flags::video);
        
        std::cout << "SDL++ Locale Example\n";
        std::cout << "====================\n\n";
        
        // Show system locale information
        print_locale_info();
        
        // Demonstrate locale matching
        demonstrate_locale_matching();
        
        // Demonstrate translations
        demonstrate_translations();
        
        // Demonstrate locale comparison
        demonstrate_locale_comparison();
        
        std::cout << "\n=== Summary ===\n";
        std::cout << "The locale system allows applications to:\n";
        std::cout << "- Detect user's preferred languages and regions\n";
        std::cout << "- Match user preferences against supported locales\n";
        std::cout << "- Provide appropriate translations and formatting\n";
        std::cout << "- Handle fallbacks when exact matches aren't available\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}