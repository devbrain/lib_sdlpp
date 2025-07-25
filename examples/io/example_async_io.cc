#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <iomanip>
#include <cstdio>
#include <cstring>

#include <sdlpp/core/core.hh>
#include <sdlpp/io/async_io.hh>
#include <../include/sdlpp/core/timer.hh>


using namespace std::chrono_literals;

void basic_async_read_example() {
    std::cout << "\n=== Basic Async Read Example ===\n";
    
    // Create async I/O queue
    auto queue_result = sdlpp::async_io_queue::create();
    if (!queue_result) {
        std::cerr << "Failed to create async queue: " << queue_result.error() << "\n";
        return;
    }
    auto& queue = *queue_result;
    
    // Create a test file
    const std::string test_file = "async_test.txt";
    FILE* fp = fopen(test_file.c_str(), "w");
    if (!fp) {
        std::cerr << "Failed to create test file\n";
        return;
    }
    fprintf(fp, "Hello, async I/O world!\nThis is line 2.\nAnd this is line 3.\n");
    fclose(fp);
    
    // Open file for async reading
    auto file_result = sdlpp::async_io::open_file(test_file, sdlpp::file_mode::read, queue);
    if (!file_result) {
        std::cerr << "Failed to open file: " << file_result.error() << "\n";
        return;
    }
    auto& file = *file_result;
    
    // Get file size
    auto size_result = file.size();
    if (!size_result) {
        std::cerr << "Failed to get file size: " << size_result.error() << "\n";
        return;
    }
    std::cout << "File size: " << *size_result << " bytes\n";
    
    // Start async read
    std::vector<std::uint8_t> buffer;
    auto read_task = file.read_async(0, static_cast<std::uint64_t>(*size_result), buffer);
    if (!read_task) {
        std::cerr << "Failed to start async read\n";
        return;
    }
    
    std::cout << "Async read started...\n";
    
    // Wait for completion
    std::shared_ptr<void> completed_task;
    sdlpp::async_io_outcome outcome;
    
    if (queue.wait_result(completed_task, outcome)) {
        if (outcome.is_complete()) {
            std::cout << "Read completed! Bytes transferred: " 
                      << outcome.bytes_transferred << "\n";
            std::cout << "Content: " << std::string(buffer.begin(), buffer.end());
        } else if (outcome.is_error()) {
            std::cerr << "Read failed: " << outcome.error_message << "\n";
        }
    }
    
    // Clean up
    std::remove(test_file.c_str());
}

void concurrent_operations_example() {
    std::cout << "\n=== Concurrent Operations Example ===\n";
    
    // Create async I/O queue
    auto queue_result = sdlpp::async_io_queue::create();
    if (!queue_result) {
        std::cerr << "Failed to create async queue: " << queue_result.error() << "\n";
        return;
    }
    auto& queue = *queue_result;
    
    // Create multiple test files
    std::vector<std::string> filenames;
    std::vector<sdlpp::async_io> files;
    std::vector<std::vector<std::uint8_t>> buffers(5);
    
    for (int i = 0; i < 5; ++i) {
        std::string filename = "async_test_" + std::to_string(i) + ".txt";
        filenames.push_back(filename);
        
        // Create file with content
        FILE* fp = fopen(filename.c_str(), "w");
        if (fp) {
            fprintf(fp, "File %d content: %d%d%d%d%d\n", i, i, i, i, i, i);
            fclose(fp);
        }
        
        // Open for async reading
        auto file_result = sdlpp::async_io::open_file(filename, sdlpp::file_mode::read, queue);
        if (file_result) {
            files.push_back(std::move(*file_result));
        }
    }
    
    // Start all reads concurrently
    std::cout << "Starting " << files.size() << " concurrent reads...\n";
    std::vector<std::shared_ptr<sdlpp::async_io::task_handle>> tasks;
    
    for (std::size_t i = 0; i < files.size(); ++i) {
        auto task = files[i].read_async(0, 50, buffers[i]);
        if (task) {
            tasks.push_back(task);
        }
    }
    
    // Process completions
    sdlpp::async_io_manager manager(queue);
    std::size_t completed = 0;
    
    [[maybe_unused]] auto start_time = std::chrono::steady_clock::now();
    
    manager.wait_and_process([&](const auto& handle, const auto& outcome) {
        auto elapsed = std::chrono::steady_clock::now() - handle->start_time;
        auto ms = static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count()) / 1000.0;
        
        if (outcome.is_complete()) {
            std::cout << "  Read completed in " << std::fixed << std::setprecision(3) 
                      << ms << " ms, bytes: " << outcome.bytes_transferred << "\n";
            ++completed;
        } else {
            std::cerr << "  Read failed: " << outcome.error_message << "\n";
        }
    }, -1ms);
    
    std::cout << "Completed " << completed << " operations\n";
    
    // Clean up
    for (const auto& filename : filenames) {
        std::remove(filename.c_str());
    }
}

