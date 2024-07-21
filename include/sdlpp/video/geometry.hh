//
// Created by igor on 03/06/2020.
//

#ifndef NEUTRINO_SDL_GEOMETRY_HH
#define NEUTRINO_SDL_GEOMETRY_HH

#include <utility>
#include <cmath>
#include <vector>
#include <type_traits>
#include <array>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	struct area_type {
		unsigned int w;
		unsigned int h;
		area_type();
		area_type(int w_, int h_);
		area_type(const area_type&) = default;
		area_type& operator=(const area_type&) = default;
	};

	d_SDLPP_OSTREAM_WITHOT_FROM_STRING(const area_type&);

	struct point2f : public SDL_FPoint{
		point2f() noexcept;
		point2f(int x_, int y_) noexcept;
		point2f(float x_, float y_) noexcept;
		explicit point2f(const SDL_Point& p) noexcept;
		point2f& operator=(const SDL_Point& p) noexcept;
		explicit point2f(const SDL_FPoint& p) noexcept;
		point2f& operator=(const SDL_FPoint& p) noexcept;
	};

	struct point : public SDL_Point {
		point() noexcept;
		point(int x_, int y_) noexcept;
		explicit point(const SDL_Point& p) noexcept;
		point& operator=(const SDL_Point& p) noexcept;
	};

	inline
	area_type operator* (const area_type& a, int scale) {
		return {scale*static_cast <int>(a.w), scale* static_cast <int>(a.h)};
	}

	inline
	area_type operator* (int scale, const area_type& a) {
		return {scale*static_cast <int>(a.w), scale* static_cast <int>(a.h)};
	}

	inline
	point operator* (const point& a, int scale) {
		return {scale*static_cast <int>(a.x), scale* static_cast <int>(a.y)};
	}

	inline
	point operator* (int scale, const point& a) {
		return {scale*static_cast <int>(a.x), scale* static_cast <int>(a.y)};
	}

	inline
	point2f operator* (const point2f& a, int scale) {
		return {static_cast <float>(scale)*a.x, static_cast <float>(scale)* a.y};
	}

	inline
	point2f operator* (int scale, const point2f& a) {
		return {static_cast <float>(scale)*a.x, static_cast <float>(scale)* a.y};
	}

	inline
	point2f operator* (const point2f& a, float scale) {
		return {scale*a.x, scale* a.y};
	}

	inline
	point2f operator* (float scale, const point2f& a) {
		return {scale*a.x, scale* a.y};
	}

	namespace detail {
		template <typename T>
		static inline constexpr auto is_point_v = std::is_same_v<T, point> || std::is_same_v<T, point2f>
		|| std::is_same_v<T, SDL_Point> || std::is_same_v<T, SDL_FPoint>;
	}


	template <typename P>
	[[nodiscard]] float distance(const P& a, const P& b, std::enable_if_t<detail::is_point_v<P>>* = nullptr) {
		return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
	}

	template <typename P>
	[[nodiscard]]
	float distance(const P& a, std::enable_if_t<detail::is_point_v<P>>* = nullptr) {
		return std::sqrt(std::pow(a.x, 2) + std::pow(a.y, 2));
	}

	template <typename P>
	[[nodiscard]]
	std::enable_if_t<detail::is_point_v<P>, P> operator+(const P& a, const P& b) noexcept {
		return {a.x + b.x, a.y + b.y};
	}

	template <typename P>
	[[nodiscard]]
	std::enable_if_t<detail::is_point_v<P>, P> operator-(const P& a, const P& b) noexcept {
		return {a.x - b.x, a.y - b.y};
	}

	template <typename P>
	[[nodiscard]]
	std::enable_if_t<detail::is_point_v<P>, bool> operator==(const P& a, const P& b) {
		return a.x == b.x && a.y == b.y;
	}

	template <typename P>
	[[nodiscard]]
	std::enable_if_t<detail::is_point_v<P>, bool> operator!=(const P& a, const P& b) {
		return !(a == b);
	}

	d_SDLPP_OSTREAM_WITHOT_FROM_STRING(const point&);
	d_SDLPP_OSTREAM_WITHOT_FROM_STRING(const point2f&);

	struct rect : public SDL_Rect {
		rect() noexcept;
		rect(int x_, int y_, int w_, int h_) noexcept;
		rect(const point& p, int w_, int h_) noexcept;
		rect(const point& p, const area_type& a) noexcept;
		explicit rect(const area_type& a) noexcept;
		rect(int w_, int h_) noexcept;
		explicit rect(const SDL_Rect& r) noexcept;
		rect& operator=(const SDL_Rect& r) noexcept;

		[[nodiscard]] area_type area() const;
		void area(const area_type& a);

		[[nodiscard]] point offset() const;
		void offset(const point& p);

		[[nodiscard]] bool inside(const point& p) const noexcept;
		[[nodiscard]] bool inside(const point* points, std::size_t size) const noexcept;

		template<typename PointsContainer>
		[[nodiscard]] bool inside(const PointsContainer& points_contanier) const noexcept;

		[[nodiscard]] rect enclose(const point* points, std::size_t size) const noexcept;

		template<typename PointsContainer>
		[[nodiscard]] rect enclose(const PointsContainer& points_contanier) const noexcept;

		[[nodiscard]] bool empty() const noexcept;
		[[nodiscard]] bool equals(const rect& other) const noexcept;
		[[nodiscard]] bool intersects(const rect& other) const noexcept;
		[[nodiscard]] bool intersects(const point& a, const point& b) const noexcept;
		[[nodiscard]] rect intersection(const rect& other) const noexcept;
		[[nodiscard]] std::pair <point, point> intersection(const point& a, const point& b) const noexcept;
		[[nodiscard]] rect union_rect(const rect& other) const noexcept;

		[[nodiscard]] point center() const noexcept;
	};

	d_SDLPP_OSTREAM_WITHOT_FROM_STRING(const rect&);

	[[nodiscard]] inline bool inside(const rect& r, const point& p) noexcept {
		return SDL_TRUE == SDL_PointInRect(&p, &r);
	}

	[[nodiscard]] inline bool empty(const rect& r) noexcept {
		return SDL_TRUE == SDL_RectEmpty(&r);
	}

	[[nodiscard]] inline bool equals(const rect& a, const rect& b) noexcept {
		return SDL_TRUE == SDL_RectEquals(&a, &b);
	}

	[[nodiscard]] inline bool operator==(const rect& a, const rect& b) noexcept {
		return equals(a, b);
	}

	[[nodiscard]] inline bool intersects(const rect& a, const rect& b) noexcept {
		return SDL_HasIntersection(&a, &b);
	}

	[[nodiscard]] inline rect intersection(const rect& a, const rect& b) noexcept {
		rect result;
		SDL_IntersectRect(&a, &b, &result);
		return result;
	}

	// intersection between rect and line
	[[nodiscard]] inline std::pair <point, point> intersection(const rect& r, const point& a, const point& b) noexcept {
		std::pair <point, point> ret;
		int x1 = a.x;
		int y1 = a.y;
		int x2 = b.x;
		int y2 = b.y;

		if (SDL_TRUE == SDL_IntersectRectAndLine(&r, &x1, &y1, &x2, &y2)) {
			ret.first = point(x1, y1);
			ret.second = point(x2, y2);
		}
		return ret;
	}

	// intersection between rect and line
	[[nodiscard]] inline bool intersects(const rect& r, const point& a, const point& b) noexcept {
		int x1 = a.x;
		int y1 = a.y;
		int x2 = b.x;
		int y2 = b.y;

		return (SDL_TRUE == SDL_IntersectRectAndLine(&r, &x1, &y1, &x2, &y2));
	}

	[[nodiscard]] inline rect union_rect(const rect& a, const rect& b) noexcept {
		rect result;
		SDL_UnionRect(&a, &b, &result);
		return result;
	}

	// ==================================================================
	template<typename T>
	struct points_container_traits;

	template<class Allocator>
	struct points_container_traits <std::vector <point, Allocator>> {
		using container = std::vector <point, Allocator>;

		static const point* data(const container& c) {
			return c.data();
		}

		static std::size_t size(const container& c) {
			return c.size();
		}
	};

	template<std::size_t N>
	struct points_container_traits <std::array <point, N>> {
		using container = std::array <point, N>;

		static const point* data(const container& c) {
			return c.data();
		}

		static std::size_t size(const container& c) {
			return c.size();
		}
	};

	[[nodiscard]] inline bool enclose(const point* points, std::size_t size, const rect& clip, rect& result) noexcept {
		return SDL_TRUE == SDL_EnclosePoints(points,
		                                     static_cast <int>(size),
		                                     &clip, &result);
	}

	template<typename PointsContainer>
	[[nodiscard]] inline bool
	enclose(const PointsContainer& points_contanier, const rect& clip, rect& result) noexcept {
		return enclose(points_container_traits <PointsContainer>::data(points_contanier),
		               static_cast <int>(points_container_traits <PointsContainer>::size(points_contanier)),
		               &clip, &result);
	}

	[[nodiscard]] inline bool inside(const rect& clip, const point* points, std::size_t size) noexcept {
		rect r;
		return enclose(points, size, clip, r);
	}

	template<typename PointsContainer>
	[[nodiscard]] inline bool inside(const rect& clip, const PointsContainer& points_contanier) noexcept {
		rect r;
		return enclose(points_contanier, clip, r);
	}
} // ns sdl
// ======================================================================
// Implementation
// ======================================================================
namespace neutrino::sdl {
	inline point::point() noexcept
		: SDL_Point{0, 0} {
	}

