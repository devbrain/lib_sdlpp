#pragma once

/**
 * @file cpu.hh
 * @brief CPU information and SIMD detection utilities
 * 
 * This header provides access to CPU information including cache sizes,
 * core counts, and SIMD instruction set detection.
 */

#include <sdlpp/core/sdl.hh>
#include <cstdint>
#include <chrono>

namespace sdlpp {
    /**
     * @brief CPU feature flags for SIMD instruction sets
     */
    namespace cpu_features {
        /**
         * @brief Check if the CPU has AltiVec features
         * @return true if AltiVec is supported
         */
        [[nodiscard]] inline bool has_altivec() noexcept {
            return SDL_HasAltiVec();
        }

        /**
         * @brief Check if the CPU has MMX features
         * @return true if MMX is supported
         */
        [[nodiscard]] inline bool has_mmx() noexcept {
            return SDL_HasMMX();
        }

        /**
         * @brief Check if the CPU has SSE features
         * @return true if SSE is supported
         */
        [[nodiscard]] inline bool has_sse() noexcept {
            return SDL_HasSSE();
        }

        /**
         * @brief Check if the CPU has SSE2 features
         * @return true if SSE2 is supported
         */
        [[nodiscard]] inline bool has_sse2() noexcept {
            return SDL_HasSSE2();
        }

        /**
         * @brief Check if the CPU has SSE3 features
         * @return true if SSE3 is supported
         */
        [[nodiscard]] inline bool has_sse3() noexcept {
            return SDL_HasSSE3();
        }

        /**
         * @brief Check if the CPU has SSE4.1 features
         * @return true if SSE4.1 is supported
         */
        [[nodiscard]] inline bool has_sse41() noexcept {
            return SDL_HasSSE41();
        }

        /**
         * @brief Check if the CPU has SSE4.2 features
         * @return true if SSE4.2 is supported
         */
        [[nodiscard]] inline bool has_sse42() noexcept {
            return SDL_HasSSE42();
        }

        /**
         * @brief Check if the CPU has AVX features
         * @return true if AVX is supported
         */
        [[nodiscard]] inline bool has_avx() noexcept {
            return SDL_HasAVX();
        }

        /**
         * @brief Check if the CPU has AVX2 features
         * @return true if AVX2 is supported
         */
        [[nodiscard]] inline bool has_avx2() noexcept {
            return SDL_HasAVX2();
        }

        /**
         * @brief Check if the CPU has AVX-512F features
         * @return true if AVX-512F is supported
         */
        [[nodiscard]] inline bool has_avx512f() noexcept {
            return SDL_HasAVX512F();
        }

        /**
         * @brief Check if the CPU has ARM SIMD features
         * @return true if ARM SIMD is supported
         */
        [[nodiscard]] inline bool has_armsimd() noexcept {
            return SDL_HasARMSIMD();
        }

        /**
         * @brief Check if the CPU has NEON features (ARM)
         * @return true if NEON is supported
         */
        [[nodiscard]] inline bool has_neon() noexcept {
            return SDL_HasNEON();
        }

        /**
         * @brief Check if the CPU has LSX features (Loongson)
         * @return true if LSX is supported
         */
        [[nodiscard]] inline bool has_lsx() noexcept {
            return SDL_HasLSX();
        }

        /**
         * @brief Check if the CPU has LASX features (Loongson)
         * @return true if LASX is supported
         */
        [[nodiscard]] inline bool has_lasx() noexcept {
            return SDL_HasLASX();
        }
    }

    /**
     * @brief CPU information and capabilities
     */
    namespace cpu_info {
        /**
         * @brief Get the number of CPU cores available
         * @return Number of CPU cores, or 0 if the information is not available
         */
        [[nodiscard]] inline size_t get_cpu_count() noexcept {
            int count = SDL_GetNumLogicalCPUCores();
            return count > 0 ? static_cast<size_t>(count) : 0;
        }

