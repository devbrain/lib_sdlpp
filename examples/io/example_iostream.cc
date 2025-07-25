#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <sdlpp/io/iostream.hh>
#include <sdlpp/core/core.hh>

void file_io_example() {
    std::cout << "\n=== File I/O Example ===\n";
    
    namespace fs = std::filesystem;
    
    // Write to a file
    {
        auto io_result = sdlpp::open_file(std::string("test_output.txt"), sdlpp::file_mode::write);
        if (!io_result) {
            std::cerr << "Failed to open file for writing: " << io_result.error() << "\n";
            return;
        }
        
        auto& io = *io_result;
        std::string data = "Hello from SDL iostream!\nThis is line 2.\nAnd this is line 3.\n";
        
        auto written_result = io.write(data.data(), data.size());
        if (!written_result || *written_result != data.size()) {
            std::cerr << "Failed to write all data\n";
        } else {
            std::cout << "Written " << *written_result << " bytes to file\n";
        }
    }
    
    // Read from a file using filesystem::path
    {
        fs::path file_path = "test_output.txt";
        auto io_result = sdlpp::open_file(file_path, sdlpp::file_mode::read);
        if (!io_result) {
            std::cerr << "Failed to open file for reading: " << io_result.error() << "\n";
            return;
        }
        
        auto& io = *io_result;
        auto size_result = io.size();
        
        if (size_result && *size_result > 0) {
            std::vector<char> buffer(static_cast<size_t>(*size_result));
            auto read_result = io.read(buffer.data(), buffer.size());
            
            if (read_result) {
                std::cout << "Read " << *read_result << " bytes from file:\n";
                std::cout.write(buffer.data(), static_cast<std::streamsize>(*read_result));
            }
            std::cout << "\n";
        }
    }
    
    // Test binary mode
    {
        auto io_result = sdlpp::open_file(std::string("test_binary.dat"), sdlpp::file_mode::write_binary);
        if (!io_result) {
            std::cerr << "Failed to open binary file: " << io_result.error() << "\n";
            return;
        }
        
        auto& io = *io_result;
        std::vector<uint32_t> data = {0x12345678, 0xABCDEF00, 0x11223344};
        
        auto written_result = io.write(data.data(), data.size() * sizeof(uint32_t));
        if (written_result) {
            std::cout << "Written " << *written_result << " bytes of binary data\n";
        }
    }
    
    // Clean up
    std::remove("test_output.txt");
    std::remove("test_binary.dat");
}

void memory_io_example() {
    std::cout << "\n=== Memory I/O Example ===\n";
    
    // Write to memory
    std::vector<uint8_t> buffer(1024);
    {
        auto io_result = sdlpp::from_memory(buffer.data(), buffer.size());
        if (!io_result) {
            std::cerr << "Failed to create memory IO: " << io_result.error() << "\n";
            return;
        }
        
        auto& io = *io_result;
        std::string data = "Data written to memory buffer";
        auto write_result = io.write(data.data(), data.size());
        
        // Seek back to beginning
        auto seek_result = io.seek(0, sdlpp::io_seek_pos::set);
        
        // Read back
        std::vector<char> read_buffer(data.size());
        auto read_result = io.read(read_buffer.data(), read_buffer.size());
        
        if (read_result) {
            std::cout << "Read from memory: ";
            std::cout.write(read_buffer.data(), static_cast<std::streamsize>(*read_result));
            std::cout << "\n";
        }
    }
}

int main() {
    try {
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "SDL IOStream Examples\n";
        std::cout << "====================\n";
        
        file_io_example();
        memory_io_example();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}