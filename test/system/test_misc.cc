#include <doctest/doctest.h>
#include <sdlpp/system/misc.hh>
#include <sdlpp/core/core.hh>

TEST_SUITE("misc") {
    TEST_CASE("url helper functions") {
        SUBCASE("has_protocol") {
            // Common protocols
            CHECK(sdlpp::url::has_protocol("http://example.com"));
            CHECK(sdlpp::url::has_protocol("https://example.com"));
            CHECK(sdlpp::url::has_protocol("file:///home/user"));
            CHECK(sdlpp::url::has_protocol("ftp://ftp.example.com"));
            CHECK(sdlpp::url::has_protocol("mailto:user@example.com"));
            CHECK(sdlpp::url::has_protocol("tel:+1234567890"));
            
            // No protocol
            CHECK(!sdlpp::url::has_protocol("example.com"));
            CHECK(!sdlpp::url::has_protocol("www.example.com"));
            CHECK(!sdlpp::url::has_protocol("/home/user/file.txt"));
            CHECK(!sdlpp::url::has_protocol("C:\\Windows\\System32"));
            
            // Special app protocols
            CHECK(sdlpp::url::has_protocol("steam://run/123456"));
            CHECK(sdlpp::url::has_protocol("discord://discord.com/invite/abc"));
            
            // Edge cases
            CHECK(!sdlpp::url::has_protocol(""));
            CHECK(!sdlpp::url::has_protocol("http"));
            CHECK(!sdlpp::url::has_protocol("https"));
        }
        
        SUBCASE("ensure_protocol") {
            // Already has protocol
            CHECK(sdlpp::url::ensure_protocol("http://example.com") == "http://example.com");
            CHECK(sdlpp::url::ensure_protocol("https://example.com") == "https://example.com");
            CHECK(sdlpp::url::ensure_protocol("ftp://example.com") == "ftp://example.com");
            CHECK(sdlpp::url::ensure_protocol("mailto:user@example.com") == "mailto:user@example.com");
            
            // Needs protocol
            CHECK(sdlpp::url::ensure_protocol("example.com") == "https://example.com");
            CHECK(sdlpp::url::ensure_protocol("www.example.com") == "https://www.example.com");
            CHECK(sdlpp::url::ensure_protocol("example.com/path") == "https://example.com/path");
            
            // Edge cases
            CHECK(sdlpp::url::ensure_protocol("") == "https://");
        }
        
        SUBCASE("make_mailto") {
            // Simple email
            CHECK(sdlpp::url::make_mailto("user@example.com") == "mailto:user@example.com");
            
            // With subject
            CHECK(sdlpp::url::make_mailto("user@example.com", "Hello") == 
                  "mailto:user@example.com?subject=Hello");
            
            // With body
            CHECK(sdlpp::url::make_mailto("user@example.com", "", "Message body") == 
                  "mailto:user@example.com?body=Message body");
            
            // With both subject and body
            CHECK(sdlpp::url::make_mailto("user@example.com", "Subject", "Body") == 
                  "mailto:user@example.com?subject=Subject&body=Body");
            
            // Empty email
            CHECK(sdlpp::url::make_mailto("") == "mailto:");
            
            // Note: This function doesn't URL-encode, so spaces remain
            CHECK(sdlpp::url::make_mailto("support@example.com", "Bug Report", "I found a bug") == 
                  "mailto:support@example.com?subject=Bug Report&body=I found a bug");
        }
        
        SUBCASE("make_file_url") {
            // Unix-style paths
            CHECK(sdlpp::url::make_file_url("/home/user/file.txt") == "file:///home/user/file.txt");
            CHECK(sdlpp::url::make_file_url("/tmp/test") == "file:///tmp/test");
            
            // Windows-style paths (backslashes converted)
            CHECK(sdlpp::url::make_file_url("C:\\Users\\User\\Documents") == "file:///C:/Users/User/Documents");
            CHECK(sdlpp::url::make_file_url("D:\\test.txt") == "file:///D:/test.txt");
            
            // Relative paths (gets leading slash added)
            CHECK(sdlpp::url::make_file_url("relative/path") == "file:///relative/path");
            
            // Already has leading slash
            CHECK(sdlpp::url::make_file_url("/already/absolute") == "file:///already/absolute");
            
            // Empty path
            CHECK(sdlpp::url::make_file_url("") == "file://");
            
            // Mixed slashes
            CHECK(sdlpp::url::make_file_url("C:\\Users\\User/Documents\\file.txt") == 
                  "file:///C:/Users/User/Documents/file.txt");
        }
    }
    
    TEST_CASE("open_url") {
        // Note: We can't actually test opening URLs in unit tests as it would
        // launch external applications. We can only test that the function exists
        // and returns the expected type.
        
        // The function should exist and be callable
        using result_type = decltype(sdlpp::open_url("https://example.com"));
        static_assert(std::is_same_v<result_type, tl::expected<void, std::string>>);
        
        // We could test with an invalid URL to see if it fails appropriately
        // but behavior is platform-dependent
        
        INFO("open_url function exists and has correct signature");
    }
}