/**
 * @file surface_renderer.cc
 * @brief Core surface renderer implementation
 */

#include <sdlpp/video/surface_renderer.hh>
#include <SDL3/SDL.h>
#include <algorithm>
#include <cstring>

namespace sdlpp {

// Surface lock implementation
surface_renderer::surface_lock::surface_lock(SDL_Surface* s) 
    : surface_(s), locked_(false) {
    if (surface_ && SDL_MUSTLOCK(surface_)) {
        locked_ = SDL_LockSurface(surface_) == 0;
    }
}

surface_renderer::surface_lock::~surface_lock() {
    if (locked_ && surface_) {
        SDL_UnlockSurface(surface_);
    }
}

surface_renderer::surface_lock::surface_lock(surface_lock&& other) noexcept
    : surface_(other.surface_), locked_(other.locked_) {
    other.surface_ = nullptr;
    other.locked_ = false;
}

surface_renderer::surface_lock& surface_renderer::surface_lock::operator=(surface_lock&& other) noexcept {
    if (this != &other) {
        if (locked_ && surface_) {
            SDL_UnlockSurface(surface_);
        }
        surface_ = other.surface_;
        locked_ = other.locked_;
        other.surface_ = nullptr;
        other.locked_ = false;
    }
    return *this;
}

// Surface renderer implementation
surface_renderer::surface_renderer(const surface& surface)
    : surface_(surface.get())
    , owns_surface_(false)
    , put_pixel_fast_(surface.get_put_pixel_fast())
    , get_pixel_fast_(surface.get_get_pixel_fast())
    , draw_color_{255, 255, 255, 255}
    , blend_mode_(blend_mode::none)
    , mapped_color_(0) {
    
    if (surface_) {
        update_mapped_color();
    }
}

surface_renderer::surface_renderer(int width, int height, SDL_PixelFormat format)
    : surface_(nullptr)
    , owns_surface_(true)
    , draw_color_{255, 255, 255, 255}
    , blend_mode_(blend_mode::none)
    , mapped_color_(0) {
    
    surface_ = SDL_CreateSurface(width, height, format);
    
    if (surface_) {
        update_mapped_color();
    }
}

surface_renderer::~surface_renderer() {
    if (owns_surface_ && surface_) {
        SDL_DestroySurface(surface_);
    }
}

surface_renderer::surface_renderer(surface_renderer&& other) noexcept
    : surface_(other.surface_)
    , owns_surface_(other.owns_surface_)
    , draw_color_(other.draw_color_)
    , blend_mode_(other.blend_mode_)
    , clip_rect_(other.clip_rect_)
    , mapped_color_(other.mapped_color_) {
    
    other.surface_ = nullptr;
    other.owns_surface_ = false;
}

surface_renderer& surface_renderer::operator=(surface_renderer&& other) noexcept {
    if (this != &other) {
        if (owns_surface_ && surface_) {
            SDL_DestroySurface(surface_);
        }
        
        surface_ = other.surface_;
        owns_surface_ = other.owns_surface_;
        draw_color_ = other.draw_color_;
        blend_mode_ = other.blend_mode_;
        clip_rect_ = other.clip_rect_;
        mapped_color_ = other.mapped_color_;
        
        other.surface_ = nullptr;
        other.owns_surface_ = false;
    }
    return *this;
}

void surface_renderer::update_mapped_color() {
    if (!surface_ || !surface_->format) return;
    
    auto details = SDL_GetPixelFormatDetails(surface_->format);
    if (details) {
        mapped_color_ = SDL_MapRGBA(details, nullptr, draw_color_.r, draw_color_.g, draw_color_.b, draw_color_.a);
    }
}

expected<void, std::string> surface_renderer::clear() {
    if (!surface_) {
        return make_unexpected("Invalid surface");
    }
    
    // Use SDL_FillSurfaceRect for efficiency
    if (SDL_FillSurfaceRect(surface_, nullptr, mapped_color_) != 0) {
        return make_unexpected(std::string(SDL_GetError()));
    }
    
    return {};
}

expected<void, std::string> surface_renderer::set_draw_color(const color& c) {
    draw_color_ = c;
    update_mapped_color();
    return {};
}

expected<color, std::string> surface_renderer::get_draw_color() const {
    return draw_color_;
}

expected<void, std::string> surface_renderer::set_draw_blend_mode(blend_mode mode) {
    blend_mode_ = mode;
    return {};
}

expected<blend_mode, std::string> surface_renderer::get_draw_blend_mode() const {
    return blend_mode_;
}

expected<std::optional<rect<int>>, std::string> surface_renderer::get_clip_rect() const {
    return clip_rect_;
}

void surface_renderer::put_pixel(int x, int y, uint32_t pixel) {
    if (!surface_ || x < 0 || y < 0 || x >= surface_->w || y >= surface_->h) {
        return;
    }
    
    // Use fast function pointer if available
    if (put_pixel_fast_ && surface_->pixels) {
        put_pixel_fast_(surface_->pixels, surface_->pitch, x, y, pixel);
        return;
    }
    
    // Fallback to manual implementation
    int bpp = surface_->format ? SDL_BYTESPERPIXEL(surface_->format) : 4;
    uint8_t* p = static_cast<uint8_t*>(surface_->pixels) + y * surface_->pitch + x * bpp;
    
    switch (bpp) {
        case 1:
            *p = static_cast<uint8_t>(pixel);
            break;
        case 2:
            *reinterpret_cast<uint16_t*>(p) = static_cast<uint16_t>(pixel);
            break;
        case 3:
            #if SDL_BYTEORDER == SDL_BIG_ENDIAN
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
            #else
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
            #endif
            break;
        case 4:
            *reinterpret_cast<uint32_t*>(p) = pixel;
            break;
        default:
            // Unsupported bytes per pixel
            break;
    }
}

uint32_t surface_renderer::get_pixel(int x, int y) const {
    if (!surface_ || x < 0 || y < 0 || x >= surface_->w || y >= surface_->h) {
        return 0;
    }
    
    // Use fast function pointer if available
    if (get_pixel_fast_ && surface_->pixels) {
        return get_pixel_fast_(surface_->pixels, surface_->pitch, x, y);
    }
    
    // Fallback to manual implementation
    int bpp = surface_->format ? SDL_BYTESPERPIXEL(surface_->format) : 4;
    uint8_t* p = static_cast<uint8_t*>(surface_->pixels) + y * surface_->pitch + x * bpp;
    
    switch (bpp) {
        case 1:
            return *p;
        case 2:
            return *reinterpret_cast<const uint16_t*>(p);
        case 3:
            #if SDL_BYTEORDER == SDL_BIG_ENDIAN
            return (static_cast<uint32_t>(p[0]) << 16) | (static_cast<uint32_t>(p[1]) << 8) | static_cast<uint32_t>(p[2]);
            #else
            return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) | (static_cast<uint32_t>(p[2]) << 16);
            #endif
        case 4:
            return *reinterpret_cast<const uint32_t*>(p);
        default:
            return 0;
    }
}

