---
sidebar_position: 1
---

# Installation

This guide will help you install SDL++ and set up your development environment.

## Prerequisites

Before installing SDL++, ensure you have:

- **C++20 compiler**:
  - GCC 10 or later
  - Clang 10 or later
  - MSVC 2019 (16.8) or later
- **CMake** 3.31 or higher
- **Git** for cloning the repository

## Installation Methods

### Method 1: CMake FetchContent (Recommended)

The easiest way to use SDL++ is through CMake's FetchContent module. Add this to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    sdlpp
    GIT_REPOSITORY https://github.com/sdlpp/sdlpp.git
    GIT_TAG        main  # or specific version tag
)

FetchContent_MakeAvailable(sdlpp)

# Link SDL++ to your target
target_link_libraries(your_target PRIVATE sdlpp)
```

This will automatically download and build SDL++ along with SDL3.

### Method 2: Git Submodule

Add SDL++ as a submodule to your project:

```bash
git submodule add https://github.com/sdlpp/sdlpp.git external/sdlpp
git submodule update --init --recursive
```

Then in your `CMakeLists.txt`:

```cmake
add_subdirectory(external/sdlpp)
target_link_libraries(your_target PRIVATE sdlpp)
```

### Method 3: System Installation

Clone and install SDL++ system-wide:

```bash
git clone https://github.com/sdlpp/sdlpp.git
cd sdlpp
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
sudo cmake --install .
```

Then in your project:

```cmake
find_package(sdlpp REQUIRED)
target_link_libraries(your_target PRIVATE sdlpp::sdlpp)
```

## Build Options

SDL++ provides several CMake options:

| Option | Default | Description |
|--------|---------|-------------|
| `SDLPP_BUILD_TESTS` | `ON` | Build unit tests |
| `SDLPP_BUILD_EXAMPLES` | `ON` | Build example programs |
| `SDLPP_USE_SYSTEM_SDL3` | `OFF` | Use system SDL3 instead of fetching |
| `SDLPP_ENABLE_ASAN` | `OFF` | Enable AddressSanitizer |
| `SDLPP_ENABLE_UBSAN` | `OFF` | Enable UndefinedBehaviorSanitizer |

Example:
```cmake
cmake .. -DSDLPP_BUILD_TESTS=OFF -DSDLPP_BUILD_EXAMPLES=ON
```

## Platform-Specific Notes

### Windows

On Windows with MSVC, ensure you're using C++20:

```cmake
target_compile_features(your_target PRIVATE cxx_std_20)
```

### Linux

Install development packages for better SDL3 functionality:

```bash
# Ubuntu/Debian
sudo apt-get install libx11-dev libxext-dev libwayland-dev \
    libpulse-dev libasound2-dev libgl1-mesa-dev

# Fedora
sudo dnf install libX11-devel libXext-devel wayland-devel \
    pulseaudio-libs-devel alsa-lib-devel mesa-libGL-devel
```

### macOS

On macOS, you may need to install Xcode command line tools:

```bash
xcode-select --install
```

## Verifying Installation

Create a simple test program to verify SDL++ is working:

```cpp title="test.cpp"
#include <sdlpp/core/core.hh>
#include <iostream>

int main() {
    auto init = sdlpp::init::create(sdlpp::init_flags::video);
    if (!init) {
        std::cerr << "SDL++ initialization failed: " << init.error() << "\n";
        return 1;
    }
    
    std::cout << "SDL++ initialized successfully!\n";
    std::cout << "SDL version: " << sdlpp::version_info::runtime() << "\n";
    
    return 0;
}
```

Build and run:

```bash
g++ -std=c++20 test.cpp -lsdlpp -lSDL3
./a.out
```

## Troubleshooting

### "SDL3 not found"

If CMake can't find SDL3:
- Let SDL++ fetch it automatically (default behavior)
- Or install SDL3 manually and set `SDL3_DIR` in CMake

### Compiler errors about C++20 features

Ensure your compiler supports C++20:
```bash
g++ --version   # Should be 10+
clang --version # Should be 10+
```

### Linking errors

Make sure you're linking both SDL++ and SDL3:
```cmake
target_link_libraries(your_target PRIVATE sdlpp SDL3::SDL3)
```

## Next Steps

Now that SDL++ is installed, check out the [Quick Start Guide](quick-start) to create your first SDL++ application!