	// -------------------------------------------------------------------
	inline point::point(int x_, int y_) noexcept
		: SDL_Point{x_, y_} {
	}

	// -------------------------------------------------------------------
	inline point::point(const SDL_Point& p) noexcept
		: SDL_Point{p.x, p.y} {
	}

	// -------------------------------------------------------------------
	inline point& point::operator=(const SDL_Point& p) noexcept {
		x = p.x;
		y = p.y;
		return *this;
	}
	// ====================================================================
	inline point2f::point2f() noexcept
		: SDL_FPoint{0, 0} {
	}

	// -------------------------------------------------------------------
	inline point2f::point2f(int x_, int y_) noexcept
		: SDL_FPoint{static_cast<float>(x_), static_cast<float>(y_)} {
	}

	inline point2f::point2f(float x_, float y_) noexcept
	: SDL_FPoint{x_, y_} {

	}

	// -------------------------------------------------------------------
	inline point2f::point2f(const SDL_Point& p) noexcept
		: SDL_FPoint{static_cast<float>(p.x), static_cast<float>(p.y)} {
	}

	inline point2f::point2f(const SDL_FPoint& p) noexcept
		: SDL_FPoint{p.x, p.y} {
	}
	// -------------------------------------------------------------------
	inline point2f& point2f::operator=(const SDL_Point& p) noexcept {
		x = static_cast<float>(p.x);
		y = static_cast<float>(p.y);
		return *this;
	}
	// -------------------------------------------------------------------
	inline point2f& point2f::operator=(const SDL_FPoint& p) noexcept {
		x = p.x;
		y = p.y;
		return *this;
	}
	// ========================================================	===========
	inline rect::rect() noexcept
		: SDL_Rect{0, 0, 0, 0} {
	}

