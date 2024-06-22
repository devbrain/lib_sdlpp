//
// Created by igor on 08/06/2020.
//

#ifndef NEUTRINO_SDL_RENDER_HH
#define NEUTRINO_SDL_RENDER_HH

#include <string>
#include <array>
#include <type_traits>

#include <sdlpp/video/surface.hh>
#include <sdlpp/video/texture.hh>
#include <sdlpp/detail/ostreamops.hh>
#include <bsw/array_view.hh>

namespace neutrino::sdl {
	/**
	 * @class renderer
	 *
	 * @brief The renderer class provides an interface to render graphics using SDL.
	 *
	 * The renderer class is used to render graphics to a window or a surface.
	 * It provides various functions to draw lines, rectangles, circles, polygons, etc.
	 * The rendering can be done with different options like blending, clipping, scaling, etc.
	 *
	 * Usage Example:
	 *
	 * @code{.cpp}
	 * // Creating a renderer for a window
	 * object<SDL_Window> window = ...;
	 * renderer rend(window, renderer::flags::ACCELERATED | renderer::flags::PRESENTVSYNC);
	 *
	 * // Setting the active color
	 * rend.active_color(color(255, 0, 0)); // Set the active color to red
	 *
	 * // Drawing a rectangle
	 * rect rec(100, 100, 200, 150);
	 * rend.draw_rectangle(rec);
	 *
	 * // Presenting the rendered graphics
	 * rend.present();
	 * @endcode
	 */
	class SDLPP_EXPORT renderer : public object<SDL_Renderer> {
	 public:
		enum class flags : uint32_t {
			NONE = 0,
			SOFTWARE = SDL_RENDERER_SOFTWARE,
			ACCELERATED = SDL_RENDERER_ACCELERATED,
			PRESENTVSYNC = SDL_RENDERER_PRESENTVSYNC,
			TARGETTEXTURE = SDL_RENDERER_TARGETTEXTURE
		};

		enum class flip : uint32_t {
			NONE = SDL_FLIP_NONE,
			HORIZONTAL = SDL_FLIP_HORIZONTAL,
			VERTICAL = SDL_FLIP_VERTICAL
		};

	 public:
		renderer () = default;

		template <typename ... Args,
			typename std::enable_if<(std::is_same_v<flags, Args> && ...), int>::type = 0>
		explicit renderer (const object<SDL_Window>& w, Args...flags);
		explicit renderer (const object<SDL_Surface>& s);

		explicit renderer (object<SDL_Renderer>&& other);
		renderer& operator= (object<SDL_Renderer>&& other) noexcept;

		/**
		 * @brief Returns the get_blend mode used by the renderer.
		 *
		 * This function returns the current get_blend mode used by the renderer.
		 * The get_blend mode determines how colors are blended when drawing on the renderer.
		 *
		 * @return The get_blend mode used by the renderer.
		 */
		[[nodiscard]] blend_mode get_blend_mode () const;
		/**
		 * @brief Sets the get_blend mode used by the renderer.
		 *
		 * This function sets the get_blend mode used by the renderer.
		 * The get_blend mode determines how colors are blended when drawing on the renderer.
		 *
		 * @param bm The get_blend mode to be set.
		 */
		void set_blend_mode (blend_mode bm);

		/**
		 * @brief Returns the active color used by the renderer.
		 *
		 * This function returns the current active color used by the renderer.
		 * The active color is the color that will be used to draw shapes and lines.
		 *
		 * @return The active color used by the renderer.
		 */
		[[nodiscard]] color get_active_color () const;
		/**
		 * @brief Sets the active color for rendering.
		 *
		 * This function sets the active color for rendering using the SDL_SetRenderDrawColor function.
		 * The color is represented by a color structure.
		 *
		 * @param c The color to set as the active color.
		 * @see color
		 */
		void set_active_color (const color& c);

		/**
		 * @brief Get the clipping rectangle.
		 *
		 * This function returns the current clipping rectangle used by the renderer.
		 * The clipping rectangle is the area of the destination where rendering can occur.
		 * Anything outside this rectangle will be discarded.
		 *
		 * @return The clipping rectangle as an instance of the rect class.
		 */
		[[nodiscard]] rect get_clip () const;
		/**
		 * @brief Clips the rendering area to the given rectangle.
		 *
		 * This function restricts the rendering area to be within the boundaries of the specified rectangle.
		 * Only the pixels inside the rectangle will be visible when rendering.
		 *
		 * @param area The rectangle to clip the rendering area to.
		 */
		void set_clip (const rect& area);

		/**
		 * @brief Disables clipping for the renderer.
		 *
		 * This function disables the clipping functionality for the renderer. Clipping determines the area of the screen
		 * where the rendering is visible. By calling this function, the renderer will render to the entire screen.
		 *
		 * Usage:
		 * @code
		 * renderer.disable_clippping();
		 * @endcode
		 *
		 * @note This function must be called after creating the renderer and before rendering any objects.
		 */
		void disable_clippping ();

		/**
		 * @brief Check if clipping is enabled for the renderer.
		 *
		 * This function checks if clipping is enabled for the renderer.
		 *
		 * @return True if clipping is enabled, false otherwise.
		 */
		[[nodiscard]] bool is_clipping_enabled () const;

		/**
		 * @brief Check if integer scaling is enabled.
		 *
		 * This function checks if integer scaling is enabled in the renderer.
		 *
		 * @return true if integer scaling is enabled, false otherwise.
		 */
		[[nodiscard]] bool has_integer_scaling () const;
		/**
		* @brief Enable or disable integer scaling for the renderer.
		*
		* Integer scaling allows the renderer to scale the output by whole numbers only,
		* which can help maintain a pixelated look in low resolution graphics.
		*
		* @param enabled true to enable integer scaling, false to disable it
		* @sa renderer::handle()
		* @sa SDL_RenderSetIntegerScale()
		*/
		void set_integer_scaling (bool enabled);

