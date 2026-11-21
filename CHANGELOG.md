# Changelog

All notable changes to SDL++ will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

#### Typed coordinate spaces
- Added `utility/coordinate_spaces.hh`: coordinates tagged with their space (`display`,
  `window`, `render`, `output`, `texel`, `normalized`, or a library's own) and kind (`pos`,
  `delta`, `extent`, `area`), with an algebra that only compiles meaningful operations
  (`pos - pos = delta`, `pos + delta = pos`; no `pos + pos`, nothing across spaces). Int
  widens to float implicitly within a space; float to int is explicit (`round_to_int`,
  `floor_to_int`, `ceil_to_int`). The float types have SDL_FPoint / SDL_FRect's layout.
- Added typed conversions: `renderer::to_render` / `to_window` for window positions and
  deltas (a delta converts with the scale only), `renderer::to_render` / `to_output` for
  output pixels, `renderer::get_logical_presentation_rect()`; `window::to_window` for
  normalized and display positions, `window::to_display`.

#### Application Framework
- Added comprehensive SDL3 application model support (`app/*.hh`)
  - `app.hh` - Base application interface and class with automatic window/renderer management
  - `app_impl.hh` - SDL3 callback implementations (SDL_AppInit, SDL_AppIterate, etc.)
  - `app_builder.hh` - Functional-style application builder with method chaining
  - `stateful_app.hh` - Type-safe state management for applications
  - `scene_app.hh` - Scene stack management for menu-driven applications
  - `game_app.hh` - Fixed timestep game loop with interpolation
  - `app_types.hh` - SDL_AppResult wrapper types
- Added convenience macros: `SDLPP_MAIN`, `SDLPP_MAIN_WITH_CONFIG`
- Added support for multiple application paradigms:
  - Class-based with virtual method overrides
  - Function-based with callbacks
  - Builder pattern with lambdas
  - Scene-based with stack management
- Added built-in timing, FPS counter, and delta time calculation
- Added performance monitoring utilities for games

#### I/O and Storage
- Added `iostream.hh` - Comprehensive IOStream wrapper for file and memory I/O
- Added `async_io.hh` - Asynchronous file loading support
- Added `io_common.hh` - Type-safe `file_mode` enum replacing string modes
- Added `storage.hh` - Persistent storage API for user preferences
- All I/O APIs now support `std::filesystem::path`

#### System Information
- Added `cpu.hh` - CPU feature detection and SIMD-aligned memory allocation
- Added `cpu_dispatch.hh` - Runtime CPU feature dispatch utilities
- Added `platform.hh` - Enhanced platform detection with device types
- Added `intrinsics.hh` - Portable CPU intrinsics (memory barriers, prefetch)
- Added platform-specific namespaces: `android`, `ios`, `linux_platform`, `windows`, `x11`

#### Process and Libraries  
- Added `process.hh` - Process creation and inter-process communication
- Added `shared_object.hh` - Dynamic library loading with type-safe symbol lookup

#### Localization
- Added `locale.hh` - Comprehensive locale detection and matching
- Added locale matching algorithms for finding best supported locale
- Added common language and country code constants

#### UI Components
- Added `message_box.hh` - Native message box support with builder pattern
- Added `dialog.hh` - File open/save dialog support
- Added `tray.hh` - System tray icon and menu support

#### Utilities
- Added `guid.hh` - GUID wrapper with C++20 spaceship operator support
- Added `misc.hh` - URL opening and URL utility functions
- Added `string_utils.hh` - String manipulation utilities
- Added `formatters.hh` - Custom formatters for SDL types

### Changed
- **Breaking:** `namespace sdlpp::window_pos` (the `centered` / `undefined` window
  placement constants) is now `sdlpp::window_placement`; `sdlpp::window_pos` is the
  window-space position type.
- Updated all wrappers to use `std::filesystem::path` where appropriate
- Improved error handling consistency across all modules
- Enhanced platform detection with runtime device type checks

### Fixed
- Fixed SDL3 API compatibility issues (function name changes, parameter updates)
- Fixed process I/O stream lifetime management
- Fixed locale API to work with SDL3's new count parameter
- Fixed various compiler warnings with strict warning flags

## [0.1.0] - Initial Release

### Added
- Core SDL3 wrapper functionality
- Window management
- Rendering system
- Surface operations
- OpenGL context support
- Event handling
- Audio subsystem
- Timer utilities
- Properties system
- Hints system
- Error handling with `tl::expected`
- Comprehensive test suite
- Example applications