#pragma once

/**
 * @file intrinsics.hh
 * @brief Low-level intrinsic functions and utilities
 * 
 * This header provides access to compiler intrinsics and low-level
 * operations like memory barriers, atomic operations, and bit manipulation.
 */

#include <sdlpp/core/sdl.hh>
#include <cstdint>
#include <type_traits>

// Include intrinsics headers for MSVC
#if defined(_MSC_VER) && defined(_M_X64)
    #include <mmintrin.h>
#endif

namespace sdlpp {
    /**
     * @brief Memory barrier and synchronization primitives
     */
    namespace memory_barrier {
        /**
         * @brief Full memory barrier
         *
         * Prevents the compiler and CPU from reordering memory operations
         * across this barrier. This is a full barrier affecting both loads and stores.
         * @note SDL3 doesn't have a full barrier, so we use both acquire and release
         */
        inline void full_barrier() noexcept {
            SDL_MemoryBarrierRelease();
            SDL_MemoryBarrierAcquire();
        }

        /**
         * @brief Compiler-only memory barrier
         *
         * Prevents only the compiler from reordering memory operations.
         * Does not emit CPU fence instructions.
         */
        inline void compiler_barrier() noexcept {
            SDL_CompilerBarrier();
        }

        /**
         * @brief Acquire memory barrier
         *
         * Prevents memory reordering of loads across this barrier.
         * Typically used after acquiring a lock or reading a synchronization variable.
         */
        inline void acquire_barrier() noexcept {
            SDL_MemoryBarrierAcquire();
        }

        /**
         * @brief Release memory barrier
         *
         * Prevents memory reordering of stores across this barrier.
         * Typically used before releasing a lock or writing a synchronization variable.
         */
        inline void release_barrier() noexcept {
            SDL_MemoryBarrierRelease();
        }
    }

    /**
     * @brief Atomic operations
     */
    namespace atomic {
        /**
         * @brief Atomically compare and swap a 32-bit value
         * @param ptr Pointer to the value to update
         * @param oldval Expected old value
         * @param newval New value to write if oldval matches
         * @return true if the swap was performed
         */
        [[nodiscard]] inline bool compare_and_swap(std::int32_t* ptr,
                                                   std::int32_t oldval,
                                                   std::int32_t newval) noexcept {
            return SDL_CompareAndSwapAtomicInt(reinterpret_cast <SDL_AtomicInt*>(ptr), oldval, newval);
        }

        /**
         * @brief Atomically compare and swap a pointer
         * @param ptr Pointer to the pointer to update
         * @param oldval Expected old value
         * @param newval New value to write if oldval matches
         * @return true if the swap was performed
         */
        [[nodiscard]] inline bool compare_and_swap(void** ptr,
                                                   void* oldval,
                                                   void* newval) noexcept {
            return SDL_CompareAndSwapAtomicPointer(ptr, oldval, newval);
        }

        /**
         * @brief Atomically set a 32-bit value and return the old value
         * @param ptr Pointer to the value to update
         * @param value New value
         * @return Previous value
         */
        [[nodiscard]] inline std::int32_t exchange(std::int32_t* ptr,
                                                   std::int32_t value) noexcept {
            return SDL_SetAtomicInt(reinterpret_cast <SDL_AtomicInt*>(ptr), value);
        }

        /**
         * @brief Atomically set a pointer and return the old value
         * @param ptr Pointer to the pointer to update
         * @param value New value
         * @return Previous value
         */
        [[nodiscard]] inline void* exchange(void** ptr, void* value) noexcept {
            return SDL_SetAtomicPointer(ptr, value);
        }

        /**
         * @brief Atomically get a 32-bit value
         * @param ptr Pointer to the value
         * @return Current value
         */
        [[nodiscard]] inline std::int32_t load(const std::int32_t* ptr) noexcept {
            return SDL_GetAtomicInt(reinterpret_cast <SDL_AtomicInt*>(const_cast <std::int32_t*>(ptr)));
        }

        /**
         * @brief Atomically get a pointer value
         * @param ptr Pointer to the pointer
         * @return Current value
         */
        [[nodiscard]] inline void* load(void** ptr) noexcept {
            return SDL_GetAtomicPointer(ptr);
        }

        /**
         * @brief Atomically add to a 32-bit value
         * @param ptr Pointer to the value
         * @param value Value to add
         * @return Previous value before addition
         */
        [[nodiscard]] inline std::int32_t add(std::int32_t* ptr,
                                              std::int32_t value) noexcept {
            return SDL_AddAtomicInt(reinterpret_cast <SDL_AtomicInt*>(ptr), value);
        }
    }

