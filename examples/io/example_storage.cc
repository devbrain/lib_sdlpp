#include <sdlpp/io/storage.hh>
#include <sdlpp/core/core.hh>
#include <../include/sdlpp/core/log.hh>

#include <iostream>
#include <iomanip>
#include <cstring>
#include <unordered_map>

// Helper to print file info
void print_file_info(const sdlpp::path_info& info, const std::string& name) {
    std::cout << "  " << std::left << std::setw(20) << name;
    
    switch (info.type) {
        case sdlpp::path_type::file:
            std::cout << "FILE   " << std::setw(10) << info.size << " bytes";
            break;
        case sdlpp::path_type::directory:
            std::cout << "DIR    " << std::setw(10) << "-";
            break;
        case sdlpp::path_type::other:
            std::cout << "OTHER  " << std::setw(10) << "-";
            break;
        default:
            std::cout << "NONE   " << std::setw(10) << "-";
            break;
    }
    
    if (info.modify_time > 0) {
        auto time_point = std::chrono::system_clock::from_time_t(info.modify_time);
        auto mod_time = std::chrono::system_clock::to_time_t(time_point);
        std::cout << "  Modified: " << std::put_time(std::localtime(&mod_time), "%Y-%m-%d %H:%M:%S");
    }
    
    std::cout << "\n";
}

