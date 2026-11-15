#include <doctest/doctest.h>

#include <sdlpp/utility/hash.hh>

#include <cstddef>
#include <span>
#include <string_view>

TEST_SUITE("hash") {
    TEST_CASE("crc32 matches the standard check value and chains") {
        constexpr std::string_view check = "123456789";
        CHECK(sdlpp::crc32(0, check.data(), check.size()) == 0xCBF43926u);
        CHECK(sdlpp::crc32(0, std::as_bytes(std::span(check))) == 0xCBF43926u);

        const auto first = sdlpp::crc32(0, check.data(), 4);
        CHECK(sdlpp::crc32(first, check.data() + 4, check.size() - 4) == 0xCBF43926u);
        CHECK(sdlpp::crc32(0, nullptr, 0) == 0u);
    }

    TEST_CASE("murmur3_32 forwards to SDL and depends on the seed") {
        constexpr std::string_view text = "hello";
        CHECK(sdlpp::murmur3_32(text.data(), text.size(), 0) == SDL_murmur3_32(text.data(), text.size(), 0));
        CHECK(sdlpp::murmur3_32(text.data(), text.size(), 0) == 0x248BFA47u);
        CHECK(sdlpp::murmur3_32(std::as_bytes(std::span(text)), 7) == SDL_murmur3_32(text.data(), text.size(), 7));
        CHECK(sdlpp::murmur3_32(text.data(), text.size(), 1) != sdlpp::murmur3_32(text.data(), text.size(), 2));
    }
}