		/**
		 * @brief Retrieves the logical size in pixels.
		 *
		 * This function returns the logical size of the renderer in pixels.
		 * The logical size is the output resolution at which rendering is performed.
		 * The logical size is defined using the SDL_RenderSetLogicalSize function.
		 *
		 * @return A std::pair<w, h> representing the width and height of the logical size in pixels.
		 */
		[[nodiscard]] std::pair<unsigned, unsigned> get_logical_size () const;
		/**
		 * @brief Set the logical size of the renderer.
		 *
		 * This function sets the logical size of the renderer to the specified width and height
		 * in pixels. The logical size is used for rendering and determines how the renderer will
		 * transform the coordinates of objects being rendered. It does not affect the actual window size.
		 *
		 * @param x The width of the logical size in pixels.
		 * @param y The height of the logical size in pixels.
		 *
		 * @see renderer::get_logical_size()
		 */
		void set_logical_size (unsigned x, unsigned y);

		/**
		 * @brief Get the current scaling factors of the renderer.
		 *
		 * @return std::pair<float, float> The current scaling factors of the renderer.
		 *
		 * This function returns the current scaling factors applied to the renderer in the form of a pair of floats.
		 * The first float represents the scaling factor along the x-axis and the second float represents the scaling
		 * factor along the y-axis.
		 *
		 * Example usage:
		 * \code{.cpp}
		 * auto s = renderer.scaling();
		 * std::cout << "X scaling factor: " << s.first << std::endl;
		 * std::cout << "Y scaling factor: " << s.second << std::endl;
		 * \endcode
		 *
		 * Note: This function returns a pair of floats, so the caller should ensure to assign it to a std::pair<float, float> variable.
		 */
		[[nodiscard]] std::pair<float, float> get_scaling () const;
		/**
		 * @brief Scales the renderer.
		 *
		 * This function is used to scale the renderer by the specified factors in the x and y directions.
		 *
		 * @param x The scaling factor in the x direction.
		 * @param y The scaling factor in the y direction.
		 *
		 * @code
		 * renderer.scaling(2.0, 3.0); // Scales the renderer by a factor of 2.0 in the x direction and 3.0 in the y direction.
		 * renderer.scaling(0.5, 1.0); // Scales the renderer by a factor of 0.5 in the x direction and 1.0 in the y direction.
		 * @endcode
		 *
		 * @attention The scaling factors should be positive values. A scaling factor of 1.0 means no scaling, while a scaling factor of 2.0 doubles the size of the renderer in the given direction.
		 * @attention The scaling operation affects all subsequent rendering operations performed by the renderer until the scaling is changed or disabled.
		 */
		void set_scaling (float x, float y);

		/**
		 * \brief Get the viewport of the renderer.
		 *
		 * This function returns the current viewport of the renderer.
		 *
		 * \return The viewport as a rect object.
		 */
		[[nodiscard]] rect get_viewport () const;
		/**
		 * @brief Sets the viewport of the renderer to the specified area.
		 *
		 * This function sets the viewport of the renderer to the specified area,
		 * controlling the portion of the window where rendering will occur.
		 *
		 * @param area The area representing the viewport. The top left corner will be
		 *             (area.x, area.y) and the width and height will be area.w and
		 *             area.h, respectively.
		 */
		void set_viewport (const rect& area);
		/**
		 * \brief Disable the current viewport of the renderer.
		 *
		 * This function disables the current viewport of the renderer. By disabling the viewport,
		 * the renderer will render to the entire window without any restrictions.
		 *
		 * \warning This function should be called only when there is an active renderer.
		 *
		 * \see renderer::set_viewport()
		 */
		void disable_viewport ();

		/**
		 * @brief Reads the pixels from the renderer and stores them in the destination buffer.
		 *
		 * This function reads the pixels from the renderer and stores them in the destination buffer provided.
		 * The pixels are converted to the specified pixel format before storing in the destination buffer.
		 *
		 * @param fmt The pixel format to which the pixels should be converted.
		 * @param dst A pointer to the destination buffer where the pixels will be stored.
		 * @param pitch The pitch or width of a row of pixels in the destination buffer.
		 *
		 * @note The destination buffer must have enough memory to store the converted pixels based on the provided pitch.
		 *       Also, the pitch should be a positive value.
		 *
		 * @see pixel_format
		 * @see SDL_RenderReadPixels
		 */
		void read_pixels (const pixel_format& fmt, void* dst, std::size_t pitch) const;
		/**
		 * @brief This function reads pixels from a specified area in the renderer's target.
		 *
		 * @param area The area to read pixels from.
		 * @param fmt The pixel format of the destination pixels.
		 * @param dst A pointer to the destination buffer.
		 * @param pitch The number of bytes per row in the destination buffer.
		 *
		 * @note The destination buffer should have enough memory to hold the pixels of the specified area.
		 *
		 * @throws SDLException if there is an error reading the pixels.
		 */
		void read_pixels (const rect& area, const pixel_format& fmt, void* dst, std::size_t pitch) const;
		/**
		 * @brief Get the pixel format of the renderer.
		 *
		 * This function returns the pixel format of the renderer.
		 *
		 * @return The pixel format of the renderer.
		 */
		[[nodiscard]] pixel_format get_pixel_format () const;

