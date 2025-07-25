//
// Created by igor on 7/13/25.
//
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include <cstdlib>

int main(int argc, char** argv) {
    // Set SDL to use dummy drivers for headless testing
    // This prevents screen flickering and audio during tests
    setenv("SDL_VIDEODRIVER", "dummy", 1);
    setenv("SDL_AUDIODRIVER", "dummy", 1);
    
    // Run doctest
    return doctest::Context(argc, argv).run();
}

