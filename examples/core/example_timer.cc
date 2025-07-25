//
// Created by igor on 7/14/25.
//
// Example: Timer and timing utilities

#include <iostream>
#include <iomanip>
#include <../include/sdlpp/core/timer.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/core/core.hh>
#include <thread>
#include <atomic>
#include <cmath>

using namespace sdlpp;
using namespace std::chrono;
using namespace std::chrono_literals;

void basic_timing_example() {
    std::cout << "\n=== Basic Timing Example ===\n";
    
    // Measure elapsed time since SDL initialization
    std::cout << "Time since SDL init: " << timer::elapsed().count() << " ms\n";
    
    // Measure a specific operation
    auto start = timer::elapsed();
    std::this_thread::sleep_for(100ms);
    auto duration = timer::elapsed_since(start);
    std::cout << "Operation took: " << duration.count() << " ms\n";
    
    // Using SDL clock
    auto t1 = timer::clock::now();
    std::this_thread::sleep_for(50ms);
    auto t2 = timer::clock::now();
    std::cout << "Clock measurement: " << (t2 - t1).count() << " ms\n";
}

void high_precision_timing_example() {
    std::cout << "\n=== High Precision Timing Example ===\n";
    
    // Performance counter for microsecond precision
    timer::performance_counter counter;
    
    // Simulate some work
    volatile double result = 0;
    for (int i = 0; i < 1000000; ++i) {
        result += std::sin(i * 0.001);
    }
    
    auto elapsed_us = counter.elapsed<microseconds>();
    auto elapsed_ms = counter.elapsed<milliseconds>();
    auto elapsed_ns = counter.elapsed<nanoseconds>();
    
    std::cout << "Calculation took:\n";
    std::cout << "  " << elapsed_ns.count() << " ns\n";
    std::cout << "  " << elapsed_us.count() << " μs\n";
    std::cout << "  " << elapsed_ms.count() << " ms\n";
    std::cout << "Result: " << result << "\n";
    
    // Performance counter frequency
    std::cout << "\nPerformance counter frequency: " 
              << timer::performance_counter::get_frequency() << " Hz\n";
}

void delay_example() {
    std::cout << "\n=== Delay Example ===\n";
    
    // Different delay methods
    std::cout << "Testing delays...\n";
    
    {
        timer::performance_counter counter;
        timer::delay(50ms);
        std::cout << "50ms delay took: " << counter.elapsed<milliseconds>().count() << " ms\n";
    }
    
    {
        timer::performance_counter counter;
        timer::delay_precise(5ms);
        std::cout << "5ms precise delay took: " << counter.elapsed<microseconds>().count() << " μs\n";
    }
    
    {
        timer::performance_counter counter;
        timer::sleep_for(25ms);
        std::cout << "25ms sleep_for took: " << counter.elapsed<milliseconds>().count() << " ms\n";
    }
}

void scoped_timer_example() {
    std::cout << "\n=== Scoped Timer Example ===\n";
    
    // Custom callback for timing results
    auto timing_callback = [](const std::string& name, nanoseconds elapsed) {
        auto ms = static_cast<double>(duration_cast<microseconds>(elapsed).count()) / 1000.0;
        std::cout << "[TIMER] " << name << " completed in " 
                  << std::fixed << std::setprecision(3) << ms << " ms\n";
    };
    
    {
        scoped_timer timer("Total Operation", timing_callback);
        
        {
            scoped_timer inner("Phase 1", timing_callback);
            std::this_thread::sleep_for(20ms);
        }
        
        {
            scoped_timer inner("Phase 2", timing_callback);
            std::this_thread::sleep_for(30ms);
        }
        
        {
            scoped_timer inner("Phase 3", timing_callback);
            std::this_thread::sleep_for(10ms);
        }
    }
}

void timer_callback_example() {
    std::cout << "\n=== Timer Callback Example ===\n";
    
    // One-shot timer
    std::cout << "Setting up one-shot timer for 1 second...\n";
    std::atomic<bool> oneshot_fired{false};
    
    auto oneshot = timer_handle::create_oneshot(
        1s,
        [&oneshot_fired]() {
            std::cout << "  -> One-shot timer fired!\n";
            oneshot_fired = true;
        }
    );
    
    if (!oneshot) {
        std::cerr << "Failed to create one-shot timer: " << oneshot.error() << "\n";
        return;
    }
    
    // Repeating timer
    std::cout << "Setting up repeating timer (500ms interval)...\n";
    std::atomic<int> repeat_count{0};
    
    auto repeating = timer_handle::create_repeating(
        500ms,
        [&repeat_count]() {
            repeat_count++;
            std::cout << "  -> Repeating timer fired (count: " << repeat_count << ")\n";
        }
    );
    
    if (!repeating) {
        std::cerr << "Failed to create repeating timer: " << repeating.error() << "\n";
        return;
    }
    
    // Variable interval timer
    std::cout << "Setting up variable interval timer...\n";
    std::atomic<int> var_count{0};
    
    auto variable = timer_handle::create(
        100ms,
        [&var_count](milliseconds current_interval) -> milliseconds {
            var_count++;
            std::cout << "  -> Variable timer fired (interval was " 
                      << current_interval.count() << "ms)\n";
            
            if (var_count >= 5) {
                std::cout << "  -> Variable timer stopping\n";
                return 0ms; // Stop the timer
            }
            
            // Increase interval each time
            return current_interval + 100ms;
        }
    );
    
    if (!variable) {
        std::cerr << "Failed to create variable timer: " << variable.error() << "\n";
        return;
    }
    
    // Let timers run
    std::cout << "\nTimers running for 3 seconds...\n";
    std::this_thread::sleep_for(3s);
    
    // Cancel the repeating timer
    std::cout << "\nCancelling repeating timer...\n";
    repeating->cancel();
    
    // Final status
    std::cout << "\nFinal status:\n";
    std::cout << "  One-shot fired: " << (oneshot_fired ? "Yes" : "No") << "\n";
    std::cout << "  Repeat count: " << repeat_count << "\n";
    std::cout << "  Variable count: " << var_count << "\n";
}

