//
// Created by igor on 9/28/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_GEOMETRY_AREA_HH_
#define SDLPP_INCLUDE_SDLPP_GEOMETRY_AREA_HH_

#include <string>
#include <sstream>
#include <ostream>


#include <sdlpp/detail/ostreamops.hh>


namespace neutrino::sdl {
	namespace generic {
		template <typename Scalar>
		struct area_type {
			Scalar w;
			Scalar h;

			area_type()
				: w{}, h{} {}

			area_type(Scalar w_, Scalar h_)
				: w(w_), h(h_) {}

			area_type(const area_type&) = default;

			area_type& operator=(const area_type&) = default;
		};
	}

	template <typename Scalar>
	std::string to_string(const generic::area_type<Scalar>& area) {
		std::ostringstream os;
		os << "(" << area.w << " x " << area.h << ")";
		return os.str();
	}

	namespace generic {
		template <typename Scalar>
		std::ostream& operator << (std::ostream& os, const generic::area_type<Scalar>& area) {
			os << to_string(area);
			return os;
		}

		template <typename Scalar>
		area_type<Scalar> operator*(const area_type<Scalar>& a, Scalar scale) {
			return {scale * a.w, scale * a.h};
		}

		template <typename Scalar>
		area_type<Scalar> operator*(Scalar scale, const area_type<Scalar>& a) {
			return {scale * a.w, scale * a.h};
		}

		template <typename Scalar>
		area_type<Scalar> operator/(const area_type<Scalar>& a, Scalar scale) {
			return {static_cast<Scalar>(a.w / scale), static_cast<Scalar>(a.h / scale)};
		}

	}

}
#endif