		/**
		 * @brief Get the current render target texture.
		 *
		 * @details This function returns the current render target texture of the renderer object.
		 *
		 * @returns An optional object of type texture. If a render target is set, it returns the target texture,
		 *          otherwise it returns std::nullopt.
		 */
		[[nodiscard]] std::optional<texture> get_target () const;
		/**
		 * @brief Set the render target to the specified texture.
		 *
		 * This function sets the render target to the provided texture,
		 * allowing any subsequent rendering operations to be directed to
		 * that texture instead of the default render target.
		 *
		 * @param t The texture to set as the render target.
		 *
		 */
		void set_target (texture& t);
		/**
		 * @brief Restores the default render target.
		 *
		 * This function sets the render target to the default render target, which is the window's surface.
		 *
		 * @note This function can be used to switch the render target back to the window's surface after setting it to a different texture using SDL_SetRenderTarget().
		 *
		 * @see SDL_SetRenderTarget()
		 */
		void restore_default_target ();

		/**
		 * @brief Returns the size of the output of the renderer.
		 *
		 * This function returns a pair of unsigned integers representing the width and height of the output of the renderer.
		 * The width and height values represent the size of the renderer's output in pixels.
		 *
		 * @return A pair of unsigned integers representing the width and height of the output of the renderer.
		 *
		 * @note This function does not modify the state of the renderer.
		 * @note The result can be used to query the size of the renderer's output,
		 * for example, to adjust the size of the window that displays the renderer's output.
		 */
		[[nodiscard]] std::pair<unsigned, unsigned> get_output_size () const;

		/**
		 * @brief Clears the renderer.
		 *
		 * This function clears the entire rendering target with the draw color.
		 * Any previously rendered content will be removed.
		 *
		 * @note The renderer must be initialized before calling this function.
		 * @note This function is equivalent to calling SDL_RenderClear().
		 *
		 * @see renderer::set_draw_color()
		 * @see renderer::fill_rect()
		 * @see renderer::present()
		 */
		void clear ();

		/**
		 * @brief Copy the contents of a texture to the renderer.
		 *
		 * This function copies the contents of the specified texture to the renderer,
		 * optionally flipping it before copying.
		 *
		 * @param t The texture to be copied.
		 * @param flip_ The flip mode to be applied before copying. Default is flip::NONE.
		 *              Valid values are:
		 *                - flip::NONE: No flipping
		 *                - flip::HORIZONTAL: Flip horizontally
		 *                - flip::VERTICAL: Flip vertically
		 *                - flip::BOTH: Flip both horizontally and vertically
		 *
		 * @see texture
		 *
		 * @note This function internally calls SDL_RenderCopyEx to perform the copy operation.
		 */
		void copy (const texture& t, flip flip_ = flip::NONE);
		/**
		 * @brief Copies a texture to the renderer.
		 *
		 * This function is used to copy a texture to the renderer, with the option to specify a source rectangle and flipping.
		 *
		 * @param t The texture to be copied.
		 * @param srcrect The source rectangle defining what portion of the texture to copy. If nullptr or {0, 0, 0, 0}, the entire texture will be copied.
		 * @param flip_ The flipping mode to be used. Default value is flip::NONE.
		 *
		 * @note This function internally calls SDL_RenderCopyEx() to perform the texture copy operation.
		 *
		 * @see SDL_RenderCopyEx()
		 */
		void copy (const texture& t, const rect& srcrect, flip flip_ = flip::NONE);
		/**
		 * @brief Copies a texture onto the renderer.
		 *
		 * This function allows you to copy a portion of a texture onto the renderer using given source and destination rectangles.
		 * Optionally, you can specify a flip for the texture.
		 *
		 * @param t The texture to be copied onto the renderer.
		 * @param srcrect The source rectangle specifying which portion of the texture to copy.
		 * @param dstrect The destination rectangle specifying where to copy the texture onto the renderer.
		 * @param flip_ The flip to apply to the texture, default is flip::NONE.
		 */
		void copy (const texture& t, const rect& srcrect, const rect& dstrect, flip flip_ = flip::NONE);
		/**
		 * Copies a portion of a texture to the renderer's target.
		 *
		 * @param t The source texture to copy from.
		 * @param srcrect The portion of the source texture to copy.
		 * @param dstrect The destination rectangle where the copied texture will be rendered.
		 * @param angle The angle of rotation in degrees clockwise.
		 * @param flip_ The flip mode to apply during rendering.
		 */
		void copy (const texture& t, const rect& srcrect, const rect& dstrect, double angle, flip flip_ = flip::NONE);
		/**
		  * @brief Copies the specified portion of a texture to the destination rectangle on the renderer.
		  *
		  * This function is used to copy a portion of a texture to a destination rectangle on the renderer's target.
		  * The source rectangle (srcrect) specifies the portion of the texture to be copied, and the destination
		  * rectangle (dstrect) specifies where the copied pixels should be rendered. The texture will be scaled and
		  * rotated as specified by the angle parameter. The rotation will be performed around the point (pt).
		  *
		  * @param t The source texture that will be copied.
		  * @param srcrect The portion of the source texture to be copied.
		  * @param dstrect The destination rectangle where the copied pixels should be rendered.
		  * @param angle The rotation angle in degrees. A positive value indicates clockwise rotation.
		  * @param pt The point around which the rotation should be performed.
		  * @param flip_ The flip option that determines how the texture is flipped.
		  */
		void copy (const texture& t, const rect& srcrect, const rect& dstrect, double angle, const point& pt, flip flip_);

