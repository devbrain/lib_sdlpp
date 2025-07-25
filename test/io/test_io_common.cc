#include <doctest/doctest.h>
#include <sdlpp/io/io_common.hh>

TEST_SUITE("io_common") {
    TEST_CASE("file_mode to_string conversion") {
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::read)) == "r");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::write)) == "w");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::append)) == "a");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::read_update)) == "r+");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::write_update)) == "w+");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::append_update)) == "a+");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::read_binary)) == "rb");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::write_binary)) == "wb");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::append_binary)) == "ab");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::read_update_binary)) == "r+b");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::write_update_binary)) == "w+b");
        CHECK(std::string(sdlpp::to_string(sdlpp::file_mode::append_update_binary)) == "a+b");
    }
    
    TEST_CASE("file_mode predicates") {
        SUBCASE("is_read_mode") {
            CHECK(sdlpp::is_read_mode(sdlpp::file_mode::read));
            CHECK(sdlpp::is_read_mode(sdlpp::file_mode::read_update));
            CHECK(sdlpp::is_read_mode(sdlpp::file_mode::write_update));
            CHECK(sdlpp::is_read_mode(sdlpp::file_mode::append_update));
            CHECK(sdlpp::is_read_mode(sdlpp::file_mode::read_binary));
            CHECK(sdlpp::is_read_mode(sdlpp::file_mode::read_update_binary));
            CHECK(sdlpp::is_read_mode(sdlpp::file_mode::write_update_binary));
            CHECK(sdlpp::is_read_mode(sdlpp::file_mode::append_update_binary));
            
            CHECK_FALSE(sdlpp::is_read_mode(sdlpp::file_mode::write));
            CHECK_FALSE(sdlpp::is_read_mode(sdlpp::file_mode::append));
            CHECK_FALSE(sdlpp::is_read_mode(sdlpp::file_mode::write_binary));
            CHECK_FALSE(sdlpp::is_read_mode(sdlpp::file_mode::append_binary));
        }
        
        SUBCASE("is_write_mode") {
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::write));
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::append));
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::read_update));
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::write_update));
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::append_update));
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::write_binary));
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::append_binary));
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::read_update_binary));
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::write_update_binary));
            CHECK(sdlpp::is_write_mode(sdlpp::file_mode::append_update_binary));
            
            CHECK_FALSE(sdlpp::is_write_mode(sdlpp::file_mode::read));
            CHECK_FALSE(sdlpp::is_write_mode(sdlpp::file_mode::read_binary));
        }
        
        SUBCASE("is_binary_mode") {
            CHECK(sdlpp::is_binary_mode(sdlpp::file_mode::read_binary));
            CHECK(sdlpp::is_binary_mode(sdlpp::file_mode::write_binary));
            CHECK(sdlpp::is_binary_mode(sdlpp::file_mode::append_binary));
            CHECK(sdlpp::is_binary_mode(sdlpp::file_mode::read_update_binary));
            CHECK(sdlpp::is_binary_mode(sdlpp::file_mode::write_update_binary));
            CHECK(sdlpp::is_binary_mode(sdlpp::file_mode::append_update_binary));
            
            CHECK_FALSE(sdlpp::is_binary_mode(sdlpp::file_mode::read));
            CHECK_FALSE(sdlpp::is_binary_mode(sdlpp::file_mode::write));
            CHECK_FALSE(sdlpp::is_binary_mode(sdlpp::file_mode::append));
            CHECK_FALSE(sdlpp::is_binary_mode(sdlpp::file_mode::read_update));
            CHECK_FALSE(sdlpp::is_binary_mode(sdlpp::file_mode::write_update));
            CHECK_FALSE(sdlpp::is_binary_mode(sdlpp::file_mode::append_update));
        }
    }
}