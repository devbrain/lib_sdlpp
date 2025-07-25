---
title: CPU Information
sidebar_label: CPU Info
---

# CPU Information and SIMD Support

SDL++ provides comprehensive CPU feature detection and SIMD-optimized memory allocation utilities.

## CPU Detection

### Basic CPU Information

```cpp
#include <sdlpp/system/cpu.hh>

// Get CPU core count
int cpu_count = sdlpp::get_cpu_count();
std::cout << "CPU cores: " << cpu_count << std::endl;

// Get CPU cache line size
int cache_line = sdlpp::get_cpu_cache_line_size();
std::cout << "Cache line size: " << cache_line << " bytes" << std::endl;

// Get system RAM
int ram_mb = sdlpp::get_system_ram();
std::cout << "System RAM: " << ram_mb << " MB" << std::endl;
```

### SIMD Feature Detection

```cpp
// Check for specific SIMD instruction sets
if (sdlpp::has_sse()) {
    std::cout << "SSE supported" << std::endl;
}

if (sdlpp::has_sse2()) {
    std::cout << "SSE2 supported" << std::endl;
}

if (sdlpp::has_sse3()) {
    std::cout << "SSE3 supported" << std::endl;
}

if (sdlpp::has_sse41()) {
    std::cout << "SSE4.1 supported" << std::endl;
}

if (sdlpp::has_sse42()) {
    std::cout << "SSE4.2 supported" << std::endl;
}

if (sdlpp::has_avx()) {
    std::cout << "AVX supported" << std::endl;
}

if (sdlpp::has_avx2()) {
    std::cout << "AVX2 supported" << std::endl;
}

if (sdlpp::has_avx512f()) {
    std::cout << "AVX-512F supported" << std::endl;
}

// ARM SIMD
if (sdlpp::has_neon()) {
    std::cout << "ARM NEON supported" << std::endl;
}

// Other features
if (sdlpp::has_mmx()) {
    std::cout << "MMX supported" << std::endl;
}

if (sdlpp::has_rdtsc()) {
    std::cout << "RDTSC instruction supported" << std::endl;
}
```

## SIMD Memory Allocation

SDL++ provides aligned memory allocation for SIMD operations:

```cpp
// Allocate SIMD-aligned memory
size_t size = 1024 * sizeof(float);
void* aligned_memory = sdlpp::simd_alloc(size);

if (aligned_memory) {
    // Use the memory for SIMD operations
    float* data = static_cast<float*>(aligned_memory);
    
    // ... perform SIMD operations ...
    
    // Free the memory
    sdlpp::simd_free(aligned_memory);
}

// Get SIMD alignment requirement
size_t alignment = sdlpp::simd_get_alignment();
std::cout << "SIMD alignment: " << alignment << " bytes" << std::endl;
```

### RAII SIMD Memory

```cpp
// RAII wrapper for SIMD memory
template<typename T>
class simd_vector {
    T* data_ = nullptr;
    size_t size_ = 0;
    
public:
    explicit simd_vector(size_t count) 
        : size_(count) {
        data_ = static_cast<T*>(
            sdlpp::simd_alloc(count * sizeof(T))
        );
        if (!data_) {
            throw std::bad_alloc();
        }
    }
    
    ~simd_vector() {
        if (data_) {
            sdlpp::simd_free(data_);
        }
    }
    
    // Disable copy
    simd_vector(const simd_vector&) = delete;
    simd_vector& operator=(const simd_vector&) = delete;
    
    // Enable move
    simd_vector(simd_vector&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }
    
    simd_vector& operator=(simd_vector&& other) noexcept {
        if (this != &other) {
            if (data_) {
                sdlpp::simd_free(data_);
            }
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
    
    T* data() { return data_; }
    const T* data() const { return data_; }
    size_t size() const { return size_; }
};

// Usage
simd_vector<float> aligned_floats(1024);
// Memory automatically freed when out of scope
```

## Memory Operations

SDL++ provides optimized memory operations:

```cpp
// SIMD-optimized memory reallocation
void* new_ptr = sdlpp::simd_realloc(old_ptr, new_size);

// Check if memory is SIMD-aligned
if (sdlpp::is_simd_aligned(ptr)) {
    // Can use SIMD instructions safely
}

// Aligned memory copy (if available)
sdlpp::simd_memcpy(dest, src, size);

// Aligned memory set (if available)
sdlpp::simd_memset(dest, value, size);
```

## CPU Intrinsics

```cpp
#include <sdlpp/system/intrinsics.hh>

// Memory barriers
sdlpp::memory_barrier_release();
sdlpp::memory_barrier_acquire();

// CPU pause instruction (for spinlocks)
sdlpp::cpu_pause();

// Example spinlock implementation
class spinlock {
    std::atomic<bool> locked{false};
    
public:
    void lock() {
        while (locked.exchange(true, std::memory_order_acquire)) {
            while (locked.load(std::memory_order_relaxed)) {
                sdlpp::cpu_pause();  // Reduce CPU usage
            }
        }
    }
    
    void unlock() {
        locked.store(false, std::memory_order_release);
    }
};
```

## CPU Dispatch

Runtime CPU feature dispatch for optimized code paths:

```cpp
#include <sdlpp/system/cpu_dispatch.hh>

// Define function variants
float dot_product_generic(const float* a, const float* b, size_t n) {
    float sum = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

float dot_product_sse(const float* a, const float* b, size_t n) {
    // SSE implementation
    // ...
}

float dot_product_avx(const float* a, const float* b, size_t n) {
    // AVX implementation
    // ...
}

// Runtime dispatch
float dot_product(const float* a, const float* b, size_t n) {
    static auto impl = []() {
        if (sdlpp::has_avx()) {
            return dot_product_avx;
        } else if (sdlpp::has_sse()) {
            return dot_product_sse;
        } else {
            return dot_product_generic;
        }
    }();
    
    return impl(a, b, n);
}
```

