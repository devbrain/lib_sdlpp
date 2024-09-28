//
// Created by igor on 06/06/2020.
//

#ifndef NEUTRINO_SDL_TEXTURE_HH
#define NEUTRINO_SDL_TEXTURE_HH

#include <optional>
#include <array>
#include <type_traits>

#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/video/pixel_format.hh>
#include <sdlpp/video/blend_mode.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/video/geometry.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
    /**
     * @class texture
     *
     * @brief Represents an SDL texture.
     *
     * This class provides functionality to create, modify, and access SDL textures.
     * It inherits from the object class, which manages the ownership and lifetime of the underlying SDL_Texture.
     *
     */
    class texture : public object <SDL_Texture> {
        public:
            enum class access : uint32_t {
                STATIC = SDL_TEXTUREACCESS_STATIC,
                STREAMING = SDL_TEXTUREACCESS_STREAMING,
                TARGET = SDL_TEXTUREACCESS_TARGET
            };

        public:
            texture() = default;

            texture(const object <SDL_Renderer>& r, const pixel_format& format, unsigned w, unsigned h, access flags);

            texture(const object <SDL_Renderer>& r, const pixel_format& format, area_type dims, access flags);
            texture(const object <SDL_Renderer>& r, const object <SDL_Surface>& s);

            explicit texture(object <SDL_Texture>&& other);
            texture& operator=(object <SDL_Texture>&& other) noexcept;

            // returns: pixel format, texture_access, w, h
            [[nodiscard]] std::tuple <pixel_format, access, unsigned, unsigned> query() const;
            [[nodiscard]] area_type get_dimensions() const;

            [[nodiscard]] uint8_t get_alpha() const;
            void set_alpha(uint8_t a);

            [[nodiscard]] blend_mode get_blend_mode() const;
            void set_blend_mode(blend_mode bm);

            [[nodiscard]] std::optional <color> get_color_mod() const;
            void set_color_mod(const color& c);

            [[nodiscard]] surface convert_to_surface(const object <SDL_Renderer>& r,
                                                     const pixel_format& format = pixel_format::RGBA32)
            const;

            /*
            Use this function to lock whole texture for write-only pixel access.
            returns: pointer to pixels and pitch, i.e., the length of one row in bytes.
            */
            [[nodiscard]] std::pair <void*, std::size_t> lock() const;
            /*
            Use this function to lock a portion of the texture for write-only pixel access.
            returns: pointer to pixels and pitch, i.e., the length of one row in bytes.
            */
            [[nodiscard]] std::pair <void*, std::size_t> lock(const rect& r) const;

            void unlock() const;

            // slow updates
            void update(const void* pixels, std::size_t pitch);
            void update(const rect& area, const void* pixels, std::size_t pitch);
    };

    d_SDLPP_OSTREAM(texture::access);
} // ns sdl
// =================================================================================================================
// Implementation
// =================================================================================================================
namespace neutrino::sdl {
    inline
    texture::texture(const object <SDL_Renderer>& r,
                     const pixel_format& format,
                     unsigned w, unsigned h, access flags)
        : object <SDL_Texture>(SAFE_SDL_CALL(SDL_CreateTexture,
                                             const_cast<SDL_Renderer*>(r.handle ()),
                                             format.value (),
                                             static_cast<std::uint32_t>(flags),
                                             static_cast<int>(w),
                                             static_cast<int>(h))                               , true) {
    }

