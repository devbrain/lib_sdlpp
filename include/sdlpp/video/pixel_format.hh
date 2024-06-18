#ifndef NEUTRION_SDL_PIXEL_FORMAT_HH
#define NEUTRION_SDL_PIXEL_FORMAT_HH

#include <ostream>
#include <cstdint>
#include <tuple>
#include <array>
#include <type_traits>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ostreamops.hh>
#include <bsw/exception.hh>

namespace neutrino::sdl {
	/**
	 * @class pixel_format
	 * @brief Represents the format of a pixel.
	 *
	 * This class provides methods to get information about the format of a pixel, such as the format type,
	 * the order of the components, and the layout of the packed pixels.
	 */
	class pixel_format {
	 public:
		/**
		 * @enum format
		 * @brief Represents the available pixel formats.
		 */
		enum format : std::uint32_t {
			INDEX1LSB = SDL_PIXELFORMAT_INDEX1LSB,
			INDEX1MSB = SDL_PIXELFORMAT_INDEX1MSB,
			INDEX2LSB = SDL_PIXELFORMAT_INDEX1LSB,
			INDEX2MSB = SDL_PIXELFORMAT_INDEX1MSB,
			INDEX4LSB = SDL_PIXELFORMAT_INDEX4LSB,
			INDEX4MSB = SDL_PIXELFORMAT_INDEX4MSB,
			INDEX8 = SDL_PIXELFORMAT_INDEX8,
			RGB332 = SDL_PIXELFORMAT_RGB332,
			RGB444 = SDL_PIXELFORMAT_RGB444,
			RGB555 = SDL_PIXELFORMAT_RGB555,
			BGR555 = SDL_PIXELFORMAT_BGR555,
			ARGB4444 = SDL_PIXELFORMAT_ARGB4444,
			RGBA4444 = SDL_PIXELFORMAT_RGBA4444,
			ABGR4444 = SDL_PIXELFORMAT_ABGR4444,
			BGRA4444 = SDL_PIXELFORMAT_BGRA4444,
			ARGB1555 = SDL_PIXELFORMAT_ARGB1555,
			RGBA5551 = SDL_PIXELFORMAT_RGBA5551,
			ABGR1555 = SDL_PIXELFORMAT_ABGR1555,
			BGRA5551 = SDL_PIXELFORMAT_BGRA5551,
			RGB565 = SDL_PIXELFORMAT_RGB565,
			BGR565 = SDL_PIXELFORMAT_BGR565,
			RGB24 = SDL_PIXELFORMAT_RGB24,
			BGR24 = SDL_PIXELFORMAT_BGR24,
			RGB888 = SDL_PIXELFORMAT_RGB888,
			RGBX8888 = SDL_PIXELFORMAT_RGBX8888,
			BGR888 = SDL_PIXELFORMAT_BGR888,
			BGRX8888 = SDL_PIXELFORMAT_BGRX8888,
			ARGB8888 = SDL_PIXELFORMAT_ARGB8888,
			RGBA8888 = SDL_PIXELFORMAT_RGBA8888,
			ABGR8888 = SDL_PIXELFORMAT_ABGR8888,
			BGRA8888 = SDL_PIXELFORMAT_BGRA8888,
			ARGB2101010 = SDL_PIXELFORMAT_ARGB2101010,

			RGBA32 = SDL_PIXELFORMAT_RGBA32,
			ARGB32 = SDL_PIXELFORMAT_ARGB32,
			BGRA32 = SDL_PIXELFORMAT_BGRA32,
			ABGR32 = SDL_PIXELFORMAT_ABGR32,

