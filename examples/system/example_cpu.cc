#include <iostream>
#include <iomanip>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <sdlpp/system/cpu.hh>
#include <sdlpp/core/core.hh>

void print_cpu_info() {
    std::cout << "\n=== CPU Information ===\n";
    
    auto details = sdlpp::cpu_info::get_cpu_details();
    
    std::cout << "CPU Cores: ";
    if (details.core_count > 0) {
        std::cout << details.core_count << "\n";
    } else {
        std::cout << "Unknown\n";
    }
    
    std::cout << "L1 Cache Line Size: ";
    if (details.cache_line_size > 0) {
        std::cout << details.cache_line_size << " bytes\n";
    } else {
        std::cout << "Unknown\n";
    }
    
    std::cout << "System RAM: ";
    if (details.system_ram_mb > 0) {
        std::cout << details.system_ram_mb << " MB\n";
    } else {
        std::cout << "Unknown\n";
    }
}

void print_simd_support() {
    std::cout << "\n=== SIMD Instruction Set Support ===\n";
    
    auto simd = sdlpp::cpu_info::get_simd_support();
    
    std::cout << "x86/x64 SIMD:\n";
    std::cout << "  MMX:      " << (simd.mmx ? "Yes" : "No") << "\n";
    std::cout << "  SSE:      " << (simd.sse ? "Yes" : "No") << "\n";
    std::cout << "  SSE2:     " << (simd.sse2 ? "Yes" : "No") << "\n";
    std::cout << "  SSE3:     " << (simd.sse3 ? "Yes" : "No") << "\n";
    std::cout << "  SSE4.1:   " << (simd.sse41 ? "Yes" : "No") << "\n";
    std::cout << "  SSE4.2:   " << (simd.sse42 ? "Yes" : "No") << "\n";
    std::cout << "  AVX:      " << (simd.avx ? "Yes" : "No") << "\n";
    std::cout << "  AVX2:     " << (simd.avx2 ? "Yes" : "No") << "\n";
    std::cout << "  AVX-512F: " << (simd.avx512f ? "Yes" : "No") << "\n";
    
    std::cout << "\nARM SIMD:\n";
    std::cout << "  ARM SIMD: " << (simd.armsimd ? "Yes" : "No") << "\n";
    std::cout << "  NEON:     " << (simd.neon ? "Yes" : "No") << "\n";
    
    std::cout << "\nOther:\n";
    std::cout << "  AltiVec:  " << (simd.altivec ? "Yes" : "No") << "\n";
    std::cout << "  LSX:      " << (simd.lsx ? "Yes" : "No") << "\n";
    std::cout << "  LASX:     " << (simd.lasx ? "Yes" : "No") << "\n";
    
    std::cout << "\nSummary:\n";
    std::cout << "  Any SSE: " << (simd.has_any_sse() ? "Yes" : "No") << "\n";
    std::cout << "  Any AVX: " << (simd.has_any_avx() ? "Yes" : "No") << "\n";
    std::cout << "  Any ARM SIMD: " << (simd.has_any_arm_simd() ? "Yes" : "No") << "\n";
}

void simd_memory_example() {
    std::cout << "\n=== SIMD Memory Alignment ===\n";
    
    auto alignment = sdlpp::alignment::get_simd_alignment();
    std::cout << "SIMD Alignment Requirement: " << alignment << " bytes\n";
    std::cout << "Needs Alignment: " << (sdlpp::alignment::simd_needs_alignment() ? "Yes" : "No") << "\n";
    
    // Allocate SIMD-aligned buffer
    const std::size_t count = 1024;
    sdlpp::alignment::simd_buffer<float> buffer(count);
    
    if (buffer) {
        std::cout << "Allocated SIMD-aligned buffer for " << count << " floats\n";
        
        // Check alignment
        auto ptr_value = reinterpret_cast<std::uintptr_t>(buffer.data());
        bool is_aligned = (ptr_value % alignment) == 0;
        std::cout << "Buffer address: 0x" << std::hex << ptr_value << std::dec << "\n";
        std::cout << "Properly aligned: " << (is_aligned ? "Yes" : "No") << "\n";
        
        // Initialize some data
        for (std::size_t i = 0; i < 10; ++i) {
            buffer[i] = static_cast<float>(i) * 1.5f;
        }
        
        std::cout << "First 10 values: ";
        for (std::size_t i = 0; i < 10; ++i) {
            std::cout << buffer[i] << " ";
        }
        std::cout << "\n";
    }
}

