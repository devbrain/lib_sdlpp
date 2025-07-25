---
sidebar_position: 1
---

# Filesystem

The filesystem module provides comprehensive file I/O operations with both synchronous and asynchronous capabilities. It integrates seamlessly with `std::filesystem` and offers RAII-based resource management.

## I/O Streams

### Basic File Operations

```cpp
#include <sdlpp/filesystem/iostream.hh>

// Open file for reading
auto file = sdlpp::iostream::open("data.txt", sdlpp::file_mode::read);
if (!file) {
    std::cerr << "Failed to open file: " << file.error() << "\n";
    return;
}

// Read entire file as string
auto content = file->read_string();
if (!content) {
    std::cerr << "Failed to read: " << content.error() << "\n";
    return;
}

std::cout << "File content: " << *content << "\n";
```

### File Modes

```cpp
// Read mode (file must exist)
auto read_file = sdlpp::iostream::open("input.txt", sdlpp::file_mode::read);

// Write mode (creates new file or truncates existing)
auto write_file = sdlpp::iostream::open("output.txt", sdlpp::file_mode::write);

// Append mode (adds to end of file)
auto append_file = sdlpp::iostream::open("log.txt", sdlpp::file_mode::append);

// Read/Write mode (file must exist)
auto rw_file = sdlpp::iostream::open("data.bin", sdlpp::file_mode::read_write);

// Write/Read mode (creates new file or truncates existing)
auto wr_file = sdlpp::iostream::open("temp.dat", sdlpp::file_mode::write_read);

// Append/Read mode
auto ar_file = sdlpp::iostream::open("journal.txt", sdlpp::file_mode::append_read);
```

### Reading Data

```cpp
// Read specific number of bytes
std::vector<uint8_t> buffer(1024);
auto bytes_read = file->read(buffer.data(), buffer.size());
if (bytes_read) {
    std::cout << "Read " << *bytes_read << " bytes\n";
}

// Read entire file as vector
auto data = file->read_vector<uint8_t>();

// Read with specific type
auto integers = file->read_vector<int32_t>(100); // Read 100 integers

// Read single value
uint32_t magic_number;
auto result = file->read(&magic_number, sizeof(magic_number));
```

### Writing Data

```cpp
// Write string
auto result = file->write_string("Hello, World!\n");

// Write binary data
std::vector<float> values = {1.0f, 2.0f, 3.0f, 4.0f};
result = file->write(values.data(), values.size() * sizeof(float));

// Write with printf-style formatting
result = file->printf("Player score: %d\n", score);

// Write single values
uint32_t header = 0xDEADBEEF;
result = file->write(&header, sizeof(header));
```

### File Positioning

```cpp
// Get current position
auto pos = file->tell();
std::cout << "Current position: " << pos << "\n";

// Seek to specific position
file->seek(100); // Absolute position

// Seek relative to current position
file->seek(50, sdlpp::seek_from::current);

// Seek from end
file->seek(-100, sdlpp::seek_from::end);

// Get file size
auto size = file->size();
std::cout << "File size: " << *size << " bytes\n";
```

## Memory I/O

### Reading from Memory

```cpp
// Create read-only memory stream
std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"
auto mem_stream = sdlpp::iostream::from_const_mem(data.data(), data.size());

// Read from memory stream
auto content = mem_stream->read_string();
```

### Writing to Memory

```cpp
// Create writable memory stream
std::vector<uint8_t> buffer(1024);
auto mem_stream = sdlpp::iostream::from_mem(buffer.data(), buffer.size());

// Write to memory
mem_stream->write_string("Data in memory");

// Get amount written
auto bytes_written = mem_stream->tell();
```

### Dynamic Memory Streams

```cpp
// Create dynamic memory stream (grows as needed)
auto dyn_stream = sdlpp::iostream::from_dynamic_mem();

// Write data (memory grows automatically)
dyn_stream->write_string("This can grow to any size needed");
for (int i = 0; i < 1000; ++i) {
    dyn_stream->printf("Line %d\n", i);
}

// Get the data
auto size = dyn_stream->size();
std::vector<uint8_t> result(size);
dyn_stream->seek(0);
dyn_stream->read(result.data(), size);
```

## Asynchronous I/O

### Async File Operations

```cpp
#include <sdlpp/filesystem/async_io.hh>

// Open file for async operations
auto async_file = sdlpp::async_io::open("large_file.dat", sdlpp::file_mode::read);
if (!async_file) {
    std::cerr << "Failed to open: " << async_file.error() << "\n";
    return;
}

// Start async read
std::vector<uint8_t> buffer(1024 * 1024); // 1MB
auto task = async_file->read(buffer.data(), buffer.size(), 0); // Read from offset 0

// Do other work while reading...

// Wait for completion
auto result = task->wait();
if (result) {
    std::cout << "Read " << *result << " bytes\n";
} else {
    std::cerr << "Async read failed: " << result.error() << "\n";
}
```

