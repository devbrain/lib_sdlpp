#include <iostream>
#include <iomanip>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <sdlpp/system/intrinsics.hh>
#include <sdlpp/core/core.hh>

void memory_barrier_example() {
    std::cout << "\n=== Memory Barrier Example ===\n";
    
    std::atomic<int> flag{0};
    int data = 0;
    
    std::thread writer([&]() {
        data = 42;  // Write data
        sdlpp::memory_barrier::release_barrier();  // Ensure write is visible
        flag.store(1, std::memory_order_relaxed);  // Signal ready
    });
    
    std::thread reader([&]() {
        while (flag.load(std::memory_order_relaxed) == 0) {
            // Wait for signal
        }
        sdlpp::memory_barrier::acquire_barrier();  // Ensure we see the write
        std::cout << "Read data: " << data << "\n";
    });
    
    writer.join();
    reader.join();
    
    std::cout << "Memory barriers ensure proper synchronization\n";
}

void atomic_operations_example() {
    std::cout << "\n=== Atomic Operations Example ===\n";
    
    std::int32_t value = 10;
    std::cout << "Initial value: " << value << "\n";
    
    // Compare and swap
    bool swapped = sdlpp::atomic::compare_and_swap(&value, 10, 20);
    std::cout << "Compare and swap (10->20): " << (swapped ? "success" : "failed") 
              << ", value = " << value << "\n";
    
    // Try again with wrong expected value
    swapped = sdlpp::atomic::compare_and_swap(&value, 10, 30);
    std::cout << "Compare and swap (10->30): " << (swapped ? "success" : "failed") 
              << ", value = " << value << "\n";
    
    // Exchange
    auto old = sdlpp::atomic::exchange(&value, 100);
    std::cout << "Exchange: old = " << old << ", new = " << value << "\n";
    
    // Add
    old = sdlpp::atomic::add(&value, 25);
    std::cout << "Add 25: old = " << old << ", new = " << value << "\n";
    
    // Load
    auto loaded = sdlpp::atomic::load(&value);
    std::cout << "Atomic load: " << loaded << "\n";
}

void bit_manipulation_example() {
    std::cout << "\n=== Bit Manipulation Example ===\n";
    
    std::uint32_t values[] = {0, 1, 2, 3, 4, 7, 8, 15, 16, 31, 32, 128, 255, 256, 1024};
    
    std::cout << "Most significant bit index:\n";
    for (auto val : values) {
        auto msb = sdlpp::bits::most_significant_bit(val);
        std::cout << "  " << std::setw(4) << val << " (0x" << std::hex << std::setw(4) 
                  << std::setfill('0') << val << std::dec << "): MSB at index " 
                  << msb << "\n";
    }
    
    std::cout << "\nPower of 2 check:\n";
    for (auto val : values) {
        bool is_pow2 = sdlpp::bits::has_exactly_one_bit_set(val);
        std::cout << "  " << std::setw(4) << val << ": " 
                  << (is_pow2 ? "Yes" : "No") << "\n";
    }
    
    // Byte swapping
    std::cout << "\nByte swapping:\n";
    std::uint16_t val16 = 0x1234;
    std::cout << "  16-bit: 0x" << std::hex << val16 << " -> 0x" 
              << sdlpp::bits::swap_bytes(val16) << std::dec << "\n";
    
    std::uint32_t val32 = 0x12345678;
    std::cout << "  32-bit: 0x" << std::hex << val32 << " -> 0x" 
              << sdlpp::bits::swap_bytes(val32) << std::dec << "\n";
    
    std::uint64_t val64 = 0x123456789ABCDEF0;
    std::cout << "  64-bit: 0x" << std::hex << val64 << " -> 0x" 
              << sdlpp::bits::swap_bytes(val64) << std::dec << "\n";
    
    float fval = 3.14159f;
    float swapped = sdlpp::bits::swap_bytes(fval);
    std::cout << "  float: " << fval << " -> ";
    std::cout << "0x" << std::hex;
    for (int i = 0; i < 4; ++i) {
        std::cout << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(reinterpret_cast<unsigned char*>(&swapped)[i]);
    }
    std::cout << std::dec << "\n";
}