void frame_limiter_example() {
    std::cout << "\n=== Frame Limiter Example ===\n";
    
    // Create a 60 FPS limiter
    frame_limiter limiter(60.0);
    
    std::cout << "Running at 60 FPS for 1 second...\n";
    
    timer::performance_counter total_timer;
    int frame_count = 0;
    double total_work_time = 0;
    double min_fps = 1000.0;
    double max_fps = 0.0;
    
    while (total_timer.elapsed<milliseconds>().count() < 1000) {
        timer::performance_counter frame_timer;
        
        // Simulate variable frame work (0-10ms)
        auto work_time = 1ms + milliseconds(rand() % 10);
        std::this_thread::sleep_for(work_time);
        
        auto work_duration = static_cast<double>(frame_timer.elapsed<microseconds>().count()) / 1000.0;
        total_work_time += work_duration;
        
        // Wait for next frame
        limiter.wait_for_next_frame();
        
        // Update stats
        frame_count++;
        double fps = limiter.get_fps();
        min_fps = std::min(min_fps, fps);
        max_fps = std::max(max_fps, fps);
    }
    
    auto total_elapsed = total_timer.elapsed<milliseconds>().count();
    
    std::cout << "\nFrame limiter statistics:\n";
    std::cout << "  Total frames: " << frame_count << "\n";
    std::cout << "  Total time: " << total_elapsed << " ms\n";
    std::cout << "  Average FPS: " << (frame_count * 1000.0 / static_cast<double>(total_elapsed)) << "\n";
    std::cout << "  Min FPS: " << min_fps << "\n";
    std::cout << "  Max FPS: " << max_fps << "\n";
    std::cout << "  Average work time: " << (total_work_time / frame_count) << " ms/frame\n";
}

void high_resolution_clock_example() {
    std::cout << "\n=== High Resolution Clock Example ===\n";
    
    // Compare different clock precisions
    const int iterations = 1000;
    
    // SDL millisecond clock
    {
        auto start = timer::clock::now();
        for (int i = 0; i < iterations; ++i) {
            [[maybe_unused]] volatile auto now = timer::clock::now();
        }
        auto end = timer::clock::now();
        auto duration = (end - start).count();
        std::cout << "SDL clock: " << iterations << " iterations took " 
                  << duration << " ms\n";
    }
    
    // High resolution clock
    {
        auto start = timer::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            [[maybe_unused]] volatile auto now = timer::high_resolution_clock::now();
        }
        auto end = timer::high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start).count();
        std::cout << "High-res clock: " << iterations << " iterations took " 
                  << duration << " μs\n";
    }
    
    // Measure clock resolution
    std::cout << "\nClock resolution test:\n";
    
    // SDL clock resolution
    {
        auto t1 = timer::clock::now();
        auto t2 = t1;
        while (t2 == t1) {
            t2 = timer::clock::now();
        }
        std::cout << "  SDL clock resolution: ~" << (t2 - t1).count() << " ms\n";
    }
    
    // High-res clock resolution
    {
        auto t1 = timer::high_resolution_clock::now();
        auto t2 = t1;
        int attempts = 0;
        while (t2 == t1 && attempts < 1000000) {
            t2 = timer::high_resolution_clock::now();
            attempts++;
        }
        auto diff = duration_cast<nanoseconds>(t2 - t1).count();
        std::cout << "  High-res clock resolution: ~" << diff << " ns\n";
    }
}

int main() {
    std::cout << "SDL++ Timer Example\n";
    std::cout << "===================\n";
    
    try {
        // Initialize SDL (timer functionality is always available in SDL3)
        sdlpp::init init(sdlpp::init_flags::none);
        
        if (!init.is_initialized()) {
            std::cerr << "Failed to initialize SDL\n";
            return 1;
        }
        
        // Run examples
        basic_timing_example();
        high_precision_timing_example();
        delay_example();
        scoped_timer_example();
        timer_callback_example();
        frame_limiter_example();
        high_resolution_clock_example();
        
        std::cout << "\nAll examples completed!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