		/**
		 * @brief Draws a line between two points.
		 *
		 * This function draws a line between the specified points (x1, y1) and (x2, y2) on the screen.
		 * The line is drawn using the current rendering color set by the renderer.
		 *
		 * @param x1 The x-coordinate of the starting point of the line.
		 * @param y1 The y-coordinate of the starting point of the line.
		 * @param x2 The x-coordinate of the ending point of the line.
		 * @param y2 The y-coordinate of the ending point of the line.
		 *
		 * @note This function is part of the `renderer` class.
		 * @see renderer
		 */
		void draw_line (int x1, int y1, int x2, int y2);
		/**
		 * @brief Draws an anti-aliased line on the renderer.
		 *
		 * This function draws an anti-aliased line on the renderer using the active color.
		 * The line is drawn from the point (x1, y1) to the point (x2, y2).
		 * The line will be smooth and blended with the existing content on the renderer.
		 *
		 * @note This function uses the SDL2_gfx library to draw the anti-aliased line.
		 *
		 * @param x1 The x-coordinate of the starting point of the line.
		 * @param y1 The y-coordinate of the starting point of the line.
		 * @param x2 The x-coordinate of the ending point of the line.
		 * @param y2 The y-coordinate of the ending point of the line.
		 */
		void draw_line_aa (int x1, int y1, int x2, int y2);
		/**
		 * @brief Draw a line between two points.
		 *
		 * This function draws a line between two points on the screen.
		 *
		 * @param p1 The first point of the line.
		 * @param p2 The second point of the line.
		 */
		void draw_line (const point& p1, const point& p2);
		/**
		 * @brief Draws an anti-aliased line between two points.
		 *
		 * This function draws an anti-aliased line between the specified points using the current rendering settings.
		 * Anti-aliasing reduces the jaggedness of the line edges by blending the colors of the line pixels with the colors of the neighboring pixels.
		 *
		 * @param p1 The first point of the line.
		 * @param p2 The second point of the line.
		 *
		 * @note This function is an inline alias for the `draw_line_aa` function in the `renderer` class.
		 */
		void draw_line_aa (const point& p1, const point& p2);
		/**
		 * @brief Draws a line with a specified thickness.
		 *
		 * This function is used to draw a line on the renderer with a specified thickness. The line
		 * is drawn from the point (x1, y1) to the point (x2, y2) on the screen.
		 *
		 * @param[in] x1 The x-coordinate of the starting point of the line.
		 * @param[in] y1 The y-coordinate of the starting point of the line.
		 * @param[in] x2 The x-coordinate of the ending point of the line.
		 * @param[in] y2 The y-coordinate of the ending point of the line.
		 * @param[in] width The thickness of the line in pixels.
		 *
		 * @return None.
		 */
		void draw_thick_line (int x1, int y1, int x2, int y2, unsigned width);
		/**
		 * @brief Draws a thick line between two points on the canvas.
		 *
		 * This function takes two points as input and draws a line between them on the canvas. The line is drawn with a specified width,
		 * which determines the thickness of the line.
		 *
		 * @param p1 The first point of the line.
		 * @param p2 The second point of the line.
		 * @param width The width of the line.
		 *
		 * @return None.
		 *
		 * @note This function relies on the `draw_thick_line` function from the `renderer` class to actually draw the line on the canvas.
		 *       The `draw_thick_line` function is defined as an `inline` function within the `renderer` class. It takes four integer arguments
		 *       representing the coordinates of the two points and the width, and draws a line using the specified width on the canvas.
		 *       Ensure that the `renderer` class is properly instantiated and its `draw_thick_line` function is accessible when calling this function.
		 *       The `point` structure should be defined or included in the code to properly compile this function.
		 */
		void draw_thick_line (const point& p1, const point& p2, unsigned width);

		/**
		 * @brief Draw a horizontal line on the renderer.
		 *
		 * This function draws a horizontal line on the renderer, using the active color.
		 * The line starts at the specified x-coordinate and ends at the specified x-coordinate,
		 * with the y-coordinate being constant.
		 *
		 * @param x1 The x-coordinate of the starting point of the line.
		 * @param x2 The x-coordinate of the ending point of the line.
		 * @param y The y-coordinate of the line.
		 *
		 */
		void draw_hline (int x1, int x2, int y);
		/**
		 * @brief Draws a vertical line on the renderer.
		 *
		 * This function draws a vertical line on the renderer, starting from the point (x, y1) and ending at the point (x, y2).
		 *
		 * @param y1 The y-coordinate of the starting point of the line.
		 * @param y2 The y-coordinate of the ending point of the line.
		 * @param x The x-coordinate of the line.
		 *
		 * @return None.
		 */
		void draw_vline (int y1, int y2, int x);
		/**
		 * @brief Draws connected lines between given vertices.
		 *
		 * This function draws a series of connected lines between the given vertices.
		 * The lines are rendered using the SDL library.
		 *
		 * @param vertices The array of points representing the vertices of the lines.
		 *        The points should be in the order they need to be connected.
		 *        The array is passed as a const reference to prevent modification.
		 *        The array is of type bsw::array_view1d<point>, which is a lightweight
		 *        non-owning view of a 1-dimensional array.
		 *
		 */
		void draw_connected_lines (const bsw::array_view1d<point>& vertices);

		/**
		 * @brief Draws a specified point on the renderer.
		 *
		 * This function is used to draw a point on the renderer at the specified coordinates (x, y).
		 *
		 * @param x The x-coordinate of the point.
		 * @param y The y-coordinate of the point.
		 *
		 * @note This function is an inline function defined in the class renderer.
		 * @note This function calls the SDL_RenderDrawPoint() function to draw the point on the renderer.
		 * @note This function is safe to use and handles any potential SDL errors.
		 */
		void draw_point (int x, int y);
		/**
		 * @brief Draws a point on the screen.
		 *
		 * This function is responsible for drawing a point on the screen based on the given point object.
		 * It calls the overloaded 'draw_point' function with the x and y coordinates of the point.
		 *
		 * @param p The point to be drawn.
		 */
		void draw_point (const point& p);
		/**
		 * @brief Renders a collection of points on the screen.
		 *
		 * This function takes an array view of points and renders them as individual points on the screen.
		 *
		 * @param points A one-dimensional array view of points to be rendered.
		 *
		 * @note The points will be rendered on the current SDL renderer.
		 *
		 * @see point, SDL_RenderDrawPoints
		 */
		void draw_points (const bsw::array_view1d<point>& points);

