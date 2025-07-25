//
// Created by igor on 7/14/25.
//

#include <doctest/doctest.h>
#include <../include/sdlpp/core/timer.hh>
#include <thread>
#include <atomic>
#include <vector>

TEST_CASE("Timer basic functionality") {
    using namespace sdlpp;
    using namespace std::chrono;
    
    SUBCASE("Elapsed time measurement") {
        auto start = timer::elapsed();
        std::this_thread::sleep_for(milliseconds(50));
        auto end = timer::elapsed();
        
        auto diff = end - start;
        CHECK(diff.count() >= 40); // Allow some timing variance
        CHECK(diff.count() <= 100); // But not too much
    }
    
    SUBCASE("Elapsed time measurement (nanoseconds)") {
        auto start = timer::elapsed_ns();
        std::this_thread::sleep_for(microseconds(500));
        auto end = timer::elapsed_ns();
        
        auto diff = end - start;
        CHECK(diff.count() >= 400'000); // At least 400μs in nanoseconds
        CHECK(diff.count() <= 1'000'000); // At most 1ms in nanoseconds
    }
    
    SUBCASE("Elapsed since") {
        auto start = timer::elapsed();
        std::this_thread::sleep_for(milliseconds(30));
        auto elapsed = timer::elapsed_since(start);
        
        CHECK(elapsed.count() >= 20);
        CHECK(elapsed.count() <= 50);
    }
    
    SUBCASE("Elapsed since (nanoseconds)") {
        auto start = timer::elapsed_ns();
        std::this_thread::sleep_for(microseconds(100));
        auto elapsed = timer::elapsed_since_ns(start);
        
        CHECK(elapsed.count() >= 80'000); // At least 80μs
        CHECK(elapsed.count() <= 200'000); // At most 200μs
    }
    
    SUBCASE("SDL clock") {
        auto t1 = timer::clock::now();
        std::this_thread::sleep_for(milliseconds(10));
        auto t2 = timer::clock::now();
        
        auto duration = t2 - t1;
        CHECK(duration.count() >= 5);
        CHECK(duration.count() <= 30);
    }
}

TEST_CASE("Performance counter") {
    using namespace sdlpp;
    using namespace std::chrono;
    
    SUBCASE("Basic measurement") {
        timer::performance_counter counter;
        std::this_thread::sleep_for(milliseconds(20));
        
        auto elapsed_ms = counter.elapsed<milliseconds>();
        CHECK(elapsed_ms.count() >= 15);
        CHECK(elapsed_ms.count() <= 40);
        
        auto elapsed_us = counter.elapsed<microseconds>();
        CHECK(elapsed_us.count() >= 15000);
        CHECK(elapsed_us.count() <= 40000);
    }
    
    SUBCASE("Reset functionality") {
        timer::performance_counter counter;
        std::this_thread::sleep_for(milliseconds(10));
        
        counter.reset();
        std::this_thread::sleep_for(milliseconds(5));
        
        auto elapsed = counter.elapsed<milliseconds>();
        CHECK(elapsed.count() >= 3);
        CHECK(elapsed.count() <= 15);
    }
    
    SUBCASE("Frequency check") {
        auto freq = timer::performance_counter::get_frequency();
        CHECK(freq > 0);
        
        // Frequency should be consistent
        auto freq2 = timer::performance_counter::get_frequency();
        CHECK(freq == freq2);
    }
    
    SUBCASE("High resolution clock") {
        auto t1 = timer::high_resolution_clock::now();
        std::this_thread::sleep_for(microseconds(100));
        auto t2 = timer::high_resolution_clock::now();
        
        auto duration = duration_cast<microseconds>(t2 - t1);
        CHECK(duration.count() >= 50);
        CHECK(duration.count() <= 500);
    }
}

TEST_CASE("Delay functions") {
    using namespace sdlpp;
    using namespace std::chrono;
    
    SUBCASE("Basic delay") {
        timer::performance_counter counter;
        timer::delay(milliseconds(20));
        auto elapsed = counter.elapsed<milliseconds>();
        
        CHECK(elapsed.count() >= 15);
        CHECK(elapsed.count() <= 40);
    }
    
    SUBCASE("Precise delay") {
        timer::performance_counter counter;
        timer::delay_precise(microseconds(5000)); // 5ms
        auto elapsed = counter.elapsed<microseconds>();
        
        CHECK(elapsed.count() >= 3000);
        CHECK(elapsed.count() <= 10000);
    }
    
    SUBCASE("Sleep for") {
        timer::performance_counter counter;
        timer::sleep_for(milliseconds(15));
        auto elapsed = counter.elapsed<milliseconds>();
        
        CHECK(elapsed.count() >= 10);
        CHECK(elapsed.count() <= 30);
    }
    
    SUBCASE("Sleep until") {
        auto now = timer::clock::now();
        auto target = now + milliseconds(25);
        
        timer::performance_counter counter;
        timer::sleep_until(target);
        auto elapsed = counter.elapsed<milliseconds>();
        
        CHECK(elapsed.count() >= 20);
        CHECK(elapsed.count() <= 40);
    }
}

TEST_CASE("Timer callbacks") {
    using namespace sdlpp;
    using namespace std::chrono;
    
    // Initialize SDL (timer functionality is always available in SDL3)
    if (!SDL_Init(0)) {
        //WARN("Failed to initialize SDL");
        return;
    }
    
    SUBCASE("One-shot timer") {
        std::atomic<int> call_count{0};
        
        auto timer_result = timer_handle::create_oneshot(
            milliseconds(50),
            [&call_count]() {
                call_count++;
            }
        );
        
        REQUIRE(timer_result.has_value());
        auto& timer = timer_result.value();
        CHECK(timer.is_active());
        
        // Wait for timer to fire
        std::this_thread::sleep_for(milliseconds(100));
        
        CHECK(call_count == 1);
        CHECK(!timer.is_active()); // Should be inactive after firing
    }
    
    SUBCASE("Repeating timer") {
        std::atomic<int> call_count{0};
        
        auto timer_result = timer_handle::create_repeating(
            milliseconds(30),
            [&call_count]() {
                call_count++;
            }
        );
        
        REQUIRE(timer_result.has_value());
        auto& timer = timer_result.value();
        CHECK(timer.is_active());
        
        // Wait for multiple firings
        std::this_thread::sleep_for(milliseconds(100));
        
        CHECK(call_count >= 2); // Should fire at least twice
        CHECK(call_count <= 5); // But not too many times
        
        // Cancel the timer
        CHECK(timer.cancel());
        CHECK(!timer.is_active());
        
        int count_after_cancel = call_count;
        std::this_thread::sleep_for(milliseconds(50));
        CHECK(call_count == count_after_cancel); // No more calls after cancel
    }
    
    SUBCASE("Custom interval timer") {
        std::atomic<int> call_count{0};
        std::atomic<bool> should_continue{true};
        
        auto timer_result = timer_handle::create(
            milliseconds(20),
            [&call_count, &should_continue](milliseconds interval) {
                call_count++;
                if (call_count >= 3) {
                    should_continue = false;
                    return milliseconds(0); // Stop timer
                }
                return interval * 2; // Double the interval each time
            }
        );
        
        REQUIRE(timer_result.has_value());
        
        // Wait for timer to complete
        std::this_thread::sleep_for(milliseconds(200));
        
        CHECK(call_count == 3);
    }
    
    SUBCASE("Timer cancellation") {
        std::atomic<int> call_count{0};
        
        auto timer_result = timer_handle::create_repeating(
            milliseconds(20),
            [&call_count]() {
                call_count++;
            }
        );
        
        REQUIRE(timer_result.has_value());
        auto& timer = timer_result.value();
        
        // Cancel immediately
        CHECK(timer.cancel());
        
        std::this_thread::sleep_for(milliseconds(50));
        CHECK(call_count == 0); // Should not have fired
    }
    
    SUBCASE("Multiple timers") {
        std::atomic<int> count1{0}, count2{0}, count3{0};
        
        auto timer1 = timer_handle::create_repeating(milliseconds(20), [&count1]() { count1++; });
        auto timer2 = timer_handle::create_repeating(milliseconds(30), [&count2]() { count2++; });
        auto timer3 = timer_handle::create_repeating(milliseconds(40), [&count3]() { count3++; });
        
        REQUIRE(timer1.has_value());
        REQUIRE(timer2.has_value());
        REQUIRE(timer3.has_value());
        
        std::this_thread::sleep_for(milliseconds(100));
        
        // Check relative firing rates
        CHECK(count1 > count2);
        CHECK(count2 > count3);
    }
    
    SDL_Quit();
}

TEST_CASE("Scoped timer") {
    using namespace sdlpp;
    using namespace std::chrono;
    
    SUBCASE("Basic scoped timing") {
        std::optional<nanoseconds> recorded_time;
        
        {
            scoped_timer timer("Test Operation", 
                [&recorded_time](const std::string& name, nanoseconds elapsed) {
                    CHECK(name == "Test Operation");
                    recorded_time = elapsed;
                }
            );
            
            std::this_thread::sleep_for(milliseconds(10));
        }
        
        REQUIRE(recorded_time.has_value());
        auto ms = duration_cast<milliseconds>(*recorded_time);
        CHECK(ms.count() >= 5);
        CHECK(ms.count() <= 30);
    }
    
    SUBCASE("Nested scoped timers") {
        std::vector<std::pair<std::string, nanoseconds>> timings;
        
        auto callback = [&timings](const std::string& name, nanoseconds elapsed) {
            timings.emplace_back(name, elapsed);
        };
        
        {
            scoped_timer outer("Outer", callback);
            std::this_thread::sleep_for(milliseconds(5));
            
            {
                scoped_timer inner("Inner", callback);
                std::this_thread::sleep_for(milliseconds(10));
            }
            
            std::this_thread::sleep_for(milliseconds(5));
        }
        
        REQUIRE(timings.size() == 2);
        CHECK(timings[0].first == "Inner");
        CHECK(timings[1].first == "Outer");
        
        // Outer should take longer than inner
        CHECK(timings[1].second > timings[0].second);
    }
}

TEST_CASE("Frame limiter") {
    using namespace sdlpp;
    using namespace std::chrono;
    
    SUBCASE("FPS limiting") {
        frame_limiter limiter(60.0); // 60 FPS
        
        timer::performance_counter total_time;
        int frame_count = 0;
        
        // Run for approximately 100ms
        while (total_time.elapsed<milliseconds>().count() < 100) {
            // Simulate some work
            std::this_thread::sleep_for(microseconds(500));
            
            limiter.wait_for_next_frame();
            frame_count++;
        }
        
        // Should be approximately 6 frames in 100ms at 60 FPS
        CHECK(frame_count >= 4);
        CHECK(frame_count <= 8);
    }
    
    SUBCASE("Frame time reporting") {
        frame_limiter limiter(milliseconds(20)); // 50 FPS
        
        std::this_thread::sleep_for(milliseconds(10));
        auto frame_time = limiter.get_frame_time();
        auto frame_ms = duration_cast<milliseconds>(frame_time);
        
        CHECK(frame_ms.count() >= 5);
        CHECK(frame_ms.count() <= 20);
        
        auto fps = limiter.get_fps();
        CHECK(fps > 0);
        CHECK(fps <= 200); // Reasonable upper bound
    }
    
    SUBCASE("Reset functionality") {
        frame_limiter limiter(30.0); // 30 FPS
        
        std::this_thread::sleep_for(milliseconds(50));
        limiter.reset();
        
        auto frame_time = limiter.get_frame_time();
        CHECK(frame_time.count() < 10'000'000); // Less than 10ms
    }
}