## Common Patterns

### SIMD-Friendly Data Layout

```cpp
// Structure of Arrays (SoA) for SIMD
struct particle_system_soa {
    simd_vector<float> pos_x;
    simd_vector<float> pos_y;
    simd_vector<float> pos_z;
    simd_vector<float> vel_x;
    simd_vector<float> vel_y;
    simd_vector<float> vel_z;
    size_t count;
    
    explicit particle_system_soa(size_t n) 
        : pos_x(n), pos_y(n), pos_z(n),
          vel_x(n), vel_y(n), vel_z(n),
          count(n) {}
    
    void update(float dt) {
        // SIMD-friendly update loop
        for (size_t i = 0; i < count; i += 4) {
            // Process 4 particles at once with SIMD
            // Compiler can auto-vectorize or use intrinsics
        }
    }
};
```

### CPU Feature Logger

```cpp
class cpu_info_logger {
public:
    static void log_capabilities() {
        std::cout << "=== CPU Information ===" << std::endl;
        std::cout << "Cores: " << sdlpp::get_cpu_count() << std::endl;
        std::cout << "Cache line: " << sdlpp::get_cpu_cache_line_size() << " bytes" << std::endl;
        std::cout << "RAM: " << sdlpp::get_system_ram() << " MB" << std::endl;
        std::cout << "SIMD alignment: " << sdlpp::simd_get_alignment() << " bytes" << std::endl;
        
        std::cout << "\n=== SIMD Features ===" << std::endl;
        
        // x86/x64 features
        log_feature("MMX", sdlpp::has_mmx());
        log_feature("SSE", sdlpp::has_sse());
        log_feature("SSE2", sdlpp::has_sse2());
        log_feature("SSE3", sdlpp::has_sse3());
        log_feature("SSE4.1", sdlpp::has_sse41());
        log_feature("SSE4.2", sdlpp::has_sse42());
        log_feature("AVX", sdlpp::has_avx());
        log_feature("AVX2", sdlpp::has_avx2());
        log_feature("AVX-512F", sdlpp::has_avx512f());
        
        // ARM features
        log_feature("NEON", sdlpp::has_neon());
        
        // Other features
        log_feature("RDTSC", sdlpp::has_rdtsc());
    }
    
private:
    static void log_feature(const char* name, bool supported) {
        std::cout << name << ": " << (supported ? "✓" : "✗") << std::endl;
    }
};
```

### Performance-Critical Memory Pool

```cpp
template<typename T>
class simd_memory_pool {
    struct block {
        void* memory;
        size_t capacity;
        std::vector<bool> used;
        
        explicit block(size_t n) 
            : memory(sdlpp::simd_alloc(n * sizeof(T))),
              capacity(n),
              used(n, false) {
            if (!memory) {
                throw std::bad_alloc();
            }
        }
        
        ~block() {
            sdlpp::simd_free(memory);
        }
    };
    
    std::vector<std::unique_ptr<block>> blocks;
    size_t block_size;
    
public:
    explicit simd_memory_pool(size_t block_sz = 1024)
        : block_size(block_sz) {}
    
    T* allocate() {
        // Find free slot in existing blocks
        for (auto& blk : blocks) {
            for (size_t i = 0; i < blk->capacity; ++i) {
                if (!blk->used[i]) {
                    blk->used[i] = true;
                    return static_cast<T*>(blk->memory) + i;
                }
            }
        }
        
        // Create new block
        blocks.push_back(std::make_unique<block>(block_size));
        blocks.back()->used[0] = true;
        return static_cast<T*>(blocks.back()->memory);
    }
    
    void deallocate(T* ptr) {
        for (auto& blk : blocks) {
            T* block_start = static_cast<T*>(blk->memory);
            if (ptr >= block_start && ptr < block_start + blk->capacity) {
                size_t index = ptr - block_start;
                blk->used[index] = false;
                return;
            }
        }
    }
};
```

## Best Practices

1. **Check Features at Runtime**: Always verify SIMD features before using
2. **Provide Fallbacks**: Have generic implementations for unsupported CPUs
3. **Align Data**: Use SIMD-aligned memory for vector operations
4. **Profile Different Paths**: Not all SIMD code is faster
5. **Consider Cache**: Respect cache line boundaries for performance

## Example: SIMD-Optimized Image Processing

```cpp
class image_processor {
    size_t width, height;
    simd_vector<uint8_t> pixels;
    
    using process_func = void(*)(uint8_t*, size_t);
    process_func blur_impl;
    
public:
    image_processor(size_t w, size_t h) 
        : width(w), height(h), pixels(w * h * 4) {
        
        // Select best implementation
        if (sdlpp::has_avx2()) {
            blur_impl = blur_avx2;
        } else if (sdlpp::has_sse41()) {
            blur_impl = blur_sse41;
        } else {
            blur_impl = blur_generic;
        }
    }
    
    void apply_blur() {
        blur_impl(pixels.data(), pixels.size());
    }
    
private:
    static void blur_generic(uint8_t* data, size_t size) {
        // Generic C++ implementation
    }
    
    static void blur_sse41(uint8_t* data, size_t size) {
        // SSE4.1 optimized implementation
    }
    
    static void blur_avx2(uint8_t* data, size_t size) {
        // AVX2 optimized implementation
    }
};
```