		/**
		 * @brief Draws a rectangle on the renderer.
		 *
		 * This function draws a rectangle on the specified renderer. The rectangle
		 * is defined by the provided `rec` parameter.
		 *
		 * @param rec The rectangle to draw.
		 * @note The rectangle is defined by its top-left corner coordinates (`x`, `y`) and its width and height (`w`, `h`):
		 * - `x` : The x-coordinate of the top-left corner of the rectangle.
		 * - `y` : The y-coordinate of the top-left corner of the rectangle.
		 * - `w` : The width of the rectangle.
		 * - `h` : The height of the rectangle.
		 *
		 * @return None.
		 *
		 * @see rect
		 */
		void draw_rectangle (const rect& rec);
		/**
		 * @brief Draws a collection of rectangles on the screen.
		 *
		 * This function uses SDL to draw a collection of rectangles on the screen.
		 *
		 * @param rec The array view containing the rectangles to be drawn.
		 */
		void draw_rectangles (const bsw::array_view1d<rect>& rec);
		/**
		 * @brief Draw a rounded rectangle with a given radius.
		 *
		 * This function is used to draw a rounded rectangle on the screen using the
		 * provided renderer. The rounded rectangle is defined by the given `rect`
		 * structure, which specifies the position and dimensions of the rectangle.
		 * The `radius` parameter specifies the radius of the rounded corners of the
		 * rectangle.
		 *
		 * @param rec    A `rect` structure defining the position and dimensions of
		 *               the rounded rectangle.
		 * @param radius The radius of the rounded corners of the rectangle.
		 */
		void draw_rounded_rect (const rect& rec, unsigned radius);
		/**
		 * @brief Draws a filled rounded rectangle with the specified parameters.
		 *
		 * This function will draw a filled rounded rectangle on the renderer's handle.
		 * The rounded rectangle's position and dimensions are determined by the given rectangle parameter,
		 * and the radius parameter specifies the radius of the rounded corners. The color used for drawing is the
		 * currently active color on the renderer at the time of the call.
		 *
		 * @param rec The rectangle that defines the position and dimensions of the rounded rectangle.
		 * @param radius The radius of the rounded corners.
		 * @return void
		 */
		void draw_rounded_rect_filled (const rect& rec, unsigned radius);

		/**
		* @brief Draw a filled rectangle on the renderer.
		*
		* This function draws a filled rectangle on the renderer using the specified rectangle dimensions.
		*
		* @param rec The rectangle to be drawn.
		*
		*/
		void draw_rectangle_filled (const rect& rec);
		/**
		 * @brief Draw filled rectangles on the screen.
		 *
		 * This function draws filled rectangles on the screen using the SDL_RenderFillRects function.
		 * Each rectangle is defined by four integers representing the x and y coordinates of the top-left corner,
		 * and the width and height of the rectangle.
		 *
		 * @param rec An array view of rectangles to be drawn on the screen.
		 *
		 * @note The rectangles must be defined using the "rect" structure.
		 * @note The "rect" structure should have the following members: x, y, width, height.
		 *
		 * @code
		 * struct rect {
		 *     int x;      // x-coordinate of the top-left corner
		 *     int y;      // y-coordinate of the top-left corner
		 *     int width;  // width of the rectangle
		 *     int height; // height of the rectangle
		 * };
		 * @endcode
		 *
		 * @see renderer
		 */
		void draw_rectangles_filled (const bsw::array_view1d<rect>& rec);



		/**
		 * @brief Draw a circle on the renderer.
		 *
		 * This function draws a circle on the renderer at the specified coordinates and with the given radius.
		 *
		 * @param x The x-coordinate of the center of the circle.
		 * @param y The y-coordinate of the center of the circle.
		 * @param radius The radius of the circle.
		 *
		 * @return None.
		 *
		 * @note The color of the circle is determined by the active color of the renderer.
		 */
		void draw_circle (int x, int y, unsigned radius);
		/**
		 * @brief Draws a circle on the screen.
		 *
		 * This function takes a center point and a radius and draws a circle on the screen using that information.
		 *
		 * @param center The center point of the circle.
		 * @param radius The radius of the circle.
		 */
		void draw_circle (const point& center, unsigned radius);

		/**
		 * @brief Draws an anti-aliased circle at the specified coordinates with the given radius.
		 *
		 * This function draws an anti-aliased circle on the screen using the active color.
		 *
		 * @param x The x-coordinate of the circle's center.
		 * @param y The y-coordinate of the circle's center.
		 * @param radius The radius of the circle.
		 *
		 * @see renderer::active_color(), aacircleRGBA()
		 */
		void draw_circle_aa (int x, int y, unsigned radius);
		/**
		 * @brief Draws an anti-aliased circle on the screen at the specified center and with the given radius.
		 *
		 * @param center The center point of the circle.
		 * @param radius The radius of the circle.
		 *
		 * The draw_circle_aa function is used to draw an anti-aliased circle on the screen. Anti-aliasing
		 * is a technique used to smooth the edges of a shape by displaying intermediate shades of color
		 * between the shape and its background. This creates a more realistic and visually pleasing
		 * appearance. The center point of the circle is specified using the point struct, which contains
		 * the x and y coordinates. The radius parameter determines the size of the circle.
		 *
		 * @note This function is an inline function that calls the draw_circle_aa function of the renderer class.
		 *       The related definition is shown below:
		 *
		 *       @code
		 *       inline void renderer::draw_circle_aa (const point& center, unsigned int radius) {
		 *           draw_circle_aa (center.x, center.y, radius);
		 *       }
		 *       @endcode
		 *
		 * @see renderer
		 * @see point
		 */
		void draw_circle_aa (const point& center, unsigned radius);

