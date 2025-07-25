# SDL++ Quick Reference

## Initialization
```cpp
auto init = sdlpp::init::create(sdlpp::init_flags::video | sdlpp::init_flags::audio);
```

## Window
```cpp
auto window = sdlpp::window::create("Title", 800, 600);
window->set_title("New Title");
window->set_size(1024, 768);
auto [w, h] = window->get_size();
```

## Rendering
```cpp
auto renderer = sdlpp::renderer::create(*window);
renderer->set_draw_color({255, 0, 0, 255});
renderer->clear();
renderer->draw_line({0, 0}, {100, 100});
renderer->fill_rect({{10, 10}, {50, 50}});
renderer->present();
```

## Events
```cpp
sdlpp::event event;
while (sdlpp::poll_event(event)) {
    std::visit([](auto&& e) {
        using T = std::decay_t<decltype(e)>;
        if constexpr (std::is_same_v<T, sdlpp::quit_event>) {
            // Handle quit
        }
    }, event);
}
```

## File I/O
```cpp
// Read
auto file = sdlpp::iostream::open("file.txt", sdlpp::file_mode::read);
auto content = file->read_string();

// Write
auto out = sdlpp::iostream::open("out.txt", sdlpp::file_mode::write);
out->write_string("Hello");

// Async
auto task = sdlpp::load_file_async("large.dat");
if (auto result = task->try_get_result()) {
    // Use result->data()
}
```

## Platform
```cpp
if constexpr (sdlpp::platform::is_windows()) { }
if (sdlpp::platform::is_tablet()) { }
auto power = sdlpp::power::get_power_info();
auto docs = sdlpp::directories::get_documents_folder();
```

## Locales
```cpp
auto locales = sdlpp::get_preferred_locales();
auto best = sdlpp::find_best_locale(supported);
if (locale.language == "en") { }
```

## Process
```cpp
auto proc = sdlpp::process::create("/bin/ls", {"-la"});
auto output = proc->read_stdout_string();
proc->wait();
```

## URLs
```cpp
sdlpp::open_url("https://example.com");
auto mailto = sdlpp::url::make_mailto("user@example.com", "Subject");
```

## Message Box
```cpp
auto result = sdlpp::message_box_builder()
    .set_title("Confirm")
    .set_message("Are you sure?")
    .add_button(1, "Yes", true)
    .add_button(0, "No")
    .show();
```

## GUID
```cpp
auto guid = sdlpp::guid::from_string("deadbeef...");
std::map<sdlpp::guid, Config> configs;
if (g1 <=> g2 == std::strong_ordering::equal) { }
```

## Timer
```cpp
sdlpp::frame_limiter limiter(60.0);
while (running) {
    // Game loop
    limiter.wait_for_next_frame();
}

auto start = sdlpp::timer::get_performance_counter();
// Work...
auto elapsed = sdlpp::timer::performance_to_ms(end - start);
```

## CPU
```cpp
auto features = sdlpp::get_cpu_features();
if (features.has_avx2) { }
auto mem = sdlpp::simd_alloc(1024, 32); // 32-byte aligned
```

## Shared Libraries
```cpp
auto lib = sdlpp::shared_object::load("mylib.so");
auto func = lib->get_function<int(const char*)>("my_func");
int result = func("arg");
```

## Error Handling
```cpp
auto result = sdlpp::some_operation();
if (!result) {
    std::cerr << "Error: " << result.error() << "\n";
    return -1;
}
// Use *result
```