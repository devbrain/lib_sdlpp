//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <type_traits>
#include <cmath>
#include <string>

#include "sdlpp/utility/dimension.hh"

using namespace sdlpp;

// Custom dimension type that satisfies the concept
template<typename T>
struct custom_dimension {
    T val;
    
    constexpr custom_dimension(T v) : val(std::max(T{0}, v)) {}
    constexpr T value() const noexcept { return val; }
    constexpr bool is_zero() const noexcept { return val == 0; }
    constexpr bool is_positive() const noexcept { return val > 0; }
};

// Custom dimensions type
template<typename T>
struct custom_dimensions {
    custom_dimension<T> width;
    custom_dimension<T> height;
    
    constexpr custom_dimensions(T w, T h) : width(w), height(h) {}
    constexpr T area() const { return width.value() * height.value(); }
    constexpr bool is_empty() const { return width.is_zero() || height.is_zero(); }
    constexpr bool is_valid() const { return width.is_positive() && height.is_positive(); }
};

TEST_SUITE("dimension_concepts") {
    TEST_CASE("concept satisfaction") {
        SUBCASE("dimensional concept") {
            static_assert(dimensional<int>);
            static_assert(dimensional<float>);
            static_assert(dimensional<double>);
            static_assert(!dimensional<bool>);
            static_assert(!dimensional<std::string>);
        }
        
        SUBCASE("non_negative_dimension concept") {
            static_assert(non_negative_dimension<dimension<int>>);
            static_assert(non_negative_dimension<dimension<float>>);
            static_assert(non_negative_dimension<custom_dimension<int>>);
            static_assert(!non_negative_dimension<int>);
        }
        
        SUBCASE("coordinate_like concept") {
            static_assert(coordinate_like<coordinate<int>>);
            static_assert(coordinate_like<coordinate<float>>);
            static_assert(!coordinate_like<dimension<int>>);  // dimension doesn't have .value member
        }
        
        SUBCASE("dimensions_like concept") {
            static_assert(dimensions_like<dimensions<int>>);
            static_assert(dimensions_like<dimensions<float>>);
            static_assert(dimensions_like<custom_dimensions<int>>);
            static_assert(!dimensions_like<int>);
        }
        
        SUBCASE("position_like concept") {
            static_assert(position_like<position<int>>);
            static_assert(position_like<position<float>>);
            static_assert(!position_like<dimensions<int>>);
        }
    }
    
    TEST_CASE("generic functions with concepts") {
        SUBCASE("are_valid_dimensions") {
            dimensions<int> valid_dims(100, 200);
            CHECK(are_valid_dimensions(valid_dims));
            
            dimensions<int> invalid_dims(0, 200);
            CHECK(!are_valid_dimensions(invalid_dims));
            
            // Works with custom type too
            custom_dimensions<int> custom(50, 75);
            CHECK(are_valid_dimensions(custom));
        }
        
        SUBCASE("get_area") {
            dimensions<int> dims(10, 20);
            CHECK(get_area(dims) == 200);
            
            custom_dimensions<float> custom(5.5f, 4.0f);
            CHECK(get_area(custom) == doctest::Approx(22.0f));
        }
        
        SUBCASE("to_sdl_dimensions") {
            dimensions<float> fdims(100.5f, 200.7f);
            auto [w, h] = to_sdl_dimensions(fdims);
            CHECK(w == 100);
            CHECK(h == 200);
        }
        
        SUBCASE("is_positive_dimension") {
            dimension<int> positive(100);
            CHECK(is_positive_dimension(positive));
            
            dimension<int> zero(0);
            CHECK(!is_positive_dimension(zero));
            
            custom_dimension<int> custom_pos(50);
            CHECK(is_positive_dimension(custom_pos));
        }
        
        SUBCASE("make_dimensions_from") {
            dimension<int> w(100);
            dimension<float> h(200.5f);
            
            auto dims = make_dimensions_from(w, h);
            // Result type is float (common_type of int and float)
            static_assert(std::is_same_v<decltype(dims), dimensions<float>>);
            CHECK(dims.width.value() == 100.0f);
            CHECK(dims.height.value() == 200.5f);
        }
        
        SUBCASE("make_position_from") {
            coordinate<int> x(-100);
            coordinate<float> y(50.5f);
            
            auto pos = make_position_from(x, y);
            static_assert(std::is_same_v<decltype(pos), position<float>>);
            CHECK(pos.x.value == -100.0f);
            CHECK(pos.y.value == 50.5f);
        }
    }
    
    TEST_CASE("concept-based function overloading") {
        // We can create functions that work with any type satisfying our concepts
        auto compute_diagonal = []<typename D>(const D& dims) -> double
            requires dimensions_like<D> {
            auto w = static_cast<double>(dims.width.value());
            auto h = static_cast<double>(dims.height.value());
            return std::sqrt(w * w + h * h);
        };
        
        dimensions<int> int_dims(3, 4);
        CHECK(compute_diagonal(int_dims) == doctest::Approx(5.0));
        
        custom_dimensions<float> custom_dims(3.0f, 4.0f);
        CHECK(compute_diagonal(custom_dims) == doctest::Approx(5.0));
    }
    
    TEST_CASE("constrained templates") {
        // Function that only accepts valid dimensions
        auto double_area = []<typename D>(const D& dims) 
            requires dimensions_like<D> {
            if (!are_valid_dimensions(dims)) {
                return decltype(get_area(dims)){0};
            }
            return get_area(dims) * 2;
        };
        
        dimensions<int> valid(10, 20);
        CHECK(double_area(valid) == 400);
        
        dimensions<int> invalid(0, 20);
        CHECK(double_area(invalid) == 0);  // Returns 0 for invalid dimensions
    }
    
    TEST_CASE("type safety with concepts") {
        SUBCASE("dimension cannot be negative") {
            dimension<int> d(-100);
            CHECK(d.value() == 0);  // Clamped to 0
            
            custom_dimension<int> cd(-50);
            CHECK(cd.value() == 0);  // Also clamped
        }
        
        SUBCASE("coordinates can be negative") {
            coordinate<int> c(-100);
            CHECK(c.value == -100);  // Preserves negative
        }
        
        SUBCASE("mixing types with common_type") {
            dimension<int> int_dim(100);
            dimension<double> double_dim(200.5);
            
            auto mixed = make_dimensions_from(int_dim, double_dim);
            static_assert(std::is_same_v<decltype(mixed.width.value()), double>);
            CHECK(mixed.width.value() == 100.0);
            CHECK(mixed.height.value() == 200.5);
        }
    }
}