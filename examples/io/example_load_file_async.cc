#include <iostream>
#include <vector>
#include <thread>
#include <cstdio>
#include <cstring>

#include <sdlpp/core/core.hh>
#include <sdlpp/io/async_io.hh>
#include <../include/sdlpp/core/timer.hh>

void load_file_async_example() {
    std::cout << "\n=== Load File Async Example ===\n";
    
    // Create async I/O queue
    auto queue_result = sdlpp::async_io_queue::create();
    if (!queue_result) {
        std::cerr << "Failed to create async queue: " << queue_result.error() << "\n";
        return;
    }
    auto& queue = *queue_result;
    
    // Create test files
    std::vector<std::string> filenames;
    for (int i = 0; i < 3; ++i) {
        std::string filename = "test_file_" + std::to_string(i) + ".txt";
        FILE* fp = fopen(filename.c_str(), "w");
        if (fp) {
            fprintf(fp, "This is test file %d\n", i);
            fprintf(fp, "It contains some test data.\n");
            fprintf(fp, "Line 3 for file %d\n", i);
            fclose(fp);
            filenames.push_back(filename);
            std::cout << "Created " << filename << "\n";
        }
    }
    
    // Start loading files asynchronously
    std::cout << "\nStarting async file loads...\n";
    for (const auto& filename : filenames) {
        if (!sdlpp::load_file_async(filename, queue)) {
            std::cerr << "Failed to start loading " << filename << "\n";
        }
    }
    
    // Process results
    std::cout << "\nWaiting for results...\n";
    int completed = 0;
    
    while (completed < static_cast<int>(filenames.size())) {
        SDL_AsyncIOOutcome outcome;
        
        if (queue.wait_result_raw(outcome, 1000)) {
            if (outcome.result == static_cast<int>(sdlpp::async_io_result::complete)) {
                auto result = sdlpp::get_load_file_result(outcome);
                if (result.is_valid()) {
                    std::cout << "File loaded successfully:\n";
                    std::cout << "  Size: " << result.size << " bytes\n";
                    std::cout << "  Content:\n" << result.as_string_view() << "\n";
                }
                completed++;
            } else if (outcome.result == static_cast<int>(sdlpp::async_io_result::error)) {
                std::cerr << "Failed to load file\n";
                completed++;
            }
        } else {
            std::cout << "Still waiting...\n";
        }
    }
    
    // Clean up
    for (const auto& filename : filenames) {
        std::remove(filename.c_str());
    }
    
    std::cout << "Example completed.\n";
}

void multiple_load_example() {
    std::cout << "\n=== Multiple Concurrent Loads Example ===\n";
    
    // Create async I/O queue
    auto queue_result = sdlpp::async_io_queue::create();
    if (!queue_result) {
        std::cerr << "Failed to create async queue: " << queue_result.error() << "\n";
        return;
    }
    auto& queue = *queue_result;
    
    // Create a larger test file
    const std::string big_file = "big_test_file.txt";
    FILE* fp = fopen(big_file.c_str(), "w");
    if (fp) {
        for (int i = 0; i < 1000; ++i) {
            fprintf(fp, "This is line %d of the big test file. It contains some repeated data to make it larger.\n", i);
        }
        fclose(fp);
        std::cout << "Created large test file\n";
    }
    
    // Start multiple loads of the same file
    std::cout << "Starting 5 concurrent loads of the same file...\n";
    for (int i = 0; i < 5; ++i) {
        // Use the iteration number as userdata
        if (!sdlpp::load_file_async(big_file, queue, reinterpret_cast<void*>(static_cast<std::intptr_t>(i)))) {
            std::cerr << "Failed to start load " << i << "\n";
        }
    }
    
    // Process results
    int completed = 0;
    auto start_time = std::chrono::steady_clock::now();
    
    while (completed < 5) {
        SDL_AsyncIOOutcome outcome;
        
        if (queue.wait_result_raw(outcome, 100)) {
            auto elapsed = std::chrono::steady_clock::now() - start_time;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
            
            if (outcome.result == static_cast<int>(sdlpp::async_io_result::complete)) {
                auto load_id = reinterpret_cast<std::intptr_t>(outcome.userdata);
                auto result = sdlpp::get_load_file_result(outcome);
                if (result.is_valid()) {
                    std::cout << "  Load " << load_id << " completed at " << ms 
                              << "ms, size: " << result.size << " bytes\n";
                }
                completed++;
            }
        }
    }
    
    // Clean up
    std::remove(big_file.c_str());
    std::cout << "Example completed.\n";
}

int main() {
    try {
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "SDL Load File Async Examples\n";
        std::cout << "============================\n";
        
        load_file_async_example();
        multiple_load_example();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}