---
sidebar_position: 1
---

# Introduction

SDL++ is a modern C++20 wrapper library for SDL3 (Simple DirectMedia Layer version 3) that provides type-safe, RAII-managed interfaces to SDL's multimedia capabilities.

## Why SDL++?

SDL3 is a powerful cross-platform library, but its C API can be error-prone and verbose in C++. SDL++ addresses these issues by providing:

- **Type Safety**: Strong typing with C++20 concepts prevents common mistakes at compile time
- **Automatic Resource Management**: RAII wrappers ensure all resources are properly cleaned up
- **Modern C++ Idioms**: Uses `std::expected` for errors, `std::chrono` for time, concepts for generic programming
- **Zero Runtime Overhead**: Template-based design ensures optimal performance
- **Comprehensive Coverage**: Wraps most SDL3 functionality with consistent, intuitive APIs

## Key Features

### 🛡️ Safety First
Every SDL resource is wrapped in a smart pointer with automatic cleanup. No more manual `SDL_Destroy*` calls or resource leaks.

### 🚀 Modern Error Handling
```cpp
auto window = sdlpp::window::create("My App", 800, 600);
if (!window) {
    std::cerr << "Error: " << window.error() << "\n";
    return 1;
}
// Use window safely - it will be destroyed automatically
```

### 🎯 Type-Safe APIs
```cpp
// Compile-time validation with concepts
template<point_like P>
void set_position(const P& pos);

// Works with any point-like type
window->set_position(MyVector2{100, 200});
```

### ⚡ Zero-Cost Abstractions
SDL++ adds no runtime overhead. The generated assembly is identical to hand-written C code calling SDL directly.

### 🔧 Flexible Design
Integration with existing codebases is easy. Use your own math libraries through concepts, and mix SDL++ with raw SDL calls when needed.

## Supported Platforms

SDL++ supports all platforms that SDL3 supports:

<div style={{display: 'flex', gap: '0.5rem', flexWrap: 'wrap', marginTop: '1rem'}}>
  <span className="platform-badge windows">Windows</span>
  <span className="platform-badge linux">Linux</span>
  <span className="platform-badge macos">macOS</span>
  <span className="platform-badge android">Android</span>
  <span className="platform-badge ios">iOS</span>
</div>

## Requirements

- **C++20** capable compiler (GCC 10+, Clang 10+, MSVC 2019+)
- **CMake 3.31** or higher
- **SDL3** (automatically fetched if not found)

## What's Next?

Ready to get started? Check out the [Installation Guide](getting-started/installation) to set up SDL++ in your project.

Looking for examples? Visit our [Examples](../examples) section for practical demonstrations.

## Getting Help

- 📖 Read the [API Reference](api/overview) for detailed documentation
- 💬 Join our [Discord Server](https://discord.gg/sdlpp) for community support
- 🐛 Report issues on [GitHub](https://github.com/sdlpp/sdlpp/issues)
- 📧 Contact the maintainers at support@sdlpp.dev