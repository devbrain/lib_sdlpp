#include <doctest/doctest.h>
#include <sdlpp/system/intrinsics.hh>
#include <thread>
#include <atomic>
#include <vector>

TEST_SUITE("intrinsics") {
    TEST_CASE("memory barriers") {
        // Just verify they compile and don't crash
        sdlpp::memory_barrier::full_barrier();
        sdlpp::memory_barrier::compiler_barrier();
        sdlpp::memory_barrier::acquire_barrier();
        sdlpp::memory_barrier::release_barrier();
        
        CHECK(true); // If we get here, no crashes
    }
    
    TEST_CASE("atomic operations") {
        SUBCASE("compare and swap int32") {
            std::int32_t value = 42;
            
            // Successful swap
            bool success = sdlpp::atomic::compare_and_swap(&value, 42, 100);
            CHECK(success);
            CHECK(value == 100);
            
            // Failed swap
            success = sdlpp::atomic::compare_and_swap(&value, 42, 200);
            CHECK_FALSE(success);
            CHECK(value == 100); // Unchanged
        }
        
        SUBCASE("compare and swap pointer") {
            int a = 1, b = 2;
            void* ptr = &a;
            
            // Successful swap
            bool success = sdlpp::atomic::compare_and_swap(&ptr, &a, &b);
            CHECK(success);
            CHECK(ptr == &b);
            
            // Failed swap
            success = sdlpp::atomic::compare_and_swap(&ptr, &a, &b);
            CHECK_FALSE(success);
            CHECK(ptr == &b);
        }
        
        SUBCASE("exchange") {
            std::int32_t value = 50;
            
            auto old = sdlpp::atomic::exchange(&value, 75);
            CHECK(old == 50);
            CHECK(value == 75);
        }
        
        SUBCASE("add") {
            std::int32_t value = 100;
            
            auto old = sdlpp::atomic::add(&value, 25);
            CHECK(old == 100);
            CHECK(value == 125);
            
            old = sdlpp::atomic::add(&value, -50);
            CHECK(old == 125);
            CHECK(value == 75);
        }
        
        SUBCASE("load") {
            std::int32_t value = 999;
            auto loaded = sdlpp::atomic::load(&value);
            CHECK(loaded == 999);
        }
    }
    
    TEST_CASE("bit manipulation") {
        SUBCASE("most significant bit") {
            CHECK(sdlpp::bits::most_significant_bit(0) == -1);
            CHECK(sdlpp::bits::most_significant_bit(1) == 0);
            CHECK(sdlpp::bits::most_significant_bit(2) == 1);
            CHECK(sdlpp::bits::most_significant_bit(3) == 1);
            CHECK(sdlpp::bits::most_significant_bit(4) == 2);
            CHECK(sdlpp::bits::most_significant_bit(7) == 2);
            CHECK(sdlpp::bits::most_significant_bit(8) == 3);
            CHECK(sdlpp::bits::most_significant_bit(255) == 7);
            CHECK(sdlpp::bits::most_significant_bit(256) == 8);
            CHECK(sdlpp::bits::most_significant_bit(0x80000000) == 31);
        }
        
        SUBCASE("power of 2 check") {
            CHECK_FALSE(sdlpp::bits::has_exactly_one_bit_set(0));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(1));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(2));
            CHECK_FALSE(sdlpp::bits::has_exactly_one_bit_set(3));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(4));
            CHECK_FALSE(sdlpp::bits::has_exactly_one_bit_set(5));
            CHECK_FALSE(sdlpp::bits::has_exactly_one_bit_set(6));
            CHECK_FALSE(sdlpp::bits::has_exactly_one_bit_set(7));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(8));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(16));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(32));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(64));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(128));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(256));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(1024));
            CHECK(sdlpp::bits::has_exactly_one_bit_set(0x80000000));
        }
        
        SUBCASE("byte swapping") {
            // 16-bit
            CHECK(sdlpp::bits::swap_bytes(static_cast<std::uint16_t>(0x1234)) == 0x3412);
            CHECK(sdlpp::bits::swap_bytes(static_cast<std::uint16_t>(0xABCD)) == 0xCDAB);
            
            // 32-bit
            CHECK(sdlpp::bits::swap_bytes(static_cast<std::uint32_t>(0x12345678)) == 0x78563412);
            CHECK(sdlpp::bits::swap_bytes(0xABCDEF00u) == 0x00EFCDAB);
            
            // 64-bit
            CHECK(sdlpp::bits::swap_bytes(static_cast<std::uint64_t>(0x123456789ABCDEF0)) == 0xF0DEBC9A78563412);
            
            // Double swap should return original
            std::uint32_t original = 0xDEADBEEF;
            CHECK(sdlpp::bits::swap_bytes(sdlpp::bits::swap_bytes(original)) == original);
        }
    }
    
    TEST_CASE("endianness") {
        SUBCASE("compile-time checks") {
            // Exactly one should be true
            CHECK((sdlpp::endian::is_big_endian() ^ sdlpp::endian::is_little_endian()));
            
            // These should be constexpr
            if constexpr (sdlpp::endian::is_little_endian()) {
                CHECK_FALSE(sdlpp::endian::is_big_endian());
            } else {
                CHECK(sdlpp::endian::is_big_endian());
            }
        }
        
        SUBCASE("endian conversions") {
            std::uint32_t test_val = 0x12345678;
            
            // Converting to and from should return original
            auto to_big = sdlpp::endian::to_big_endian(test_val);
            auto from_big = sdlpp::endian::from_big_endian(to_big);
            CHECK(from_big == test_val);
            
            auto to_little = sdlpp::endian::to_little_endian(test_val);
            auto from_little = sdlpp::endian::from_little_endian(to_little);
            CHECK(from_little == test_val);
            
            // On little endian systems, to_big should swap bytes
            if (sdlpp::endian::is_little_endian()) {
                CHECK(to_big == sdlpp::bits::swap_bytes(test_val));
                CHECK(to_little == test_val);
            } else {
                CHECK(to_big == test_val);
                CHECK(to_little == sdlpp::bits::swap_bytes(test_val));
            }
        }
    }
    
    TEST_CASE("math intrinsics") {
        SUBCASE("next power of two") {
            CHECK(sdlpp::math::next_power_of_two(0) == 0);
            CHECK(sdlpp::math::next_power_of_two(1) == 1);
            CHECK(sdlpp::math::next_power_of_two(2) == 2);
            CHECK(sdlpp::math::next_power_of_two(3) == 4);
            CHECK(sdlpp::math::next_power_of_two(4) == 4);
            CHECK(sdlpp::math::next_power_of_two(5) == 8);
            CHECK(sdlpp::math::next_power_of_two(7) == 8);
            CHECK(sdlpp::math::next_power_of_two(8) == 8);
            CHECK(sdlpp::math::next_power_of_two(9) == 16);
            CHECK(sdlpp::math::next_power_of_two(17) == 32);
            CHECK(sdlpp::math::next_power_of_two(33) == 64);
            CHECK(sdlpp::math::next_power_of_two(1000) == 1024);
        }
        
        SUBCASE("alignment") {
            // Align up
            CHECK(sdlpp::math::align_up(0u, 16u) == 0);
            CHECK(sdlpp::math::align_up(1u, 16u) == 16);
            CHECK(sdlpp::math::align_up(15u, 16u) == 16);
            CHECK(sdlpp::math::align_up(16u, 16u) == 16);
            CHECK(sdlpp::math::align_up(17u, 16u) == 32);
            CHECK(sdlpp::math::align_up(31u, 16u) == 32);
            CHECK(sdlpp::math::align_up(32u, 16u) == 32);
            
            // Align down
            CHECK(sdlpp::math::align_down(0u, 16u) == 0);
            CHECK(sdlpp::math::align_down(1u, 16u) == 0);
            CHECK(sdlpp::math::align_down(15u, 16u) == 0);
            CHECK(sdlpp::math::align_down(16u, 16u) == 16);
            CHECK(sdlpp::math::align_down(17u, 16u) == 16);
            CHECK(sdlpp::math::align_down(31u, 16u) == 16);
            CHECK(sdlpp::math::align_down(32u, 16u) == 32);
            
            // Is aligned
            CHECK(sdlpp::math::is_aligned(0u, 16u));
            CHECK_FALSE(sdlpp::math::is_aligned(1u, 16u));
            CHECK_FALSE(sdlpp::math::is_aligned(15u, 16u));
            CHECK(sdlpp::math::is_aligned(16u, 16u));
            CHECK_FALSE(sdlpp::math::is_aligned(17u, 16u));
            CHECK(sdlpp::math::is_aligned(32u, 16u));
            CHECK(sdlpp::math::is_aligned(64u, 16u));
        }
    }
    
    TEST_CASE("prefetch") {
        // Just verify prefetch compiles and doesn't crash
        std::vector<int> data(1024);
        
        for (std::size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<int>(i);
        }
        
        // Try different locality levels
        for (int locality = 0; locality <= 3; ++locality) {
            sdlpp::prefetch::for_read(&data[0], locality);
            sdlpp::prefetch::for_write(&data[0], locality);
        }
        
        // Prefetch various addresses
        for (std::size_t i = 0; i < data.size(); i += 64) {
            sdlpp::prefetch::for_read(&data[i]);
        }
        
        CHECK(true); // If we get here, no crashes
    }
}