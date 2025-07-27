# SDL++ API Reference

## Table of Contents

1. [Core Components](#core-components)
2. [Application Framework](#application-framework)
3. [Video Subsystem](#video-subsystem)
4. [Audio Subsystem](#audio-subsystem)
5. [Event System](#event-system)
6. [I/O and Storage](#io-and-storage)
7. [System Information](#system-information)
8. [Process and Libraries](#process-and-libraries)
9. [UI Components](#ui-components)
10. [Utilities](#utilities)
11. [Concepts](#concepts)
12. [DDA Rendering](#dda-rendering)

## Core Components

### Initialization (`core/core.hh`)

```cpp
// Initialize SDL subsystems
auto init = sdlpp::init::create(sdlpp::init_flags::video | sdlpp::init_flags::audio);

// Check specific subsystem
bool has_video = sdlpp::was_init(sdlpp::init_flags::video);
```

### Error Handling (`core/error.hh`)

```cpp
// Get last SDL error
std::string error = sdlpp::get_error();

// Clear error state
sdlpp::clear_error();

// Set custom error
sdlpp::set_error("Custom error message");
```

### Version Information (`core/version.hh`)

```cpp
// Get compile-time version
auto compile_ver = sdlpp::version_info::compile_time;

// Get runtime version
auto runtime_ver = sdlpp::version_info::runtime();

// Check version compatibility
if (runtime_ver >= sdlpp::version{3, 2, 0}) {
    // Use SDL 3.2.0+ features
}
```

## Application Framework

### SDL3 Application Model (`app/app.hh`, `app/app_impl.hh`)

SDL++ provides a modern C++ wrapper around SDL3's new application lifecycle callbacks.

```cpp
// Basic application
class my_app : public sdlpp::application {
protected:
    bool on_init() override {
        // Initialize your app
        return true;
    }
    
    void on_frame() override {
        // Called every frame
        auto& renderer = get_renderer();
        renderer.clear();
        // Draw your content
    }
    
    bool on_event(const sdlpp::event& e) override {
        // Handle events
        return true; // Return false to quit
    }
};

SDLPP_MAIN(my_app)
```

### Application Types

#### Basic Application (`app/app.hh`)
```cpp
// With configuration
SDLPP_MAIN_WITH_CONFIG(my_app,
    .window_title = "My Game",
    .window_width = 1920,
    .window_height = 1080,
    .auto_create_renderer = true
)
```

#### Stateful Application (`app/stateful_app.hh`)
```cpp
struct game_state {
    int score = 0;
    float player_x = 100.0f;
};

class my_game : public sdlpp::stateful_application<game_state> {
    void on_frame() override {
        state().player_x += 100.0f * get_delta_time();
    }
};
```

#### Scene-Based Application (`app/scene_app.hh`)
```cpp
class menu_scene : public sdlpp::scene {
    void render(sdlpp::renderer& r) override { /* ... */ }
    bool handle_event(const sdlpp::event& e) override { /* ... */ }
};

class my_app : public sdlpp::scene_application {
    bool on_init() override {
        push_scene<menu_scene>();
        return true;
    }
};
```

#### Game Application (`app/game_app.hh`)
```cpp
class physics_game : public sdlpp::game_application {
    void fixed_update(float dt) override {
        // Called at fixed rate (60Hz default)
    }
    
    void render(float interpolation) override {
        // Called once per frame with interpolation
    }
};
```

### App Builder (`app/app_builder.hh`)
```cpp
// Functional style
sdlpp::app_builder()
    .with_window("Demo", 800, 600)
    .with_renderer()
    .on_frame([]() {
        // Render
        return true;
    })
    .on_event([](const auto& e) {
        return e.type() != sdlpp::event_type::quit;
    })
    .run(argc, argv);
```

### Callbacks (`app/app_builder.hh`)
```cpp
// Simple callback-based app
return sdlpp::run(argc, argv, {
    .init = [](int, char*[]) { return true; },
    .iterate = []() { return true; },
    .event = [](const auto& e) { return true; },
    .quit = []() { std::cout << "Bye!\n"; }
});
```

## Video Subsystem

### Window Management (`video/window.hh`)

```cpp
// Create window
auto window = sdlpp::window::create("Title", 800, 600);

// Window with flags
auto fullscreen = sdlpp::window::create("Game", 1920, 1080, 
    sdlpp::window_flags::fullscreen | sdlpp::window_flags::vulkan);

// Window operations
window->set_title("New Title");
window->set_size(1024, 768);
window->set_position(100, 100);
window->maximize();
window->minimize();
window->raise();

// Get window properties
auto [w, h] = window->get_size();
auto pos = window->get_position();
bool visible = window->is_visible();
```

### Rendering (`video/renderer.hh`)

```cpp
// Create renderer
auto renderer = sdlpp::renderer::create(*window);

// Render operations
renderer->set_draw_color({255, 0, 0, 255}); // Red
renderer->clear();
renderer->draw_point({100, 100});
renderer->draw_line({0, 0}, {100, 100});
renderer->fill_rect({{10, 10}, {50, 50}});
renderer->present();

// Textures
auto texture = renderer->create_texture(surface);
renderer->copy(texture);
renderer->copy_ex(texture, src_rect, dst_rect, 45.0, center, flip);
```

### Surface Operations (`video/surface.hh`)

```cpp
// Create surface
auto surface = sdlpp::surface::create_rgb(800, 600, 32);

// Load from file (requires SDL_image)
auto loaded = sdlpp::surface::load("image.png");

// Surface operations
surface->fill({255, 0, 0, 255}); // Fill with red
surface->blit(other_surface, src_rect, dst_point);
surface->convert_format(sdlpp::pixel_format::rgba8888);

// Pixel access
auto pixels = surface->pixels();
auto format = surface->format();
```

### OpenGL Support (`video/gl.hh`)

```cpp
// Set OpenGL attributes
sdlpp::gl::set_attribute(sdlpp::gl::attr::context_major_version, 3);
sdlpp::gl::set_attribute(sdlpp::gl::attr::context_minor_version, 3);
sdlpp::gl::set_attribute(sdlpp::gl::attr::context_profile_mask, 
                        sdlpp::gl::profile::core);

// Create context
auto context = sdlpp::gl_context::create(window);

// Make current
context->make_current(window);

// Swap buffers
window->gl_swap();
```

## Audio Subsystem

### Audio Device (`audio/audio.hh`)

```cpp
// Open audio device
sdlpp::audio_spec desired{
    .freq = 48000,
    .format = sdlpp::audio_format::f32,
    .channels = 2,
    .samples = 4096
};

auto device = sdlpp::audio_device::open(nullptr, false, desired);

// Control playback
device->pause(false); // Start playback
device->pause(true);  // Pause

// Queue audio
std::vector<float> audio_data;
device->queue(audio_data);
```

## Event System

### Event Handling (`events/events.hh`)

```cpp
// Poll events
sdlpp::event event;
while (sdlpp::poll_event(event)) {
    std::visit([](auto&& e) {
        using T = std::decay_t<decltype(e)>;
        
        if constexpr (std::is_same_v<T, sdlpp::quit_event>) {
            // Handle quit
        } else if constexpr (std::is_same_v<T, sdlpp::keyboard_event>) {
            if (e.type == sdlpp::event_type::key_down) {
                // Handle key press
            }
        } else if constexpr (std::is_same_v<T, sdlpp::mouse_motion_event>) {
            // Handle mouse motion
        }
    }, event);
}

// Wait for event with timeout
if (sdlpp::wait_event_timeout(event, std::chrono::seconds(1))) {
    // Process event
}
```

## I/O and Storage

### File I/O (`io/iostream.hh`, `io/io_common.hh`)

```cpp
// Open file for reading
auto file = sdlpp::iostream::open("/path/to/file.txt", sdlpp::file_mode::read);
if (file) {
    auto content = file->read_string();
    // or
    std::vector<uint8_t> data = file->read_all();
}

// Write file
auto out = sdlpp::iostream::open("output.txt", sdlpp::file_mode::write);
if (out) {
    out->write_string("Hello, World!");
}

// Memory I/O
std::vector<uint8_t> buffer(1024);
auto mem_io = sdlpp::iostream::from_memory(buffer);
```

### Async I/O (`io/async_io.hh`)

```cpp
// Async file loading
auto task = sdlpp::load_file_async("/path/to/large/file.dat");

// Check if complete
if (task->try_get_result()) {
    auto result = task->get_result();
    if (result) {
        // Use result->data() and result->size()
    }
}
```

### Storage (`io/storage.hh`)

```cpp
// Get storage interface
auto storage = sdlpp::get_storage_interface();

// Read/write values
storage->write_string("username", "player1");
storage->write_int("high_score", 10000);

auto name = storage->read_string("username");
auto score = storage->read_int("high_score", 0); // default: 0
```

## System Information

### CPU Information (`system/cpu.hh`)

```cpp
// Get CPU features
auto features = sdlpp::get_cpu_features();
if (features.has_avx2) {
    // Use AVX2 optimized code
}

// Aligned memory
auto memory = sdlpp::simd_alloc(1024, 32); // 32-byte aligned
// Automatically freed

// CPU dispatch
auto func = sdlpp::cpu_dispatch(
    features.has_avx2,   avx2_implementation,
    features.has_sse42,  sse42_implementation,
    true,                scalar_implementation
);
```

### Platform Detection (`system/platform.hh`)

```cpp
// Compile-time checks
if constexpr (sdlpp::platform::is_windows()) {
    // Windows-specific code
}

// Runtime checks
if (sdlpp::platform::is_tablet()) {
    // Tablet UI adjustments
}

// Platform info
auto info = sdlpp::platform::get_platform_info();
std::cout << "Platform: " << info.name << "\n";

// Power info
auto power = sdlpp::power::get_power_info();
if (power.is_on_battery()) {
    std::cout << "Battery: " << power.percent_left << "%\n";
}

// System paths
auto docs = sdlpp::directories::get_documents_folder();
auto prefs = sdlpp::directories::get_pref_path("MyCompany", "MyApp");
```

### Locale Support (`system/locale.hh`)

```cpp
// Get user's preferred locales
auto locales = sdlpp::get_preferred_locales();
for (const auto& locale : locales) {
    std::cout << locale.to_string() << "\n"; // e.g., "en-US"
}

// Find best matching locale
std::vector<sdlpp::locale> supported = {
    {"en", "US"}, {"en", "GB"}, {"fr", "FR"}, {"de"}
};
auto best = sdlpp::find_best_locale(supported);

// Locale comparison
sdlpp::locale en_us("en", "US");
sdlpp::locale en_gb("en", "GB");
if (en_us.matches(en_gb, true)) { // true = allow language-only match
    // Both are English
}
```

## Process and Libraries

### Process Management (`system/process.hh`)

```cpp
// Create process
auto proc = sdlpp::process::create("/usr/bin/grep", {"pattern", "file.txt"});
if (proc) {
    // Write to stdin
    proc->write_stdin("input data\n");
    
    // Read output
    std::string output = proc->read_stdout_string();
    std::string errors = proc->read_stderr_string();
    
    // Wait for completion
    auto exit_code = proc->wait();
}

// Background process
auto bg_proc = sdlpp::process::create_background("server", {"--port", "8080"});
```

### Shared Libraries (`system/shared_object.hh`)

```cpp
// Load library
auto lib = sdlpp::shared_object::load("mylib.so");
if (lib) {
    // Get function pointer
    auto func = lib->get_function<int(const char*)>("my_function");
    if (func) {
        int result = func("argument");
    }
    
    // Get object pointer
    auto obj = lib->get_object<MyClass>("global_object");
}
```

## UI Components

### Message Boxes (`ui/message_box.hh`)

```cpp
// Simple message box
sdlpp::show_simple_message_box(
    sdlpp::message_box_flags::information,
    "Title", 
    "Message"
);

// Message box with buttons
auto result = sdlpp::message_box_builder()
    .set_title("Confirm")
    .set_message("Are you sure?")
    .set_type(sdlpp::message_box_flags::warning)
    .add_button(1, "Yes", true)      // return_key
    .add_button(0, "No", false, true) // escape_key
    .show();

if (result && *result == 1) {
    // User clicked Yes
}
```

### File Dialogs (`ui/dialog.hh`)

```cpp
// File open dialog
sdlpp::file_dialog_builder builder;
builder.add_filter("Images", {"png", "jpg", "jpeg"});
builder.add_filter("All Files", {"*"});

auto callback = [](std::vector<std::string> paths, int filter) {
    if (!paths.empty()) {
        std::cout << "Selected: " << paths[0] << "\n";
    }
};

sdlpp::show_open_file_dialog(builder.get_filters(), callback);

// Folder selection
sdlpp::show_open_folder_dialog([](std::vector<std::string> paths, int) {
    if (!paths.empty()) {
        std::cout << "Selected folder: " << paths[0] << "\n";
    }
});
```

### System Tray (`ui/tray.hh`)

```cpp
// Create tray entry
auto icon = sdlpp::surface::create_rgb(16, 16, 32);
// ... draw icon ...

auto tray = sdlpp::tray_entry::create(icon, "My App");
if (tray) {
    // Create menu
    auto menu = sdlpp::tray_menu::create();
    menu->add_item(1, "Show", true);  // enabled
    menu->add_item(2, "Settings", true);
    menu->add_separator();
    menu->add_item(3, "Quit", true);
    
    tray->set_menu(menu);
}
```

## Utilities

### GUID Support (`utility/guid.hh`)

```cpp
// Create GUID from string
auto guid = sdlpp::guid::from_string("deadbeefcafebabe1234567890abcdef");

// GUID operations
if (guid && guid->is_valid()) {
    std::string str = guid->to_string();
    
    // Use in containers
    std::map<sdlpp::guid, std::string> device_names;
    device_names[*guid] = "Xbox Controller";
    
    // Get device info
    auto info = sdlpp::get_joystick_guid_info(*guid);
    std::cout << "Vendor: " << std::hex << info.vendor << "\n";
}

// Comparison with spaceship operator
sdlpp::guid g1, g2;
auto ordering = g1 <=> g2;
```

### URL Opening (`utility/misc.hh`)

```cpp
// Open URL
auto result = sdlpp::open_url("https://example.com");
if (!result) {
    std::cerr << "Failed: " << result.error() << "\n";
}

// URL utilities
if (!sdlpp::url::has_protocol(url)) {
    url = sdlpp::url::ensure_protocol(url); // adds https://
}

// Create mailto URL
auto mailto = sdlpp::url::make_mailto(
    "support@example.com",
    "Bug Report",
    "I found a bug..."
);
sdlpp::open_url(mailto);

// File URLs
auto file_url = sdlpp::url::make_file_url("/home/user/document.pdf");
sdlpp::open_url(file_url);
```

### Timer System (`core/timer.hh`)

```cpp
// High-resolution timing
auto start = sdlpp::timer::get_performance_counter();
// ... do work ...
auto end = sdlpp::timer::get_performance_counter();
auto elapsed_ms = sdlpp::timer::performance_to_ms(end - start);

// Delays
sdlpp::timer::delay(std::chrono::milliseconds(16));

// Frame limiting
sdlpp::frame_limiter limiter(60.0); // 60 FPS
while (running) {
    // ... game loop ...
    limiter.wait_for_next_frame();
}

// Timer callbacks
auto timer = sdlpp::timer_callback::create(
    std::chrono::seconds(1),
    [](std::chrono::milliseconds interval) -> std::chrono::milliseconds {
        std::cout << "Tick!\n";
        return interval; // repeat
    }
);
```

## Concepts

### Geometry Concepts (`concepts/geometry_concepts.hh`)

SDL++ provides concepts to work with any geometry library:

```cpp
// Point-like types (must have x, y members)
template<sdlpp::point_like P>
void draw_at_point(const P& pos) {
    auto x = sdlpp::get_x(pos);
    auto y = sdlpp::get_y(pos);
    // ...
}

// Rectangle-like types (must have x, y, w, h or left, top, width, height)
template<sdlpp::rect_like R>
void fill_area(const R& rect) {
    auto w = sdlpp::get_width(rect);
    auto h = sdlpp::get_height(rect);
    // ...
}

// Size-like types (must have width, height members)
template<sdlpp::size_like S>
auto calculate_area(const S& size) {
    return sdlpp::get_area(size);
}

// Other concepts
- sdlpp::line_like      // x1, y1, x2, y2
- sdlpp::circle_like    // x, y, radius
- sdlpp::triangle_like  // a, b, c (points)
- sdlpp::polygon_like   // size(), operator[]
```

### Renderer Concepts (`concepts/renderer_concepts.hh`)

Concepts that generalize renderer and surface_renderer:

```cpp
// Basic renderer operations
template<sdlpp::basic_renderer R>
void clear_screen(R& renderer) {
    renderer.set_draw_color({0, 0, 0, 255});
    renderer.clear();
}

// Primitive drawing
template<sdlpp::primitive_renderer R>
void draw_ui(R& renderer) {
    renderer.draw_line(0, 0, 100, 100);
    renderer.fill_rect(10, 10, 50, 50);
    renderer.draw_point(55, 55);
}

// DDA algorithms
template<sdlpp::dda_renderer R>
void draw_smooth_graphics(R& renderer) {
    renderer.draw_line_aa(0.5f, 0.5f, 99.5f, 99.5f);
    renderer.draw_circle(50, 50, 30);
    renderer.fill_ellipse(100, 100, 40, 20);
    renderer.draw_line_thick(10, 10, 90, 90, 3.0f);
}

// Full renderer
template<sdlpp::renderer_like R>
void render_scene(R& renderer) {
    // Can use all renderer features
}
```

## DDA Rendering

### Software Surface Renderer (`video/surface_renderer.hh`)

A software renderer that operates directly on SDL surfaces using DDA algorithms:

```cpp
// Create surface renderer
sdlpp::surface surf(800, 600, SDL_PIXELFORMAT_ARGB8888);
sdlpp::surface_renderer renderer(surf);

// Basic operations (same as hardware renderer)
renderer.set_draw_color({255, 0, 0, 255});
renderer.clear();
renderer.draw_point({100, 100});
renderer.draw_line({0, 0}, {100, 100});
renderer.fill_rect({{10, 10}, {50, 50}});
```

### DDA Primitives (Hardware & Software)

Both `renderer` and `surface_renderer` support DDA algorithms:

```cpp
// Antialiased lines
renderer.draw_line_aa({0.5f, 0.5f}, {99.5f, 99.5f});

// Thick lines
renderer.draw_line_thick({10, 10}, {90, 90}, 5.0f);

// Circles
renderer.draw_circle({100, 100}, 50);
renderer.fill_circle({200, 200}, 30);

// Ellipses
renderer.draw_ellipse({300, 300}, 60, 40);
renderer.fill_ellipse({400, 400}, 80, 50);

// Ellipse arcs
renderer.draw_ellipse_arc({500, 500}, 100, 75, 0.0f, M_PI);

// With euler angles
renderer.draw_ellipse_arc({600, 600}, 100, 75, 
                         euler::radian<float>(0.0f), 
                         euler::radian<float>(M_PI));
```

### Bezier Curves

```cpp
// Quadratic Bezier
renderer.draw_bezier_quad({0, 100}, {50, 0}, {100, 100});

// Cubic Bezier
renderer.draw_bezier_cubic({0, 0}, {30, 100}, {70, 100}, {100, 0});

// With point-like types
euler::point2<float> p0{0, 0}, p1{50, 100}, p2{100, 0};
renderer.draw_bezier_quad(p0, p1, p2);
```

### Splines

```cpp
// B-spline
std::vector<euler::point2<float>> controls = {
    {0, 0}, {50, 100}, {100, 50}, {150, 100}, {200, 0}
};
renderer.draw_bspline(controls, 3); // degree 3 (cubic)

// Catmull-Rom spline
std::vector<point<float>> points = {
    {0, 50}, {50, 0}, {100, 100}, {150, 50}
};
renderer.draw_catmull_rom(points, 0.5f); // tension
```

### Parametric Curves

```cpp
// Any parametric curve
auto heart = [](float t) -> point<float> {
    float x = 16 * std::pow(std::sin(t), 3);
    float y = 13 * std::cos(t) - 5 * std::cos(2*t) - 
              2 * std::cos(3*t) - std::cos(4*t);
    return {x * 5 + 400, -y * 5 + 300};
};
renderer.draw_curve(heart, 0.0f, 2.0f * M_PI, 200);
```

### Polygons

```cpp
// Polygon outline
std::vector<point<int>> vertices = {
    {100, 100}, {200, 100}, {250, 200}, {150, 250}, {50, 200}
};
renderer.draw_polygon(vertices);

// Filled polygon
renderer.fill_polygon(vertices);

// Antialiased polygon
renderer.draw_polygon_aa(vertices);
```

### Euler Angle Support

SDL++ integrates with the euler library for type-safe angles:

```cpp
// Using euler angles in rendering
#include <euler/angles/angle.hh>

// Rotation with type-safe angles
renderer.copy_ex(texture, src_rect, dst_rect, 
                euler::degree<double>(45.0), 
                center, flip_mode::none);

// Arc drawing with radians
renderer.draw_ellipse_arc(center, 100, 75,
                         euler::radian<float>(0.0f),
                         euler::radian<float>(M_PI_2));

// Convert between angle types
auto deg = euler::degree<float>(90.0f);
auto rad = euler::to_radians(deg);
```

### Clipping

Both renderers support clipping:

```cpp
// Set clip rectangle
renderer.set_clip_rect({{100, 100}, {400, 300}});

// Check if clipping is enabled
if (renderer.is_clip_enabled()) {
    auto clip = renderer.get_clip_rect();
    // ...
}

// Disable clipping
renderer.set_clip_rect(std::nullopt);
```

### Blending

```cpp
// Set blend mode
renderer.set_draw_blend_mode(sdlpp::blend_mode::add);
renderer.set_draw_blend_mode(sdlpp::blend_mode::blend);
renderer.set_draw_blend_mode(); // defaults to none

// Surface blending (surface_renderer only)
surface_renderer src_renderer(source_surface);
surface_renderer dst_renderer(dest_surface);
dst_renderer.blend_surface(src_renderer, src_rect, {100, 100}, 
                          sdlpp::blend_mode::blend);
```

### Performance Tips

1. **Use batched operations** - The DDA implementations use batched pixel writes
2. **Prefer integer coordinates** - When possible, use integer coordinates for better performance
3. **Lock surfaces** - Surface renderer automatically handles locking
4. **Choose the right renderer**:
   - Hardware renderer: Fast for simple shapes, texture operations
   - Surface renderer: Better for complex DDA algorithms, pixel-perfect rendering