bool surface_renderer::clip_line(float& x0, float& y0, float& x1, float& y1) const {
    if (!clip_rect_) return true;
    
    const auto& clip = *clip_rect_;
    
    // Cohen-Sutherland line clipping algorithm
    enum OutCode {
        INSIDE = 0,
        LEFT = 1,
        RIGHT = 2,
        BOTTOM = 4,
        TOP = 8
    };
    
    auto compute_outcode = [&clip](float x, float y) {
        int code = INSIDE;
        if (x < static_cast<float>(clip.x)) code |= LEFT;
        else if (x >= static_cast<float>(clip.x + clip.w)) code |= RIGHT;
        if (y < static_cast<float>(clip.y)) code |= TOP;
        else if (y >= static_cast<float>(clip.y + clip.h)) code |= BOTTOM;
        return code;
    };
    
    int outcode0 = compute_outcode(x0, y0);
    int outcode1 = compute_outcode(x1, y1);
    
    while (true) {
        if (!(outcode0 | outcode1)) {
            // Both points inside
            return true;
        } else if (outcode0 & outcode1) {
            // Both points on same side outside
            return false;
        } else {
            // Line crosses clip boundary
            float x, y;
            int outcode_out = outcode0 ? outcode0 : outcode1;
            
            if (outcode_out & TOP) {
                x = x0 + (x1 - x0) * (static_cast<float>(clip.y) - y0) / (y1 - y0);
                y = static_cast<float>(clip.y);
            } else if (outcode_out & BOTTOM) {
                x = x0 + (x1 - x0) * (static_cast<float>(clip.y + clip.h - 1) - y0) / (y1 - y0);
                y = static_cast<float>(clip.y + clip.h - 1);
            } else if (outcode_out & RIGHT) {
                y = y0 + (y1 - y0) * (static_cast<float>(clip.x + clip.w - 1) - x0) / (x1 - x0);
                x = static_cast<float>(clip.x + clip.w - 1);
            } else { // LEFT
                y = y0 + (y1 - y0) * (static_cast<float>(clip.x) - x0) / (x1 - x0);
                x = static_cast<float>(clip.x);
            }
            
            if (outcode_out == outcode0) {
                x0 = x;
                y0 = y;
                outcode0 = compute_outcode(x0, y0);
            } else {
                x1 = x;
                y1 = y;
                outcode1 = compute_outcode(x1, y1);
            }
        }
    }
}

