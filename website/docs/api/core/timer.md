---
title: Timer & Frame Rate
sidebar_label: Timer & Frame Rate
---

# Timer and Frame Rate Control

SDL++ provides high-resolution timing facilities and frame rate control utilities using modern C++ chrono types.

## Basic Timer Usage

```cpp
#include <sdlpp/core/timer.hh>

// Get current time
auto now = sdlpp::timer::get_ticks();

// Delay for specific duration
sdlpp::timer::delay(std::chrono::milliseconds(16));  // ~60 FPS

// Measure elapsed time
auto start = sdlpp::timer::get_ticks();
do_something();
auto elapsed = sdlpp::timer::get_ticks() - start;
std::cout << "Operation took: " << elapsed.count() << "ms" << std::endl;
```

## Performance Counter

For high-precision timing:

```cpp
// Get performance counter frequency
auto frequency = sdlpp::timer::get_performance_frequency();

// Measure with performance counter
auto start = sdlpp::timer::get_performance_counter();
do_precise_operation();
auto end = sdlpp::timer::get_performance_counter();

// Convert to seconds
double seconds = static_cast<double>(end - start) / frequency;
```

## Frame Rate Limiting

### Simple Frame Limiter

```cpp
#include <sdlpp/core/timer.hh>

class simple_frame_limiter {
    std::chrono::steady_clock::time_point last_frame;
    std::chrono::duration<double> target_duration;
    
public:
    explicit simple_frame_limiter(double target_fps)
        : last_frame(std::chrono::steady_clock::now())
        , target_duration(1.0 / target_fps) {}
    
    void wait() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - last_frame;
        
        if (elapsed < target_duration) {
            auto sleep_time = target_duration - elapsed;
            std::this_thread::sleep_for(sleep_time);
        }
        
        last_frame = std::chrono::steady_clock::now();
    }
};
```

### SDL++ Frame Limiter

```cpp
// Create frame limiter for 60 FPS
sdlpp::frame_limiter limiter(60.0);

// Game loop
while (running) {
    // Process events and update game state
    update();
    render();
    
    // Wait for next frame
    limiter.wait_for_next_frame();
    
    // Get frame statistics
    double current_fps = limiter.get_current_fps();
    double average_fps = limiter.get_average_fps();
}
```

## Timer Callbacks

SDL++ supports timer callbacks with RAII management:

```cpp
// One-shot timer
auto timer1 = sdlpp::timer::add_timer(
    std::chrono::seconds(5),
    [](std::chrono::milliseconds interval) -> std::chrono::milliseconds {
        std::cout << "Timer fired after 5 seconds!" << std::endl;
        return std::chrono::milliseconds(0);  // Don't repeat
    }
);

// Repeating timer
auto timer2 = sdlpp::timer::add_timer(
    std::chrono::milliseconds(100),
    [](std::chrono::milliseconds interval) -> std::chrono::milliseconds {
        std::cout << "Tick!" << std::endl;
        return interval;  // Repeat with same interval
    }
);

// Timers are automatically removed when objects go out of scope
```

## Frame Time Measurement

```cpp
class frame_timer {
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    using duration = std::chrono::duration<double>;
    
    time_point last_frame;
    duration frame_time{0};
    double smoothed_fps = 60.0;
    
public:
    frame_timer() : last_frame(clock::now()) {}
    
    void update() {
        auto now = clock::now();
        frame_time = now - last_frame;
        last_frame = now;
        
        // Smooth FPS calculation
        double instant_fps = 1.0 / frame_time.count();
        smoothed_fps = smoothed_fps * 0.95 + instant_fps * 0.05;
    }
    
    double get_frame_time() const { 
        return frame_time.count(); 
    }
    
    double get_fps() const { 
        return smoothed_fps; 
    }
    
    double get_frame_time_ms() const {
        return frame_time.count() * 1000.0;
    }
};
```

## Fixed Timestep Game Loop

```cpp
void fixed_timestep_game_loop() {
    const auto timestep = std::chrono::milliseconds(16);  // 60 Hz
    auto current_time = std::chrono::steady_clock::now();
    auto accumulator = std::chrono::duration<double>::zero();
    
    while (running) {
        auto new_time = std::chrono::steady_clock::now();
        auto frame_time = new_time - current_time;
        current_time = new_time;
        
        accumulator += frame_time;
        
        // Process input
        handle_events();
        
        // Fixed timestep updates
        while (accumulator >= timestep) {
            update_physics(timestep);
            accumulator -= timestep;
        }
        
        // Interpolate rendering
        double alpha = accumulator.count() / 
                      std::chrono::duration<double>(timestep).count();
        render(alpha);
    }
}
```

## Profiling Helper

```cpp
class scoped_timer {
    using clock = std::chrono::high_resolution_clock;
    std::string name;
    clock::time_point start;
    
public:
    explicit scoped_timer(std::string timer_name)
        : name(std::move(timer_name))
        , start(clock::now()) {}
    
    ~scoped_timer() {
        auto end = clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start);
        std::cout << name << " took " << duration.count() << "ms" << std::endl;
    }
};

// Usage
void process_frame() {
    {
        scoped_timer timer("Event processing");
        handle_events();
    }
    {
        scoped_timer timer("Physics update");
        update_physics();
    }
    {
        scoped_timer timer("Rendering");
        render();
    }
}
```

## Best Practices

1. **Use std::chrono**: Prefer chrono types over raw milliseconds
2. **Fixed Timestep**: Use fixed timestep for deterministic physics
3. **Frame Limiting**: Always limit frame rate to avoid wasting resources
4. **Profile First**: Measure before optimizing
5. **Smooth Metrics**: Average FPS over multiple frames for stable display

## Example: Complete Game Loop

```cpp
#include <sdlpp/core/timer.hh>

class game {
    sdlpp::frame_limiter limiter{60.0};
    frame_timer timer;
    bool show_fps = true;
    
public:
    void run() {
        while (!should_quit()) {
            timer.update();
            
            process_events();
            update(timer.get_frame_time());
            render();
            
            if (show_fps) {
                draw_fps_counter(timer.get_fps());
            }
            
            limiter.wait_for_next_frame();
        }
    }
    
private:
    void update(double dt) {
        // Update with delta time
        player.position += player.velocity * dt;
    }
};
```