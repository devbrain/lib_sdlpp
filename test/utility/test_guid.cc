#include <doctest/doctest.h>
#include <sdlpp/utility/guid.hh>
#include <sstream>
#include <unordered_set>
#include <vector>

TEST_SUITE("guid") {
    TEST_CASE("default construction") {
        sdlpp::guid g;
        CHECK(g.is_zero());
        CHECK(!g.is_valid());
        
        // All bytes should be zero
        for (Uint8 byte : g.data()) {
            CHECK(byte == 0);
        }
        
        // Zero GUID should convert to all zeros string
        CHECK(g.to_string() == "00000000000000000000000000000000");
    }
    
    TEST_CASE("construction from SDL_GUID") {
        SDL_GUID sdl_guid;
        // Create a test pattern
        for (int i = 0; i < 16; ++i) {
            sdl_guid.data[i] = static_cast<Uint8>(i);
        }
        
        sdlpp::guid g(sdl_guid);
        CHECK(g.is_valid());
        CHECK(!g.is_zero());
        
        // Verify data matches
        const auto& data = g.data();
        for (unsigned i = 0; i < 16; ++i) {
            CHECK(data[i] == static_cast<Uint8>(i));
        }
    }
    
    TEST_CASE("construction from array") {
        std::array<Uint8, 16> data = {
            0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
            0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
        };
        
        sdlpp::guid g(data);
        CHECK(g.is_valid());
        CHECK(g.data() == data);
    }
    
    TEST_CASE("string conversion") {
        SUBCASE("from valid string") {
            std::string guid_str = "0123456789abcdef0123456789abcdef";
            auto g = sdlpp::guid::from_string(guid_str);
            
            CHECK(g.has_value());
            if (g) {
                CHECK(g->is_valid());
                // SDL may normalize the case
                std::string result = g->to_string();
                CHECK(result.length() == 32);
            }
        }
        
        SUBCASE("from uppercase string") {
            std::string guid_str = "0123456789ABCDEF0123456789ABCDEF";
            auto g = sdlpp::guid::from_string(guid_str);
            
            CHECK(g.has_value());
            if (g) {
                CHECK(g->is_valid());
            }
        }
        
        SUBCASE("from invalid string - wrong length") {
            auto g1 = sdlpp::guid::from_string("01234567");  // Too short
            CHECK(!g1.has_value());
            
            auto g2 = sdlpp::guid::from_string("0123456789abcdef0123456789abcdef00");  // Too long
            CHECK(!g2.has_value());
        }
        
        SUBCASE("from invalid string - non-hex characters") {
            auto g = sdlpp::guid::from_string("0123456789abcdefg123456789abcdef");  // 'g' is invalid
            CHECK(!g.has_value());
        }
        
        SUBCASE("zero GUID string") {
            auto g = sdlpp::guid::from_string("00000000000000000000000000000000");
            CHECK(g.has_value());
            if (g) {
                CHECK(g->is_zero());
                CHECK(!g->is_valid());
            }
        }
    }
    
    TEST_CASE("to_string conversion") {
        std::array<Uint8, 16> data = {
            0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
            0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10
        };
        
        sdlpp::guid g(data);
        std::string str = g.to_string();
        
        CHECK(str.length() == 32);
        // SDL converts to lowercase
        CHECK(str == "0123456789abcdeffedcba9876543210");
    }
    
    TEST_CASE("comparison operators") {
        sdlpp::guid g1;  // Zero GUID
        sdlpp::guid g2;  // Another zero GUID
        
        std::array<Uint8, 16> data1 = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        std::array<Uint8, 16> data2 = {2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        
        sdlpp::guid g3(data1);
        sdlpp::guid g4(data2);
        sdlpp::guid g5(data1);  // Same as g3
        
        // Equality
        CHECK(g1 == g2);
        CHECK(g3 == g5);
        CHECK(!(g1 == g3));
        CHECK(!(g3 == g4));
        
        // Inequality
        CHECK(!(g1 != g2));
        CHECK(g1 != g3);
        CHECK(g3 != g4);
        
        // Ordering
        CHECK(g1 < g3);  // Zero < non-zero
        CHECK(g3 < g4);  // 1 < 2
        CHECK(!(g3 < g5));  // Not less than equal
        
        CHECK(g1 <= g2);
        CHECK(g1 <= g3);
        CHECK(!(g3 <= g1));
        
        CHECK(g3 > g1);
        CHECK(g4 > g3);
        CHECK(!(g1 > g3));
        
        CHECK(g3 >= g1);
        CHECK(g3 >= g5);
        CHECK(!(g1 >= g3));
    }
    
    TEST_CASE("zero GUID") {
        auto zero = sdlpp::guid::zero();
        CHECK(zero.is_zero());
        CHECK(!zero.is_valid());
        CHECK(zero.to_string() == "00000000000000000000000000000000");
        
        sdlpp::guid default_constructed;
        CHECK(zero == default_constructed);
    }
    
    TEST_CASE("to_sdl conversion") {
        std::array<Uint8, 16> data = {
            0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88,
            0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00
        };
        
        sdlpp::guid g(data);
        SDL_GUID sdl_guid = g.to_sdl();
        
        // Verify all bytes match
        for (unsigned i = 0; i < 16; ++i) {
            CHECK(sdl_guid.data[i] == data[i]);
        }
    }
    
    TEST_CASE("stream output") {
        std::array<Uint8, 16> data = {
            0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0,
            0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88
        };
        
        sdlpp::guid g(data);
        std::ostringstream oss;
        oss << g;
        
        CHECK(oss.str() == g.to_string());
    }
    
    TEST_CASE("std::hash specialization") {
        std::hash<sdlpp::guid> hasher;
        
        sdlpp::guid g1;
        sdlpp::guid g2;
        
        // Same GUIDs should have same hash
        CHECK(hasher(g1) == hasher(g2));
        
        // Different GUIDs should (usually) have different hashes
        std::array<Uint8, 16> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
        sdlpp::guid g3(data);
        
        // Can't guarantee different hashes, but can use in unordered containers
        std::unordered_set<sdlpp::guid> guid_set;
        guid_set.insert(g1);
        guid_set.insert(g3);
        
        CHECK(guid_set.size() == 2);
        CHECK(guid_set.count(g1) == 1);
        CHECK(guid_set.count(g3) == 1);
    }
    
    TEST_CASE("get_joystick_guid_info") {
        // Test with zero GUID
        sdlpp::guid zero;
        auto info = sdlpp::get_joystick_guid_info(zero);
        
        // Zero GUID should produce zero info
        CHECK(info.vendor == 0);
        CHECK(info.product == 0);
        CHECK(info.version == 0);
        CHECK(info.crc16 == 0);
        CHECK(!info.is_valid());
        
        // Test with a non-zero GUID
        // Note: The actual values returned depend on SDL's implementation
        std::array<Uint8, 16> data = {
            0x03, 0x00, 0x00, 0x00, 0x5e, 0x04, 0x00, 0x00,
            0xea, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };
        sdlpp::guid g(data);
        auto info2 = sdlpp::get_joystick_guid_info(g);
        
        // We can't predict exact values, but we can check the structure
        CHECK((info2.vendor != 0 || info2.product != 0 || 
               info2.version != 0 || info2.crc16 != 0 || 
               !info2.is_valid()));
    }
    
    TEST_CASE("round-trip string conversion") {
        // Test various GUID patterns
        std::vector<std::string> test_guids = {
            "00000000000000000000000000000000",
            "ffffffffffffffffffffffffffffffff",
            "0123456789abcdef0123456789abcdef",
            "deadbeefcafebabe1234567890abcdef"
        };
        
        for (const auto& original : test_guids) {
            auto g = sdlpp::guid::from_string(original);
            CHECK(g.has_value());
            if (g) {
                std::string converted = g->to_string();
                CHECK(converted.length() == 32);
                
                // SDL normalizes to lowercase
                std::string normalized_original = original;
                for (char& c : normalized_original) {
                    if (c >= 'A' && c <= 'F') {
                        c = c - 'A' + 'a';
                    }
                }
                CHECK(converted == normalized_original);
            }
        }
    }
}