//
// Created by igor on 9/28/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_GEOMETRY_RECT_HH_
#define SDLPP_INCLUDE_SDLPP_GEOMETRY_RECT_HH_

#include <algorithm>
#include <optional>
#include <neutrino/math/math.hh>
#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/geometry/area.hh>

namespace neutrino::sdl::generic {
	template <typename Scalar>
	struct rect_traits;

	template <>
	struct rect_traits<int> : public SDL_Rect {
		using base = SDL_Rect;
		using dim_t = unsigned int;
		rect_traits()
			: SDL_Rect{} {}
	};



	template <>
	struct rect_traits<float> : public SDL_FRect {
		using base = SDL_FRect;
		using dim_t = float;
		rect_traits()
			: SDL_FRect{} {}
	};

	template <typename RectType>
	constexpr bool is_rect_v =  std::is_base_of_v<SDL_Rect, RectType> || std::is_base_of_v<SDL_FRect, RectType>;

	template <typename Scalar>
	struct rect : public rect_traits<Scalar> {

		using dimension_t = typename rect_traits<Scalar>::dim_t;

		rect() noexcept
			: rect_traits<Scalar>{} {}

		template <typename DimT>
		rect(Scalar x_, Scalar y_, DimT w_, DimT h_) noexcept
			: rect_traits<Scalar>{} {
			this->x = x_;
			this->y = y_;
			this->w = static_cast<Scalar>(w_);
			this->h = static_cast<Scalar>(h_);
		}

		template <typename PointT, typename DimT>
		rect(const math::vector<PointT, 2>& p, DimT w_, DimT h_) noexcept
			: rect_traits<Scalar>{} {
			this->x = static_cast<Scalar>(p.x);
			this->y = static_cast<Scalar>(p.y);
			this->w = static_cast<Scalar>(w_);
			this->h = static_cast<Scalar>(h_);
		}

		template <typename PointT, typename DimT>
		rect(const math::vector<PointT, 2>& p, const area_type<DimT>& a) noexcept
			: rect_traits<Scalar>{} {
			this->x = static_cast<Scalar>(p.x);
			this->y = static_cast<Scalar>(p.y);
			this->w = static_cast<Scalar>(a.w);
			this->h = static_cast<Scalar>(a.h);
		}

		template <typename DimT>
		explicit rect(const area_type<DimT>& a) noexcept
			: rect_traits<Scalar>{} {
			this->x = static_cast<Scalar>(0);
			this->y = static_cast<Scalar>(0);
			this->w = static_cast<Scalar>(a.w);
			this->h = static_cast<Scalar>(a.h);
		}

		template <typename DimT>
		rect(DimT w_, DimT h_) noexcept
			: rect_traits<Scalar>{} {
			this->x = static_cast<Scalar>(0);
			this->y = static_cast<Scalar>(0);
			this->w = static_cast<Scalar>(w_);
			this->h = static_cast<Scalar>(h_);
		}

		explicit rect(const SDL_Rect& r) noexcept
			: rect_traits<Scalar>{} {
			this->x = static_cast<Scalar>(r.x);
			this->y = static_cast<Scalar>(r.y);
			this->w = static_cast<Scalar>(r.w);
			this->h = static_cast<Scalar>(r.h);
		}

		explicit rect(const SDL_FRect& r) noexcept
			: rect_traits<Scalar>{} {
			this->x = static_cast<Scalar>(r.x);
			this->y = static_cast<Scalar>(r.y);
			this->w = static_cast<Scalar>(r.w);
			this->h = static_cast<Scalar>(r.h);
		}

		rect& operator=(const SDL_Rect& r) noexcept {
			if (this != &r) {
				this->x = static_cast<Scalar>(r.x);
				this->y = static_cast<Scalar>(r.y);
				this->w = static_cast<Scalar>(r.w);
				this->h = static_cast<Scalar>(r.h);
			}
			return *this;
		}

		rect& operator=(const SDL_FRect& r) noexcept {
			if (this != &r) {
				this->x = static_cast<Scalar>(r.x);
				this->y = static_cast<Scalar>(r.y);
				this->w = static_cast<Scalar>(r.w);
				this->h = static_cast<Scalar>(r.h);
			}
			return *this;
		}

		[[nodiscard]] area_type<dimension_t> area() const {
			return {static_cast<dimension_t>(this->w), static_cast<dimension_t>(this->h)};
		}

