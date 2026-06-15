//
// Created by igor on 27/05/2026.
//

#pragma once

#include <cmath>
#include <type_traits>
#include <ostream>
#include <simplex/detail/export.hh>

namespace simplex {
    // ===================================================================
    // Global UI scale.
    //
    // Set once per scene (typically at on_enter) from
    // services::window().display_scale(). All subsequent dp -> physical
    // pixel conversions multiply by this value.
    // ===================================================================

    namespace detail {
         SIMPLEX_EXPORT float& current_scale();
    }

    inline void set_scale(float s) noexcept { detail::current_scale() = s; }
    [[nodiscard]] inline float scale() noexcept { return detail::current_scale(); }

    // ===================================================================
    // Strong type for a quantity in design pixels.
    //
    // Conversion to physical pixels is *explicit* via .px() / .pxi().
    // There is no implicit dp <-> float conversion in either direction —
    // that is the entire point of the type. Arithmetic between dp values
    // is allowed; arithmetic between dp and a unitless scalar (float / int)
    // is allowed; arithmetic between dp and raw physical pixels is not.
    //
    // The escape hatch dp::from_px() exists for the rare case where you
    // need to round-trip a value Nuklear gave you (window bounds, command
    // rects) back into dp-space for further design-time math.
    // ===================================================================

    class dp {
        public:
            constexpr dp() = default;

            constexpr explicit dp(float design_px) noexcept
                : value_(design_px) {
            }

            [[nodiscard]] explicit constexpr operator float() const noexcept { return value_; }

            // Escape hatch: build a dp from an already-scaled physical-pixel
            // value (e.g. coordinates returned by Nuklear).
            [[nodiscard]] static dp from_px(float physical_px) noexcept {
                return dp{physical_px / detail::current_scale()};
            }

            [[nodiscard]] constexpr float design() const noexcept { return value_; }

            // Convert to physical pixels. Rounded to nearest integer so bitmap
            // UI stays crisp — sub-pixel widget sizes produce visible jitter,
            // especially with NEAREST-filtered glyph atlases.
            [[nodiscard]] float px() const noexcept {
                return std::round(value_ * detail::current_scale());
            }

            [[nodiscard]] int pxi() const noexcept {
                return static_cast <int>(px());
            }

            // dp + dp, dp - dp.
            friend constexpr dp operator+(dp a, dp b) noexcept { return dp{a.value_ + b.value_}; }
            friend constexpr dp operator-(dp a, dp b) noexcept { return dp{a.value_ - b.value_}; }
            friend constexpr dp operator-(dp a) noexcept { return dp{-a.value_}; }

            // dp * unitless scalar, scalar * dp, dp / unitless scalar.
            template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
            friend constexpr dp operator*(dp a, T k) noexcept {
                return dp{a.value_ * static_cast<float>(k)};
            }

            template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
            friend constexpr dp operator*(T k, dp a) noexcept {
                return dp{static_cast<float>(k) * a.value_};
            }

            template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
            friend constexpr dp operator/(dp a, T k) noexcept {
                return dp{a.value_ / static_cast<float>(k)};
            }

            constexpr dp& operator+=(dp o) noexcept {
                value_ += o.value_;
                return *this;
            }

            constexpr dp& operator-=(dp o) noexcept {
                value_ -= o.value_;
                return *this;
            }

            template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
            constexpr dp& operator*=(T k) noexcept {
                value_ *= static_cast<float>(k);
                return *this;
            }

            template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
            constexpr dp& operator/=(T k) noexcept {
                value_ /= static_cast<float>(k);
                return *this;
            }

            friend std::ostream& operator<<(std::ostream& os, dp d) {
                return os << d.design() << "_dp";
            }

            friend constexpr auto operator<=>(dp, dp) = default;

        private:
            float value_{};
    };

    // User-defined literals: 24_dp / 24.5_dp.
    //
    // Lives in an INLINE sub-namespace so:
    //   * Code inside `simplex` sees the
    //     literal without any using-declaration.
    //   * Code outside opts in
    //     with `using namespace simplex::literals;` — the same pattern
    //     `std::chrono::literals` uses.
    inline namespace literals {
        constexpr dp operator""_dp(long double v) noexcept { return dp{static_cast <float>(v)}; }
        constexpr dp operator""_dp(unsigned long long v) noexcept { return dp{static_cast <float>(v)}; }
    }
} // namespace bane::ui
