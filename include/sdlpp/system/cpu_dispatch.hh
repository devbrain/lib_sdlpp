#pragma once

/**
 * @file cpu_dispatch.hh
 * @brief CPU feature detection macros for compile-time and runtime dispatch
 * 
 * This header provides convenient macros and utilities for CPU-specific
 * code paths and optimizations.
 */

#include <sdlpp/system/cpu.hh>

namespace sdlpp {
    /**
     * @brief Runtime CPU dispatch helper
     *
     * This class caches CPU feature detection results for efficient runtime dispatch.
     * It's designed to be used as a static or thread_local variable.
     */
    class cpu_dispatcher {
        private:
            cpu_info::simd_support simd_;

        public:
            cpu_dispatcher()
                : simd_(cpu_info::get_simd_support()) {
            }

            // x86/x64 features
            [[nodiscard]] bool has_sse() const noexcept { return simd_.sse; }
            [[nodiscard]] bool has_sse2() const noexcept { return simd_.sse2; }
            [[nodiscard]] bool has_sse3() const noexcept { return simd_.sse3; }
            [[nodiscard]] bool has_sse41() const noexcept { return simd_.sse41; }
            [[nodiscard]] bool has_sse42() const noexcept { return simd_.sse42; }
            [[nodiscard]] bool has_avx() const noexcept { return simd_.avx; }
            [[nodiscard]] bool has_avx2() const noexcept { return simd_.avx2; }
            [[nodiscard]] bool has_avx512f() const noexcept { return simd_.avx512f; }

            // ARM features
            [[nodiscard]] bool has_neon() const noexcept { return simd_.neon; }

            // Summary checks
            [[nodiscard]] bool has_any_sse() const noexcept { return simd_.has_any_sse(); }
            [[nodiscard]] bool has_any_avx() const noexcept { return simd_.has_any_avx(); }

            /**
             * @brief Get the best SIMD level available
             * @return String describing the best SIMD available
             */
            [[nodiscard]] const char* best_simd_level() const noexcept {
                if (simd_.avx512f) return "AVX-512F";
                if (simd_.avx2) return "AVX2";
                if (simd_.avx) return "AVX";
                if (simd_.sse42) return "SSE4.2";
                if (simd_.sse41) return "SSE4.1";
                if (simd_.sse3) return "SSE3";
                if (simd_.sse2) return "SSE2";
                if (simd_.sse) return "SSE";
                if (simd_.mmx) return "MMX";
                if (simd_.neon) return "NEON";
                if (simd_.armsimd) return "ARM SIMD";
                if (simd_.altivec) return "AltiVec";
                if (simd_.lasx) return "LASX";
                if (simd_.lsx) return "LSX";
                return "None";
            }
    };

    /**
     * @brief Get the global CPU dispatcher instance
     * @return Reference to the CPU dispatcher
     */
    inline const cpu_dispatcher& get_cpu_dispatcher() noexcept {
        static const cpu_dispatcher dispatcher;
        return dispatcher;
    }
} // namespace sdlpp

// Convenience macros for runtime CPU dispatch
#define SDLPP_CPU_HAS_SSE() (::sdlpp::get_cpu_dispatcher().has_sse())
#define SDLPP_CPU_HAS_SSE2() (::sdlpp::get_cpu_dispatcher().has_sse2())
#define SDLPP_CPU_HAS_SSE3() (::sdlpp::get_cpu_dispatcher().has_sse3())
#define SDLPP_CPU_HAS_SSE41() (::sdlpp::get_cpu_dispatcher().has_sse41())
#define SDLPP_CPU_HAS_SSE42() (::sdlpp::get_cpu_dispatcher().has_sse42())
#define SDLPP_CPU_HAS_AVX() (::sdlpp::get_cpu_dispatcher().has_avx())
#define SDLPP_CPU_HAS_AVX2() (::sdlpp::get_cpu_dispatcher().has_avx2())
#define SDLPP_CPU_HAS_AVX512F() (::sdlpp::get_cpu_dispatcher().has_avx512f())
#define SDLPP_CPU_HAS_NEON() (::sdlpp::get_cpu_dispatcher().has_neon())

/**
 * @brief Example usage for runtime CPU dispatch:
 *
 * @code
 * void process_data(float* data, size_t count) {
 *     if (SDLPP_CPU_HAS_AVX2()) {
 *         process_data_avx2(data, count);
 *     } else if (SDLPP_CPU_HAS_SSE2()) {
 *         process_data_sse2(data, count);
 *     } else {
 *         process_data_scalar(data, count);
 *     }
 * }
 * @endcode
 */