bool surface_renderer::clip_rect_to_clip(rect<int>& r) const {
    if (!clip_rect_) return true;
    
    const auto& clip = *clip_rect_;
    
    int x0 = std::max(r.x, clip.x);
    int y0 = std::max(r.y, clip.y);
    int x1 = std::min(r.x + r.w, clip.x + clip.w);
    int y1 = std::min(r.y + r.h, clip.y + clip.h);
    
    if (x1 <= x0 || y1 <= y0) {
        return false;
    }
    
    r.x = x0;
    r.y = y0;
    r.w = x1 - x0;
    r.h = y1 - y0;
    return true;
}

expected<void, std::string> surface_renderer::set_alpha_mod(uint8_t alpha) {
    if (!surface_) {
        return make_unexpected("Invalid surface");
    }
    
    if (!SDL_SetSurfaceAlphaMod(surface_, alpha)) {
        return make_unexpected(get_error());
    }
    
    return {};
}

expected<uint8_t, std::string> surface_renderer::get_alpha_mod() const {
    if (!surface_) {
        return make_unexpected("Invalid surface");
    }
    
    uint8_t alpha;
    if (!SDL_GetSurfaceAlphaMod(surface_, &alpha)) {
        return make_unexpected(get_error());
    }
    
    return alpha;
}

expected<void, std::string> surface_renderer::set_color_mod(uint8_t r, uint8_t g, uint8_t b) {
    if (!surface_) {
        return make_unexpected("Invalid surface");
    }
    
    if (!SDL_SetSurfaceColorMod(surface_, r, g, b)) {
        return make_unexpected(get_error());
    }
    
    return {};
}

expected<std::tuple<uint8_t, uint8_t, uint8_t>, std::string> surface_renderer::get_color_mod() const {
    if (!surface_) {
        return make_unexpected("Invalid surface");
    }
    
    uint8_t r, g, b;
    if (!SDL_GetSurfaceColorMod(surface_, &r, &g, &b)) {
        return make_unexpected(get_error());
    }
    
    return std::make_tuple(r, g, b);
}