### Async Write Operations

```cpp
// Async write
std::vector<uint8_t> data(1024 * 1024, 0xFF);
auto write_task = async_file->write(data.data(), data.size(), 0);

// Check if complete without blocking
if (write_task->try_wait()) {
    auto result = write_task->get_result();
    std::cout << "Write completed immediately\n";
} else {
    // Do other work, then wait
    auto result = write_task->wait();
}
```

### Multiple Async Operations

```cpp
// Queue multiple reads
std::vector<std::unique_ptr<sdlpp::async_io_task>> tasks;
std::vector<std::vector<uint8_t>> buffers(10);

for (int i = 0; i < 10; ++i) {
    buffers[i].resize(4096);
    auto task = async_file->read(
        buffers[i].data(), 
        buffers[i].size(), 
        i * 4096  // Offset
    );
    tasks.push_back(std::move(task));
}

// Wait for all to complete
for (auto& task : tasks) {
    auto result = task->wait();
    if (!result) {
        std::cerr << "Read failed: " << result.error() << "\n";
    }
}
```

## Path Utilities

### Working with Paths

```cpp
#include <sdlpp/filesystem/path.hh>

// Get base path (executable directory)
auto base_path = sdlpp::get_base_path();
std::cout << "Base path: " << base_path << "\n";

// Get preference path (user data)
auto pref_path = sdlpp::get_pref_path("MyCompany", "MyGame");
std::cout << "Preference path: " << pref_path << "\n";

// Combine paths
auto save_file = pref_path / "savegame.dat";
auto config_file = pref_path / "config" / "settings.json";
```

### Path Operations

```cpp
// Check if file exists
if (std::filesystem::exists(save_file)) {
    std::cout << "Save file found\n";
}

// Create directories
std::filesystem::create_directories(pref_path / "screenshots");

// List directory contents
for (const auto& entry : std::filesystem::directory_iterator(pref_path)) {
    std::cout << entry.path().filename() << "\n";
}
```

## Storage API

### Persistent Storage

```cpp
#include <sdlpp/filesystem/storage.hh>

// Open persistent storage
auto storage = sdlpp::storage_container::open("game_data");
if (!storage) {
    std::cerr << "Failed to open storage: " << storage.error() << "\n";
    return;
}

// Check if ready (for cloud sync)
if (storage->ready()) {
    // Read data
    auto save_data = storage->read("player_save.dat");
    if (save_data) {
        process_save_data(*save_data);
    }
    
    // Write data
    std::vector<uint8_t> new_save = create_save_data();
    auto result = storage->write("player_save.dat", new_save);
    if (!result) {
        std::cerr << "Failed to save: " << result.error() << "\n";
    }
}

// Get space info
auto space_info = storage->get_space_info();
std::cout << "Total space: " << space_info.total << " bytes\n";
std::cout << "Used space: " << space_info.used << " bytes\n";
```

### Storage Enumeration

```cpp
// List all files in storage
auto result = storage->enumerate([](const std::string& path) {
    std::cout << "Found: " << path << "\n";
});

// Get file info
auto info = storage->get_path_info("saves/slot1.dat");
if (info) {
    std::cout << "File size: " << info->size << " bytes\n";
    std::cout << "Modified: " << info->modify_time << "\n";
}

// Delete file
auto delete_result = storage->remove("old_save.dat");
if (!delete_result) {
    std::cerr << "Failed to delete: " << delete_result.error() << "\n";
}

// Rename file
auto rename_result = storage->rename("temp.dat", "final.dat");
```

## Common Patterns

### Configuration File Management

```cpp
class config_manager {
    std::filesystem::path config_path;
    
public:
    config_manager() {
        auto pref_path = sdlpp::get_pref_path("MyCompany", "MyGame");
        config_path = pref_path / "config.json";
    }
    
    auto load() -> sdlpp::expected<json, std::string> {
        auto file = sdlpp::iostream::open(config_path, sdlpp::file_mode::read);
        if (!file) {
            // Create default config
            return create_default_config();
        }
        
        auto content = file->read_string();
        if (!content) {
            return sdlpp::unexpected(content.error());
        }
        
        try {
            return json::parse(*content);
        } catch (const std::exception& e) {
            return sdlpp::unexpected(std::string("Parse error: ") + e.what());
        }
    }
    
    auto save(const json& config) -> sdlpp::expected<void, std::string> {
        // Ensure directory exists
        std::filesystem::create_directories(config_path.parent_path());
        
        auto file = sdlpp::iostream::open(config_path, sdlpp::file_mode::write);
        if (!file) {
            return sdlpp::unexpected(file.error());
        }
        
        return file->write_string(config.dump(2));
    }
};
```

### Binary File Format