		/**
		 * @brief Draws a filled circle on the screen.
		 *
		 * This function draws a filled circle with the specified center coordinates and radius on the screen.
		 *
		 * @param x The x-coordinate of the center of the circle.
		 * @param y The y-coordinate of the center of the circle.
		 * @param radius The radius of the circle.
		 *
		 * @note This is a wrapper function that calls the 'filledCircleRGBA' function from the SDL_gfx library.
		 *       The color used for drawing is obtained from 'active_color()' function.
		 *       The screen handle is acquired from the 'handle()' function.
		 */
		void draw_circle_filled (int x, int y, unsigned radius);
		/**
		 * @brief Draws a filled circle on the screen.
		 *
		 * This function draws a filled circle on the screen with a specified center
		 * and radius. The color of the circle is determined by the current rendering
		 * color.
		 *
		 * @param center The center point of the circle.
		 * @param radius The radius of the circle.
		 */
		void draw_circle_filled (const point& center, unsigned radius);

		void draw_arc(int x, int y, int start, int end, unsigned radius);
		void draw_arc(const point& p, int start, int end, unsigned radius);

		void draw_arc_filled(int x, int y, int start, int end, unsigned radius);
		void draw_arc_filled(const point& p, int start, int end, unsigned radius);

		void draw_ellipse(int x, int y, unsigned rx, unsigned ry);
		void draw_ellipse(const point& center, unsigned rx, unsigned ry);

		void draw_ellipse_aa(int x, int y, unsigned rx, unsigned ry);
		void draw_ellipse_aa(const point& center, unsigned rx, unsigned ry);

		void draw_ellipse_filled(int x, int y, unsigned rx, unsigned ry);
		void draw_ellipse_filled(const point& center, unsigned rx, unsigned ry);

		void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3);
		void draw_triangle(const point& a, const point& b, const point& c);

		void draw_triangle_aa(int x1, int y1, int x2, int y2, int x3, int y3);
		void draw_triangle_aa(const point& a, const point& b, const point& c);

		void draw_triangle_filled(int x1, int y1, int x2, int y2, int x3, int y3);
		void draw_triangle_filled(const point& a, const point& b, const point& c);

		void draw_polygon (const bsw::array_view1d<point>& points);
		void draw_polygon_aa (const bsw::array_view1d<point>& points);
		void draw_polygon (const int16_t* vx, const int16_t* vy, std::size_t n);
		void draw_polygon_aa (const int16_t* vx, const int16_t* vy, std::size_t n);

		void draw_polygon (const bsw::array_view1d<point>& points, const object<SDL_Surface>& tex,
						   int texture_dx, int texture_dy);
		void draw_polygon (const int16_t* vx, const int16_t* vy, std::size_t n, const object<SDL_Surface>& tex,
						   int texture_dx, int texture_dy);

		void draw_polygon_filled (const bsw::array_view1d<point>& points);
		void draw_polygon_filled (const int16_t* vx, const int16_t* vy, std::size_t n);

		void draw_bezier(const bsw::array_view1d<point>& points, unsigned steps);
		void draw_bezier(const int16_t* vx, const int16_t* vy, std::size_t n, unsigned steps);
		void draw_latin1_string (int x, int y, const std::string& s);
		void draw_latin1_string (const point& p, const std::string& s);

		void present () const noexcept;

	};

	d_SDLPP_OSTREAM(renderer::flags);
	d_SDLPP_OSTREAM(renderer::flip);
}

// ===========================================================================================================
// Implementation
// ===========================================================================================================

