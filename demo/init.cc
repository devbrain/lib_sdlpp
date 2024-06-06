//
// Created by igor on 1/3/24.
//

#include "sdlpp/sdlpp.hh"

int main([[maybe_unused]] int argc,[[maybe_unused]] char* argv[]) {
  neutrino::sdl::system init(neutrino::sdl::init_flags::VIDEO, neutrino::sdl::init_flags::EVENTS, neutrino::sdl::init_flags::TIMER);
  return 0;
}