		template <typename DimT>
		void area(const area_type<DimT>& a) {
			this->w = static_cast<Scalar>(a.w);
			this->h = static_cast<Scalar>(a.h);
		}

		[[nodiscard]] math::vector<Scalar, 2> offset() const {
			return {this->x, this->y};
		}

		template <typename PointT>
		void offset(const math::vector<PointT, 2>& p) {
			this->x = static_cast<Scalar>(p.x);
			this->y = static_cast<Scalar>(p.y);
		}

		template <typename PointT>
		[[nodiscard]] bool inside(const math::vector<PointT, 2>& p) const noexcept {
			auto px = static_cast<Scalar>(p.x);
			auto py = static_cast<Scalar>(p.y);
			return ( (px >= this->x) && (px < (this->x + this->w)) &&
					 (py >= this->y) && (py < (this->y + this->h)) );
		}

		template <typename PointsIterator>
		[[nodiscard]] bool all_inside(PointsIterator&& points_begin, PointsIterator&& points_end) const noexcept {
			return std::all_of(std::forward<PointsIterator>(points_begin),
			    			   std::forward<PointsIterator>(points_end),
			    			       [this] (const auto& p) {
				return inside(p);
			} );
		}

		template <typename PointsIterator>
		[[nodiscard]] bool any_inside(PointsIterator&& points_begin, PointsIterator&& points_end) const noexcept {
			return std::any_of(std::forward<PointsIterator>(points_begin),
							   std::forward<PointsIterator>(points_end),
							   [this] (const auto& p) {
								 return inside(p);
							   } );
		}

		[[nodiscard]] math::vector<Scalar, 2> center() const noexcept {
			return {this->x + static_cast<Scalar>(this->w/2), this->y + static_cast<Scalar>(this->h/2)};
		}

		[[nodiscard]] Scalar left() const noexcept {
			return this->x;
		}

		[[nodiscard]] Scalar right() const noexcept {
			return this->x + this->w;
		}

		[[nodiscard]] Scalar top() const noexcept {
			return this->y;
		}

		[[nodiscard]] Scalar bottom() const noexcept {
			return this->y + this->h;
		}

		template <typename RectType, class = std::enable_if<is_rect_v<RectType>>>
		[[nodiscard]] bool contains(const RectType& r) const noexcept {
			const auto box_left = static_cast<Scalar>(r.x);
			const auto box_right = static_cast<Scalar>(r.x + r.w);
			const auto box_top = static_cast<Scalar>(r.y);
			const auto box_bottom = static_cast<Scalar>(r.y + r.h);

			return left() <= box_left && box_right() <= right() &&
				   top() <= box_top && box_bottom() <= bottom();
		}

		template <typename RectType, class = std::enable_if<is_rect_v<RectType>>>
		[[nodiscard]] bool intersects(const RectType& r) const noexcept
		{
			const auto box_left = static_cast<Scalar>(r.x);
			const auto box_right = static_cast<Scalar>(r.x + r.w);
			const auto box_top = static_cast<Scalar>(r.y);
			const auto box_bottom = static_cast<Scalar>(r.y + r.h);

			return !(left() >= box_right || right() <= box_left ||
					 top() >= box_bottom || bottom() <= box_top);
		}

		template <typename RectType, class = std::enable_if<is_rect_v<RectType>>>
		[[nodiscard]] std::optional<rect<Scalar>> intersection(const RectType& r) const noexcept {
			const auto x1 = left();
			const auto y1 = top();
			const auto x2 = right();
			const auto y2 = bottom();

			const auto x3 = static_cast<Scalar>(r.x);
			const auto y3 = static_cast<Scalar>(r.y);
			const auto x4 = static_cast<Scalar>(r.x + r.w);
			const auto y4 = static_cast<Scalar>(r.y + r.h);
			if ( !(x1 >= x4 || x2 <= x3 || y1 >= y4 || y2 <= y3) ) {
				auto x5 = std::max(x1, x3);
				auto y5 = std::max(y1, y3);
				auto x6 = std::min(x2, x4);
				auto y6 = std::min(y2, y4);
				return rect<Scalar>(x5, y5, static_cast<Scalar>(x6 - x5), static_cast<Scalar>(y6 - y5));
			}
			return std::nullopt;
		}

		[[nodiscard]] bool empty() const noexcept {
			return this->w > 0 && this->h > 0;
		}

	};
}

#endif