namespace neutrino::sdl {
	template <typename ... Args,
		typename std::enable_if<(std::is_same_v<renderer::flags, Args> && ...), int>::type>
	renderer::renderer (const object<SDL_Window>& w, Args...flags)
		: object<SDL_Renderer> (SAFE_SDL_CALL(SDL_CreateRenderer,
											  const_cast<SDL_Window*>(w.handle ()),
											  -1,
											  (static_cast<std::uint32_t>(flags) | ... | 0u)
								), true) {

	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	renderer::renderer (const object<SDL_Surface>& s)
		: object<SDL_Renderer> (SAFE_SDL_CALL(SDL_CreateSoftwareRenderer, const_cast<SDL_Surface*>(s
		.handle ())), true) {

	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	renderer::renderer (object<SDL_Renderer>&& other)
		: object<SDL_Renderer> (std::move (other)) {

	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	renderer& renderer::operator= (object<SDL_Renderer>&& other) noexcept {
		object<SDL_Renderer>::operator= (std::move (other));
		return *this;
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	blend_mode renderer::get_blend_mode () const {
		SDL_BlendMode m;
		SAFE_SDL_CALL(SDL_GetRenderDrawBlendMode, const_handle (), &m);
		return static_cast<blend_mode>(m);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::set_blend_mode (blend_mode bm) {
		SAFE_SDL_CALL(SDL_SetRenderDrawBlendMode, handle (), static_cast<SDL_BlendMode>(bm));
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	color renderer::get_active_color () const {
		color c;
		SAFE_SDL_CALL(SDL_GetRenderDrawColor, const_handle (), &c.r, &c.g, &c.b, &c.a);
		return c;
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::set_active_color (const color& c) {
		SAFE_SDL_CALL(SDL_SetRenderDrawColor, handle (), c.r, c.g, c.b, c.a);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	rect renderer::get_clip () const {
		rect r;
		SDL_RenderGetClipRect (const_handle (), &r);
		return r;
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::set_clip (const rect& area) {
		SAFE_SDL_CALL(SDL_RenderSetClipRect, handle (), &area);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::disable_clippping () {
		SAFE_SDL_CALL(SDL_RenderSetClipRect, handle (), nullptr);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	bool renderer::is_clipping_enabled () const {
		return SDL_RenderIsClipEnabled (const_handle ()) == SDL_TRUE;
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	bool renderer::has_integer_scaling () const {
		return SDL_TRUE == SDL_RenderGetIntegerScale (const_handle ());
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::set_integer_scaling (bool enabled) {
		SAFE_SDL_CALL(SDL_RenderSetIntegerScale, handle (), enabled ? SDL_TRUE : SDL_FALSE);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	std::pair<unsigned, unsigned> renderer::get_logical_size () const {
		int w, h;
		SDL_RenderGetLogicalSize (const_handle (), &w, &h);
		return {static_cast<unsigned >(w), static_cast<unsigned >(h)};
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::set_logical_size (unsigned x, unsigned y) {
		SAFE_SDL_CALL(SDL_RenderSetLogicalSize, handle (), static_cast<int>(x), static_cast<int>(y));
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	std::pair<float, float> renderer::get_scaling () const {
		float x, y;
		SDL_RenderGetScale (const_handle (), &x, &y);
		return {x, y};
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::set_scaling (float x, float y) {
		SAFE_SDL_CALL(SDL_RenderSetScale, handle (), x, y);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	rect renderer::get_viewport () const {
		rect r;
		SDL_RenderGetViewport (const_handle (), &r);
		return r;
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::set_viewport (const rect& area) {
		SAFE_SDL_CALL(SDL_RenderSetViewport, handle (), &area);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::disable_viewport () {
		SAFE_SDL_CALL(SDL_RenderSetViewport, handle (), nullptr);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::read_pixels (const pixel_format& fmt, void* dst, std::size_t pitch) const {
		SAFE_SDL_CALL(SDL_RenderReadPixels,
					  const_handle (),
					  nullptr,
					  fmt.value (),
					  dst,
					  static_cast<int>(pitch)
		);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	pixel_format renderer::get_pixel_format () const {
		auto window = SDL_RenderGetWindow (const_handle ());
		if (window) {
			return pixel_format (SDL_GetWindowPixelFormat (window));
		} else {
			uint32_t format;
			auto t = get_target ();
			if (t) {
				SAFE_SDL_CALL(SDL_QueryTexture, t->const_handle (), &format, nullptr, nullptr, nullptr);
				return pixel_format (format);
			}
			RAISE_EX("Can not determine pixel format");
		}
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::read_pixels (const rect& area, const pixel_format& fmt, void* dst, std::size_t pitch) const {
		SAFE_SDL_CALL(SDL_RenderReadPixels,
					  const_handle (),
					  &area,
					  fmt.value (),
					  dst,
					  static_cast<int>(pitch)
		);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	std::optional<texture> renderer::get_target () const {
		SDL_Texture* t = SDL_GetRenderTarget (const_handle ());
		if (t) {
			return texture (object<SDL_Texture> (t, false));
		}
		return std::nullopt;
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::set_target (texture& t) {
		SAFE_SDL_CALL(SDL_SetRenderTarget, handle (), t.handle ());
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::restore_default_target () {
		SAFE_SDL_CALL(SDL_SetRenderTarget, handle (), nullptr);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	std::pair<unsigned, unsigned> renderer::get_output_size () const {
		int w, h;
		SAFE_SDL_CALL(SDL_GetRendererOutputSize, const_handle (), &w, &h);
		return {static_cast<unsigned>(w), static_cast<unsigned>(h)};
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::clear () {
		SAFE_SDL_CALL(SDL_RenderClear, handle ());
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::copy (const texture& t, flip flip_) {
		SAFE_SDL_CALL(SDL_RenderCopyEx,
					  handle (),
					  t.const_handle (),
					  nullptr,
					  nullptr,
					  0.0,
					  nullptr,
					  static_cast<SDL_RendererFlip>(flip_)
		);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::copy (const texture& t, const rect& srcrect, flip flip_) {
		SAFE_SDL_CALL(SDL_RenderCopyEx,
					  handle (),
					  t.const_handle (),
					  &srcrect,
					  nullptr,
					  0.0,
					  nullptr,
					  static_cast<SDL_RendererFlip>(flip_)
		);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::copy (const texture& t, const rect& srcrect, const rect& dstrect, flip flip_) {
		SAFE_SDL_CALL(SDL_RenderCopyEx,
					  handle (),
					  t.const_handle (),
					  &srcrect,
					  &dstrect,
					  0.0,
					  nullptr,
					  static_cast<SDL_RendererFlip>(flip_)
		);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::copy (const texture& t, const rect& srcrect, const rect& dstrect, double angle, flip flip_) {
		SAFE_SDL_CALL(SDL_RenderCopyEx,
					  handle (),
					  t.const_handle (),
					  &srcrect,
					  &dstrect,
					  angle,
					  nullptr,
					  static_cast<SDL_RendererFlip>(flip_)
		);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void
	renderer::copy (const texture& t, const rect& srcrect, const rect& dstrect, double angle, const point& pt,
					flip flip_) {
		SAFE_SDL_CALL(SDL_RenderCopyEx,
					  handle (),
					  t.const_handle (),
					  &srcrect,
					  &dstrect,
					  angle,
					  &pt,
					  static_cast<SDL_RendererFlip>(flip_)
		);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_line (int x1, int y1, int x2, int y2) {
		SAFE_SDL_CALL(SDL_RenderDrawLine, handle (), x1, y1, x2, y2);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_line (const point& p1, const point& p2) {
		draw_line (p1.x, p1.y, p2.x, p2.y);
	}

	inline
	void renderer::draw_thick_line(const point& p1, const point& p2, unsigned width) {
		draw_thick_line (p1.x, p1.y, p2.x, p2.y, width);
	}

	inline
	void renderer::draw_line_aa (const point& p1, const point& p2) {
		draw_line_aa (p1.x, p1.y, p2.x, p2.y);
	}
	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_connected_lines (const bsw::array_view1d<point>& vertices) {
#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable : 4267)
#endif
		SAFE_SDL_CALL(SDL_RenderDrawLines, handle (), vertices.data (), vertices.size ());
#if defined(_MSC_VER)
#pragma warning ( pop )
#endif
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_point (int x, int y) {
		SAFE_SDL_CALL(SDL_RenderDrawPoint, handle (), x, y);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_point (const point& p) {
		draw_point (p.x, p.y);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_points (const bsw::array_view1d<point>& points) {
		SAFE_SDL_CALL(SDL_RenderDrawPoints, handle (), points.data (), points.size ());
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_rectangle (const rect& rec) {
		SAFE_SDL_CALL(SDL_RenderDrawRect, handle (), &rec);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_rectangles (const bsw::array_view1d<rect>& rec) {
#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable : 4267)
#endif
		SAFE_SDL_CALL(SDL_RenderDrawRects, handle (), rec.data (), rec.size ());
#if defined(_MSC_VER)
#pragma warning ( pop )
#endif
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_rectangle_filled (const rect& rec) {
		SAFE_SDL_CALL(SDL_RenderFillRect, handle (), &rec);
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::draw_rectangles_filled (const bsw::array_view1d<rect>& rec) {
		SAFE_SDL_CALL(SDL_RenderFillRects, handle (), rec.data (), rec.size ());
	}

	// ----------------------------------------------------------------------------------------------------------------
	inline
	void renderer::present () const noexcept {
		SDL_RenderPresent (const_handle ());
	}

	inline
	void renderer::draw_circle (const point& center, unsigned int radius) {
		draw_circle (center.x, center.y, radius);
	}

	inline
	void renderer::draw_circle_aa (const point& center, unsigned int radius) {
		draw_circle_aa (center.x, center.y, radius);
	}

	inline
	void renderer::draw_circle_filled (const point& center, unsigned int radius) {
		draw_circle_filled (center.x, center.y, radius);
	}

	inline
	void renderer::draw_arc (const point& p, int start, int end, unsigned int radius) {
		draw_arc (p.x, p.y, start, end, radius);
	}

	inline
	void renderer::draw_arc_filled (const point& p, int start, int end, unsigned int radius) {
		draw_arc_filled (p.x, p.y, start, end, radius);
	}

	inline
	void renderer::draw_ellipse (const point& center, unsigned int rx, unsigned int ry) {
		draw_ellipse (center.x, center.y, rx, ry);
	}

	inline
	void renderer::draw_ellipse_aa (const point& center, unsigned int rx, unsigned int ry) {
		draw_ellipse_aa (center.x, center.y, rx, ry);
	}

	inline
	void renderer::draw_ellipse_filled (const point& center, unsigned int rx, unsigned int ry) {
		draw_ellipse_filled(center.x, center.y, rx, ry);
	}

	inline
	void renderer::draw_triangle (const point& a, const point& b, const point& c) {
		draw_triangle (a.x, a.y, b.x, b.y, c.x, c.y);
	}

	inline
	void renderer::draw_triangle_aa (const point& a, const point& b, const point& c) {
		draw_triangle_aa (a.x, a.y, b.x, b.y, c.x, c.y);
	}

	inline
	void renderer::draw_triangle_filled (const point& a, const point& b, const point& c) {
		draw_triangle_filled (a.x, a.y, b.x, b.y, c.x, c.y);
	}

	inline
	void renderer::draw_latin1_string (const point& p, const std::string& s) {
		draw_latin1_string (p.x, p.y, s);
	}

	namespace detail {
		static inline constexpr std::array<renderer::flags, 5> s_vals_of_renderer_flags = {
			renderer::flags::NONE,
			renderer::flags::SOFTWARE,
			renderer::flags::ACCELERATED,
			renderer::flags::PRESENTVSYNC,
			renderer::flags::TARGETTEXTURE,
		};
	}
	template <typename T>
	static constexpr const decltype(detail::s_vals_of_renderer_flags)&
	values(std::enable_if_t<std::is_same_v<renderer::flags, T>>* = nullptr) {
		return detail::s_vals_of_renderer_flags;
	}
	template <typename T>
	static constexpr auto
	begin(std::enable_if_t<std::is_same_v<renderer::flags, T>>* = nullptr) {
		return detail::s_vals_of_renderer_flags.begin();
	}
	template <typename T>
	static constexpr auto
	end(std::enable_if_t<std::is_same_v<renderer::flags, T>>* = nullptr) {
		return detail::s_vals_of_renderer_flags.end();
	}


	namespace detail {
		static inline constexpr std::array<renderer::flip, 3> s_vals_of_renderer_flip = {
			renderer::flip::NONE,
			renderer::flip::HORIZONTAL,
			renderer::flip::VERTICAL,
		};
	}
	template <typename T>
	static constexpr const decltype(detail::s_vals_of_renderer_flip)&
	values(std::enable_if_t<std::is_same_v<renderer::flip, T>>* = nullptr) {
		return detail::s_vals_of_renderer_flip;
	}
	template <typename T>
	static constexpr auto
	begin(std::enable_if_t<std::is_same_v<renderer::flip, T>>* = nullptr) {
		return detail::s_vals_of_renderer_flip.begin();
	}
	template <typename T>
	static constexpr auto
	end(std::enable_if_t<std::is_same_v<renderer::flip, T>>* = nullptr) {
		return detail::s_vals_of_renderer_flip.end();
	}

}

#endif
