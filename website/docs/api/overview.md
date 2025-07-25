---
sidebar_position: 1
---

# SDL++ API Reference

SDL++ is a modern C++20 wrapper for SDL3 that provides type-safe, RAII-managed interfaces to SDL's multimedia capabilities.

## Module Organization

### Core Systems

#### [Core Module](/docs/api/core/initialization)
Foundation functionality required by all SDL++ applications.

- **[Initialization](/docs/api/core/initialization)** - SDL subsystem initialization
- **[Error Handling](/docs/api/core/error)** - Type-safe error handling with `expected`
- **[Timer & Frame Rate](/docs/api/core/timer)** - High-resolution timing and frame control
- **[Version](/docs/api/core/version)** - Version information and feature detection
- **Logging** - Structured logging system *(coming soon)*
- **Power State** - Battery and power management *(coming soon)*

#### [Application Framework](/docs/api/app/application-framework)
High-level application structure and lifecycle management.

- **Application Base** - Main application class and lifecycle
- **Game Application** - Specialized base for games
- **Scene Management** - Scene-based application structure
- **Fixed Timestep** - Deterministic game loops

### Graphics & Rendering

#### [Video Module](/docs/api/video/window)
Everything related to graphics and rendering.

- **[Window Management](/docs/api/video/window)** - Window creation and control
- **[2D Renderer](/docs/api/video/renderer)** - Hardware-accelerated 2D graphics
- **[Surfaces](/docs/api/video/surface)** - Software rendering and pixel manipulation
- **[Textures](/docs/api/video/texture)** - GPU texture management
- **[OpenGL Support](/docs/api/video/gl)** - OpenGL context management
- **GPU API** - Modern GPU compute and graphics *(coming soon)*
- **Display Info** - Monitor and display mode queries *(coming soon)*
- **Camera** - Camera capture support *(coming soon)*

### Audio Systems

#### [Audio Module](/docs/api/audio/audio-playback)
Audio playback and recording functionality.

- **[Audio Playback](/docs/api/audio/audio-playback)** - Sound and music playback
- **Audio Streams** - Streaming audio with format conversion
- **Audio Devices** - Device enumeration and management
- **Audio Recording** - Microphone input capture *(coming soon)*

### Input Handling

#### [Input Module](/docs/api/input/keyboard)
Comprehensive input device support.

- **[Keyboard](/docs/api/input/keyboard)** - Keyboard input and text entry
- **[Mouse](/docs/api/input/mouse)** - Mouse input and cursor management
- **[Gamepad](/docs/api/input/gamepad)** - Modern game controller support
- **Joystick** - Low-level joystick access *(coming soon)*
- **Touch** - Multi-touch input handling *(coming soon)*
- **Pen/Stylus** - Pressure-sensitive input *(coming soon)*
- **Sensors** - Accelerometer and gyroscope *(coming soon)*
- **Haptic** - Force feedback devices *(coming soon)*

### Event System

#### [Events Module](/docs/api/events/event-queue)
Unified event handling system.

- **[Event Queue](/docs/api/events/event-queue)** - Main event processing
- **Event Types** - All SDL event types *(coming soon)*
- **Custom Events** - User-defined events *(coming soon)*

### User Interface

#### [UI Module](/docs/api/ui/message-box)
Native UI elements and system integration.

- **[Message Boxes](/docs/api/ui/message-box)** - Native alert dialogs
- **[Clipboard](/docs/api/ui/clipboard)** - System clipboard access
- **File Dialogs** - Open/Save file dialogs *(coming soon)*
- **System Tray** - System tray icons and menus *(coming soon)*

### Configuration

#### [Config Module](/docs/api/config/hints)
Runtime configuration and settings.

- **[Hints System](/docs/api/config/hints)** - SDL behavior configuration
- **[Properties](/docs/api/config/properties)** - Key-value storage system (SDL 3.2.0+)