    /**
     * @brief Bit manipulation utilities
     */
    namespace bits {
        /**
         * @brief Get the index of the most significant bit set
         * @param value Value to check
         * @return 0-based index of MSB, or -1 if value is 0
         */
        [[nodiscard]] inline int most_significant_bit(std::uint32_t value) noexcept {
            return SDL_MostSignificantBitIndex32(value);
        }

        /**
         * @brief Check if a value has exactly one bit set (is a power of 2)
         * @param value Value to check
         * @return true if value is a power of 2
         */
        [[nodiscard]] inline bool has_exactly_one_bit_set(std::uint32_t value) noexcept {
            return SDL_HasExactlyOneBitSet32(value);
        }

        /**
         * @brief Swap byte order of a 16-bit value
         * @param value Value to swap
         * @return Byte-swapped value
         */
        [[nodiscard]] inline std::uint16_t swap_bytes(std::uint16_t value) noexcept {
            return SDL_Swap16(value);
        }

        /**
         * @brief Swap byte order of a 32-bit value
         * @param value Value to swap
         * @return Byte-swapped value
         */
        [[nodiscard]] inline std::uint32_t swap_bytes(std::uint32_t value) noexcept {
            return SDL_Swap32(value);
        }

        /**
         * @brief Swap byte order of a 64-bit value
         * @param value Value to swap
         * @return Byte-swapped value
         */
        [[nodiscard]] inline std::uint64_t swap_bytes(std::uint64_t value) noexcept {
            return SDL_Swap64(value);
        }

        /**
         * @brief Swap byte order of a float
         * @param value Value to swap
         * @return Byte-swapped value
         */
        [[nodiscard]] inline float swap_bytes(float value) noexcept {
            return SDL_SwapFloat(value);
        }
    }

    /**
     * @brief Endianness utilities
     */
    namespace endian {
        /**
         * @brief Check if system is big endian
         * @return true if big endian
         */
        [[nodiscard]] inline constexpr bool is_big_endian() noexcept {
            return SDL_BYTEORDER == SDL_BIG_ENDIAN;
        }

        /**
         * @brief Check if system is little endian
         * @return true if little endian
         */
        [[nodiscard]] inline constexpr bool is_little_endian() noexcept {
            return SDL_BYTEORDER == SDL_LIL_ENDIAN;
        }

        /**
         * @brief Convert 16-bit value from big endian to native
         * @param value Big endian value
         * @return Native endian value
         */
        [[nodiscard]] inline std::uint16_t from_big_endian(std::uint16_t value) noexcept {
            return SDL_Swap16BE(value);
        }

        /**
         * @brief Convert 32-bit value from big endian to native
         * @param value Big endian value
         * @return Native endian value
         */
        [[nodiscard]] inline std::uint32_t from_big_endian(std::uint32_t value) noexcept {
            return SDL_Swap32BE(value);
        }

        /**
         * @brief Convert 64-bit value from big endian to native
         * @param value Big endian value
         * @return Native endian value
         */
        [[nodiscard]] inline std::uint64_t from_big_endian(std::uint64_t value) noexcept {
            return SDL_Swap64BE(value);
        }

        /**
         * @brief Convert float from big endian to native
         * @param value Big endian value
         * @return Native endian value
         */
        [[nodiscard]] inline float from_big_endian(float value) noexcept {
            return SDL_SwapFloatBE(value);
        }

        /**
         * @brief Convert 16-bit value from little endian to native
         * @param value Little endian value
         * @return Native endian value
         */
        [[nodiscard]] inline std::uint16_t from_little_endian(std::uint16_t value) noexcept {
            return SDL_Swap16LE(value);
        }

        /**
         * @brief Convert 32-bit value from little endian to native
         * @param value Little endian value
         * @return Native endian value
         */
        [[nodiscard]] inline std::uint32_t from_little_endian(std::uint32_t value) noexcept {
            return SDL_Swap32LE(value);
        }

        /**
         * @brief Convert 64-bit value from little endian to native
         * @param value Little endian value
         * @return Native endian value
         */
        [[nodiscard]] inline std::uint64_t from_little_endian(std::uint64_t value) noexcept {
            return SDL_Swap64LE(value);
        }

        /**
         * @brief Convert float from little endian to native
         * @param value Little endian value
         * @return Native endian value
         */
        [[nodiscard]] inline float from_little_endian(float value) noexcept {
            return SDL_SwapFloatLE(value);
        }

        /**
         * @brief Convert value to big endian (same as from_big_endian)
         */
        template<typename T>
        [[nodiscard]] inline T to_big_endian(T value) noexcept {
            return from_big_endian(value);
        }

        /**
         * @brief Convert value to little endian (same as from_little_endian)
         */
        template<typename T>
        [[nodiscard]] inline T to_little_endian(T value) noexcept {
            return from_little_endian(value);
        }
    }

