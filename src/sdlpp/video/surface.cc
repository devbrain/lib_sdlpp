/**
 * @file surface.cc
 * @brief Surface class implementation
 */

#include <sdlpp/video/surface.hh>
#include <SDL3/SDL.h>

namespace sdlpp {

// Fast pixel access functions for different formats
namespace {

// 8-bit pixel access
void put_pixel_8(void* pixels, int pitch, int x, int y, uint32_t pixel) {
    uint8_t* p = static_cast<uint8_t*>(pixels) + y * pitch + x;
    *p = static_cast<uint8_t>(pixel);
}

uint32_t get_pixel_8(const void* pixels, int pitch, int x, int y) {
    const uint8_t* p = static_cast<const uint8_t*>(pixels) + y * pitch + x;
    return *p;
}

// 16-bit pixel access
void put_pixel_16(void* pixels, int pitch, int x, int y, uint32_t pixel) {
    uint8_t* p = static_cast<uint8_t*>(pixels) + y * pitch + x * 2;
    *reinterpret_cast<uint16_t*>(p) = static_cast<uint16_t>(pixel);
}

uint32_t get_pixel_16(const void* pixels, int pitch, int x, int y) {
    const uint8_t* p = static_cast<const uint8_t*>(pixels) + y * pitch + x * 2;
    return *reinterpret_cast<const uint16_t*>(p);
}

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
// 24-bit pixel access (big endian)
void put_pixel_24_be(void* pixels, int pitch, int x, int y, uint32_t pixel) {
    uint8_t* p = static_cast<uint8_t*>(pixels) + y * pitch + x * 3;
    p[0] = (pixel >> 16) & 0xff;
    p[1] = (pixel >> 8) & 0xff;
    p[2] = pixel & 0xff;
}

uint32_t get_pixel_24_be(const void* pixels, int pitch, int x, int y) {
    const uint8_t* p = static_cast<const uint8_t*>(pixels) + y * pitch + x * 3;
    return (p[0] << 16) | (p[1] << 8) | p[2];
}
#endif

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
// 24-bit pixel access (little endian)
void put_pixel_24_le(void* pixels, int pitch, int x, int y, uint32_t pixel) {
    uint8_t* p = static_cast<uint8_t*>(pixels) + y * pitch + x * 3;
    p[0] = pixel & 0xff;
    p[1] = (pixel >> 8) & 0xff;
    p[2] = (pixel >> 16) & 0xff;
}

uint32_t get_pixel_24_le(const void* pixels, int pitch, int x, int y) {
    const uint8_t* p = static_cast<const uint8_t*>(pixels) + y * pitch + x * 3;
    return p[0] | (p[1] << 8) | (p[2] << 16);
}
#endif

// 32-bit pixel access
void put_pixel_32(void* pixels, int pitch, int x, int y, uint32_t pixel) {
    uint8_t* p = static_cast<uint8_t*>(pixels) + y * pitch + x * 4;
    *reinterpret_cast<uint32_t*>(p) = pixel;
}

uint32_t get_pixel_32(const void* pixels, int pitch, int x, int y) {
    const uint8_t* p = static_cast<const uint8_t*>(pixels) + y * pitch + x * 4;
    return *reinterpret_cast<const uint32_t*>(p);
}

} // anonymous namespace

void surface::setup_pixel_functions() {
    if (!ptr || !ptr->format) {
        put_pixel_fast = nullptr;
        get_pixel_fast = nullptr;
        return;
    }
    
    int bpp = SDL_BYTESPERPIXEL(ptr->format);
    
    switch (bpp) {
        case 1:
            put_pixel_fast = put_pixel_8;
            get_pixel_fast = get_pixel_8;
            break;
            
        case 2:
            put_pixel_fast = put_pixel_16;
            get_pixel_fast = get_pixel_16;
            break;
            
        case 3:
            #if SDL_BYTEORDER == SDL_BIG_ENDIAN
            put_pixel_fast = put_pixel_24_be;
            get_pixel_fast = get_pixel_24_be;
            #else
            put_pixel_fast = put_pixel_24_le;
            get_pixel_fast = get_pixel_24_le;
            #endif
            break;
            
        case 4:
            put_pixel_fast = put_pixel_32;
            get_pixel_fast = get_pixel_32;
            break;
            
        default:
            put_pixel_fast = nullptr;
            get_pixel_fast = nullptr;
            break;
    }
}

} // namespace sdlpp