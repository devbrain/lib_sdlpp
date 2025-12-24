//
// Created by igor on 7/13/25.
//
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
#include <cstdlib>

#ifdef _WIN32
#include <stdlib.h>
inline int portable_setenv(const char* name, const char* value) {
    return _putenv_s(name, value);
}
#else
inline int portable_setenv(const char* name, const char* value) {
    return setenv(name, value, 1);
}
#endif

int main(int argc, char** argv) {
    // Set SDL to use dummy drivers for headless testing
    // This prevents screen flickering and audio during tests
    portable_setenv("SDL_VIDEODRIVER", "dummy");
    portable_setenv("SDL_AUDIODRIVER", "dummy");

    // Run doctest
    return doctest::Context(argc, argv).run();
}

