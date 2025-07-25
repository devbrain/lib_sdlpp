//
// Created by igor on 7/14/25.
//

#pragma once

/**
 * @file renderer.hh
 * @brief Modern C++ wrapper for SDL3 renderer functionality
 * 
 * This header provides RAII-managed wrappers around SDL3's rendering system,
 * which provides hardware-accelerated 2D rendering capabilities.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/utility/geometry.hh>
#include <sdlpp/video/color.hh>
#include <sdlpp/video/blend_mode.hh>
#include <sdlpp/video/window.hh>
#include <string>
#include <vector>
#include <span>
#include <optional>
#include <array>
#include <limits>

// Forward declarations
namespace sdlpp {
    class texture;
    class surface;
}

namespace sdlpp {
    /**
     * @brief Smart pointer type for SDL_Renderer with automatic cleanup
     */
    using renderer_ptr = pointer <SDL_Renderer, SDL_DestroyRenderer>;

    // SDL conversion helpers for renderer-specific types
    namespace detail {
        template<point_like P>
        [[nodiscard]] SDL_FPoint to_sdl_fpoint(const P& p) {
            return SDL_FPoint{
                static_cast<float>(get_x(p)),
                static_cast<float>(get_y(p))
            };
        }

        template<rect_like R>
        [[nodiscard]] SDL_FRect to_sdl_frect(const R& r) {
            return SDL_FRect{
                static_cast<float>(get_x(r)),
                static_cast<float>(get_y(r)),
                static_cast<float>(get_width(r)),
                static_cast<float>(get_height(r))
            };
        }
    }

    /**
     * @brief Renderer driver names
     */
    namespace renderer_driver {
        constexpr const char* software = "software";
        constexpr const char* opengl = "opengl";
        constexpr const char* opengles2 = "opengles2";
        constexpr const char* metal = "metal";
        constexpr const char* vulkan = "vulkan";
        constexpr const char* direct3d11 = "direct3d11";
        constexpr const char* direct3d12 = "direct3d12";
    }

    /**
     * @brief Texture access patterns
     */
    enum class texture_access : int {
        static_access = SDL_TEXTUREACCESS_STATIC, ///< Changes rarely, not lockable
        streaming = SDL_TEXTUREACCESS_STREAMING, ///< Changes frequently, lockable
        target = SDL_TEXTUREACCESS_TARGET ///< Can be used as render target
    };

    /**
     * @brief RAII wrapper for SDL_Renderer
     *
     * This class provides a safe, RAII-managed interface to SDL's hardware-accelerated
     * 2D rendering functionality. The renderer is automatically destroyed when the
     * object goes out of scope.
     */
    class renderer {
        private:
            renderer_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates an empty renderer
             */
            renderer() = default;

            /**
             * @brief Construct from existing SDL_Renderer pointer
             * @param r Existing SDL_Renderer pointer (takes ownership)
             */
            explicit renderer(SDL_Renderer* r)
                : ptr(r) {
            }

            /**
             * @brief Move constructor
             */
            renderer(renderer&&) = default;

            /**
             * @brief Move assignment operator
             */
            renderer& operator=(renderer&&) = default;

            // Delete copy operations - renderers are move-only
            renderer(const renderer&) = delete;
            renderer& operator=(const renderer&) = delete;

            /**
             * @brief Check if the renderer is valid
             */
            [[nodiscard]] bool is_valid() const { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Renderer pointer
             */
            [[nodiscard]] SDL_Renderer* get() const { return ptr.get(); }

            /**
             * @brief Clear the entire rendering target with draw color
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> clear() {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_RenderClear(ptr.get());

                return {};
            }

            /**
             * @brief Present the backbuffer to the screen
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> present() {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_RenderPresent(ptr.get());

                return {};
            }

            /**
             * @brief Set the draw color
             * @param c Color to use for drawing operations
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_draw_color(const color& c) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_SetRenderDrawColor(ptr.get(), c.r, c.g, c.b, c.a);

                return {};
            }

            /**
             * @brief Get the current draw color
             * @return Expected containing color, or error message
             */
            expected <color, std::string> get_draw_color() const {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                uint8_t r, g, b, a;
                if (!SDL_GetRenderDrawColor(ptr.get(), &r, &g, &b, &a)) {
                    return make_unexpected(get_error());
                }

                return color{r, g, b, a};
            }

            /**
             * @brief Set the blend mode for drawing operations
             * @param mode Blend mode to use
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_draw_blend_mode(blend_mode mode) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (!SDL_SetRenderDrawBlendMode(ptr.get(), static_cast <SDL_BlendMode>(mode))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get the blend mode for drawing operations
             * @return Expected containing blend mode, or error message
             */
            expected <blend_mode, std::string> get_draw_blend_mode() const {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_BlendMode mode;
                if (!SDL_GetRenderDrawBlendMode(ptr.get(), &mode)) {
                    return make_unexpected(get_error());
                }

                return static_cast <blend_mode>(mode);
            }

            /**
             * @brief Draw a point
             * @param x X coordinate
             * @param y Y coordinate
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> draw_point(int x, int y) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (!SDL_RenderPoint(ptr.get(), static_cast <float>(x), static_cast <float>(y))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Draw a point
             * @param p Point to draw
             * @return Expected<void> - empty on success, error message on failure
             */
            template<point_like P>
            expected <void, std::string> draw_point(const P& p) {
                return draw_point(static_cast <float>(p.x), static_cast <float>(p.y));
            }

            /**
             * @brief Draw a point with floating-point precision
             * @param x X coordinate
             * @param y Y coordinate
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> draw_point(float x, float y) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (!SDL_RenderPoint(ptr.get(), x, y)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Draw a point with floating-point type
             * @tparam P Point type (must satisfy point_like)
             * @param p Point to draw
             * @return Expected<void> - empty on success, error message on failure
             */
            template<point_like P>
            requires std::is_floating_point_v<typename P::value_type>
            expected <void, std::string> draw_point(const P& p) {
                return draw_point(static_cast<float>(get_x(p)), static_cast<float>(get_y(p)));
            }

            /**
             * @brief Draw multiple points
             * @tparam Container Container type holding point_like elements
             * @param points Container of points to draw
             * @return Expected<void> - empty on success, error message on failure
             */
            template<typename Container>
            requires requires(Container c) {
                typename Container::value_type;
                requires point_like<typename Container::value_type>;
                { std::begin(c) };
                { std::end(c) };
            }
            expected <void, std::string> draw_points(const Container& points) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (std::begin(points) == std::end(points)) {
                    return {};
                }

                // Convert to SDL_FPoint
                std::vector <SDL_FPoint> sdl_points;
                sdl_points.reserve(static_cast<size_t>(std::distance(std::begin(points), std::end(points))));
                for (const auto& p : points) {
                    sdl_points.push_back({static_cast <float>(get_x(p)), static_cast <float>(get_y(p))});
                }

                if (sdl_points.size() > static_cast <size_t>(std::numeric_limits <int>::max())) {
                    return make_unexpected("Too many points for SDL API");
                }

                if (!SDL_RenderPoints(ptr.get(), sdl_points.data(),
                                      static_cast <int>(sdl_points.size()))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Draw a line
             * @param x1 Start X coordinate
             * @param y1 Start Y coordinate
             * @param x2 End X coordinate
             * @param y2 End Y coordinate
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> draw_line(int x1, int y1, int x2, int y2) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (!SDL_RenderLine(ptr.get(),
                                    static_cast <float>(x1), static_cast <float>(y1),
                                    static_cast <float>(x2), static_cast <float>(y2))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Draw a line
             * @param start Start point
             * @param end End point
             * @return Expected<void> - empty on success, error message on failure
             */
            template<point_like P1, point_like P2>
            expected <void, std::string> draw_line(const P1& start, const P2& end) {
                return draw_line(static_cast <float>(start.x), static_cast <float>(start.y),
                                 static_cast <float>(end.x), static_cast <float>(end.y));
            }

            /**
             * @brief Draw a line with floating-point precision
             * @param x1 Start X coordinate
             * @param y1 Start Y coordinate
             * @param x2 End X coordinate
             * @param y2 End Y coordinate
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> draw_line(float x1, float y1, float x2, float y2) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (!SDL_RenderLine(ptr.get(), x1, y1, x2, y2)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Draw connected lines
             * @tparam Container Container type holding point_like elements
             * @param points Container of points defining the lines
             * @return Expected<void> - empty on success, error message on failure
             */
            template<typename Container>
            requires requires(Container c) {
                typename Container::value_type;
                requires point_like<typename Container::value_type>;
                { std::begin(c) };
                { std::end(c) };
            }
            expected <void, std::string> draw_lines(const Container& points) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                auto count = std::distance(std::begin(points), std::end(points));
                if (count < 2) {
                    return {};
                }

                // Convert to SDL_FPoint
                std::vector <SDL_FPoint> sdl_points;
                sdl_points.reserve(static_cast<size_t>(count));
                for (const auto& p : points) {
                    sdl_points.push_back({static_cast <float>(get_x(p)), static_cast <float>(get_y(p))});
                }

                if (sdl_points.size() > static_cast <size_t>(std::numeric_limits <int>::max())) {
                    return make_unexpected("Too many points for SDL API");
                }

                if (!SDL_RenderLines(ptr.get(), sdl_points.data(),
                                     static_cast <int>(sdl_points.size()))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Draw a rectangle outline
             * @param r Rectangle to draw
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R>
            expected <void, std::string> draw_rect(const R& r) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_FRect sdl_rect{
                    static_cast <float>(get_x(r)), static_cast <float>(get_y(r)),
                    static_cast <float>(get_width(r)), static_cast <float>(get_height(r))
                };

                if (!SDL_RenderRect(ptr.get(), &sdl_rect)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Draw a rectangle outline with floating-point type
             * @tparam R Rectangle type (must satisfy rect_like)
             * @param r Rectangle to draw
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R>
            requires std::is_floating_point_v<typename R::value_type>
            expected <void, std::string> draw_rect(const R& r) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_FRect sdl_rect{
                    static_cast <float>(get_x(r)), static_cast <float>(get_y(r)),
                    static_cast <float>(get_width(r)), static_cast <float>(get_height(r))
                };

                if (!SDL_RenderRect(ptr.get(), &sdl_rect)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Draw multiple rectangle outlines
             * @tparam Container Container type holding rect_like elements
             * @param rects Container of rectangles to draw
             * @return Expected<void> - empty on success, error message on failure
             */
            template<typename Container>
            requires requires(Container c) {
                typename Container::value_type;
                requires rect_like<typename Container::value_type>;
                { std::begin(c) };
                { std::end(c) };
            }
            expected <void, std::string> draw_rects(const Container& rects) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (std::begin(rects) == std::end(rects)) {
                    return {};
                }

                // Convert to SDL_FRect
                std::vector <SDL_FRect> sdl_rects;
                sdl_rects.reserve(static_cast<size_t>(std::distance(std::begin(rects), std::end(rects))));
                for (const auto& r : rects) {
                    sdl_rects.push_back({
                        static_cast <float>(r.x), static_cast <float>(r.y),
                        static_cast <float>(r.w), static_cast <float>(r.h)
                    });
                }

                if (sdl_rects.size() > static_cast <size_t>(std::numeric_limits <int>::max())) {
                    return make_unexpected("Too many rectangles for SDL API");
                }

                if (!SDL_RenderRects(ptr.get(), sdl_rects.data(),
                                     static_cast <int>(sdl_rects.size()))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Fill a rectangle
             * @param r Rectangle to fill
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R>
            expected <void, std::string> fill_rect(const R& r) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_FRect sdl_rect{
                    static_cast <float>(get_x(r)), static_cast <float>(get_y(r)),
                    static_cast <float>(get_width(r)), static_cast <float>(get_height(r))
                };

                if (!SDL_RenderFillRect(ptr.get(), &sdl_rect)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Fill a rectangle with floating-point type
             * @tparam R Rectangle type (must satisfy rect_like)
             * @param r Rectangle to fill
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R>
            requires std::is_floating_point_v<typename R::value_type>
            expected <void, std::string> fill_rect(const R& r) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_FRect sdl_rect{
                    static_cast <float>(get_x(r)), static_cast <float>(get_y(r)),
                    static_cast <float>(get_width(r)), static_cast <float>(get_height(r))
                };

                if (!SDL_RenderFillRect(ptr.get(), &sdl_rect)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Fill multiple rectangles
             * @tparam Container Container type holding rect_like elements
             * @param rects Container of rectangles to fill
             * @return Expected<void> - empty on success, error message on failure
             */
            template<typename Container>
            requires requires(Container c) {
                typename Container::value_type;
                requires rect_like<typename Container::value_type>;
                { std::begin(c) };
                { std::end(c) };
            }
            expected <void, std::string> fill_rects(const Container& rects) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (std::begin(rects) == std::end(rects)) {
                    return {};
                }

                // Convert to SDL_FRect
                std::vector <SDL_FRect> sdl_rects;
                sdl_rects.reserve(static_cast<size_t>(std::distance(std::begin(rects), std::end(rects))));
                for (const auto& r : rects) {
                    sdl_rects.push_back({
                        static_cast <float>(r.x), static_cast <float>(r.y),
                        static_cast <float>(r.w), static_cast <float>(r.h)
                    });
                }

                if (sdl_rects.size() > static_cast <size_t>(std::numeric_limits <int>::max())) {
                    return make_unexpected("Too many rectangles for SDL API");
                }

                if (!SDL_RenderFillRects(ptr.get(), sdl_rects.data(),
                                         static_cast <int>(sdl_rects.size()))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Set viewport (clipping rectangle)
             * @tparam R Rectangle type (must satisfy rect_like)
             * @param viewport Optional rectangle defining the viewport (nullopt for entire target)
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R = void>
            expected <void, std::string> set_viewport(const std::optional <R>& viewport) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (viewport) {
                    SDL_Rect sdl_rect{
                        static_cast<int>(get_x(*viewport)),
                        static_cast<int>(get_y(*viewport)),
                        static_cast<int>(get_width(*viewport)),
                        static_cast<int>(get_height(*viewport))
                    };
                    if (!SDL_SetRenderViewport(ptr.get(), &sdl_rect)) {
                        return make_unexpected(get_error());
                    }
                } else {
                    if (!SDL_SetRenderViewport(ptr.get(), nullptr)) {
                        return make_unexpected(get_error());
                    }
                }

                return {};
            }

            /**
             * @brief Get current viewport
             * @tparam R Rectangle type to return (defaults to built-in rect if available)
             * @return Expected containing viewport rectangle, or error message
             */
            template<rect_like R = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                rect_i
#else
                void
#endif
            >
            expected <R, std::string> get_viewport() const
                requires (!std::is_void_v<R>) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_Rect viewport;
                if (!SDL_GetRenderViewport(ptr.get(), &viewport)) {
                    return make_unexpected(get_error());
                }

                return R{viewport.x, viewport.y, viewport.w, viewport.h};
            }

            /**
             * @brief Set clipping rectangle
             * @tparam R Rectangle type (must satisfy rect_like)
             * @param clip Optional rectangle defining the clipping area (nullopt to disable)
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R = void>
            expected <void, std::string> set_clip_rect(const std::optional <R>& clip) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (clip) {
                    SDL_Rect sdl_rect{
                        static_cast<int>(get_x(*clip)),
                        static_cast<int>(get_y(*clip)),
                        static_cast<int>(get_width(*clip)),
                        static_cast<int>(get_height(*clip))
                    };
                    if (!SDL_SetRenderClipRect(ptr.get(), &sdl_rect)) {
                        return make_unexpected(get_error());
                    }
                } else {
                    if (!SDL_SetRenderClipRect(ptr.get(), nullptr)) {
                        return make_unexpected(get_error());
                    }
                }

                return {};
            }

            /**
             * @brief Get current clipping rectangle
             * @tparam R Rectangle type to return (defaults to built-in rect if available)
             * @return Expected containing clipping rectangle (nullopt if disabled), or error message
             */
            template<rect_like R = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                rect_i
#else
                void
#endif
            >
            expected <std::optional <R>, std::string> get_clip_rect() const
                requires (!std::is_void_v<R>) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                SDL_Rect clip;
                if (!SDL_GetRenderClipRect(ptr.get(), &clip)) {
                    return make_unexpected(get_error());
                }

                // Check if clipping is enabled (SDL returns non-zero rect)
                if (clip.w == 0 || clip.h == 0) {
                    return std::nullopt;
                }

                return R{clip.x, clip.y, clip.w, clip.h};
            }

            /**
             * @brief Check if clipping is enabled
             * @return true if clipping is enabled
             */
            [[nodiscard]] bool is_clip_enabled() const {
                return ptr && SDL_RenderClipEnabled(ptr.get());
            }

            /**
             * @brief Set render scale
             * @param scale_x X axis scale factor
             * @param scale_y Y axis scale factor
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_scale(float scale_x, float scale_y) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (!SDL_SetRenderScale(ptr.get(), scale_x, scale_y)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get render scale
             * @tparam P Point type to return (defaults to built-in fpoint if available)
             * @return Expected containing scale factors as point, or error message
             */
            template<point_like P = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                point_f
#else
                void
#endif
            >
            expected <P, std::string> get_scale() const
                requires (!std::is_void_v<P>) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                float scale_x, scale_y;
                if (!SDL_GetRenderScale(ptr.get(), &scale_x, &scale_y)) {
                    return make_unexpected(get_error());
                }

                return P{static_cast<typename P::value_type>(scale_x), static_cast<typename P::value_type>(scale_y)};
            }

            /**
             * @brief Get output size of the renderer
             * @tparam S Size type to return (defaults to built-in size if available)
             * @return Expected containing size, or error message
             */
            template<size_like S = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                size_i
#else
                void
#endif
            >
            expected <S, std::string> get_output_size() const
                requires (!std::is_void_v<S>) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                int w, h;
                if (!SDL_GetRenderOutputSize(ptr.get(), &w, &h)) {
                    return make_unexpected(get_error());
                }

                return S{w, h};
            }

            /**
             * @brief Get current render target size
             * @tparam S Size type to return (defaults to built-in size if available)
             * @return Expected containing size, or error message
             */
            template<size_like S = 
#ifdef SDLPP_HAS_BUILTIN_GEOMETRY
                size_i
#else
                void
#endif
            >
            expected <S, std::string> get_current_output_size() const
                requires (!std::is_void_v<S>) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                int w, h;
                if (!SDL_GetCurrentRenderOutputSize(ptr.get(), &w, &h)) {
                    return make_unexpected(get_error());
                }

                return S{w, h};
            }

            /**
             * @brief Set VSync mode
             * @param vsync 0 to disable, 1 to enable, -1 for adaptive vsync
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_vsync(int vsync) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (!SDL_SetRenderVSync(ptr.get(), vsync)) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Get VSync mode
             * @return Expected containing vsync mode, or error message
             */
            expected <int, std::string> get_vsync() const {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                int vsync;
                if (!SDL_GetRenderVSync(ptr.get(), &vsync)) {
                    return make_unexpected(get_error());
                }

                return vsync;
            }

            /**
             * @brief Flush any pending rendering commands
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> flush() {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (!SDL_FlushRenderer(ptr.get())) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            // Texture-related methods (defined in texture.hh after texture class)

            /**
             * @brief Copy texture to rendering target
             * @tparam R Rectangle type (must satisfy rect_like)
             * @param texture Texture to copy
             * @param src_rect Source rectangle (nullopt for entire texture)
             * @param dst_rect Destination rectangle (nullopt for entire target)
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R = void>
            expected <void, std::string> copy(
                const texture& texture,
                const std::optional <R>& src_rect = std::nullopt,
                const std::optional <R>& dst_rect = std::nullopt);

            /**
             * @brief Copy texture to rendering target with floating-point rectangles
             * @tparam R Rectangle type (must satisfy rect_like with floating-point value_type)
             * @param texture Texture to copy
             * @param src_rect Source rectangle (nullopt for entire texture)
             * @param dst_rect Destination rectangle (nullopt for entire target)
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R>
            requires std::is_floating_point_v<typename R::value_type>
            expected <void, std::string> copy(
                const texture& texture,
                const std::optional <R>& src_rect,
                const std::optional <R>& dst_rect);

            /**
             * @brief Copy texture with rotation and flipping (integer rectangles)
             * @param texture Texture to copy
             * @param src_rect Source rectangle (nullopt for entire texture)
             * @param dst_rect Destination rectangle (nullopt for entire target)
             * @param angle Rotation angle in degrees
             * @param center Rotation center (nullopt for dst_rect center)
             * @param flip Flip mode
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R = void, point_like P = void>
            expected <void, std::string> copy_ex(
                const texture& texture,
                const std::optional <R>& src_rect,
                const std::optional <R>& dst_rect,
                double angle,
                const std::optional <P>& center,
                flip_mode flip = flip_mode::none);

            /**
             * @brief Copy texture with rotation and flipping (floating-point rectangles)
             * @param texture Texture to copy
             * @param src_rect Source rectangle (nullopt for entire texture)
             * @param dst_rect Destination rectangle (nullopt for entire target)
             * @param angle Rotation angle in degrees
             * @param center Rotation center (nullopt for dst_rect center)
             * @param flip Flip mode
             * @return Expected<void> - empty on success, error message on failure
             */
            template<rect_like R, point_like P>
            requires (std::is_floating_point_v<typename R::value_type> && 
                     std::is_floating_point_v<typename P::value_type>)
            expected <void, std::string> copy_ex(
                const texture& texture,
                const std::optional <R>& src_rect,
                const std::optional <R>& dst_rect,
                double angle,
                const std::optional <P>& center,
                flip_mode flip = flip_mode::none);

            /**
             * @brief Get current render target
             * @return Expected containing target texture (empty texture for default), or error
             */
            [[nodiscard]] expected <texture, std::string> get_target() const;

            /**
             * @brief Set render target
             * @param target Target texture (empty texture for default target)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> set_target(const texture& target);

            // Geometry rendering methods

            /**
             * @brief Render textured triangles (SDL_RenderGeometry)
             * @param texture Texture to use (nullptr for solid color)
             * @param vertices Vertex data (positions, colors, texture coords)
             * @param indices Index data for triangles (3 indices per triangle)
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> render_geometry(
                SDL_Texture* texture,
                std::span <const SDL_Vertex> vertices,
                std::span <const int> indices) {
                if (!ptr) {
                    return make_unexpected("Invalid renderer");
                }

                if (vertices.empty() || indices.empty()) {
                    return {}; // Nothing to render
                }

                if (indices.size() % 3 != 0) {
                    return make_unexpected("Index count must be multiple of 3 for triangles");
                }

                // Check for size limits
                if (vertices.size() > static_cast <size_t>(std::numeric_limits <int>::max()) ||
                    indices.size() > static_cast <size_t>(std::numeric_limits <int>::max())) {
                    return make_unexpected("Too many vertices or indices for SDL API");
                }

                // Validate indices are within bounds
                for (const auto& idx : indices) {
                    if (idx < 0 || idx >= static_cast <int>(vertices.size())) {
                        return make_unexpected("Index out of bounds");
                    }
                }

                if (!SDL_RenderGeometry(ptr.get(), texture,
                                        vertices.data(), static_cast <int>(vertices.size()),
                                        indices.data(), static_cast <int>(indices.size()))) {
                    return make_unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Render colored triangles without texture
             * @param vertices Vertex data (positions and colors)
             * @param indices Index data for triangles
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> render_geometry(
                std::span <const SDL_Vertex> vertices,
                std::span <const int> indices) {
                return render_geometry(nullptr, vertices, indices);
            }

            /**
             * @brief Render a single textured triangle
             * @param texture Texture to use (nullptr for solid color)
             * @param v0 First vertex
             * @param v1 Second vertex
             * @param v2 Third vertex
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> render_triangle(
                SDL_Texture* texture,
                const SDL_Vertex& v0,
                const SDL_Vertex& v1,
                const SDL_Vertex& v2) {
                std::array <SDL_Vertex, 3> vertices = {v0, v1, v2};
                std::array <int, 3> indices = {0, 1, 2};

                return render_geometry(texture, vertices, indices);
            }

            /**
             * @brief Render a single colored triangle
             * @param v0 First vertex
             * @param v1 Second vertex
             * @param v2 Third vertex
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> render_triangle(
                const SDL_Vertex& v0,
                const SDL_Vertex& v1,
                const SDL_Vertex& v2) {
                return render_triangle(nullptr, v0, v1, v2);
            }

            /**
             * @brief Helper to create vertex from point and color
             * @param p Position
             * @param c Color
             * @param tex_coord Texture coordinate (default 0,0)
             * @return SDL_Vertex structure
             */
            template<point_like P1, point_like P2 = P1>
            static constexpr SDL_Vertex make_vertex(const P1& p, const color& c,
                                                    const P2& tex_coord = P2{0, 0}) {
                return SDL_Vertex{
                    {static_cast<float>(get_x(p)), static_cast<float>(get_y(p))},
                    {
                        static_cast <float>(c.r) / 255.0f,
                        static_cast <float>(c.g) / 255.0f,
                        static_cast <float>(c.b) / 255.0f,
                        static_cast <float>(c.a) / 255.0f
                    },
                    {static_cast<float>(get_x(tex_coord)), static_cast<float>(get_y(tex_coord))}
                };
            }

            /**
             * @brief Render a triangle from geometry types
             * @tparam T Triangle type (must satisfy triangle_like)
             * @param tri Triangle to render
             * @param c Color for all vertices
             * @return Expected<void> - empty on success, error message on failure
             */
            template<triangle_like T>
            expected <void, std::string> render_triangle(
                const T& tri,
                const color& c) {
                auto v0 = make_vertex(tri.a, c);
                auto v1 = make_vertex(tri.b, c);
                auto v2 = make_vertex(tri.c, c);

                return render_triangle(v0, v1, v2);
            }

            /**
             * @brief Render a textured triangle from geometry types
             * @tparam T1 Triangle type for vertices (must satisfy triangle_like)
             * @tparam T2 Triangle type for texture coordinates (must satisfy triangle_like)
             * @param texture Texture to use
             * @param tri Triangle to render
             * @param c Color modulation for all vertices
             * @param tex_tri Texture coordinates for each vertex
             * @return Expected<void> - empty on success, error message on failure
             */
            template<triangle_like T1, triangle_like T2>
            expected <void, std::string> render_textured_triangle(
                SDL_Texture* texture,
                const T1& tri,
                const color& c,
                const T2& tex_tri) {
                auto v0 = make_vertex(tri.a, c, tex_tri.a);
                auto v1 = make_vertex(tri.b, c, tex_tri.b);
                auto v2 = make_vertex(tri.c, c, tex_tri.c);

                return render_triangle(texture, v0, v1, v2);
            }

            // Static factory methods

            /**
             * @brief Create a renderer for a window
             * @param window Window to create renderer for
             * @param driver_name Name of rendering driver (nullptr for best available)
             * @return Expected containing new renderer, or error message
             */
            static expected <renderer, std::string> create(const window& window,
                                                           const char* driver_name = nullptr) {
                if (!window) {
                    return make_unexpected("Invalid window");
                }

                SDL_Renderer* r = SDL_CreateRenderer(window.get(), driver_name);
                if (!r) {
                    return make_unexpected(get_error());
                }

                return renderer(r);
            }

            /**
             * @brief Create a renderer for a window (SDL_Window overload for compatibility)
             * @param window SDL_Window pointer
             * @param driver_name Name of rendering driver (nullptr for best available)
             * @return Expected containing new renderer, or error message
             */
            static expected <renderer, std::string> create(SDL_Window* window,
                                                           const char* driver_name = nullptr) {
                if (!window) {
                    return make_unexpected("Invalid window");
                }

                SDL_Renderer* r = SDL_CreateRenderer(window, driver_name);
                if (!r) {
                    return make_unexpected(get_error());
                }

                return renderer(r);
            }

            /**
             * @brief Create a software renderer for a surface
             * @param surface Surface to render to
             * @return Expected containing new renderer, or error message
             */
            static expected <renderer, std::string> create_software(SDL_Surface* surface) {
                if (!surface) {
                    return make_unexpected("Invalid surface");
                }

                SDL_Renderer* r = SDL_CreateSoftwareRenderer(surface);
                if (!r) {
                    return make_unexpected(get_error());
                }

                return renderer(r);
            }
    };
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for texture_access
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, texture_access value);

/**
 * @brief Stream input operator for texture_access
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, texture_access& value);

}