    /**
     * @brief Math intrinsics
     */
    namespace math {
        /**
         * @brief Get the next power of 2 greater than or equal to value
         * @param value Input value
         * @return Next power of 2, or 0 on overflow
         */
        [[nodiscard]] inline std::uint32_t next_power_of_two(std::uint32_t value) noexcept {
            if (value == 0) return 0;
            if (value > 0x80000000) return 0; // Overflow

            // Check if already a power of 2
            if (sdlpp::bits::has_exactly_one_bit_set(value)) {
                return value;
            }

            // Find the next power of 2
            value--;
            value |= value >> 1;
            value |= value >> 2;
            value |= value >> 4;
            value |= value >> 8;
            value |= value >> 16;
            value++;

            return value;
        }

        /**
         * @brief Align a value up to the nearest multiple of alignment
         * @param value Value to align
         * @param alignment Alignment (must be power of 2)
         * @return Aligned value
         */
        template<typename T>
        [[nodiscard]] inline T align_up(T value, T alignment) noexcept {
            static_assert(std::is_integral_v <T>, "align_up requires integral type");
            return (value + alignment - 1) & ~(alignment - 1);
        }

        /**
         * @brief Align a value down to the nearest multiple of alignment
         * @param value Value to align
         * @param alignment Alignment (must be power of 2)
         * @return Aligned value
         */
        template<typename T>
        [[nodiscard]] inline T align_down(T value, T alignment) noexcept {
            static_assert(std::is_integral_v <T>, "align_down requires integral type");
            return value & ~(alignment - 1);
        }

        /**
         * @brief Check if a value is aligned
         * @param value Value to check
         * @param alignment Alignment to check (must be power of 2)
         * @return true if aligned
         */
        template<typename T>
        [[nodiscard]] inline bool is_aligned(T value, T alignment) noexcept {
            static_assert(std::is_integral_v <T>, "is_aligned requires integral type");
            return (value & (alignment - 1)) == 0;
        }
    }

    /**
     * @brief Prefetch hints for CPU cache
     */
    namespace prefetch {
        /**
         * @brief Prefetch data into cache for reading
         * @param addr Address to prefetch
         * @param locality 0-3, where 0 means no temporal locality (use once)
         *                 and 3 means high temporal locality (use many times)
         */
        inline void for_read(const void* addr, int locality = 0) noexcept {
#if defined(__GNUC__) || defined(__clang__)
            switch (locality) {
                case 0: __builtin_prefetch(addr, 0, 0);
                    break;
                case 1: __builtin_prefetch(addr, 0, 1);
                    break;
                case 2: __builtin_prefetch(addr, 0, 2);
                    break;
                case 3: __builtin_prefetch(addr, 0, 3);
                    break;
                default: __builtin_prefetch(addr, 0, 3);
                    break;
            }
#elif defined(_MSC_VER) && defined(_M_X64)
            // MSVC x64 uses _mm_prefetch
            switch (locality) {
                case 0:  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_NTA); break;
                case 1:  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T2); break;
                case 2:  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T1); break;
                case 3:  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T0); break;
                default: _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T0); break;
            }
#elif defined(_MSC_VER) && defined(_M_ARM64)
            // ARM64 MSVC uses __prefetch
            __prefetch(addr);
            (void)locality;
#else
            // No prefetch available
            (void)addr;
            (void)locality;
#endif
        }

        /**
         * @brief Prefetch data into cache for writing
         * @param addr Address to prefetch
         * @param locality 0-3, where 0 means no temporal locality (use once)
         *                 and 3 means high temporal locality (use many times)
         */
        inline void for_write(void* addr, int locality = 0) noexcept {
#if defined(__GNUC__) || defined(__clang__)
            switch (locality) {
                case 0: __builtin_prefetch(addr, 1, 0);
                    break;
                case 1: __builtin_prefetch(addr, 1, 1);
                    break;
                case 2: __builtin_prefetch(addr, 1, 2);
                    break;
                case 3: __builtin_prefetch(addr, 1, 3);
                    break;
                default: __builtin_prefetch(addr, 1, 3);
                    break;
            }
#elif defined(_MSC_VER) && defined(_M_X64)
            // MSVC x64 uses _mm_prefetch (no separate write hint)
            switch (locality) {
                case 0:  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_NTA); break;
                case 1:  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T2); break;
                case 2:  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T1); break;
                case 3:  _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T0); break;
                default: _mm_prefetch(static_cast<const char*>(addr), _MM_HINT_T0); break;
            }
#elif defined(_MSC_VER) && defined(_M_ARM64)
            // ARM64 MSVC uses __prefetch
            __prefetch(addr);
            (void)locality;
#else
            // No prefetch available
            (void)addr;
            (void)locality;
#endif
        }
    }
} // namespace sdlpp