void endianness_example() {
    std::cout << "\n=== Endianness Example ===\n";
    
    // Compile-time endianness check
    if constexpr (sdlpp::endian::is_little_endian()) {
        std::cout << "System is Little Endian\n";
    } else if constexpr (sdlpp::endian::is_big_endian()) {
        std::cout << "System is Big Endian\n";
    }
    
    // Endian conversions
    std::uint32_t native_val = 0x12345678;
    std::cout << "\nNative value: 0x" << std::hex << native_val << std::dec << "\n";
    
    auto big_endian = sdlpp::endian::to_big_endian(native_val);
    std::cout << "To big endian: 0x" << std::hex << big_endian << std::dec << "\n";
    
    auto from_big = sdlpp::endian::from_big_endian(big_endian);
    std::cout << "From big endian: 0x" << std::hex << from_big << std::dec << "\n";
    
    auto little_endian = sdlpp::endian::to_little_endian(native_val);
    std::cout << "To little endian: 0x" << std::hex << little_endian << std::dec << "\n";
    
    auto from_little = sdlpp::endian::from_little_endian(little_endian);
    std::cout << "From little endian: 0x" << std::hex << from_little << std::dec << "\n";
}

void math_intrinsics_example() {
    std::cout << "\n=== Math Intrinsics Example ===\n";
    
    std::uint32_t values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 15, 16, 17, 31, 32, 33, 63, 64, 65};
    
    std::cout << "Next power of 2:\n";
    for (auto val : values) {
        auto next = sdlpp::math::next_power_of_two(val);
        std::cout << "  " << std::setw(3) << val << " -> " << std::setw(3) << next << "\n";
    }
    
    // Alignment examples
    std::cout << "\nAlignment operations (align to 16):\n";
    for (std::uint32_t i = 0; i <= 32; i += 3) {
        auto up = sdlpp::math::align_up(i, 16u);
        auto down = sdlpp::math::align_down(i, 16u);
        bool aligned = sdlpp::math::is_aligned(i, 16u);
        std::cout << "  " << std::setw(2) << i << ": up=" << std::setw(2) << up 
                  << ", down=" << std::setw(2) << down 
                  << ", aligned=" << (aligned ? "yes" : "no") << "\n";
    }
}

void prefetch_example() {
    std::cout << "\n=== Prefetch Example ===\n";
    
    const std::size_t size = 1024 * 1024;  // 1MB
    std::vector<int> data(size);
    
    // Initialize with some data
    for (std::size_t i = 0; i < size; ++i) {
        data[i] = static_cast<int>(i);
    }
    
    // Sum without prefetch
    auto start = std::chrono::high_resolution_clock::now();
    long long sum1 = 0;
    for (std::size_t i = 0; i < size; ++i) {
        sum1 += data[i];
    }
    auto time1 = std::chrono::high_resolution_clock::now() - start;
    
    // Sum with prefetch
    start = std::chrono::high_resolution_clock::now();
    long long sum2 = 0;
    const std::size_t prefetch_distance = 64;  // Prefetch 64 elements ahead
    for (std::size_t i = 0; i < size; ++i) {
        if (i + prefetch_distance < size) {
            sdlpp::prefetch::for_read(&data[i + prefetch_distance], 0);
        }
        sum2 += data[i];
    }
    auto time2 = std::chrono::high_resolution_clock::now() - start;
    
    auto us1 = std::chrono::duration_cast<std::chrono::microseconds>(time1).count();
    auto us2 = std::chrono::duration_cast<std::chrono::microseconds>(time2).count();
    
    std::cout << "Without prefetch: " << us1 << " µs\n";
    std::cout << "With prefetch: " << us2 << " µs\n";
    std::cout << "Sums match: " << (sum1 == sum2 ? "Yes" : "No") << "\n";
    
    // Note: Prefetch effectiveness depends on many factors
    std::cout << "\nNote: Prefetch effectiveness varies by CPU and memory subsystem\n";
}

int main() {
    try {
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "SDL Intrinsics Example\n";
        std::cout << "======================\n";
        
        memory_barrier_example();
        atomic_operations_example();
        bit_manipulation_example();
        endianness_example();
        math_intrinsics_example();
        prefetch_example();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}