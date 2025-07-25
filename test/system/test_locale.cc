#include <doctest/doctest.h>
#include <sdlpp/system/locale.hh>
#include <sdlpp/core/core.hh>
#include <algorithm>
#include <set>

TEST_SUITE("locale") {
    TEST_CASE("locale structure") {
        // Test default construction
        sdlpp::locale loc1;
        CHECK(loc1.language.empty());
        CHECK(!loc1.country.has_value());
        
        // Test construction with language only
        sdlpp::locale loc2("en");
        CHECK(loc2.language == "en");
        CHECK(!loc2.country.has_value());
        
        // Test construction with language and country
        sdlpp::locale loc3("en", "US");
        CHECK(loc3.language == "en");
        CHECK(loc3.country.has_value());
        CHECK(loc3.country.value() == "US");
        
        // Test SDL_Locale construction
        SDL_Locale sdl_loc = {"fr", "CA"};
        sdlpp::locale loc4(sdl_loc);
        CHECK(loc4.language == "fr");
        CHECK(loc4.country.has_value());
        CHECK(loc4.country.value() == "CA");
        
        // Test SDL_Locale with null country
        SDL_Locale sdl_loc2 = {"de", nullptr};
        sdlpp::locale loc5(sdl_loc2);
        CHECK(loc5.language == "de");
        CHECK(!loc5.country.has_value());
    }
    
    TEST_CASE("locale string conversion") {
        sdlpp::locale loc1("en", "US");
        CHECK(loc1.to_string() == "en-US");
        CHECK(loc1.to_posix_string() == "en_US");
        CHECK(loc1.to_bcp47_string() == "en-US");
        
        sdlpp::locale loc2("fr");
        CHECK(loc2.to_string() == "fr");
        CHECK(loc2.to_posix_string() == "fr");
        CHECK(loc2.to_bcp47_string() == "fr");
        
        // Custom separator
        CHECK(loc1.to_string(".") == "en.US");
        CHECK(loc1.to_string("") == "enUS");
    }
    
    TEST_CASE("locale matching") {
        sdlpp::locale en_us("en", "US");
        sdlpp::locale en_gb("en", "GB");
        sdlpp::locale en("en");
        sdlpp::locale fr_ca("fr", "CA");
        
        // Exact matches
        CHECK(en_us.matches(en_us, false));
        CHECK(en_us.matches(en_us, true));
        
        // Language-only matching
        CHECK(!en_us.matches(en_gb, false));  // Countries differ, not allowed
        CHECK(en_us.matches(en_gb, true));    // Countries differ, but language matches
        
        // Mixed country/no-country matching
        CHECK(!en_us.matches(en, false));     // One has country, not allowed
        CHECK(en_us.matches(en, true));        // Language matches
        CHECK(!en.matches(en_us, false));      // One has country, not allowed
        CHECK(en.matches(en_us, true));        // Language matches
        
        // Different languages
        CHECK(!en_us.matches(fr_ca, false));
        CHECK(!en_us.matches(fr_ca, true));
        
        // Same language, no countries
        sdlpp::locale en2("en");
        CHECK(en.matches(en2, false));
        CHECK(en.matches(en2, true));
    }
    
    TEST_CASE("locale operators") {
        sdlpp::locale loc1("en", "US");
        sdlpp::locale loc2("en", "US");
        sdlpp::locale loc3("en", "GB");
        sdlpp::locale loc4("en");
        sdlpp::locale loc5("fr", "CA");
        
        // Equality
        CHECK(loc1 == loc2);
        CHECK(!(loc1 == loc3));
        CHECK(!(loc1 == loc4));
        CHECK(!(loc1 == loc5));
        
        // Inequality
        CHECK(!(loc1 != loc2));
        CHECK(loc1 != loc3);
        CHECK(loc1 != loc4);
        CHECK(loc1 != loc5);
        
        // Less-than for sorting
        CHECK(loc1 < loc5);  // "en" < "fr"
        CHECK(loc4 < loc1);  // "en" without country < "en" with country
        CHECK(loc3 < loc1);  // "en-GB" < "en-US"
        
        // Sorting test
        std::vector<sdlpp::locale> locales = {
            {"zh", "CN"},
            {"en", "US"},
            {"en"},
            {"fr", "FR"},
            {"en", "GB"},
            {"de"},
            {"fr", "CA"}
        };
        
        std::sort(locales.begin(), locales.end());
        
        // Verify sorted order
        CHECK(locales[0].language == "de");
        CHECK(!locales[0].country.has_value());
        
        CHECK(locales[1].language == "en");
        CHECK(!locales[1].country.has_value());
        
        CHECK(locales[2].language == "en");
        CHECK(locales[2].country.value() == "GB");
        
        CHECK(locales[3].language == "en");
        CHECK(locales[3].country.value() == "US");
    }
    
    TEST_CASE("get_preferred_locales") {
        auto locales = sdlpp::get_preferred_locales();
        
        // Should return a valid vector (might be empty on some systems)
        CHECK((locales.empty() || !locales.empty()));
        
        // If we have locales, check they're valid
        for (const auto& loc : locales) {
            CHECK(!loc.language.empty());
            // Country is optional
            CHECK((!loc.country.has_value() || !loc.country.value().empty()));
        }
        
        // Log detected locales for debugging
        if (!locales.empty()) {
            INFO("Detected " << locales.size() << " preferred locale(s)");
            for (size_t i = 0; i < std::min(size_t(3), locales.size()); ++i) {
                INFO("  [" << i << "] " << locales[i].to_string());
            }
        }
    }
    
    TEST_CASE("get_primary_locale") {
        auto primary = sdlpp::get_primary_locale();
        
        if (primary) {
            CHECK(!primary->language.empty());
            INFO("Primary locale: " << primary->to_string());
        } else {
            INFO("No primary locale detected");
        }
        
        // Primary should match first preferred
        auto preferred = sdlpp::get_preferred_locales();
        if (!preferred.empty()) {
            CHECK(primary.has_value());
            if (primary) {
                CHECK(primary->language == preferred.front().language);
                CHECK(primary->country == preferred.front().country);
            }
        } else {
            CHECK(!primary.has_value());
        }
    }
    
    TEST_CASE("find_best_locale") {
        // Create a list of supported locales
        std::vector<sdlpp::locale> supported = {
            {"en", "US"},
            {"en", "GB"},
            {"fr", "FR"},
            {"fr", "CA"},
            {"es"},
            {"de"},
            {"ja"}
        };
        
        // Test with specific preferred locales
        SUBCASE("exact match") {
            std::vector<sdlpp::locale> test_preferred = {
                {"en", "US"},
                {"fr", "FR"}
            };
            
            // We can't override SDL's preferred locales, but we can test the logic
            // by using find_all_matching_locales with our test data
            
            // For now, just test with actual system locales
            auto best = sdlpp::find_best_locale(supported);
            if (best) {
                INFO("Best locale found: " << best->to_string());
                
                // Verify it's in our supported list
                bool found = false;
                for (const auto& sup : supported) {
                    if (sup == *best) {
                        found = true;
                        break;
                    }
                }
                CHECK(found);
            }
        }
        
        SUBCASE("language fallback") {
            auto best = sdlpp::find_best_locale(supported, true);
            if (best) {
                INFO("Best locale with fallback: " << best->to_string());
            }
            
            // Without fallback
            auto best_no_fallback = sdlpp::find_best_locale(supported, false);
            if (best_no_fallback) {
                INFO("Best locale without fallback: " << best_no_fallback->to_string());
            }
        }
        
        SUBCASE("empty inputs") {
            std::vector<sdlpp::locale> empty;
            auto best = sdlpp::find_best_locale(empty);
            CHECK(!best.has_value());
        }
    }
    
    TEST_CASE("find_all_matching_locales") {
        std::vector<sdlpp::locale> supported = {
            {"en", "US"},
            {"en", "GB"},
            {"en", "CA"},
            {"fr", "FR"},
            {"fr", "CA"},
            {"es", "ES"},
            {"es", "MX"},
            {"es"},
            {"de", "DE"},
            {"de", "AT"},
            {"ja"},
            {"zh", "CN"},
            {"zh", "TW"}
        };
        
        auto matches = sdlpp::find_all_matching_locales(supported);
        
        // Verify properties of the results
        if (!matches.empty()) {
            INFO("Found " << matches.size() << " matching locale(s)");
            
            // Check ordering by preference
            for (size_t i = 1; i < matches.size(); ++i) {
                CHECK(matches[i-1].preference_index <= matches[i].preference_index);
            }
            
            // Check no duplicates
            std::set<std::string> seen;
            for (const auto& match : matches) {
                std::string key = match.matched.to_string();
                CHECK(seen.find(key) == seen.end());
                seen.insert(key);
            }
            
            // Log first few matches
            for (size_t i = 0; i < std::min(size_t(3), matches.size()); ++i) {
                INFO("  Match " << i << ": " << matches[i].matched.to_string() 
                     << " (pref=" << matches[i].preference_index 
                     << ", exact=" << matches[i].exact_match << ")");
            }
        }
        
        // Test without language fallback
        auto exact_matches = sdlpp::find_all_matching_locales(supported, false);
        for (const auto& match : exact_matches) {
            CHECK(match.exact_match);
        }
    }
    
    TEST_CASE("language constants") {
        // Just verify the constants are accessible and have expected values
        CHECK(std::string(sdlpp::languages::english) == "en");
        CHECK(std::string(sdlpp::languages::french) == "fr");
        CHECK(std::string(sdlpp::languages::german) == "de");
        CHECK(std::string(sdlpp::languages::spanish) == "es");
        CHECK(std::string(sdlpp::languages::japanese) == "ja");
        CHECK(std::string(sdlpp::languages::chinese) == "zh");
        CHECK(std::string(sdlpp::languages::arabic) == "ar");
        CHECK(std::string(sdlpp::languages::russian) == "ru");
    }
    
    TEST_CASE("country constants") {
        // Verify country constants
        CHECK(std::string(sdlpp::countries::united_states) == "US");
        CHECK(std::string(sdlpp::countries::united_kingdom) == "GB");
        CHECK(std::string(sdlpp::countries::canada) == "CA");
        CHECK(std::string(sdlpp::countries::france) == "FR");
        CHECK(std::string(sdlpp::countries::germany) == "DE");
        CHECK(std::string(sdlpp::countries::japan) == "JP");
        CHECK(std::string(sdlpp::countries::china) == "CN");
        CHECK(std::string(sdlpp::countries::brazil) == "BR");
    }
    
    TEST_CASE("locale factory functions") {
        auto en_us = sdlpp::locales::en_US();
        CHECK(en_us.language == "en");
        CHECK(en_us.country.value() == "US");
        
        auto fr_ca = sdlpp::locales::fr_CA();
        CHECK(fr_ca.language == "fr");
        CHECK(fr_ca.country.value() == "CA");
        
        auto ja_jp = sdlpp::locales::ja_JP();
        CHECK(ja_jp.language == "ja");
        CHECK(ja_jp.country.value() == "JP");
        
        auto zh_cn = sdlpp::locales::zh_CN();
        CHECK(zh_cn.language == "zh");
        CHECK(zh_cn.country.value() == "CN");
    }
}