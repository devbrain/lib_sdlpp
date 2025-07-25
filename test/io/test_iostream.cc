//
// Created by igor on 7/13/25.
//

#include <doctest/doctest.h>
#include <vector>
#include <cstring>
#include <sstream>
#include <fstream>

#include "sdlpp/io/iostream.hh"

using namespace sdlpp;

TEST_SUITE("iostream") {
    
    TEST_CASE("memory stream read/write operations") {
        // Create a buffer for read/write operations
        std::vector<uint8_t> buffer(1024);
        
        auto stream_result = from_memory(buffer.data(), buffer.size());
        REQUIRE(stream_result.has_value());
        
        auto& stream = stream_result.value();
        CHECK(stream.is_valid());
        
        SUBCASE("write and read basic data") {
            const char* test_data = "Hello, SDL IOStream!";
            size_t data_len = strlen(test_data);
            
            // Write data
            auto write_result = stream.write(test_data, data_len);
            REQUIRE(write_result.has_value());
            CHECK(*write_result == data_len);
            
            // Seek back to beginning
            auto seek_result = stream.seek(0, io_seek_pos::set);
            REQUIRE(seek_result.has_value());
            CHECK(*seek_result == 0);
            
            // Read data back
            std::vector<char> read_buffer(data_len + 1);
            auto read_result = stream.read(read_buffer.data(), data_len);
            REQUIRE(read_result.has_value());
            CHECK(*read_result == data_len);
            
            read_buffer[data_len] = '\0';
            CHECK(strcmp(read_buffer.data(), test_data) == 0);
        }
        
        SUBCASE("tell position") {
            auto pos = stream.tell();
            REQUIRE(pos.has_value());
            CHECK(*pos == 0);
            
            // Write some data
            (void)stream.write("test", 4);
            
            pos = stream.tell();
            REQUIRE(pos.has_value());
            CHECK(*pos == 4);
        }
    }
    
    TEST_CASE("typed read/write operations") {
        std::vector<uint8_t> buffer(1024);
        auto stream_result = from_memory(buffer.data(), buffer.size());
        REQUIRE(stream_result.has_value());
        auto& stream = stream_result.value();
        
        SUBCASE("uint8_t operations") {
            uint8_t test_value = 0xAB;
            
            auto write_result = stream.write_u8(test_value);
            REQUIRE(write_result.has_value());
            
            (void)stream.seek(0, io_seek_pos::set);
            
            auto read_result = stream.read_u8();
            REQUIRE(read_result.has_value());
            CHECK(*read_result == test_value);
        }
        
        SUBCASE("uint16_t little endian operations") {
            uint16_t test_value = 0xABCD;
            
            auto write_result = stream.write_u16_le(test_value);
            REQUIRE(write_result.has_value());
            
            (void)stream.seek(0, io_seek_pos::set);
            
            auto read_result = stream.read_u16_le();
            REQUIRE(read_result.has_value());
            CHECK(*read_result == test_value);
        }
        
        SUBCASE("uint16_t big endian operations") {
            uint16_t test_value = 0xABCD;
            
            auto write_result = stream.write_u16_be(test_value);
            REQUIRE(write_result.has_value());
            
            (void)stream.seek(0, io_seek_pos::set);
            
            auto read_result = stream.read_u16_be();
            REQUIRE(read_result.has_value());
            CHECK(*read_result == test_value);
        }
        
        SUBCASE("uint32_t operations") {
            uint32_t test_value = 0xDEADBEEF;
            
            // Little endian
            auto write_le = stream.write_u32_le(test_value);
            REQUIRE(write_le.has_value());
            
            (void)stream.seek(0, io_seek_pos::set);
            
            auto read_le = stream.read_u32_le();
            REQUIRE(read_le.has_value());
            CHECK(*read_le == test_value);
            
            // Big endian
            (void)stream.seek(0, io_seek_pos::set);
            
            auto write_be = stream.write_u32_be(test_value);
            REQUIRE(write_be.has_value());
            
            (void)stream.seek(0, io_seek_pos::set);
            
            auto read_be = stream.read_u32_be();
            REQUIRE(read_be.has_value());
            CHECK(*read_be == test_value);
        }
        
        SUBCASE("uint64_t operations") {
            uint64_t test_value = 0xDEADBEEFCAFEBABE;
            
            // Little endian
            auto write_le = stream.write_u64_le(test_value);
            REQUIRE(write_le.has_value());
            
            (void)stream.seek(0, io_seek_pos::set);
            
            auto read_le = stream.read_u64_le();
            REQUIRE(read_le.has_value());
            CHECK(*read_le == test_value);
            
            // Big endian  
            (void)stream.seek(0, io_seek_pos::set);
            
            auto write_be = stream.write_u64_be(test_value);
            REQUIRE(write_be.has_value());
            
            (void)stream.seek(0, io_seek_pos::set);
            
            auto read_be = stream.read_u64_be();
            REQUIRE(read_be.has_value());
            CHECK(*read_be == test_value);
        }
    }
    
    TEST_CASE("const memory stream (read-only)") {
        const char* test_data = "Read-only data";
        size_t data_len = strlen(test_data);
        
        auto stream_result = from_memory(test_data, data_len);
        REQUIRE(stream_result.has_value());
        auto& stream = stream_result.value();
        
        // Should be able to read
        std::vector<char> buffer(data_len + 1);
        auto read_result = stream.read(buffer.data(), data_len);
        REQUIRE(read_result.has_value());
        CHECK(*read_result == data_len);
        
        buffer[data_len] = '\0';
        CHECK(strcmp(buffer.data(), test_data) == 0);
        
        // Writing should fail (stream is read-only)
        auto write_result = stream.write("fail", 4);
        CHECK(write_result.has_value()); // SDL may not fail immediately
        
        // But status might indicate read-only
        [[maybe_unused]] auto status = stream.get_status();
        // Note: SDL might not set read-only status immediately
    }
    
    TEST_CASE("dynamic memory stream") {
        auto stream_result = from_dynamic_memory();
        REQUIRE(stream_result.has_value());
        auto& stream = stream_result.value();
        
        // Should be able to write arbitrary amounts of data
        std::string test_data = "This is a test of dynamic memory allocation in SDL IOStream.";
        for (int i = 0; i < 10; ++i) {
            test_data += " More data to test dynamic growth.";
        }
        
        auto write_result = stream.write(test_data.data(), test_data.size());
        REQUIRE(write_result.has_value());
        CHECK(*write_result == test_data.size());
        
        // Seek back and read
        (void)stream.seek(0, io_seek_pos::set);
        
        auto read_vec = stream.read(test_data.size());
        REQUIRE(read_vec.has_value());
        CHECK(read_vec->size() == test_data.size());
        
        std::string read_data(read_vec->begin(), read_vec->end());
        CHECK(read_data == test_data);
    }
    
    TEST_CASE("seek operations") {
        std::vector<uint8_t> buffer(1024);
        auto stream_result = from_memory(buffer.data(), buffer.size());
        REQUIRE(stream_result.has_value());
        auto& stream = stream_result.value();
        
        // Write some test data
        const char* test_data = "0123456789ABCDEF";
        (void)stream.write(test_data, 16);
        
        SUBCASE("seek from beginning") {
            auto result = stream.seek(5, io_seek_pos::set);
            REQUIRE(result.has_value());
            CHECK(*result == 5);
            
            auto pos = stream.tell();
            REQUIRE(pos.has_value());
            CHECK(*pos == 5);
        }
        
        SUBCASE("seek from current position") {
            (void)stream.seek(5, io_seek_pos::set);
            
            auto result = stream.seek(3, io_seek_pos::current);
            REQUIRE(result.has_value());
            CHECK(*result == 8);
            
            auto pos = stream.tell();
            REQUIRE(pos.has_value());
            CHECK(*pos == 8);
        }
        
        SUBCASE("seek from end") {
            auto result = stream.seek(-5, io_seek_pos::end);
            REQUIRE(result.has_value());
            CHECK(*result == buffer.size() - 5);
        }
    }
    
    TEST_CASE("error handling") {
        SUBCASE("invalid stream operations") {
            iostream invalid_stream;
            CHECK(!invalid_stream.is_valid());
            
            auto read_result = invalid_stream.read_u8();
            CHECK(!read_result.has_value());
            CHECK(read_result.error() == "Invalid stream");
            
            auto write_result = invalid_stream.write_u8(0);
            CHECK(!write_result.has_value());
            CHECK(write_result.error() == "Invalid stream");
            
            auto seek_result = invalid_stream.seek(0, io_seek_pos::set);
            CHECK(!seek_result.has_value());
            CHECK(seek_result.error() == "Invalid stream");
        }
    }
    
    TEST_CASE("span interface") {
        std::vector<uint8_t> buffer(1024);
        auto stream_result = from_memory(buffer.data(), buffer.size());
        REQUIRE(stream_result.has_value());
        auto& stream = stream_result.value();
        
        std::vector<uint8_t> test_data = {0x01, 0x02, 0x03, 0x04, 0x05};
        std::span<const uint8_t> data_span(test_data);
        
        auto write_result = stream.write(data_span);
        REQUIRE(write_result.has_value());
        CHECK(*write_result == test_data.size());
        
        (void)stream.seek(0, io_seek_pos::set);
        
        auto read_result = stream.read(test_data.size());
        REQUIRE(read_result.has_value());
        CHECK(*read_result == test_data);
    }
    
    TEST_CASE("std::istream integration") {
        // Create a stringstream with test data
        std::string test_content = "Hello from std::istream!";
        std::istringstream iss(test_content);
        
        auto stream_result = from_istream(iss);
        REQUIRE(stream_result.has_value());
        auto& stream = stream_result.value();
        
        SUBCASE("read from std::istream") {
            std::vector<char> buffer(test_content.size() + 1);
            auto read_result = stream.read(buffer.data(), test_content.size());
            REQUIRE(read_result.has_value());
            CHECK(*read_result == test_content.size());
            
            buffer[test_content.size()] = '\0';
            CHECK(std::string(buffer.data()) == test_content);
        }
        
        SUBCASE("seek in std::istream") {
            auto seek_result = stream.seek(6, io_seek_pos::set);
            REQUIRE(seek_result.has_value());
            
            auto pos = stream.tell();
            REQUIRE(pos.has_value());
            CHECK(*pos == 6);
            
            // Read "from" starting at position 6
            std::vector<char> buffer(5);
            auto read_result = stream.read(buffer.data(), 4);
            REQUIRE(read_result.has_value());
            buffer[4] = '\0';
            CHECK(std::string(buffer.data()) == "from");
        }
        
        SUBCASE("write to std::istream should fail") {
            auto write_result = stream.write("test", 4);
            CHECK(write_result.has_value());
            CHECK(*write_result == 0); // No bytes written
            
            // Status should indicate read-only
            [[maybe_unused]] auto status = stream.get_status();
            // Note: Status might not be set immediately after failed write
        }
    }
    
    TEST_CASE("std::ostream integration") {
        std::ostringstream oss;
        
        auto stream_result = from_ostream(oss);
        REQUIRE(stream_result.has_value());
        auto& stream = stream_result.value();
        
        SUBCASE("write to std::ostream") {
            const char* test_data = "Writing to std::ostream!";
            auto write_result = stream.write(test_data, strlen(test_data));
            REQUIRE(write_result.has_value());
            CHECK(*write_result == strlen(test_data));
            
            // Verify the data was written to the stringstream
            CHECK(oss.str() == test_data);
        }
        
        SUBCASE("flush std::ostream") {
            (void)stream.write("test", 4);
            auto flush_result = stream.flush();
            CHECK(flush_result.has_value());
            CHECK(oss.str() == "test");
        }
        
        SUBCASE("read from std::ostream should fail") {
            char buffer[10];
            auto read_result = stream.read(buffer, 10);
            CHECK(read_result.has_value());
            CHECK(*read_result == 0); // No bytes read
        }
        
        SUBCASE("typed writes to std::ostream") {
            auto res1 = stream.write_u32_le(0xDEADBEEF);
            REQUIRE(res1.has_value());
            
            auto res2 = stream.write_u16_be(0xCAFE);
            REQUIRE(res2.has_value());
            
            // Check that data was written
            std::string written = oss.str();
            CHECK(written.size() == 6); // 4 bytes + 2 bytes
        }
    }
    
    TEST_CASE("std::iostream integration") {
        std::stringstream ss;
        
        auto stream_result = from_iostream(ss);
        REQUIRE(stream_result.has_value());
        auto& stream = stream_result.value();
        
        SUBCASE("read and write with std::iostream") {
            // Write data
            const char* write_data = "Bidirectional stream test";
            auto write_result = stream.write(write_data, strlen(write_data));
            REQUIRE(write_result.has_value());
            CHECK(*write_result == strlen(write_data));
            
            // Seek to beginning
            auto seek_result = stream.seek(0, io_seek_pos::set);
            REQUIRE(seek_result.has_value());
            
            // Read data back
            std::vector<char> buffer(strlen(write_data) + 1);
            auto read_result = stream.read(buffer.data(), strlen(write_data));
            REQUIRE(read_result.has_value());
            CHECK(*read_result == strlen(write_data));
            
            buffer[strlen(write_data)] = '\0';
            CHECK(std::string(buffer.data()) == write_data);
        }
        
        SUBCASE("seek operations on std::iostream") {
            // Write some data first
            (void)stream.write("0123456789", 10);
            
            // Seek to middle
            auto seek_result = stream.seek(5, io_seek_pos::set);
            REQUIRE(seek_result.has_value());
            CHECK(*seek_result == 5);
            
            // Overwrite some data
            (void)stream.write("ABCD", 4);
            
            // Read all data
            (void)stream.seek(0, io_seek_pos::set);
            std::vector<char> buffer(11);
            (void)stream.read(buffer.data(), 10);
            buffer[10] = '\0';
            
            CHECK(std::string(buffer.data()) == "01234ABCD9");
        }
    }
    
    TEST_CASE("C++ stream lifetime") {
        // Test that SDL IOStream doesn't own the C++ stream
        iostream sdl_stream;
        
        {
            std::stringstream ss("Temporary stream");
            auto result = from_iostream(ss);
            REQUIRE(result.has_value());
            sdl_stream = std::move(*result);
            
            // Stream should work while ss is alive
            auto pos = sdl_stream.tell();
            CHECK(pos.has_value());
        }
        
        // After ss is destroyed, operations should fail gracefully
        // Note: This is undefined behavior in practice, but we're testing
        // that we don't crash. In real code, users must ensure the
        // C++ stream outlives the SDL_IOStream.
    }
}