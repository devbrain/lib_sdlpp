//
// Created by igor on 6/5/24.
//
#include <doctest/doctest.h>
#include <sdlpp/detail/ttf_helper.hh>
#include "sdlpp/video/ttf.hh"
#include <iostream>

static int utf8_func(const char* x, int n) {
	int a = (int)(x[0] & 0xFF);
	int b = (int)(x[1] & 0xFF);
	return (int)(a*n + b);
}
static int ucs_func(const Uint16* x, int n) {
	int a = (int)(x[0] & 0xFF);
	int b = (int)(x[1] & 0xFF);
	return (int)(a*n + b);
}

namespace neutrino::sdl::detail {
	d_TTF_CALLER_PROXY(ttf_caller, utf8_func, ucs_func);
}

template <typename T>
static constexpr bool is_string_impl ([[maybe_unused]] T arg) {
	//std::cout << type_name_v<T> << std::endl;
	return neutrino::sdl::detail::string_traits<T>::is_string;
}

template <typename ... T>
static constexpr bool is_string (T... arg) {
	return is_string_impl (std::forward<T>(arg)...);
}

template <typename T>
static constexpr bool is_utf8_impl ([[maybe_unused]] T arg) {
	return neutrino::sdl::detail::string_traits<T>::is_utf8;
}

template <typename ... T>
static constexpr bool is_utf8 (T... arg) {
	return is_utf8_impl (std::forward<T>(arg)...);
}

template <typename T>
static constexpr bool is_ucs_impl ([[maybe_unused]] T arg) {
	return neutrino::sdl::detail::string_traits<T>::is_ucs;
}

template <typename ... T>
static constexpr bool is_ucs (T... arg) {
	return is_ucs_impl (std::forward<T>(arg)...);;
}


TEST_CASE("Test string traits") {
	REQUIRE(is_string ("a"));
	REQUIRE(is_string (L"a"));
	REQUIRE(is_string (std::string("a")));
	REQUIRE(is_string (std::wstring (L"a")));

	REQUIRE(is_utf8 ("a"));
	REQUIRE(!is_utf8 (L"a"));
	REQUIRE(is_utf8 (std::string("a")));
	REQUIRE(!is_utf8 (std::wstring (L"a")));

	REQUIRE(!is_ucs ("a"));
	REQUIRE(is_ucs (L"a"));
	REQUIRE(!is_ucs (std::string("a")));
	REQUIRE(is_ucs (std::wstring (L"a")));
}

TEST_CASE("test caller") {
	using namespace neutrino::sdl::detail;
	int a = ttf_caller::call ("12", 3);
	int a1 = ttf_caller::call (std::string("12"), 3);
	int a2 = ttf_caller::call (std::string_view("12"), 3);
	int b = ttf_caller::call (L"12", 3);
	int b1 = ttf_caller::call (std::wstring (L"12"), 3);
	int b2 = ttf_caller::call (std::wstring_view (L"12"), 3);
	REQUIRE_EQ(a, 197);
	REQUIRE_EQ(a, b);
	REQUIRE_EQ(a, a1);
	REQUIRE_EQ(a, a2);
	REQUIRE_EQ(b, b1);
	REQUIRE_EQ(b, b2);

}
