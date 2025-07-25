//
// Created by igor on 7/14/25.
//
// Simple test program for timer wrapper

#include <iostream>
#include <sdlpp/core/timer.hh>
#include <sdlpp/core/error.hh>
#include <thread>
#include <iomanip>

using namespace sdlpp;
using namespace std::chrono;

int main() {
    std::cout << "SDL++ Timer Test\n";
    std::cout << "=================\n\n";
    
    // Initialize SDL
    if (!SDL_Init(0)) {
        std::cerr << "Failed to initialize SDL: " << sdlpp::get_error() << "\n";
        return 1;
    }
    
    // Test basic timing
    std::cout << "1. Basic timing test:\n";
    auto start = timer::elapsed();
    std::cout << "   Start time: " << start.count() << " ms\n";
    
    std::this_thread::sleep_for(100ms);
    
    auto end = timer::elapsed();
    std::cout << "   End time: " << end.count() << " ms\n";
    std::cout << "   Elapsed: " << (end - start).count() << " ms\n\n";
    
    // Test performance counter
    std::cout << "2. Performance counter test:\n";
    timer::performance_counter counter;
    
    // Do some work
    volatile int sum = 0;
    for (int i = 0; i < 10000000; ++i) {
        sum += i;
    }
    
    auto elapsed_ns = counter.elapsed<nanoseconds>();
    auto elapsed_us = counter.elapsed<microseconds>();
    auto elapsed_ms = counter.elapsed<milliseconds>();
    
    std::cout << "   Work completed in:\n";
    std::cout << "   - " << elapsed_ns.count() << " ns\n";
    std::cout << "   - " << elapsed_us.count() << " μs\n";
    std::cout << "   - " << elapsed_ms.count() << " ms\n";
    std::cout << "   Sum: " << sum << "\n\n";
    
    // Test delays
    std::cout << "3. Delay test:\n";
    {
        std::cout << "   Testing 50ms delay...";
        timer::performance_counter delay_counter;
        timer::delay(50ms);
        auto actual = delay_counter.elapsed<milliseconds>();
        std::cout << " actual: " << actual.count() << " ms\n";
    }
    
    {
        std::cout << "   Testing 10ms precise delay...";
        timer::performance_counter delay_counter;
        timer::delay_precise(10ms);
        auto actual = delay_counter.elapsed<microseconds>();
        std::cout << " actual: " << actual.count() << " μs\n";
    }
    
    // Test scoped timer
    std::cout << "\n4. Scoped timer test:\n";
    {
        scoped_timer timer("Processing", 
            [](const std::string& name, nanoseconds elapsed) {
                auto ms = static_cast<double>(duration_cast<microseconds>(elapsed).count()) / 1000.0;
                std::cout << "   [" << name << "] took " 
                          << std::fixed << std::setprecision(3) 
                          << ms << " ms\n";
            });
        
        std::this_thread::sleep_for(25ms);
    }
    
    // Test frame limiter
    std::cout << "\n5. Frame limiter test (30 FPS for 0.5 seconds):\n";
    {
        frame_limiter limiter(30.0); // 30 FPS
        timer::performance_counter total;
        int frames = 0;
        
        while (total.elapsed<milliseconds>().count() < 500) {
            // Simulate variable work
            std::this_thread::sleep_for(milliseconds(5 + (frames % 3) * 5));
            
            limiter.wait_for_next_frame();
            frames++;
            
            if (frames % 5 == 0) {
                std::cout << "   Frame " << frames << ": " 
                          << std::fixed << std::setprecision(1)
                          << limiter.get_fps() << " FPS\n";
            }
        }
        
        std::cout << "   Total frames: " << frames << " in " 
                  << total.elapsed<milliseconds>().count() << " ms\n";
    }
    
    // Test clock types
    std::cout << "\n6. Clock comparison:\n";
    {
        auto sdl_t1 = timer::clock::now();
        auto hr_t1 = timer::high_resolution_clock::now();
        
        std::this_thread::sleep_for(1ms);
        
        auto sdl_t2 = timer::clock::now();
        auto hr_t2 = timer::high_resolution_clock::now();
        
        auto sdl_diff = (sdl_t2 - sdl_t1).count();
        auto hr_diff = duration_cast<microseconds>(hr_t2 - hr_t1).count();
        
        std::cout << "   SDL clock: " << sdl_diff << " ms\n";
        std::cout << "   High-res clock: " << hr_diff << " μs\n";
    }
    
    std::cout << "\nAll tests completed successfully!\n";
    
    SDL_Quit();
    return 0;
}