	// ------------------------------------------------------------------
	inline rect::rect(int x_, int y_, int w_, int h_) noexcept
		: SDL_Rect{x_, y_, w_, h_} {
	}

	// ------------------------------------------------------------------
	inline rect::rect(const point& p, int w_, int h_) noexcept
		: SDL_Rect{p.x, p.y, w_, h_} {
	}

	inline
	rect::rect(const point& p, const area_type& a) noexcept
		: SDL_Rect{p.x, p.y, (int)a.w, (int)a.h} {
	}

	inline
	rect::rect(const area_type& a) noexcept
		: SDL_Rect{0, 0, (int)a.w, (int)a.h} {
	}

	inline
	rect::rect(int w_, int h_) noexcept
		: SDL_Rect{0, 0, w_, h_} {
	}

	// ------------------------------------------------------------------
	inline rect::rect(const SDL_Rect& r) noexcept
		: SDL_Rect{r.x, r.y, r.w, r.h} {
	}

	// ------------------------------------------------------------------
	inline rect& rect::operator=(const SDL_Rect& r) noexcept {
		x = r.x;
		y = r.y;
		w = r.w;
		h = r.h;
		return *this;
	}

	inline bool rect::inside(const point& p) const noexcept {
		return sdl::inside(*this, p);
	}

	// -------------------------------------------------------------------
	inline bool rect::empty() const noexcept {
		return sdl::empty(*this);
	}

	// -------------------------------------------------------------------
	inline bool rect::equals(const rect& other) const noexcept {
		return sdl::equals(*this, other);
	}

	// -------------------------------------------------------------------
	inline bool rect::intersects(const rect& other) const noexcept {
		return sdl::intersects(*this, other);
	}

	// -------------------------------------------------------------------
	inline bool rect::intersects(const point& a, const point& b) const noexcept {
		return sdl::intersects(*this, a, b);
	}

	// -------------------------------------------------------------------
	inline rect rect::intersection(const rect& other) const noexcept {
		return sdl::intersection(*this, other);
	}

	// -------------------------------------------------------------------
	inline std::pair <point, point> rect::intersection(const point& a, const point& b) const noexcept {
		return sdl::intersection(*this, a, b);
	}

	// -------------------------------------------------------------------
	inline rect rect::union_rect(const rect& other) const noexcept {
		return sdl::union_rect(*this, other);
	}

	// -------------------------------------------------------------------
	inline bool rect::inside(const point* points, std::size_t size) const noexcept {
		return sdl::inside(*this, points, size);
	}

	// -------------------------------------------------------------------
	template<typename PointsContainer>
	inline bool rect::inside(const PointsContainer& points_contanier) const noexcept {
		return sdl::inside(*this, points_contanier);
	}

	// -------------------------------------------------------------------
	inline rect rect::enclose(const point* points, std::size_t size) const noexcept {
		rect rc;
		(void)sdl::enclose(points, size, *this, rc);
		return rc;
	}

	inline
	point rect::center() const noexcept {
		return {x + w / 2, y + h / 2};
	}

	inline
	area_type rect::area() const {
		return {w, h};
	}

	inline
	point rect::offset() const {
		return {x, y};
	}

	inline
	void rect::area(const area_type& a) {
		w = (int)a.w;
		h = (int)a.h;
	}

	inline
	void rect::offset(const point& p) {
		x = p.x;
		y = p.y;
	}

	// -------------------------------------------------------------------
	template<typename PointsContainer>
	inline rect rect::enclose(const PointsContainer& points_contanier) const noexcept {
		rect rc;
		sdl::enclose(points_contanier, *this, rc);
		return rc;
	}

	inline
	area_type::area_type()
		: w(0), h(0) {
	}

	inline
	area_type::area_type(int w_, int h_)
		: w(w_),
		  h(h_) {
	}
} // ns sdl

#endif