    inline
    texture::texture(const object <SDL_Renderer>& r, const pixel_format& format, area_type dims, access flags)
        : object <SDL_Texture>(SAFE_SDL_CALL(SDL_CreateTexture,
                                             const_cast<SDL_Renderer*>(r.handle ()),
                                             format.value (),
                                             static_cast<std::uint32_t>(flags),
                                             static_cast<int>(dims.w),
                                             static_cast<int>(dims.h))                               , true) {
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    texture::texture(const object <SDL_Renderer>& r, const object <SDL_Surface>& s)
        : object <SDL_Texture>(SAFE_SDL_CALL(SDL_CreateTextureFromSurface,
                                             const_cast<SDL_Renderer*>(r.handle ()),
                                             const_cast<SDL_Surface*>(s.handle ())
        )                               , true) {
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    texture::texture(object <SDL_Texture>&& other)
        : object <SDL_Texture>(std::move(other)) {
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    texture& texture::operator=(object <SDL_Texture>&& other) noexcept {
        object <SDL_Texture>::operator=(std::move(other));
        return *this;
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    surface texture::convert_to_surface(const object <SDL_Renderer>& r, const pixel_format& format) const {
        const auto dims = get_dimensions();
        const auto w = dims.w;
        const auto h = dims.h;
        texture target(r, format, w, h, access::TARGET);
        surface srf(dims, format);
        SDL_Texture* original = SDL_GetRenderTarget(r.const_handle());
        SAFE_SDL_CALL(SDL_SetRenderTarget, r.const_handle(), target.handle());
        try {
            SAFE_SDL_CALL(SDL_RenderCopy, r.const_handle(), this->const_handle(), nullptr, nullptr);
            const int pitch = srf->pitch;
            std::vector <char> pixels(h*pitch, 0);
            SAFE_SDL_CALL(SDL_RenderReadPixels, r.const_handle(), nullptr, format.value(), pixels.data(), pitch);
            SDL_memcpy(srf->pixels, pixels.data(), pixels.size());
            SDL_SetRenderTarget (r.const_handle(), original);
            return srf;
        } catch (...) {
            SDL_SetRenderTarget (r.const_handle(), original);
            throw;
        }
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    std::tuple <pixel_format, texture::access, unsigned, unsigned> texture::query() const {
        uint32_t format;
        int w;
        int h;
        int acc;
        SAFE_SDL_CALL(SDL_QueryTexture, const_handle (), &format, &acc, &w, &h);
        return {pixel_format(format), (access)acc, static_cast <unsigned>(w), static_cast <unsigned>(h)};
    }

    inline
    area_type texture::get_dimensions() const {
        int w;
        int h;
        SAFE_SDL_CALL(SDL_QueryTexture, const_handle (), nullptr, nullptr, &w, &h);
        return {static_cast<unsigned int>(w), static_cast<unsigned int>(h)};
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    uint8_t texture::get_alpha() const {
        uint8_t a;
        SAFE_SDL_CALL(SDL_GetTextureAlphaMod, const_handle (), &a);
        return a;
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    void texture::set_alpha(uint8_t a) {
        SAFE_SDL_CALL(SDL_SetTextureAlphaMod, handle (), a);
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    blend_mode texture::get_blend_mode() const {
        SDL_BlendMode x;
        SAFE_SDL_CALL(SDL_GetTextureBlendMode, const_handle (), &x);
        return static_cast <blend_mode>(x);
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    void texture::set_blend_mode(blend_mode bm) {
        SAFE_SDL_CALL(SDL_SetTextureBlendMode, handle (), static_cast<SDL_BlendMode>(bm));
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    std::optional <color> texture::get_color_mod() const {
        color c{0, 0, 0, 0};
        if (0 == SDL_GetTextureColorMod(const_handle(), &c.r, &c.g, &c.b)) {
            return c;
        }
        return std::nullopt;
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    void texture::set_color_mod(const color& c) {
        SAFE_SDL_CALL(SDL_SetTextureColorMod, handle (), c.r, c.g, c.b);
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    std::pair <void*, std::size_t> texture::lock() const {
        void* pixels;
        int pitch;
        SAFE_SDL_CALL(SDL_LockTexture, const_cast<SDL_Texture*>(handle ()), nullptr, &pixels, &pitch);
        return {pixels, static_cast <std::size_t>(pitch)};
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    std::pair <void*, std::size_t> texture::lock(const rect& r) const {
        void* pixels;
        int pitch;
        SAFE_SDL_CALL(SDL_LockTexture, const_cast<SDL_Texture*>(handle ()), &r, &pixels, &pitch);
        return {pixels, static_cast <std::size_t>(pitch)};
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    void texture::unlock() const {
        SAFE_SDL_CALL(SDL_UnlockTexture, const_handle ());
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    void texture::update(const void* pixels, std::size_t pitch) {
        SAFE_SDL_CALL(SDL_UpdateTexture, handle (), nullptr, pixels, static_cast<int>(pitch));
    }

    // ---------------------------------------------------------------------------------------------------------------
    inline
    void texture::update(const rect& area, const void* pixels, std::size_t pitch) {
        SAFE_SDL_CALL(SDL_UpdateTexture, handle (), &area, pixels, static_cast<int>(pitch));
    }

    namespace detail {
        static inline constexpr std::array <texture::access, 3> s_vals_of_provides_access = {
            texture::access::STATIC,
            texture::access::STREAMING,
            texture::access::TARGET,
        };
    }

    template<typename T>
    static constexpr const decltype(detail::s_vals_of_provides_access)&
    values(std::enable_if_t <std::is_same_v <texture::access, T>>* = nullptr) {
        return detail::s_vals_of_provides_access;
    }

    template<typename T>
    static constexpr auto
    begin(std::enable_if_t <std::is_same_v <texture::access, T>>* = nullptr) {
        return detail::s_vals_of_provides_access.begin();
    }

    template<typename T>
    static constexpr auto
    end(std::enable_if_t <std::is_same_v <texture::access, T>>* = nullptr) {
        return detail::s_vals_of_provides_access.end();
    }
}
#endif