        /**
         * @brief Get the L1 cache line size
         * @return L1 cache line size in bytes, or 0 if unknown
         */
        [[nodiscard]] inline size_t get_cpu_cache_line_size() noexcept {
            int size = SDL_GetCPUCacheLineSize();
            return size > 0 ? static_cast<size_t>(size) : 0;
        }

        /**
         * @brief Get the amount of RAM configured in the system
         * @return Amount of RAM in MB, or 0 if the information is not available
         */
        [[nodiscard]] inline size_t get_system_ram() noexcept {
            int ram = SDL_GetSystemRAM();
            return ram > 0 ? static_cast<size_t>(ram) : 0;
        }

        /**
         * @brief Structure containing comprehensive SIMD support information
         */
        struct simd_support {
            bool altivec{false};
            bool mmx{false};
            bool sse{false};
            bool sse2{false};
            bool sse3{false};
            bool sse41{false};
            bool sse42{false};
            bool avx{false};
            bool avx2{false};
            bool avx512f{false};
            bool armsimd{false};
            bool neon{false};
            bool lsx{false};
            bool lasx{false};

            /**
             * @brief Check if any SSE variant is supported
             * @return true if any SSE is supported
             */
            [[nodiscard]] bool has_any_sse() const noexcept {
                return sse || sse2 || sse3 || sse41 || sse42;
            }

            /**
             * @brief Check if any AVX variant is supported
             * @return true if any AVX is supported
             */
            [[nodiscard]] bool has_any_avx() const noexcept {
                return avx || avx2 || avx512f;
            }

            /**
             * @brief Check if any ARM SIMD is supported
             * @return true if any ARM SIMD is supported
             */
            [[nodiscard]] bool has_any_arm_simd() const noexcept {
                return armsimd || neon;
            }

            /**
             * @brief Check if any Loongson SIMD is supported
             * @return true if any Loongson SIMD is supported
             */
            [[nodiscard]] bool has_any_loongson_simd() const noexcept {
                return lsx || lasx;
            }
        };

        /**
         * @brief Get comprehensive SIMD support information
         * @return Structure containing all SIMD feature flags
         */
        [[nodiscard]] inline simd_support get_simd_support() noexcept {
            return simd_support{
                .altivec = cpu_features::has_altivec(),
                .mmx = cpu_features::has_mmx(),
                .sse = cpu_features::has_sse(),
                .sse2 = cpu_features::has_sse2(),
                .sse3 = cpu_features::has_sse3(),
                .sse41 = cpu_features::has_sse41(),
                .sse42 = cpu_features::has_sse42(),
                .avx = cpu_features::has_avx(),
                .avx2 = cpu_features::has_avx2(),
                .avx512f = cpu_features::has_avx512f(),
                .armsimd = cpu_features::has_armsimd(),
                .neon = cpu_features::has_neon(),
                .lsx = cpu_features::has_lsx(),
                .lasx = cpu_features::has_lasx()
            };
        }

        /**
         * @brief Structure containing all CPU information
         */
        struct cpu_details {
            size_t core_count{0};
            size_t cache_line_size{0};
            size_t system_ram_mb{0};
            simd_support simd;
        };

        /**
         * @brief Get all CPU information in one call
         * @return Structure containing all CPU details
         */
        [[nodiscard]] inline cpu_details get_cpu_details() noexcept {
            return cpu_details{
                .core_count = get_cpu_count(),
                .cache_line_size = get_cpu_cache_line_size(),
                .system_ram_mb = get_system_ram(),
                .simd = get_simd_support()
            };
        }
    }

    /**
     * @brief Memory alignment utilities
     */
    namespace alignment {
        /**
         * @brief Check if the CPU needs aligned memory accesses
         *
         * Some CPU architectures (like ARM) require aligned memory access
         * for SIMD operations. This function helps determine if alignment
         * is necessary for optimal performance.
         *
         * @return true if aligned access is required for SIMD operations
         */
        [[nodiscard]] inline bool simd_needs_alignment() noexcept {
            return SDL_GetSIMDAlignment() > 1;
        }

