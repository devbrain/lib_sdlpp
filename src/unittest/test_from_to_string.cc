//
// Created by igor on 6/18/24.
//
#include <doctest/doctest.h>
#include <sdlpp/events/event_types.hh>

TEST_CASE("test from/to string") {
	using namespace neutrino::sdl;
	auto x = to_string (scancode::RIGHT);
	auto res = from_string<scancode>(x);
	REQUIRE(res);
	REQUIRE_EQ(*res, scancode::RIGHT);
}