void surface_renderer::apply_blend_mode(int x, int y, uint32_t src_pixel) {
    if (!clip_point(x, y)) return;
    
    if (blend_mode_ == blend_mode::none) {
        put_pixel(x, y, src_pixel);
        return;
    }
    
    // Get RGBA components of source pixel
    uint8_t sr, sg, sb, sa;
    auto details = SDL_GetPixelFormatDetails(surface_->format);
    SDL_GetRGBA(src_pixel, details, nullptr, &sr, &sg, &sb, &sa);
    
    if (sa == 0) return; // Fully transparent
    
    if (sa == 255 && blend_mode_ == blend_mode::blend) {
        // Fully opaque with alpha blend - just write the pixel
        put_pixel(x, y, src_pixel);
        return;
    }
    
    // Get destination pixel
    uint32_t dst_pixel = get_pixel(x, y);
    uint8_t dr, dg, db, da;
    SDL_GetRGBA(dst_pixel, details, nullptr, &dr, &dg, &db, &da);
    
    uint8_t r, g, b, a;
    
    switch (blend_mode_) {
        case blend_mode::blend: {
            // Standard alpha blending: result = src * alpha + dst * (1 - alpha)
            float alpha = sa / 255.0f;
            float inv_alpha = 1.0f - alpha;
            
            r = static_cast<uint8_t>(sr * alpha + dr * inv_alpha);
            g = static_cast<uint8_t>(sg * alpha + dg * inv_alpha);
            b = static_cast<uint8_t>(sb * alpha + db * inv_alpha);
            a = static_cast<uint8_t>(sa + da * inv_alpha);
            break;
        }
        
        case blend_mode::add: {
            // Additive blending: result = src + dst
            r = static_cast<uint8_t>(std::min(static_cast<int>(sr) + dr, 255));
            g = static_cast<uint8_t>(std::min(static_cast<int>(sg) + dg, 255));
            b = static_cast<uint8_t>(std::min(static_cast<int>(sb) + db, 255));
            a = static_cast<uint8_t>(std::min(static_cast<int>(sa) + da, 255));
            break;
        }
        
        case blend_mode::mod: {
            // Modulate blending: result = src * dst / 255
            r = static_cast<uint8_t>((static_cast<int>(sr) * dr) / 255);
            g = static_cast<uint8_t>((static_cast<int>(sg) * dg) / 255);
            b = static_cast<uint8_t>((static_cast<int>(sb) * db) / 255);
            a = static_cast<uint8_t>((static_cast<int>(sa) * da) / 255);
            break;
        }
        
        case blend_mode::mul: {
            // Multiply blending: result = (src * dst) / 255
            r = static_cast<uint8_t>((static_cast<int>(sr) * dr) / 255);
            g = static_cast<uint8_t>((static_cast<int>(sg) * dg) / 255);
            b = static_cast<uint8_t>((static_cast<int>(sb) * db) / 255);
            a = sa; // Keep source alpha
            break;
        }
        
        default:
            // For any other mode, fall back to simple copy
            r = sr;
            g = sg;
            b = sb;
            a = sa;
            break;
    }
    
    uint32_t result_pixel = SDL_MapRGBA(details, nullptr, r, g, b, a);
    put_pixel(x, y, result_pixel);
}

void surface_renderer::blend_pixel(int x, int y, uint32_t pixel, float alpha) {
    if (!surface_ || x < 0 || y < 0 || x >= surface_->w || y >= surface_->h) {
        return;
    }
    
    // Get the existing pixel
    uint32_t existing = get_pixel(x, y);
    
    // Get pixel format details
    auto details = SDL_GetPixelFormatDetails(surface_->format);
    if (!details) return;
    
    // Extract RGBA components from source pixel
    uint8_t sr, sg, sb, sa;
    SDL_GetRGBA(pixel, details, nullptr, &sr, &sg, &sb, &sa);
    
    // Extract RGBA components from existing pixel
    uint8_t dr, dg, db, da;
    SDL_GetRGBA(existing, details, nullptr, &dr, &dg, &db, &da);
    
    // Apply alpha blending
    float src_alpha = alpha * (sa / 255.0f);
    float inv_alpha = 1.0f - src_alpha;
    
    uint8_t r = static_cast<uint8_t>(sr * src_alpha + dr * inv_alpha);
    uint8_t g = static_cast<uint8_t>(sg * src_alpha + dg * inv_alpha);
    uint8_t b = static_cast<uint8_t>(sb * src_alpha + db * inv_alpha);
    uint8_t a = static_cast<uint8_t>(std::max(sa, da)); // Use max alpha
    
    // Map back to pixel format
    uint32_t blended = SDL_MapRGBA(details, nullptr, r, g, b, a);
    put_pixel(x, y, blended);
}

} // namespace sdlpp