        /**
         * @brief Get the SIMD alignment boundary
         *
         * This will return the minimum alignment required for SIMD operations,
         * or 1 if the CPU doesn't have any SIMD at all.
         *
         * @return Alignment boundary in bytes
         */
        [[nodiscard]] inline std::size_t get_simd_alignment() noexcept {
            return SDL_GetSIMDAlignment();
        }

        /**
         * @brief Allocate aligned memory for SIMD operations
         *
         * The memory returned is padded for the fastest SIMD operations,
         * and will be aligned to a multiple of SDL_GetSIMDAlignment().
         *
         * @param num_bytes Number of bytes to allocate
         * @return Pointer to allocated memory, or nullptr on failure
         * @note Memory must be freed with free_simd_memory()
         */
        [[nodiscard]] inline void* allocate_simd_memory(std::size_t size) noexcept {
            return SDL_aligned_alloc(get_simd_alignment(), size);
        }

        /**
         * @brief Free memory allocated with allocate_simd_memory()
         * @param ptr Pointer to memory to free
         */
        inline void free_simd_memory(void* ptr) noexcept {
            SDL_aligned_free(ptr);
        }

        /**
         * @brief RAII wrapper for SIMD-aligned memory
         */
        template<typename T>
        class simd_buffer {
            private:
                T* data_{nullptr};
                std::size_t size_{0};

            public:
                simd_buffer() = default;

                explicit simd_buffer(std::size_t count)
                    : size_(count) {
                    if (count > 0) {
                        data_ = static_cast <T*>(allocate_simd_memory(count * sizeof(T)));
                    }
                }

                ~simd_buffer() {
                    if (data_) {
                        free_simd_memory(data_);
                    }
                }

                // Delete copy operations
                simd_buffer(const simd_buffer&) = delete;
                simd_buffer& operator=(const simd_buffer&) = delete;

                // Move operations
                simd_buffer(simd_buffer&& other) noexcept
                    : data_(other.data_), size_(other.size_) {
                    other.data_ = nullptr;
                    other.size_ = 0;
                }

                simd_buffer& operator=(simd_buffer&& other) noexcept {
                    if (this != &other) {
                        if (data_) {
                            free_simd_memory(data_);
                        }
                        data_ = other.data_;
                        size_ = other.size_;
                        other.data_ = nullptr;
                        other.size_ = 0;
                    }
                    return *this;
                }

                [[nodiscard]] T* data() noexcept { return data_; }
                [[nodiscard]] const T* data() const noexcept { return data_; }
                [[nodiscard]] std::size_t size() const noexcept { return size_; }
                [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

                [[nodiscard]] T& operator[](std::size_t index) noexcept {
                    return data_[index];
                }

                [[nodiscard]] const T& operator[](std::size_t index) const noexcept {
                    return data_[index];
                }

                [[nodiscard]] explicit operator bool() const noexcept {
                    return data_ != nullptr;
                }
        };
    }

    /**
     * @brief CPU pause/yield utilities for spinlocks
     */
    namespace cpu_pause {
        /**
         * @brief Pause the CPU for a very short amount of time
         *
         * This can be useful in spin-wait loops to reduce power consumption
         * and potentially improve performance by reducing memory bus contention.
         *
         * On x86, this will use the PAUSE instruction. On ARM, it will use YIELD.
         * On other platforms, it's a no-op.
         */
        inline void pause() noexcept {
            SDL_CPUPauseInstruction();
        }

        /**
         * @brief Spin-wait with CPU pause for a condition
         *
         * @tparam Predicate Callable that returns bool
         * @param pred Condition to wait for (should return true when ready)
         * @param max_duration Maximum time to wait
         * @return true if condition was met, false if timed out
         */
        template<typename Predicate>
        [[nodiscard]] bool spin_wait_for(Predicate pred,
                                         std::chrono::microseconds max_duration) noexcept {
            const auto start = std::chrono::steady_clock::now();
            const auto deadline = start + max_duration;

            while (!pred()) {
                if (std::chrono::steady_clock::now() >= deadline) {
                    return false;
                }
                pause();
            }
            return true;
        }
    }
} // namespace sdlpp
