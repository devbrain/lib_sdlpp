#pragma once

/**
 * @file hash.hh
 * @brief SDL's checksum and hash functions: CRC-32 and murmur3
 */

#include <sdlpp/core/sdl.hh>

#include <cstddef>
#include <cstdint>
#include <span>

namespace sdlpp {
    /**
     * @brief Update a CRC-32 (the zlib / gzip polynomial) with @p len bytes at @p data.
     * @param crc The CRC so far; 0 to start a new one
     * @return The updated CRC
     */
    [[nodiscard]] inline std::uint32_t crc32(std::uint32_t crc, const void* data, std::size_t len) noexcept {
        return SDL_crc32(crc, data, len);
    }

    /**
     * @brief Update a CRC-32 (the zlib / gzip polynomial) with @p data.
     * @param crc The CRC so far; 0 to start a new one
     * @return The updated CRC
     */
    [[nodiscard]] inline std::uint32_t crc32(std::uint32_t crc, std::span <const std::byte> data) noexcept {
        return SDL_crc32(crc, data.data(), data.size());
    }

    /**
     * @brief The 32-bit murmur3 hash of @p len bytes at @p data.
     *
     * Not cryptographic. The seed changes the result, but unlike crc32() a previous result
     * cannot be fed back as the seed to hash data in chunks.
     */
    [[nodiscard]] inline std::uint32_t murmur3_32(const void* data, std::size_t len, std::uint32_t seed) noexcept {
        return SDL_murmur3_32(data, len, seed);
    }

    /**
     * @brief The 32-bit murmur3 hash of @p data.
     *
     * Not cryptographic. The seed changes the result, but unlike crc32() a previous result
     * cannot be fed back as the seed to hash data in chunks.
     */
    [[nodiscard]] inline std::uint32_t murmur3_32(std::span <const std::byte> data, std::uint32_t seed) noexcept {
        return SDL_murmur3_32(data.data(), data.size(), seed);
    }
}