			YV12 = SDL_PIXELFORMAT_YV12,  /**< Planar mode: Y + V + U  (3 planes) */
			IYUV = SDL_PIXELFORMAT_IYUV,  /**< Planar mode: Y + U + V  (3 planes) */
			YUY2 = SDL_PIXELFORMAT_YUY2,  /**< Packed mode: Y0+U0+Y1+V0 (1 plane) */
			UYVY = SDL_PIXELFORMAT_UYVY,  /**< Packed mode: U0+Y0+V0+Y1 (1 plane) */
			YVYU = SDL_PIXELFORMAT_YVYU,  /**< Packed mode: Y0+V0+Y1+U0 (1 plane) */
			NV12 = SDL_PIXELFORMAT_NV12,  /**< Planar mode: Y + U/V interleaved  (2 planes) */
			NV21 = SDL_PIXELFORMAT_NV21,  /**< Planar mode: Y + V/U interleaved  (2 planes) */
			OES = SDL_PIXELFORMAT_EXTERNAL_OES,       /**< Android video texture format */
		};

		/**
		 * @brief The type enum class represents the pixel types for a given SDL pixel format.
		 *
		 * The type enum class defines the following pixel types:
		 * - UNKNOWN: Unknown pixel type
		 * - INDEX1: 1-bit index pixel type
		 * - INDEX4: 4-bit index pixel type
		 * - INDEX8: 8-bit index pixel type
		 * - PACKED8: 8-bit packed pixel type
		 * - PACKED16: 16-bit packed pixel type
		 * - PACKED32: 32-bit packed pixel type
		 * - ARRAYU8: 8-bit unsigned pixel array type
		 * - ARRAYU16: 16-bit unsigned pixel array type
		 * - ARRAYU32: 32-bit unsigned pixel array type
		 * - ARRAYF16: 16-bit floating point pixel array type
		 * - ARRAYF32: 32-bit floating point pixel array type
		 */
		enum class type : uint8_t {
			UNKNOWN = SDL_PIXELTYPE_UNKNOWN,
			INDEX1 = SDL_PIXELTYPE_INDEX1,
			INDEX4 = SDL_PIXELTYPE_INDEX4,
			INDEX8 = SDL_PIXELTYPE_INDEX8,
			PACKED8 = SDL_PIXELTYPE_PACKED8,
			PACKED16 = SDL_PIXELTYPE_PACKED16,
			PACKED32 = SDL_PIXELTYPE_PACKED32,
			ARRAYU8 = SDL_PIXELTYPE_ARRAYU8,
			ARRAYU16 = SDL_PIXELTYPE_ARRAYU16,
			ARRAYU32 = SDL_PIXELTYPE_ARRAYU32,
			ARRAYF16 = SDL_PIXELTYPE_ARRAYF16,
			ARRAYF32 = SDL_PIXELTYPE_ARRAYF32
		};
		/**
		 * @enum order
		 * @brief The enumeration class for ordering of bits in pixel formats.
		 *
		 * The order enumeration class defines the possible bit ordering in pixel formats.
		 * It is used to specify the desired bit ordering when creating pixel formats.
		 */
		enum class order : uint8_t {
			NONE = SDL_BITMAPORDER_NONE,
			ORDER_4321 = SDL_BITMAPORDER_4321,
			ORDER_1234 = SDL_BITMAPORDER_1234
		};
		/**
		 * @enum component_order
		 * An enumeration representing the component order of a packed pixel format.
		 * The component order determines the order of the color components in a pixel value.
		 *
		 * The available component orders are:
		 * - NONE: No specific component order (SDL_PACKEDORDER_NONE).
		 * - XRGB: XRGB component order (SDL_PACKEDORDER_XRGB).
		 * - RGBX: RGBX component order (SDL_PACKEDORDER_RGBX).
		 * - ARGB: ARGB component order (SDL_PACKEDORDER_ARGB).
		 * - RGBA: RGBA component order (SDL_PACKEDORDER_RGBA).
		 * - XBGR: XBGR component order (SDL_PACKEDORDER_XBGR).
		 * - BGRX: BGRX component order (SDL_PACKEDORDER_BGRX).
		 * - ABGR: ABGR component order (SDL_PACKEDORDER_ABGR).
		 * - BGRA: BGRA component order (SDL_PACKEDORDER_BGRA).
		 *
		 * @warning This enumeration is dependent on the SDL library and should not be modified directly.
		 */
		enum class component_order : uint8_t {
			NONE = SDL_PACKEDORDER_NONE,
			XRGB = SDL_PACKEDORDER_XRGB,
			RGBX = SDL_PACKEDORDER_RGBX,
			ARGB = SDL_PACKEDORDER_ARGB,
			RGBA = SDL_PACKEDORDER_RGBA,
			XBGR = SDL_PACKEDORDER_XBGR,
			BGRX = SDL_PACKEDORDER_BGRX,
			ABGR = SDL_PACKEDORDER_ABGR,
			BGRA = SDL_PACKEDORDER_BGRA
		};
		/**
		 * @brief The array_order enum class represents the order of pixel components in an array.
		 *
		 * The array_order enum class provides values for different pixel component orders that can be used in an array of pixels.
		 * These values are used to specify the arrangement of components (e.g. red, green, blue, alpha) in an array of pixels.
		 * The underlying type of the enum class is uint8_t.
		 *
		 * The possible values of the array_order enum class are:
		 *   - NONE: No specific pixel component order.
		 *   - RGB: Red, Green, Blue.
		 *   - RGBA: Red, Green, Blue, Alpha.
		 *   - ARGB: Alpha, Red, Green, Blue.
		 *   - BGR: Blue, Green, Red.
		 *   - BGRA: Blue, Green, Red, Alpha.
		 *   - ABGR: Alpha, Blue, Green, Red.
		 *
		 * Usage example:
		 * @code
		 * array_order order = array_order::RGBA;
		 * @endcode
		 *
		 * @note This enum class is based on the SDL_array_order enum from the Simple DirectMedia Layer (SDL) library.
		 *       The SDL_array_order enum values are used as the underlying values for the array_order enum class.
		 *       More information about SDL can be found at https://www.libsdl.org/
		 */
		enum class array_order : uint8_t {
			NONE = SDL_ARRAYORDER_NONE,
			RGB = SDL_ARRAYORDER_RGB,
			RGBA = SDL_ARRAYORDER_RGBA,
			ARGB = SDL_ARRAYORDER_ARGB,
			BGR = SDL_ARRAYORDER_BGR,
			BGRA = SDL_ARRAYORDER_BGRA,
			ABGR = SDL_ARRAYORDER_ABGR
		};