void async_write_example() {
    std::cout << "\n=== Async Write Example ===\n";
    
    // Create async I/O queue
    auto queue_result = sdlpp::async_io_queue::create();
    if (!queue_result) {
        std::cerr << "Failed to create async queue: " << queue_result.error() << "\n";
        return;
    }
    auto& queue = *queue_result;
    
    // Open file for writing
    const std::string test_file = "async_write_test.txt";
    auto file_result = sdlpp::async_io::open_file(test_file, sdlpp::file_mode::write, queue);
    if (!file_result) {
        std::cerr << "Failed to open file: " << file_result.error() << "\n";
        return;
    }
    auto& file = *file_result;
    
    // Prepare data to write
    std::string data = "This is async write test data.\n";
    data += "Line 2 of the test file.\n";
    data += "And the final line.\n";
    
    // Start async write
    auto write_task = file.write_async(0, data.data(), data.size());
    if (!write_task) {
        std::cerr << "Failed to start async write\n";
        return;
    }
    
    std::cout << "Async write started (" << data.size() << " bytes)...\n";
    
    // Wait for completion
    std::shared_ptr<void> completed_task;
    sdlpp::async_io_outcome outcome;
    
    if (queue.wait_result(completed_task, outcome)) {
        if (outcome.is_complete()) {
            std::cout << "Write completed! Bytes transferred: " 
                      << outcome.bytes_transferred << "\n";
        } else if (outcome.is_error()) {
            std::cerr << "Write failed: " << outcome.error_message << "\n";
        }
    }
    
    // Close the file asynchronously
    auto close_task = file.close_async();
    if (close_task) {
        std::cout << "Async close started...\n";
        
        if (queue.wait_result(completed_task, outcome)) {
            if (outcome.is_complete()) {
                std::cout << "File closed successfully\n";
            }
        }
    }
    
    // Verify the write by reading back
    std::cout << "\nVerifying written content:\n";
    FILE* fp = fopen(test_file.c_str(), "r");
    if (fp) {
        char buffer[256];
        while (fgets(buffer, sizeof(buffer), fp)) {
            std::cout << "  " << buffer;
        }
        fclose(fp);
    }
    
    // Clean up
    std::remove(test_file.c_str());
}

void large_concurrent_reads_example() {
    std::cout << "\n=== Large Concurrent Reads Example ===\n";
    
    // Create async I/O queue
    auto queue_result = sdlpp::async_io_queue::create();
    if (!queue_result) {
        std::cerr << "Failed to create async queue: " << queue_result.error() << "\n";
        return;
    }
    auto& queue = *queue_result;
    
    // Create a large test file
    const std::string test_file = "async_cancel_test.txt";
    FILE* fp = fopen(test_file.c_str(), "w");
    if (!fp) {
        std::cerr << "Failed to create test file\n";
        return;
    }
    
    // Write 10MB of data
    std::vector<char> chunk(1024 * 1024, 'X');
    for (int i = 0; i < 10; ++i) {
        fwrite(chunk.data(), 1, chunk.size(), fp);
    }
    fclose(fp);
    
    // Open for async reading
    auto file_result = sdlpp::async_io::open_file(test_file, sdlpp::file_mode::read, queue);
    if (!file_result) {
        std::cerr << "Failed to open file: " << file_result.error() << "\n";
        std::remove(test_file.c_str());
        return;
    }
    auto& file = *file_result;
    
    // Start multiple reads
    std::vector<std::vector<std::uint8_t>> buffers(5);
    std::vector<std::shared_ptr<sdlpp::async_io::task_handle>> tasks;
    
    std::cout << "Starting 5 large reads...\n";
    for (int i = 0; i < 5; ++i) {
        buffers[static_cast<std::size_t>(i)].resize(2 * 1024 * 1024); // 2MB each
        auto task = file.read_async(static_cast<std::uint64_t>(i * 1024 * 1024), buffers[static_cast<std::size_t>(i)].data(), buffers[static_cast<std::size_t>(i)].size());
        if (task) {
            tasks.push_back(task);
        }
    }
    
    // Wait a bit, then cancel
    std::this_thread::sleep_for(10ms);
    std::cout << "Canceling all operations...\n";
    
    // Note: SDL3 doesn't support canceling async operations
    std::cout << "Waiting for operations to complete naturally...\n";
    
    // Process results
    sdlpp::async_io_manager manager(queue);
    std::atomic<int> completed{0}, canceled{0}, errors{0};
    
    while ((completed + canceled + errors) < 5) {
        manager.wait_and_process_ms([&]([[maybe_unused]] const auto& handle, const auto& outcome) {
            if (outcome.is_complete()) {
                completed++;
                std::cout << "  Operation completed (bytes: " 
                          << outcome.bytes_transferred << ")\n";
            } else if (outcome.is_canceled()) {
                canceled++;
                std::cout << "  Operation canceled\n";
            } else {
                errors++;
                std::cout << "  Operation failed: " << outcome.error_message << "\n";
            }
        }, 100);
    }
    
    std::cout << "Summary: " << completed << " completed, " 
              << canceled << " canceled, " << errors << " errors\n";
    
    // Clean up
    std::remove(test_file.c_str());
}

int main() {
    // Initialize SDL
    try {
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "SDL Async I/O Examples\n";
        std::cout << "======================\n";
        
        basic_async_read_example();
        concurrent_operations_example();
        async_write_example();
        large_concurrent_reads_example();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}