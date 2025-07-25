#include <doctest/doctest.h>
#include <sdlpp/system/clipboard.hh>
#include <sdlpp/core/core.hh>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

TEST_SUITE("clipboard") {
    TEST_CASE("clipboard text operations") {
        // Initialize SDL (clipboard requires video subsystem)
        auto init = sdlpp::init(sdlpp::init_flags::video);
        if (init) {
            SUBCASE("set and get text") {
                const std::string test_text = "Hello, SDL++ clipboard!";
                
                // Set text
                auto result = sdlpp::clipboard::set_text(test_text);
                CHECK(result.has_value());
                
                // Get text back
                std::string retrieved = sdlpp::clipboard::get_text();
                CHECK(retrieved == test_text);
                
                // Check has_text
                CHECK(sdlpp::clipboard::has_text());
            }
            
            SUBCASE("empty text") {
                auto result = sdlpp::clipboard::set_text("");
                CHECK(result.has_value());
                
                std::string retrieved = sdlpp::clipboard::get_text();
                CHECK(retrieved.empty());
            }
            
            SUBCASE("unicode text") {
                const std::string unicode_text = reinterpret_cast<const char*>(u8"Hello 世界 🌍 émojis!");
                
                auto result = sdlpp::clipboard::set_text(unicode_text);
                CHECK(result.has_value());
                
                std::string retrieved = sdlpp::clipboard::get_text();
                CHECK(retrieved == unicode_text);
            }
            
            SUBCASE("clear clipboard") {
                // Set some text first
                [[maybe_unused]] auto r = sdlpp::clipboard::set_text("test");
                CHECK(sdlpp::clipboard::has_text());
                
                // Clear it
                auto result = sdlpp::clipboard::clear();
                CHECK(result.has_value());
                
                // Should have no text now
                CHECK(!sdlpp::clipboard::has_text());
            }
        }
    }
    
    TEST_CASE("primary selection operations") {
        auto init = sdlpp::init(sdlpp::init_flags::video);
        if (init) {
            SUBCASE("set and get primary selection") {
                const std::string test_text = "Primary selection text";
                
                // Set primary selection
                auto result = sdlpp::clipboard::set_primary_selection_text(test_text);
                // Note: May fail on non-X11 platforms
                if (result) {
                    std::string retrieved = sdlpp::clipboard::get_primary_selection_text();
                    CHECK(retrieved == test_text);
                    
                    CHECK(sdlpp::clipboard::has_primary_selection_text());
                }
            }
        }
    }
    
    TEST_CASE("clipboard MIME types") {
        auto init = sdlpp::init(sdlpp::init_flags::video);
        if (init) {
            SUBCASE("get MIME types for text") {
                // Set plain text
                [[maybe_unused]] auto r = sdlpp::clipboard::set_text("Test");
                
                auto types = sdlpp::clipboard::get_mime_types();
                CHECK(!types.empty());
                
                // Should at least have text/plain
                bool has_text_plain = false;
                for (const auto& type : types) {
                    if (type == "text/plain" || type == "text/plain;charset=utf-8") {
                        has_text_plain = true;
                        break;
                    }
                }
                CHECK(has_text_plain);
            }
            /*
            SUBCASE("check has_data for MIME type") {
                [[maybe_unused]] auto r = sdlpp::clipboard::set_text("Test");
                
                // Should have text/plain data
                CHECK(sdlpp::clipboard::has_data("text/plain"));
                
                // Should not have arbitrary type
                CHECK(!sdlpp::clipboard::has_data("application/octet-stream"));
            }
            */
        }
    }
    
    TEST_CASE("clipboard data operations") {
        auto init = sdlpp::init(sdlpp::init_flags::video);
        if (init) {
            SUBCASE("set and get binary data") {
                // Prepare test data
                const char* mime_types[] = {"application/octet-stream", "text/plain"};
                std::vector<uint8_t> binary_data = {0x01, 0x02, 0x03, 0x04, 0xFF};
                std::string text_data = "Plain text";
                
                std::span<const uint8_t> data_spans[] = {
                    {binary_data.data(), binary_data.size()},
                    {reinterpret_cast<const uint8_t*>(text_data.data()), text_data.size()}
                };
                
                // Set multiple formats
                auto result = sdlpp::clipboard::set_data(mime_types, data_spans, 2);
                CHECK(result.has_value());
                
                // Get binary data back
                auto retrieved_binary = sdlpp::clipboard::get_data("application/octet-stream");
                CHECK(retrieved_binary == binary_data);
                
                // Get text data back
                auto retrieved_text_data = sdlpp::clipboard::get_data("text/plain");
                std::string retrieved_text(retrieved_text_data.begin(), retrieved_text_data.end());
                CHECK(retrieved_text == text_data);
                
                // Check MIME types
                auto types = sdlpp::clipboard::get_mime_types();
                CHECK(types.size() >= 2);
            }
            
            SUBCASE("set HTML and plain text") {
                const char* mime_types[] = {"text/html", "text/plain"};
                std::string html = "<b>Bold text</b>";
                std::string plain = "Bold text";
                
                std::span<const uint8_t> data_spans[] = {
                    {reinterpret_cast<const uint8_t*>(html.data()), html.size()},
                    {reinterpret_cast<const uint8_t*>(plain.data()), plain.size()}
                };
                
                auto result = sdlpp::clipboard::set_data(mime_types, data_spans, 2);
                CHECK(result.has_value());
                
                // Should have both types
                CHECK(sdlpp::clipboard::has_data("text/html"));
                CHECK(sdlpp::clipboard::has_data("text/plain"));
                
                // Get HTML back
                auto html_data = sdlpp::clipboard::get_data("text/html");
                std::string retrieved_html(html_data.begin(), html_data.end());
                CHECK(retrieved_html == html);
            }
        }
    }
    
    TEST_CASE("clipboard data provider") {
        auto init = sdlpp::init(sdlpp::init_flags::video);
        if (init) {
            SUBCASE("lazy data generation") {
                struct provider_data {
                    int call_count = 0;
                    std::string generated_data;
                };
                
                provider_data pdata;
                
                auto data_callback = [](void* userdata, const char* mime_type, size_t* size) -> const void* {
                    auto* pd = static_cast<provider_data*>(userdata);
                    pd->call_count++;
                    
                    if (std::string(mime_type) == "text/plain") {
                        pd->generated_data = "Generated at call " + std::to_string(pd->call_count);
                        *size = pd->generated_data.size();
                        return pd->generated_data.data();
                    }
                    
                    return nullptr;
                };
                
                std::vector<std::string> types = {"text/plain", "text/custom"};
                auto result = sdlpp::clipboard::data_provider::set(
                    types, data_callback, nullptr, &pdata);
                CHECK(result.has_value());
                
                // Provider should not be called yet
                CHECK(pdata.call_count == 0);
                
                // Getting data should trigger the callback
                auto data = sdlpp::clipboard::get_data("text/plain");
                if (!data.empty()) {
                    // Callback may or may not be called depending on platform
                    std::string text(data.begin(), data.end());
                    MESSAGE("Retrieved: " << text);
                }
            }
        }
    }
}