		/**
		 * @enum layout
		 * Enum class representing the layout options for packed pixel formats.
		 */
		enum class layout : uint8_t {
			NONE = SDL_PACKEDLAYOUT_NONE,
			LAYOUT_332 = SDL_PACKEDLAYOUT_332,
			LAYOUT_4444 = SDL_PACKEDLAYOUT_4444,
			LAYOUT_1555 = SDL_PACKEDLAYOUT_1555,
			LAYOUT_5551 = SDL_PACKEDLAYOUT_5551,
			LAYOUT_565 = SDL_PACKEDLAYOUT_565,
			LAYOUT_8888 = SDL_PACKEDLAYOUT_8888,
			LAYOUT_2101010 = SDL_PACKEDLAYOUT_2101010,
			LAYOUT_1010102 = SDL_PACKEDLAYOUT_1010102
		};
	 public:
		explicit pixel_format (std::uint32_t f);
		explicit pixel_format (format f);
		pixel_format (uint8_t bpp, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask);

		/**
		 * @brief Creates a pixel format with 8 bits per pixel.
		 *
		 * This static function creates a pixel format object with 8 bits per pixel.
		 *
		 * @return The pixel format object with 8 bits per pixel.
		 */
		static pixel_format make_8bit ();
		/**
		 * @brief Creates a 32-bit RGBA pixel format.
		 *
		 * This function creates a pixel format object that represents a 32-bit RGBA format.
		 * The pixel format is returned as an instance of the pixel_format class.
		 *
		 * @return A pixel_format object representing a 32-bit RGBA format.
		 */
		static pixel_format make_rgba_32bit ();

		[[nodiscard]] explicit operator std::uint32_t () const noexcept;
		[[nodiscard]] uint32_t value () const noexcept;