### Utilities

#### [Utility Module](/docs/api/utility/geometry)
Helper classes and utilities.

- **[Geometry](/docs/api/utility/geometry)** - Points, sizes, rectangles with concepts
- **Dimensions** - Type-safe dimension types *(coming soon)*
- **GUID** - Globally unique identifiers *(coming soon)*
- **Color** - Color manipulation utilities *(coming soon)*

### System Integration

#### [System Module](/docs/api/system/filesystem)
Platform-specific functionality.

- **[Filesystem](/docs/api/system/filesystem)** - File I/O operations
- **Platform Detection** - OS and device detection *(coming soon)*
- **CPU Info** - Processor feature detection *(coming soon)*
- **Process Management** - Launch and control processes *(coming soon)*
- **Locale** - Localization support *(coming soon)*
- **Shared Libraries** - Dynamic library loading *(coming soon)*

### I/O Operations

#### I/O Module
File and data I/O operations.

- **IOStream** - SDL IOStream wrapper *(coming soon)*
- **Async I/O** - Asynchronous file operations *(coming soon)*
- **Storage** - Persistent data storage *(coming soon)*

## Getting Started

1. Start with the [Initialization](/docs/api/core/initialization) guide
2. Create a [Window](/docs/api/video/window) and [Renderer](/docs/api/video/renderer)
3. Handle [Events](/docs/api/events/event-queue) and [Input](/docs/api/input/keyboard)
4. Explore other modules as needed

## Design Principles

### RAII Resource Management
All SDL resources are automatically cleaned up when objects go out of scope.

```cpp
{
    sdlpp::window window("Title", 800, 600);
    sdlpp::renderer renderer(window);
    // Resources automatically cleaned up
}
```

### Type Safety with Concepts
C++20 concepts ensure compile-time type checking.

```cpp
template<sdlpp::point_like P>
void draw_at(const P& point) {
    // Works with any type that has x and y members
}
```

### Error Handling with Expected
No exceptions for SDL errors - clear error propagation.

```cpp
auto result = sdlpp::window::create("Title", 800, 600);
if (!result) {
    std::cerr << "Error: " << result.error() << std::endl;
}
```

### Zero-Cost Abstractions
Template-based design ensures no runtime overhead compared to raw SDL.

## Platform Support

SDL++ supports all platforms that SDL3 supports:
- Windows (7+)
- macOS (10.9+)  
- Linux (X11, Wayland)
- iOS
- Android
- Web (Emscripten)

## Version Requirements

- **SDL**: 3.0.0 or higher (some features require newer versions)
- **C++ Compiler**: C++20 support required
  - GCC 10+
  - Clang 11+
  - MSVC 2019+ (v16.8+)
- **CMake**: 3.16 or higher

## Documentation Coverage

This documentation covers the most commonly used SDL++ modules. Additional documentation for specialized modules is being added regularly. Check back for updates or contribute to the documentation effort!

### Currently Documented
✅ Core (Initialization, Error, Timer, Version)  
✅ Application Framework  
✅ Video (Window, Renderer, Surface, Texture, OpenGL)  
✅ Audio (Playback)  
✅ Events (Event Queue)  
✅ Input (Keyboard, Mouse, Gamepad)  
✅ Config (Hints, Properties)  
✅ UI (Message Box, Clipboard)  
✅ Utility (Geometry)  
✅ System (Filesystem)

### Coming Soon
🚧 Additional Input devices (Joystick, Touch, Sensors)  
🚧 Advanced Audio (Recording, Effects)  
🚧 GPU API  
🚧 I/O Module (Async I/O, Streams)  
🚧 Complete System module  
🚧 More examples and tutorials

## Next Steps

- Explore specific subsystems in detail
- Check out the [examples](/docs/examples) for practical usage
- Read the [migration guide](/docs/getting-started/migration-guide) if coming from SDL2
- Join the community to get help and share your projects