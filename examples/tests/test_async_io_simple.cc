#include <iostream>
#include <sdlpp/core/core.hh>
#include <sdlpp/io/async_io.hh>

int main() {
    try {
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "Creating async I/O queue...\n";
        auto queue_result = sdlpp::async_io_queue::create();
        if (!queue_result) {
            std::cerr << "Failed to create queue: " << queue_result.error() << "\n";
            return 1;
        }
        std::cout << "Queue created successfully\n";
        
        // Try the simplest possible async operation
        auto& queue = *queue_result;
        
        // Create a simple test file
        const char* test_content = "Hello, async world!\n";
        FILE* fp = fopen("test_simple.txt", "w");
        if (fp) {
            fputs(test_content, fp);
            fclose(fp);
            std::cout << "Test file created\n";
        }
        
        // Try to open it for async reading
        std::cout << "Opening file for async I/O...\n";
        auto file_result = sdlpp::async_io::open_file(std::string("test_simple.txt"), sdlpp::file_mode::read, queue);
        if (!file_result) {
            std::cerr << "Failed to open file: " << file_result.error() << "\n";
            std::remove("test_simple.txt");
            return 1;
        }
        std::cout << "File opened successfully\n";
        
        auto& file = *file_result;
        
        // Get file size
        auto size_result = file.size();
        if (!size_result) {
            std::cerr << "Failed to get file size: " << size_result.error() << "\n";
            std::remove("test_simple.txt");
            return 1;
        }
        std::cout << "File size: " << *size_result << " bytes\n";
        
        // Clean up
        std::remove("test_simple.txt");
        std::cout << "Test completed successfully\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}