		[[nodiscard]] format get_format () const noexcept;
		[[nodiscard]] type get_type () const noexcept;
		[[nodiscard]] order get_order () const noexcept;
		[[nodiscard]] component_order get_component_order () const noexcept;
		[[nodiscard]] array_order get_array_order () const noexcept;
		[[nodiscard]] layout get_layout_order () const noexcept;
		[[nodiscard]] std::uint8_t get_bits_per_pixels () const noexcept;
		[[nodiscard]] std::uint8_t get_bytes_per_pixels () const noexcept;
		[[nodiscard]] bool is_indexed () const noexcept;
		[[nodiscard]] bool is_array () const noexcept;
		[[nodiscard]] bool is_alpha () const noexcept;
		[[nodiscard]] bool is_fourcc () const noexcept;
		[[nodiscard]] bool is_packed () const noexcept;

		/**
		 * @fn std::tuple<std::uint8_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t> pixel_format::get_mask () const
		 * @brief Retrieves the pixel mask information.
		 *
		 * This function returns the bit mask information for the pixels in the format, including the number of bits per pixel
		 * and the masks for the red, green, blue, and alpha components. The returned information is stored in a tuple.
		 *
		 * @return A tuple containing the number of bits per pixel, and the masks for the red, green, blue, and alpha components.
		 *
		 * @throws std::runtime_error If no pixel format conversion is possible.
		 */
// bpp, rmask, gmask, bmask, amask
		[[nodiscard]] std::tuple<std::uint8_t,
								 std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t> get_mask () const;
	 private:
		std::uint32_t m_value;
	};


}