```cpp
struct file_header {
    uint32_t magic = 0x12345678;
    uint32_t version = 1;
    uint64_t data_offset;
    uint64_t data_size;
};

class binary_file {
    sdlpp::iostream file;
    
public:
    auto write_data(const std::vector<uint8_t>& data) -> sdlpp::expected<void, std::string> {
        file_header header;
        header.data_offset = sizeof(file_header);
        header.data_size = data.size();
        
        // Write header
        TRY(file.write(&header, sizeof(header)));
        
        // Write data
        TRY(file.write(data.data(), data.size()));
        
        return {};
    }
    
    auto read_data() -> sdlpp::expected<std::vector<uint8_t>, std::string> {
        // Read header
        file_header header;
        TRY(file.read(&header, sizeof(header)));
        
        // Validate
        if (header.magic != 0x12345678) {
            return sdlpp::unexpected("Invalid file format");
        }
        
        // Read data
        std::vector<uint8_t> data(header.data_size);
        TRY(file.seek(header.data_offset));
        TRY(file.read(data.data(), data.size()));
        
        return data;
    }
};
```

### Streaming Large Files

```cpp
class file_streamer {
    static constexpr size_t CHUNK_SIZE = 1024 * 1024; // 1MB chunks
    
public:
    auto process_large_file(const std::filesystem::path& path,
                          std::function<void(const uint8_t*, size_t)> processor)
        -> sdlpp::expected<void, std::string> {
        
        auto file = TRY(sdlpp::iostream::open(path, sdlpp::file_mode::read));
        auto file_size = TRY(file->size());
        
        std::vector<uint8_t> buffer(CHUNK_SIZE);
        size_t total_read = 0;
        
        while (total_read < file_size) {
            auto to_read = std::min(CHUNK_SIZE, file_size - total_read);
            auto bytes_read = TRY(file->read(buffer.data(), to_read));
            
            processor(buffer.data(), bytes_read);
            total_read += bytes_read;
        }
        
        return {};
    }
};
```

## Error Handling

```cpp
// Comprehensive error handling
auto load_game_data() -> sdlpp::expected<game_data, std::string> {
    // Get save path
    auto pref_path = sdlpp::get_pref_path("MyCompany", "MyGame");
    if (pref_path.empty()) {
        return sdlpp::unexpected("Failed to get preference path");
    }
    
    auto save_path = pref_path / "save.dat";
    
    // Open file
    auto file = TRY(sdlpp::iostream::open(save_path, sdlpp::file_mode::read));
    
    // Read header
    save_header header;
    TRY(file->read(&header, sizeof(header)));
    
    // Validate version
    if (header.version > CURRENT_VERSION) {
        return sdlpp::unexpected("Save file version too new");
    }
    
    // Read data
    auto data = TRY(file->read_vector<uint8_t>(header.data_size));
    
    // Decompress if needed
    if (header.compressed) {
        data = TRY(decompress(data));
    }
    
    return deserialize_game_data(data);
}
```

## Best Practices

1. **Use RAII**: Files are automatically closed when iostream goes out of scope
2. **Check Results**: Always check `expected<T>` returns
3. **Prefer Async**: Use async I/O for large files to avoid blocking
4. **Buffer Appropriately**: Use reasonable buffer sizes for performance
5. **Validate Data**: Always validate file headers and data
6. **Handle Paths Safely**: Use std::filesystem for path operations
7. **Consider Compression**: Compress large save files

## Performance Tips

```cpp
// Pre-allocate buffers
std::vector<uint8_t> buffer;
buffer.reserve(1024 * 1024); // 1MB

// Use memory mapping for large files (through platform APIs)
// SDL doesn't provide memory mapping directly

// Batch small writes
class buffered_writer {
    sdlpp::iostream file;
    std::vector<uint8_t> buffer;
    static constexpr size_t BUFFER_SIZE = 65536;
    
public:
    void write(const void* data, size_t size) {
        auto bytes = static_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), bytes, bytes + size);
        
        if (buffer.size() >= BUFFER_SIZE) {
            flush();
        }
    }
    
    void flush() {
        if (!buffer.empty()) {
            file.write(buffer.data(), buffer.size());
            buffer.clear();
        }
    }
};
```

## API Reference

### Classes

- `iostream` - File I/O stream
- `async_io` - Asynchronous file I/O
- `async_io_task` - Async operation handle
- `storage_container` - Persistent storage
- `storage_interface` - Storage implementation interface

### Enums

- `file_mode` - File open modes
- `seek_from` - Seek origin

### Free Functions

- `get_base_path()` - Get executable directory
- `get_pref_path()` - Get user data directory
- `iostream::open()` - Open file
- `iostream::from_mem()` - Create memory stream
- `async_io::open()` - Open file for async I/O
- `storage_container::open()` - Open persistent storage