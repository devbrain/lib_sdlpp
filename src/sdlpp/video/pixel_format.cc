//
// Created by igor on 6/10/24.
//
#include <sdlpp/video/pixel_format.hh>
#include "utils/switch_ostream.hh"

namespace neutrino::sdl {
	BEGIN_IMPL_OUTPUT(pixel_format::format)
			case pixel_format::format::INDEX1LSB: return "INDEX1LSB";
			case pixel_format::format::INDEX1MSB: return "INDEX1MSB";
			case pixel_format::format::INDEX4LSB: return "INDEX4LSB";
			case pixel_format::format::INDEX4MSB: return "INDEX4MSB";
			case pixel_format::format::INDEX8: return "INDEX8";
			case pixel_format::format::RGB332: return "RGB332";
			case pixel_format::format::RGB444: return "RGB444";
			case pixel_format::format::RGB555: return "RGB555";
			case pixel_format::format::BGR555: return "BGR555";
			case pixel_format::format::ARGB4444: return "ARGB4444";
			case pixel_format::format::RGBA4444: return "RGBA4444";
			case pixel_format::format::ABGR4444: return "ABGR4444";
			case pixel_format::format::BGRA4444: return "BGRA4444";
			case pixel_format::format::ARGB1555: return "ARGB1555";
			case pixel_format::format::RGBA5551: return "RGBA5551";
			case pixel_format::format::ABGR1555: return "ABGR1555";
			case pixel_format::format::BGRA5551: return "BGRA5551";
			case pixel_format::format::RGB565: return "RGB565";
			case pixel_format::format::BGR565: return "BGR565";
			case pixel_format::format::RGB24: return "RGB24";
			case pixel_format::format::BGR24: return "BGR24";
			case pixel_format::format::RGB888: return "RGB888";
			case pixel_format::format::RGBX8888: return "RGBX8888";
			case pixel_format::format::BGR888: return "BGR888";
			case pixel_format::format::BGRX8888: return "BGRX8888";
			case pixel_format::format::ARGB8888: return "ARGB8888";
			case pixel_format::format::RGBA8888: return "RGBA8888";
			case pixel_format::format::ABGR8888: return "ABGR8888";
			case pixel_format::format::BGRA8888: return "BGRA8888";
			case pixel_format::format::ARGB2101010: return "ARGB2101010";
			case pixel_format::format::YV12: return "YV12";
			case pixel_format::format::IYUV: return "IYUV";
			case pixel_format::format::YUY2: return "YUY2";
			case pixel_format::format::UYVY: return "UYVY";
			case pixel_format::format::YVYU: return "YVYU";
			case pixel_format::format::NV12: return "NV12";
			case pixel_format::format::NV21: return "NV21";
			case pixel_format::format::OES: return "OES";
	END_IMPL_OUTPUT(pixel_format::format)

	BEGIN_IMPL_OUTPUT(pixel_format::type)
			case pixel_format::type::UNKNOWN: return "UNKNOWN";
			case pixel_format::type::INDEX1: return "INDEX1";
			case pixel_format::type::INDEX4: return "INDEX4";
			case pixel_format::type::INDEX8: return "INDEX8";
			case pixel_format::type::PACKED8: return "PACKED8";
			case pixel_format::type::PACKED16: return "PACKED16";
			case pixel_format::type::PACKED32: return "PACKED32";
			case pixel_format::type::ARRAYU8: return "ARRAYU8";
			case pixel_format::type::ARRAYU16: return "ARRAYU16";
			case pixel_format::type::ARRAYU32: return "ARRAYU32";
			case pixel_format::type::ARRAYF16: return "ARRAYF16";
			case pixel_format::type::ARRAYF32: return "ARRAYF32";
	END_IMPL_OUTPUT(pixel_format::type)

	BEGIN_IMPL_OUTPUT(pixel_format::order)
			case pixel_format::order::NONE: return "NONE";
			case pixel_format::order::ORDER_4321: return "ORDER_4321";
			case pixel_format::order::ORDER_1234: return "ORDER_1234";
	END_IMPL_OUTPUT(pixel_format::order)

	BEGIN_IMPL_OUTPUT(pixel_format::component_order)
			case pixel_format::component_order::NONE: return "NONE";
			case pixel_format::component_order::XRGB: return "XRGB";
			case pixel_format::component_order::RGBX: return "RGBX";
			case pixel_format::component_order::ARGB: return "ARGB";
			case pixel_format::component_order::RGBA: return "RGBA";
			case pixel_format::component_order::XBGR: return "XBGR";
			case pixel_format::component_order::BGRX: return "BGRX";
			case pixel_format::component_order::ABGR: return "ABGR";
			case pixel_format::component_order::BGRA: return "BGRA";
	END_IMPL_OUTPUT(pixel_format::component_order)

	BEGIN_IMPL_OUTPUT(pixel_format::array_order)
			case pixel_format::array_order::NONE: return "NONE";
			case pixel_format::array_order::RGB: return "RGB";
			case pixel_format::array_order::RGBA: return "RGBA";
			case pixel_format::array_order::ARGB: return "ARGB";
			case pixel_format::array_order::BGR: return "BGR";
			case pixel_format::array_order::BGRA: return "BGRA";
			case pixel_format::array_order::ABGR: return "ABGR";
	END_IMPL_OUTPUT(pixel_format::array_order)

	BEGIN_IMPL_OUTPUT(pixel_format::layout)
			case pixel_format::layout::NONE: return "NONE";
			case pixel_format::layout::LAYOUT_332: return "LAYOUT_332";
			case pixel_format::layout::LAYOUT_4444: return "LAYOUT_4444";
			case pixel_format::layout::LAYOUT_1555: return "LAYOUT_1555";
			case pixel_format::layout::LAYOUT_5551: return "LAYOUT_5551";
			case pixel_format::layout::LAYOUT_565: return "LAYOUT_565";
			case pixel_format::layout::LAYOUT_8888: return "LAYOUT_8888";
			case pixel_format::layout::LAYOUT_2101010: return "LAYOUT_2101010";
			case pixel_format::layout::LAYOUT_1010102: return "LAYOUT_1010102";
	END_IMPL_OUTPUT(pixel_format::layout)

	std::string to_string(const pixel_format& p) {
		return to_string (p.get_format());
	}

	std::ostream& operator << (std::ostream& os, const pixel_format& p) {
		os << to_string(p);
		return os;
	}

}