inline
std::ostream& operator<< (std::ostream& os, const neutrino::sdl::pixel_format& f) {
	os << SDL_GetPixelFormatName (f.value ());
	return os;
}
// ===================================================================================================
// Implementation
// ===================================================================================================
namespace neutrino::sdl {
	// ----------------------------------------------------------------------------------------------
	inline
	pixel_format pixel_format::make_8bit () {
		return pixel_format (pixel_format::INDEX8);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	pixel_format pixel_format::make_rgba_32bit () {
		return pixel_format (pixel_format::RGBA32);
	}

	// ----------------------------------------------------------------------------------------------
	inline
	pixel_format::pixel_format (uint8_t bpp, uint32_t rmask, uint32_t gmask, uint32_t bmask, uint32_t amask)
		: m_value (SDL_MasksToPixelFormatEnum (static_cast<int>(bpp), rmask, gmask, bmask, amask)) {
		if (m_value == SDL_PIXELFORMAT_UNKNOWN) {
			RAISE_EX("Can not create pixel format from the provided parameters");
		}
	}

	// --------------------------------------------------------------------------------
	inline
	pixel_format::pixel_format (std::uint32_t f)
		: m_value (f) {

	}

	// --------------------------------------------------------------------------------
	inline
	pixel_format::pixel_format (format f)
		: m_value (static_cast<uint32_t>(f)) {

	}

	// --------------------------------------------------------------------------------
	inline
	pixel_format::operator std::uint32_t () const noexcept {
		return value ();
	}

	// --------------------------------------------------------------------------------
	inline
	uint32_t pixel_format::value () const noexcept {
		return m_value;
	}

	// --------------------------------------------------------------------------------
	inline
	pixel_format::format pixel_format::get_format () const noexcept {
		return static_cast<format>(m_value);
	}

	// --------------------------------------------------------------------------------
	inline
	pixel_format::type pixel_format::get_type () const noexcept {
		return static_cast<type>(SDL_PIXELFLAG(m_value));
	}

	// --------------------------------------------------------------------------------
	inline
	pixel_format::order pixel_format::get_order () const noexcept {
		return static_cast<order>(SDL_PIXELORDER(m_value));
	}

	// --------------------------------------------------------------------------------
	inline
	pixel_format::component_order pixel_format::get_component_order () const noexcept {
		return static_cast<component_order>(SDL_PIXELORDER(m_value));
	}

	// --------------------------------------------------------------------------------
	inline
	pixel_format::array_order pixel_format::get_array_order () const noexcept {
		return static_cast<array_order>(SDL_PIXELORDER(m_value));
	}

	// --------------------------------------------------------------------------------
	inline
	pixel_format::layout pixel_format::get_layout_order () const noexcept {
		return static_cast<layout>(SDL_PIXELLAYOUT(m_value));
	}

	// --------------------------------------------------------------------------------
	inline
	std::uint8_t pixel_format::get_bits_per_pixels () const noexcept {
		return SDL_BITSPERPIXEL(m_value);
	}

	// --------------------------------------------------------------------------------
	inline
	std::uint8_t pixel_format::get_bytes_per_pixels () const noexcept {
		return SDL_BYTESPERPIXEL(m_value);
	}

	// --------------------------------------------------------------------------------
	inline
	bool pixel_format::is_indexed () const noexcept {
		return SDL_ISPIXELFORMAT_INDEXED(m_value) != 0;
	}

	// --------------------------------------------------------------------------------
	inline
	bool pixel_format::is_array () const noexcept {
		return SDL_ISPIXELFORMAT_ARRAY(m_value) != 0;
	}

	// --------------------------------------------------------------------------------
	inline
	bool pixel_format::is_alpha () const noexcept {
		return SDL_ISPIXELFORMAT_ALPHA(m_value) != 0;
	}

	// --------------------------------------------------------------------------------
	inline
	bool pixel_format::is_fourcc () const noexcept {
		return SDL_ISPIXELFORMAT_FOURCC(m_value) != 0;
	}

	// --------------------------------------------------------------------------------
	inline
	bool pixel_format::is_packed () const noexcept {
		return SDL_ISPIXELFORMAT_PACKED(m_value);
	}

	// --------------------------------------------------------------------------------
	inline
	std::tuple<std::uint8_t,
			   std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t> pixel_format::get_mask () const {
		int bpp;
		uint32_t r, g, b, a;
		if (SDL_TRUE == SDL_PixelFormatEnumToMasks (m_value, &bpp, &r, &g, &b, &a)) {
			return {static_cast<uint8_t>(bpp), r, g, b, a};
		}
		RAISE_EX("No pixel format conversion is possible");
	}

	namespace detail {
		static inline constexpr std::array<pixel_format::format, 44> s_vals_of_provides_format = {
			pixel_format::format::INDEX1LSB,
			pixel_format::format::INDEX1MSB,
			pixel_format::format::INDEX2LSB,
			pixel_format::format::INDEX2MSB,
			pixel_format::format::INDEX4LSB,
			pixel_format::format::INDEX4MSB,
			pixel_format::format::INDEX8,
			pixel_format::format::RGB332,
			pixel_format::format::RGB444,
			pixel_format::format::RGB555,
			pixel_format::format::BGR555,
			pixel_format::format::ARGB4444,
			pixel_format::format::RGBA4444,
			pixel_format::format::ABGR4444,
			pixel_format::format::BGRA4444,
			pixel_format::format::ARGB1555,
			pixel_format::format::RGBA5551,
			pixel_format::format::ABGR1555,
			pixel_format::format::BGRA5551,
			pixel_format::format::RGB565,
			pixel_format::format::BGR565,
			pixel_format::format::RGB24,
			pixel_format::format::BGR24,
			pixel_format::format::RGB888,
			pixel_format::format::RGBX8888,
			pixel_format::format::BGR888,
			pixel_format::format::BGRX8888,
			pixel_format::format::ARGB8888,
			pixel_format::format::RGBA8888,
			pixel_format::format::ABGR8888,
			pixel_format::format::BGRA8888,
			pixel_format::format::ARGB2101010,
			pixel_format::format::RGBA32,
			pixel_format::format::ARGB32,
			pixel_format::format::BGRA32,
			pixel_format::format::ABGR32,
			pixel_format::format::YV12,
			pixel_format::format::IYUV,
			pixel_format::format::YUY2,
			pixel_format::format::UYVY,
			pixel_format::format::YVYU,
			pixel_format::format::NV12,
			pixel_format::format::NV21,
			pixel_format::format::OES,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_provides_format)&
	values(typename std::enable_if<std::is_same_v<pixel_format::format, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_format;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<pixel_format::format, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_format.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<pixel_format::format, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_format.end();
	}


	namespace detail {
		static inline constexpr std::array<pixel_format::type, 12> s_vals_of_provides_type = {
			pixel_format::type::UNKNOWN,
			pixel_format::type::INDEX1,
			pixel_format::type::INDEX4,
			pixel_format::type::INDEX8,
			pixel_format::type::PACKED8,
			pixel_format::type::PACKED16,
			pixel_format::type::PACKED32,
			pixel_format::type::ARRAYU8,
			pixel_format::type::ARRAYU16,
			pixel_format::type::ARRAYU32,
			pixel_format::type::ARRAYF16,
			pixel_format::type::ARRAYF32,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_provides_type)&
	values(typename std::enable_if<std::is_same_v<pixel_format::type, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_type;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<pixel_format::type, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_type.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<pixel_format::type, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_type.end();
	}


	namespace detail {
		static inline constexpr std::array<pixel_format::order, 3> s_vals_of_provides_order = {
			pixel_format::order::NONE,
			pixel_format::order::ORDER_4321,
			pixel_format::order::ORDER_1234,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_provides_order)&
	values(typename std::enable_if<std::is_same_v<pixel_format::order, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_order;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<pixel_format::order, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_order.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<pixel_format::order, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_order.end();
	}


	namespace detail {
		static inline constexpr std::array<pixel_format::component_order, 9> s_vals_of_provides_component_order = {
			pixel_format::component_order::NONE,
			pixel_format::component_order::XRGB,
			pixel_format::component_order::RGBX,
			pixel_format::component_order::ARGB,
			pixel_format::component_order::RGBA,
			pixel_format::component_order::XBGR,
			pixel_format::component_order::BGRX,
			pixel_format::component_order::ABGR,
			pixel_format::component_order::BGRA,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_provides_component_order)&
	values(typename std::enable_if<std::is_same_v<pixel_format::component_order, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_component_order;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<pixel_format::component_order, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_component_order.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<pixel_format::component_order, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_component_order.end();
	}


	namespace detail {
		static inline constexpr std::array<pixel_format::array_order, 7> s_vals_of_provides_array_order = {
			pixel_format::array_order::NONE,
			pixel_format::array_order::RGB,
			pixel_format::array_order::RGBA,
			pixel_format::array_order::ARGB,
			pixel_format::array_order::BGR,
			pixel_format::array_order::BGRA,
			pixel_format::array_order::ABGR,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_provides_array_order)&
	values(typename std::enable_if<std::is_same_v<pixel_format::array_order, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_array_order;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<pixel_format::array_order, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_array_order.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<pixel_format::array_order, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_array_order.end();
	}


	namespace detail {
		static inline constexpr std::array<pixel_format::layout, 9> s_vals_of_provides_layout = {
			pixel_format::layout::NONE,
			pixel_format::layout::LAYOUT_332,
			pixel_format::layout::LAYOUT_4444,
			pixel_format::layout::LAYOUT_1555,
			pixel_format::layout::LAYOUT_5551,
			pixel_format::layout::LAYOUT_565,
			pixel_format::layout::LAYOUT_8888,
			pixel_format::layout::LAYOUT_2101010,
			pixel_format::layout::LAYOUT_1010102,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_provides_layout)&
	values(typename std::enable_if<std::is_same_v<pixel_format::layout, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_layout;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<pixel_format::layout, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_layout.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<pixel_format::layout, T>>::type* = nullptr) {
		return detail::s_vals_of_provides_layout.end();
	}

	d_SDLPP_OSTREAM(pixel_format::format);
	d_SDLPP_OSTREAM(pixel_format::type);
	d_SDLPP_OSTREAM(pixel_format::order);
	d_SDLPP_OSTREAM(pixel_format::component_order);
	d_SDLPP_OSTREAM(pixel_format::array_order);
	d_SDLPP_OSTREAM(pixel_format::layout);

	d_SDLPP_OSTREAM(const pixel_format&);
}

#endif