void cpu_pause_example() {
    std::cout << "\n=== CPU Pause Example ===\n";
    
    std::atomic<bool> ready{false};
    std::atomic<int> counter{0};
    
    // Start a thread that will set ready after a delay
    std::thread worker([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        counter = 42;
        ready = true;
    });
    
    // Spin-wait for the condition
    std::cout << "Waiting for worker thread...\n";
    auto start = std::chrono::steady_clock::now();
    
    bool success = sdlpp::cpu_pause::spin_wait_for([&]() {
        return ready.load();
    }, std::chrono::milliseconds(100));
    
    auto elapsed = std::chrono::steady_clock::now() - start;
    auto ms = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()) / 1000.0;
    
    if (success) {
        std::cout << "Worker completed in " << std::fixed << std::setprecision(3) 
                  << ms << " ms\n";
        std::cout << "Counter value: " << counter.load() << "\n";
    } else {
        std::cout << "Timed out waiting for worker\n";
    }
    
    worker.join();
}

void benchmark_memory_access() {
    std::cout << "\n=== Memory Access Benchmark ===\n";
    
    const std::size_t size = 1024 * 1024; // 1MB of floats
    const int iterations = 100;
    
    // Regular allocation
    std::vector<float> regular_buffer(size);
    
    // SIMD-aligned allocation
    sdlpp::alignment::simd_buffer<float> simd_buffer(size);
    
    if (!simd_buffer) {
        std::cout << "Failed to allocate SIMD buffer\n";
        return;
    }
    
    // Initialize buffers
    for (std::size_t i = 0; i < size; ++i) {
        regular_buffer[i] = static_cast<float>(i);
        simd_buffer[i] = static_cast<float>(i);
    }
    
    // Benchmark regular buffer
    auto start = std::chrono::high_resolution_clock::now();
    float sum1 = 0;
    for (int iter = 0; iter < iterations; ++iter) {
        for (std::size_t i = 0; i < size; ++i) {
            sum1 += regular_buffer[i];
        }
    }
    auto regular_time = std::chrono::high_resolution_clock::now() - start;
    
    // Benchmark SIMD-aligned buffer
    start = std::chrono::high_resolution_clock::now();
    float sum2 = 0;
    for (int iter = 0; iter < iterations; ++iter) {
        for (std::size_t i = 0; i < size; ++i) {
            sum2 += simd_buffer[i];
        }
    }
    auto simd_time = std::chrono::high_resolution_clock::now() - start;
    
    auto regular_ms = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(regular_time).count()) / 1000.0;
    auto simd_ms = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(simd_time).count()) / 1000.0;
    
    std::cout << "Regular buffer time: " << std::fixed << std::setprecision(3) 
              << regular_ms << " ms\n";
    std::cout << "SIMD-aligned buffer time: " << std::fixed << std::setprecision(3) 
              << simd_ms << " ms\n";
    std::cout << "Speedup: " << std::fixed << std::setprecision(2) 
              << (regular_ms / simd_ms) << "x\n";
    
    // Prevent optimization
    volatile float prevent_opt = sum1 + sum2;
    (void)prevent_opt;
}

int main() {
    try {
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "SDL CPU Information Example\n";
        std::cout << "===========================\n";
        
        print_cpu_info();
        print_simd_support();
        simd_memory_example();
        cpu_pause_example();
        benchmark_memory_access();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}