int main() {
    // Initialize SDL
    auto init = sdlpp::init(sdlpp::init_flags::none);
    if (!init) {
        std::cerr << "Failed to initialize SDL\n";
        return 1;
    }

    std::cout << "SDL++ Storage Example\n";
    std::cout << "====================\n\n";

    // Example 1: File Storage (for testing/development)
    std::cout << "=== File Storage Example ===\n";
    {
        // Create a temporary directory for our file storage
        auto temp_path = std::filesystem::temp_directory_path() / "sdlpp_storage_example";
        std::filesystem::create_directories(temp_path);
        
        auto storage_result = sdlpp::storage::open_file(temp_path);
        if (!storage_result) {
            std::cerr << "Failed to open file storage: " << storage_result.error() << "\n";
            return 1;
        }
        
        auto& storage = storage_result.value();
        std::cout << "Opened file storage at: " << temp_path << "\n";
        std::cout << "Storage ready: " << (storage.is_ready() ? "Yes" : "No") << "\n";
        std::cout << "Space remaining: " << storage.get_space_remaining() << " bytes\n\n";
        
        // Write some files
        std::cout << "Writing test files...\n";
        [[maybe_unused]] auto r1 = storage.write_file("readme.txt", "This is a test storage system.");
        [[maybe_unused]] auto r2 = storage.write_file("data.bin", std::string_view("\x01\x02\x03\x04\x05", 5));
        [[maybe_unused]] auto r3 = storage.create_directory("logs");
        [[maybe_unused]] auto r4 = storage.write_file("logs/app.log", "Application started\nOperation completed\n");
        
        // List directory contents
        std::cout << "\nDirectory listing:\n";
        auto list_result = storage.list_directory("/");
        if (list_result) {
            for (const auto& entry : list_result.value()) {
                auto info_result = storage.get_path_info(entry);
                if (info_result) {
                    print_file_info(info_result.value(), entry);
                }
            }
        }
        
        // Demonstrate glob functionality
        // NOTE: SDL_GlobStorageDirectory has a known bug where it truncates
        // the first few characters of filenames
        std::cout << "\nText files (*.txt):\n";
        auto glob_result = storage.glob_directory("/", "*.txt");
        if (glob_result) {
            for (const auto& file : glob_result.value()) {
                std::cout << "  " << file << "\n";
            }
            if (!glob_result->empty()) {
                std::cout << "  (Note: SDL may truncate filenames)\n";
            }
        }
        
        // Read a file
        std::cout << "\nReading readme.txt:\n";
        auto read_result = storage.read_file("readme.txt");
        if (read_result) {
            std::string content(
                reinterpret_cast<const char*>(read_result->data()),
                read_result->size()
            );
            std::cout << "  Content: \"" << content << "\"\n";
        }
        
        // Copy and rename operations
        std::cout << "\nFile operations:\n";
        if (storage.copy_file("readme.txt", "readme_backup.txt")) {
            std::cout << "  Copied readme.txt to readme_backup.txt\n";
        }
        
        if (storage.rename_path("data.bin", "data_renamed.bin")) {
            std::cout << "  Renamed data.bin to data_renamed.bin\n";
        }
        
        // Clean up (optional, directory will be cleaned when storage is destroyed)
        std::filesystem::remove_all(temp_path);
    }
    
    // Example 2: User Storage (persistent user data)
    std::cout << "\n=== User Storage Example ===\n";
    {
        auto user_storage_result = sdlpp::storage::open_user("MyCompany", "MyApp");
        if (!user_storage_result) {
            std::cout << "Failed to open user storage: " << user_storage_result.error() << "\n";
            std::cout << "(This is normal on some platforms)\n";
        } else {
            auto& storage = user_storage_result.value();
            std::cout << "Opened user storage\n";
            std::cout << "Storage ready: " << (storage.is_ready() ? "Yes" : "No") << "\n";
            
            // Save user preferences
            const std::string prefs = R"({
    "volume": 0.8,
    "fullscreen": false,
    "language": "en"
})";
            
            if (storage.write_file("preferences.json", prefs)) {
                std::cout << "Saved user preferences\n";
                
                // Read them back
                auto read_result = storage.read_file("preferences.json");
                if (read_result) {
                    std::cout << "Successfully read back preferences\n";
                }
            }
            
            // Create a save game directory
            if (storage.create_directory("saves")) {
                [[maybe_unused]] auto r5 = storage.write_file("saves/autosave.dat", "Game state data here");
                std::cout << "Created save game directory and file\n";
            }
        }
    }
    
    // Example 3: Custom Storage Implementation
    std::cout << "\n=== Custom Storage Example ===\n";
    {
        // This is a simple in-memory storage implementation
        struct memory_storage {
            std::unordered_map<std::string, std::vector<uint8_t>> files;
            
            static bool close_impl([[maybe_unused]] void* userdata) {
                // Clean up if needed
                return true;
            }
            
            static bool ready_impl(void* userdata) {
                return userdata != nullptr;
            }
            
            static bool write_file_impl(void* userdata, const char* path, 
                                       const void* source, uint64_t length) {
                auto* storage = static_cast<memory_storage*>(userdata);
                std::vector<uint8_t> data(
                    static_cast<const uint8_t*>(source),
                    static_cast<const uint8_t*>(source) + length
                );
                storage->files[path] = std::move(data);
                return true;
            }
            
            static bool read_file_impl(void* userdata, const char* path,
                                      void* destination, uint64_t length) {
                auto* storage = static_cast<memory_storage*>(userdata);
                auto it = storage->files.find(path);
                if (it == storage->files.end() || it->second.size() != length) {
                    return false;
                }
                std::memcpy(destination, it->second.data(), length);
                return true;
            }
            
            static bool info_impl(void* userdata, const char* path, SDL_PathInfo* info) {
                auto* storage = static_cast<memory_storage*>(userdata);
                auto it = storage->files.find(path);
                if (it == storage->files.end()) {
                    info->type = static_cast<SDL_PathType>(sdlpp::path_type::none);
                    return false;
                }
                info->type = static_cast<SDL_PathType>(sdlpp::path_type::file);
                info->size = it->second.size();
                info->modify_time = std::time(nullptr);
                return true;
            }
        };
        
        memory_storage mem_storage;
        
        sdlpp::storage_interface iface{
            .close = memory_storage::close_impl,
            .ready = memory_storage::ready_impl,
            .info = memory_storage::info_impl,
            .read_file = memory_storage::read_file_impl,
            .write_file = memory_storage::write_file_impl,
        };
        
        auto custom_storage_result = sdlpp::storage::open_custom(iface, &mem_storage);
        if (custom_storage_result) {
            auto& storage = custom_storage_result.value();
            std::cout << "Created custom in-memory storage\n";
            
            // Use it like any other storage
            [[maybe_unused]] auto r6 = storage.write_file("memory.txt", "This is stored in memory!");
            
            auto info_result = storage.get_path_info("memory.txt");
            if (info_result) {
                std::cout << "File 'memory.txt' exists with size: " 
                         << info_result->size << " bytes\n";
            }
        }
    }
    
    std::cout << "\nStorage example completed.\n";
    return 0;
}