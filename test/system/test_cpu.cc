#include <doctest/doctest.h>
#include <sdlpp/system/cpu.hh>
#include <vector>
#include <thread>
#include <atomic>

TEST_SUITE("cpu") {
    TEST_CASE("cpu_info basic functionality") {
        SUBCASE("get_cpu_count") {
            auto cores = sdlpp::cpu_info::get_cpu_count();
            // Most systems should have at least 1 core
            CHECK(cores >= 1);
        }
        
        SUBCASE("get_cpu_cache_line_size") {
            // Common cache line sizes are 32, 64, or 128 bytes
            if (auto cache_size = sdlpp::cpu_info::get_cpu_cache_line_size(); cache_size > 0) {
                CHECK((cache_size == 32 || cache_size == 64 || cache_size == 128));
            }
        }
        
        SUBCASE("get_system_ram") {
            // Most modern systems have at least 256MB RAM
            if (auto ram = sdlpp::cpu_info::get_system_ram(); ram > 0) {
                CHECK(ram >= 256);
            }
        }
    }
    
    TEST_CASE("cpu_features SIMD detection") {
        // At least check that the functions don't crash
        // We can't assert specific features as they're hardware-dependent
        
        SUBCASE("x86/x64 features") {
            [[maybe_unused]] bool mmx = sdlpp::cpu_features::has_mmx();
            [[maybe_unused]] bool sse = sdlpp::cpu_features::has_sse();
            [[maybe_unused]] bool sse2 = sdlpp::cpu_features::has_sse2();
            [[maybe_unused]] bool sse3 = sdlpp::cpu_features::has_sse3();
            [[maybe_unused]] bool sse41 = sdlpp::cpu_features::has_sse41();
            [[maybe_unused]] bool sse42 = sdlpp::cpu_features::has_sse42();
            [[maybe_unused]] bool avx = sdlpp::cpu_features::has_avx();
            [[maybe_unused]] bool avx2 = sdlpp::cpu_features::has_avx2();
            [[maybe_unused]] bool avx512f = sdlpp::cpu_features::has_avx512f();
            
            // No crashes = success
            CHECK(true);
        }
        
        SUBCASE("ARM features") {
            [[maybe_unused]] bool armsimd = sdlpp::cpu_features::has_armsimd();
            [[maybe_unused]] bool neon = sdlpp::cpu_features::has_neon();
            
            CHECK(true);
        }
        
        SUBCASE("Other features") {
            [[maybe_unused]] bool altivec = sdlpp::cpu_features::has_altivec();
            [[maybe_unused]] bool lsx = sdlpp::cpu_features::has_lsx();
            [[maybe_unused]] bool lasx = sdlpp::cpu_features::has_lasx();
            
            CHECK(true);
        }
    }
    
    TEST_CASE("simd_support structure") {
        auto simd = sdlpp::cpu_info::get_simd_support();
        
        // Check that summary functions work correctly
        if (simd.sse || simd.sse2 || simd.sse3 || simd.sse41 || simd.sse42) {
            CHECK(simd.has_any_sse());
        } else {
            CHECK_FALSE(simd.has_any_sse());
        }
        
        if (simd.avx || simd.avx2 || simd.avx512f) {
            CHECK(simd.has_any_avx());
        } else {
            CHECK_FALSE(simd.has_any_avx());
        }
        
        if (simd.armsimd || simd.neon) {
            CHECK(simd.has_any_arm_simd());
        } else {
            CHECK_FALSE(simd.has_any_arm_simd());
        }
        
        if (simd.lsx || simd.lasx) {
            CHECK(simd.has_any_loongson_simd());
        } else {
            CHECK_FALSE(simd.has_any_loongson_simd());
        }
    }
    
    TEST_CASE("cpu_details comprehensive info") {
        auto details = sdlpp::cpu_info::get_cpu_details();
        
        // Just verify the structure is populated
        // Values are hardware-dependent
        CHECK(details.core_count != 0);  // -1 is valid for "unknown"
        CHECK(details.cache_line_size != 0);
        CHECK(details.system_ram_mb != 0);
        
        // SIMD structure should be populated
        // At least one field should be set on modern hardware
        bool has_any_simd = details.simd.mmx || details.simd.sse || details.simd.sse2 ||
                           details.simd.armsimd || details.simd.neon || details.simd.altivec;
        (void)has_any_simd; // May or may not be true depending on hardware
    }
    
    TEST_CASE("alignment utilities") {
        SUBCASE("get_simd_alignment") {
            auto alignment = sdlpp::alignment::get_simd_alignment();
            // Should be a power of 2
            CHECK(alignment > 0);
            CHECK((alignment & (alignment - 1)) == 0);
            
            // Common alignments are 1, 16, 32, or 64
            CHECK((alignment == 1 || alignment == 16 || alignment == 32 || alignment == 64));
        }
        
        SUBCASE("simd_needs_alignment") {
            bool needs = sdlpp::alignment::simd_needs_alignment();
            auto alignment = sdlpp::alignment::get_simd_alignment();
            
            if (alignment > 1) {
                CHECK(needs);
            } else {
                CHECK_FALSE(needs);
            }
        }
        
        SUBCASE("allocate and free simd memory") {
            const std::size_t size = 1024;
            void* ptr = sdlpp::alignment::allocate_simd_memory(size);
            
            if (ptr) {
                // Check alignment
                auto alignment = sdlpp::alignment::get_simd_alignment();
                auto ptr_value = reinterpret_cast<std::uintptr_t>(ptr);
                CHECK((ptr_value % alignment) == 0);
                
                // Should be able to write to it
                auto* data = static_cast<std::uint8_t*>(ptr);
                for (std::size_t i = 0; i < size; ++i) {
                    data[i] = static_cast<std::uint8_t>(i & 0xFF);
                }
                
                // Verify write
                bool correct = true;
                for (std::size_t i = 0; i < size; ++i) {
                    if (data[i] != static_cast<std::uint8_t>(i & 0xFF)) {
                        correct = false;
                        break;
                    }
                }
                CHECK(correct);
                
                sdlpp::alignment::free_simd_memory(ptr);
            }
        }
    }
    
    TEST_CASE("simd_buffer RAII wrapper") {
        SUBCASE("default construction") {
            sdlpp::alignment::simd_buffer<float> buffer;
            CHECK(buffer.empty());
            CHECK(buffer.size() == 0);
            CHECK(buffer.data() == nullptr);
            CHECK_FALSE(buffer);
        }
        
        SUBCASE("construction with size") {
            const std::size_t count = 100;
            sdlpp::alignment::simd_buffer<float> buffer(count);
            
            if (buffer) {
                CHECK_FALSE(buffer.empty());
                CHECK(buffer.size() == count);
                CHECK(buffer.data() != nullptr);
                
                // Check alignment
                auto alignment = sdlpp::alignment::get_simd_alignment();
                auto ptr_value = reinterpret_cast<std::uintptr_t>(buffer.data());
                CHECK((ptr_value % alignment) == 0);
                
                // Test access
                for (std::size_t i = 0; i < count; ++i) {
                    buffer[i] = static_cast<float>(i) * 2.0f;
                }
                
                for (std::size_t i = 0; i < count; ++i) {
                    CHECK(buffer[i] == static_cast<float>(i) * 2.0f);
                }
            }
        }
        
        SUBCASE("move construction") {
            sdlpp::alignment::simd_buffer<int> buffer1(50);
            if (buffer1) {
                auto* original_ptr = buffer1.data();
                auto original_size = buffer1.size();
                
                sdlpp::alignment::simd_buffer<int> buffer2(std::move(buffer1));
                
                CHECK(buffer2.data() == original_ptr);
                CHECK(buffer2.size() == original_size);
                CHECK(buffer1.data() == nullptr);
                CHECK(buffer1.size() == 0);
            }
        }
        
        SUBCASE("move assignment") {
            sdlpp::alignment::simd_buffer<double> buffer1(30);
            sdlpp::alignment::simd_buffer<double> buffer2(60);
            
            if (buffer1 && buffer2) {
                auto* original_ptr = buffer1.data();
                auto original_size = buffer1.size();
                
                buffer2 = std::move(buffer1);
                
                CHECK(buffer2.data() == original_ptr);
                CHECK(buffer2.size() == original_size);
                CHECK(buffer1.data() == nullptr);
                CHECK(buffer1.size() == 0);
            }
        }
    }
    
    TEST_CASE("cpu_pause functionality") {
        SUBCASE("pause instruction") {
            // Just verify it doesn't crash
            for (int i = 0; i < 10; ++i) {
                sdlpp::cpu_pause::pause();
            }
            CHECK(true);
        }
        
        SUBCASE("spin_wait_for immediate success") {
            bool result = sdlpp::cpu_pause::spin_wait_for([]() { return true; },
                                                          std::chrono::microseconds(100));
            CHECK(result);
        }
        
        SUBCASE("spin_wait_for timeout") {
            bool result = sdlpp::cpu_pause::spin_wait_for([]() { return false; },
                                                          std::chrono::microseconds(10));
            CHECK_FALSE(result);
        }
        
        SUBCASE("spin_wait_for with condition") {
            std::atomic<bool> flag{false};
            
            std::thread setter([&flag]() {
                std::this_thread::sleep_for(std::chrono::microseconds(500));
                flag = true;
            });
            
            bool result = sdlpp::cpu_pause::spin_wait_for([&flag]() { return flag.load(); },
                                                          std::chrono::milliseconds(10));
            CHECK(result);
            
            setter.join();
        }
    }
}