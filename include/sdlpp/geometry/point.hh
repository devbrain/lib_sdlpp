//
// Created by igor on 9/28/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_GEOMETRY_POINT_HH_
#define SDLPP_INCLUDE_SDLPP_GEOMETRY_POINT_HH_

#include <neutrino/math/math.hh>
#include <sdlpp/detail/sdl2.hh>

namespace neutrino::math {

	template <>
	class vector<float, 2>;

	template <>
	class vector<int, 2> : public vector_ops<int, 2>, public SDL_Point {
	 public:
		using value_type = int;

		static constexpr std::size_t size() {
			return 2;
		}

		vector() noexcept
			: SDL_Point() {
			x = 0;
			y = 0;
		};

		vector(const vector<float, 2>& other);

		vector(const vector& other) noexcept = default;

		vector(vector&& other) noexcept = default;

		explicit vector(const value_type value) noexcept
			: SDL_Point() {
			x = value;
			y = value;
		};

		vector(const value_type x_val, const value_type y_val) noexcept
			: SDL_Point() {
			x = x_val;
			y = y_val;
		}

		explicit constexpr vector(const std::array<value_type, 2>& args) noexcept
			: SDL_Point() {
			x = args[0];
			y = args[1];
		}

		explicit vector(const value_type (& data)[2]) noexcept
			: SDL_Point() {
			x = data[0];
			y = data[1];
		}

		constexpr vector& operator=(const vector& other) noexcept = default;

		constexpr vector& operator=(vector&& other) noexcept = default;

		template <class Expr, MATH_VEC_ENABLE_IF_EXPR>
		constexpr vector(Expr const& expr) noexcept
			: SDL_Point() {
			x = expr[0];
			y = expr[1];
		}

		template <class Expr, MATH_VEC_ENABLE_IF_EXPR>
		vector& operator=(Expr const& expr) {
			x = expr[0];
			y = expr[1];
			return *this; // this line was missing in the slides and in the talk
		}

		constexpr const value_type& operator[](const std::size_t i) const {
			if (i == 0) return x;
			else if (i == 1) return y;
			else throw std::out_of_range("point2Di: Index out of range");
		}

		constexpr value_type& operator[](const std::size_t i) {
			if (i == 0) return x;
			else if (i == 1) return y;
			else throw std::out_of_range("point2Di: Index out of range");
		}

		explicit vector(const SDL_Point& p) noexcept
			: SDL_Point() {
			x = p.x;
			y = p.y;
		}

		vector& operator=(const SDL_Point& p) noexcept {
			x = p.x;
			y = p.y;
			return *this;
		}
	};

	// ============================================================================
	template <>
	class vector<float, 2> : public vector_ops<float, 2>, public SDL_FPoint {
	 public:
		using value_type = float;

		static constexpr std::size_t size() {
			return 2;
		}

		vector() noexcept
			: SDL_FPoint() {
			x = 0;
			y = 0;
		};

		vector(const vector<int, 2>& other)
			: SDL_FPoint() {
			x = other.x;
			y = other.y;
		}

		vector(const vector& other) noexcept = default;

		vector(vector&& other) noexcept = default;

		explicit vector(const value_type value) noexcept
			: SDL_FPoint() {
			x = value;
			y = value;
		};

		vector(const value_type x_val, const value_type y_val) noexcept
			: SDL_FPoint() {
			x = x_val;
			y = y_val;
		}

		vector(const int x_val, const int y_val) noexcept
			: SDL_FPoint() {
			x = static_cast <value_type>(x_val);
			y = static_cast <value_type>(y_val);
		}

		explicit constexpr vector(const std::array<value_type, 2>& args) noexcept
			: SDL_FPoint() {
			x = args[0];
			y = args[1];
		}

		explicit vector(const value_type (& data)[2]) noexcept
			: SDL_FPoint() {
			x = data[0];
			y = data[1];
		}

		constexpr vector& operator=(const vector& other) noexcept = default;

		constexpr vector& operator=(vector&& other) noexcept = default;

		template <class Expr, MATH_VEC_ENABLE_IF_EXPR>
		constexpr vector(Expr const& expr) noexcept
			: SDL_FPoint() {
			x = expr[0];
			y = expr[1];
		}

		template <class Expr, MATH_VEC_ENABLE_IF_EXPR>
		vector& operator=(Expr const& expr) {
			x = expr[0];
			y = expr[1];
			return *this; // this line was missing in the slides and in the talk
		}

		constexpr const value_type& operator[](const std::size_t i) const {
			if (i == 0) return x;
			else if (i == 1) return y;
			else throw std::out_of_range("point2Di: Index out of range");
		}

		constexpr value_type& operator[](const std::size_t i) {
			if (i == 0) return x;
			else if (i == 1) return y;
			else throw std::out_of_range("point2Di: Index out of range");
		}

		explicit vector(const SDL_FPoint& p) noexcept
			: SDL_FPoint() {
			x = p.x;
			y = p.y;
		}

		vector& operator=(const SDL_FPoint& p) noexcept {
			x = p.x;
			y = p.y;
			return *this;
		}

		explicit vector(const SDL_Point& p) noexcept
			: SDL_FPoint() {
			x = static_cast<value_type>(p.x);
			y = static_cast<value_type>(p.y);
		}

		vector& operator=(const SDL_Point& p) noexcept {
			x = static_cast<value_type>(p.x);
			y = static_cast<value_type>(p.y);
			return *this;
		}
	};

	inline
	vector<int, 2>::vector(const vector<float, 2>& other)
		: SDL_Point() {
		x = static_cast<int>(other.x);
		y = static_cast<int>(other.y);
	}

	using point2f = math::vector<float, 2>;
	using point = math::vector<int, 2>;

	inline
	bool operator==(const point& a, const point2f& b) {
		return static_cast<float>(a.x) == b.x && static_cast<float>(a.y) == b.y;
	}

	inline
	bool operator!=(const point& a, const point2f& b) {
		return !(a == b);
	}

	inline
	bool operator==(const point2f& b, const point& a) {
		return static_cast<float>(a.x) == b.x && static_cast<float>(a.y) == b.y;
	}

	inline
	bool operator!=(const point2f& b, const point& a) {
		return !(a == b);
	}
}

#endif
