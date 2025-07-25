---
title: IOStream
sidebar_label: IOStream
---

# SDL IOStream Wrapper

SDL++ provides a modern C++ wrapper around SDL's IOStream functionality for unified file and memory I/O operations.

## Opening Files

### Basic File Operations

```cpp
#include <sdlpp/io/iostream.hh>

// Open file for reading
auto stream_result = sdlpp::iostream::open("data.txt", sdlpp::file_mode::read);
if (!stream_result) {
    std::cerr << "Failed to open file: " << stream_result.error() << std::endl;
    return;
}

auto& stream = stream_result.value();

// Read entire file
std::string content = stream.read_string();
std::cout << "File content: " << content << std::endl;
```

### File Modes

```cpp
enum class file_mode {
    read = SDL_IO_RDONLY,      // Read only
    write = SDL_IO_WRONLY,     // Write only (truncate)
    read_write = SDL_IO_RDWR,  // Read and write
    append = SDL_IO_APPEND,    // Append mode
    create = SDL_IO_CREATE     // Create if doesn't exist
};

// Combine modes
auto mode = sdlpp::file_mode::write | sdlpp::file_mode::create;
auto file = sdlpp::iostream::open("output.txt", mode);
```

### File Path Support

```cpp
// Using std::filesystem::path
std::filesystem::path config_path = get_config_directory() / "settings.ini";
auto stream = sdlpp::iostream::open(config_path, sdlpp::file_mode::read);

// Cross-platform path handling
#ifdef _WIN32
std::filesystem::path save_file = "C:/Users/Name/Documents/save.dat";
#else
std::filesystem::path save_file = "/home/user/Documents/save.dat";
#endif

auto save_stream = sdlpp::iostream::open(save_file, 
    sdlpp::file_mode::write | sdlpp::file_mode::create);
```

## Reading Data

### Reading Basic Types

```cpp
auto stream = sdlpp::iostream::open("data.bin", sdlpp::file_mode::read);
if (stream) {
    // Read 8-bit unsigned
    auto byte_result = stream->read_u8();
    if (byte_result) {
        Uint8 byte = byte_result.value();
    }
    
    // Read 16-bit (native endian)
    auto u16_result = stream->read_u16();
    
    // Read 32-bit (big endian)
    auto u32_be = stream->read_u32_be();
    
    // Read 64-bit (little endian)  
    auto u64_le = stream->read_u64_le();
    
    // Read signed integers
    auto i32_result = stream->read_s32();
    
    // Read floating point
    auto float_result = stream->read_float();
    auto double_result = stream->read_double();
}
```

### Reading Buffers

```cpp
// Read exact number of bytes
std::vector<Uint8> buffer(1024);
auto bytes_read = stream.read(buffer.data(), buffer.size());
if (bytes_read != buffer.size()) {
    std::cerr << "Short read: " << bytes_read << " bytes" << std::endl;
}

// Read up to a certain amount
std::vector<char> chunk(4096);
size_t total_read = 0;
while (!stream.eof()) {
    auto read = stream.read(chunk.data(), chunk.size());
    if (read > 0) {
        process_data(chunk.data(), read);
        total_read += read;
    }
}
```

### Reading Strings

```cpp
// Read entire file as string
std::string content = stream.read_string();

// Read fixed-size string
std::string fixed_str(256, '\0');
stream.read(fixed_str.data(), fixed_str.size());
fixed_str.resize(strlen(fixed_str.c_str())); // Trim nulls

// Read null-terminated string
std::string read_cstring(sdlpp::iostream& stream) {
    std::string result;
    char ch;
    while (stream.read(&ch, 1) == 1 && ch != '\0') {
        result += ch;
    }
    return result;
}
```

## Writing Data

### Writing Basic Types

```cpp
auto stream = sdlpp::iostream::open("output.bin", 
    sdlpp::file_mode::write | sdlpp::file_mode::create);

if (stream) {
    // Write 8-bit
    stream->write_u8(0xFF);
    
    // Write 16-bit (native endian)
    stream->write_u16(0x1234);
    
    // Write 32-bit (big endian)
    stream->write_u32_be(0x12345678);
    
    // Write 64-bit (little endian)
    stream->write_u64_le(0x123456789ABCDEF0);
    
    // Write signed integers
    stream->write_s32(-42);
    
    // Write floating point
    stream->write_float(3.14159f);
    stream->write_double(2.71828);
}
```

### Writing Buffers

```cpp
// Write buffer
std::vector<Uint8> data = {0x00, 0x01, 0x02, 0x03};
auto written = stream.write(data.data(), data.size());
if (written != data.size()) {
    std::cerr << "Short write: " << written << " bytes" << std::endl;
}

// Write string
std::string text = "Hello, SDL++!";
stream.write(text.c_str(), text.length());

// Write with null terminator
stream.write(text.c_str(), text.length() + 1);
```

## Seeking and Position

```cpp
// Get current position
Sint64 current_pos = stream.tell();

// Seek to absolute position
stream.seek(100); // Seek to byte 100

// Seek relative to current position
stream.seek(50, sdlpp::seek_from::current);

// Seek relative to end
stream.seek(-100, sdlpp::seek_from::end);

// Get file size
stream.seek(0, sdlpp::seek_from::end);
Sint64 file_size = stream.tell();
stream.seek(0); // Back to start

// Check if at end
bool at_end = stream.eof();
```

## Memory Streams

### Creating Memory Streams

```cpp
// Read-only memory stream
std::vector<Uint8> data = load_resource();
auto mem_stream = sdlpp::iostream::from_memory(data.data(), data.size());

// Read/write memory stream
std::vector<Uint8> buffer(1024);
auto rw_stream = sdlpp::iostream::from_memory_rw(buffer.data(), buffer.size());

// Dynamic memory stream (grows as needed)
auto dynamic_stream = sdlpp::iostream::from_dynamic_memory();
```

### Memory Stream Usage

```cpp
// Write to dynamic memory stream
auto stream = sdlpp::iostream::from_dynamic_memory();
stream->write_string("Header\n");
stream->write_u32(12345);
stream->write_float(3.14f);

// Get resulting buffer
auto buffer_size = stream->tell();
stream->seek(0);
std::vector<Uint8> result(buffer_size);
stream->read(result.data(), result.size());
```

## Error Handling

```cpp
// Check stream status
if (!stream.is_valid()) {
    std::cerr << "Invalid stream" << std::endl;
}

// Get error details
auto status = stream.get_status();
if (status == sdlpp::io_status::error) {
    std::string error = sdlpp::get_error();
    std::cerr << "IO Error: " << error << std::endl;
}

// Safe reading with error checking
template<typename T>
std::optional<T> safe_read(sdlpp::iostream& stream, 
                          auto (sdlpp::iostream::*read_func)()) {
    auto result = (stream.*read_func)();
    if (!result) {
        std::cerr << "Read error: " << result.error() << std::endl;
        return std::nullopt;
    }
    return result.value();
}
```

## Common Patterns

### File Reader Class

```cpp
class binary_file_reader {
    sdlpp::iostream stream;
    std::string filename;
    
public:
    explicit binary_file_reader(const std::filesystem::path& path)
        : filename(path.string()) {
        auto result = sdlpp::iostream::open(path, sdlpp::file_mode::read);
        if (result) {
            stream = std::move(result.value());
        }
    }
    
    bool is_open() const {
        return stream.is_valid();
    }
    
    template<typename T>
    bool read_struct(T& out) {
        static_assert(std::is_trivially_copyable_v<T>);
        auto bytes = stream.read(&out, sizeof(T));
        return bytes == sizeof(T);
    }
    
    std::vector<Uint8> read_all() {
        stream.seek(0, sdlpp::seek_from::end);
        size_t size = stream.tell();
        stream.seek(0);
        
        std::vector<Uint8> buffer(size);
        stream.read(buffer.data(), buffer.size());
        return buffer;
    }
};
```

### Serialization Helper

```cpp
class serializer {
    sdlpp::iostream& stream;
    
public:
    explicit serializer(sdlpp::iostream& s) : stream(s) {}
    
    // Write primitives
    serializer& operator<<(Uint8 value) {
        stream.write_u8(value);
        return *this;
    }
    
    serializer& operator<<(Uint32 value) {
        stream.write_u32_le(value);
        return *this;
    }
    
    serializer& operator<<(float value) {
        stream.write_float(value);
        return *this;
    }
    
    // Write string
    serializer& operator<<(const std::string& str) {
        stream.write_u32_le(str.length());
        stream.write(str.c_str(), str.length());
        return *this;
    }
    
    // Write vector
    template<typename T>
    serializer& operator<<(const std::vector<T>& vec) {
        stream.write_u32_le(vec.size());
        for (const auto& item : vec) {
            *this << item;
        }
        return *this;
    }
};

class deserializer {
    sdlpp::iostream& stream;
    
public:
    explicit deserializer(sdlpp::iostream& s) : stream(s) {}
    
    // Read primitives
    deserializer& operator>>(Uint8& value) {
        auto result = stream.read_u8();
        if (result) value = result.value();
        return *this;
    }
    
    deserializer& operator>>(Uint32& value) {
        auto result = stream.read_u32_le();
        if (result) value = result.value();
        return *this;
    }
    
    deserializer& operator>>(float& value) {
        auto result = stream.read_float();
        if (result) value = result.value();
        return *this;
    }
    
    // Read string
    deserializer& operator>>(std::string& str) {
        Uint32 length;
        *this >> length;
        
        str.resize(length);
        stream.read(str.data(), length);
        return *this;
    }
    
    // Read vector
    template<typename T>
    deserializer& operator>>(std::vector<T>& vec) {
        Uint32 size;
        *this >> size;
        
        vec.clear();
        vec.reserve(size);
        
        for (Uint32 i = 0; i < size; ++i) {
            T item;
            *this >> item;
            vec.push_back(std::move(item));
        }
        return *this;
    }
};
```

### Archive File Reader

```cpp
class archive_reader {
    struct file_entry {
        std::string name;
        Uint64 offset;
        Uint64 size;
        Uint32 crc32;
    };
    
    sdlpp::iostream stream;
    std::vector<file_entry> entries;
    
public:
    bool open(const std::filesystem::path& path) {
        auto result = sdlpp::iostream::open(path, sdlpp::file_mode::read);
        if (!result) return false;
        
        stream = std::move(result.value());
        return read_header();
    }
    
    std::vector<std::string> list_files() const {
        std::vector<std::string> names;
        for (const auto& entry : entries) {
            names.push_back(entry.name);
        }
        return names;
    }
    
    std::optional<std::vector<Uint8>> read_file(const std::string& name) {
        auto it = std::find_if(entries.begin(), entries.end(),
            [&](const file_entry& e) { return e.name == name; });
            
        if (it == entries.end()) {
            return std::nullopt;
        }
        
        stream.seek(it->offset);
        std::vector<Uint8> data(it->size);
        
        if (stream.read(data.data(), data.size()) != it->size) {
            return std::nullopt;
        }
        
        // Verify CRC32
        if (calculate_crc32(data) != it->crc32) {
            return std::nullopt;
        }
        
        return data;
    }
    
private:
    bool read_header() {
        // Read magic number
        char magic[4];
        if (stream.read(magic, 4) != 4 || 
            memcmp(magic, "ARCH", 4) != 0) {
            return false;
        }
        
        // Read file count
        auto count_result = stream.read_u32_le();
        if (!count_result) return false;
        
        Uint32 file_count = count_result.value();
        entries.reserve(file_count);
        
        // Read file entries
        for (Uint32 i = 0; i < file_count; ++i) {
            file_entry entry;
            
            // Read name length and name
            auto name_len = stream.read_u32_le();
            if (!name_len) return false;
            
            entry.name.resize(name_len.value());
            if (stream.read(entry.name.data(), name_len.value()) != 
                name_len.value()) {
                return false;
            }
            
            // Read offset, size, CRC
            auto offset = stream.read_u64_le();
            auto size = stream.read_u64_le();
            auto crc = stream.read_u32_le();
            
            if (!offset || !size || !crc) return false;
            
            entry.offset = offset.value();
            entry.size = size.value();
            entry.crc32 = crc.value();
            
            entries.push_back(std::move(entry));
        }
        
        return true;
    }
    
    static Uint32 calculate_crc32(const std::vector<Uint8>& data) {
        // CRC32 implementation
        return 0; // Placeholder
    }
};
```

## Best Practices

1. **Always Check Results**: File operations can fail
2. **Use RAII**: Stream automatically closes when destroyed
3. **Prefer Binary**: For data files, binary is more efficient
4. **Handle Endianness**: Use BE/LE functions for portability
5. **Buffer Large Operations**: Don't read/write byte by byte

## Example: Game Save System

```cpp
class save_game_system {
    static constexpr Uint32 SAVE_VERSION = 1;
    static constexpr char MAGIC[4] = {'S', 'A', 'V', 'E'};
    
public:
    struct save_data {
        std::string player_name;
        Uint32 level;
        Uint32 score;
        float play_time;
        std::vector<std::string> unlocked_items;
        std::unordered_map<std::string, int> stats;
    };
    
    static bool save(const std::filesystem::path& path, 
                    const save_data& data) {
        auto stream = sdlpp::iostream::open(path,
            sdlpp::file_mode::write | sdlpp::file_mode::create);
        
        if (!stream) return false;
        
        // Write header
        stream->write(MAGIC, 4);
        stream->write_u32_le(SAVE_VERSION);
        
        // Write timestamp
        auto now = std::chrono::system_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count();
        stream->write_u64_le(timestamp);
        
        // Serialize data
        serializer s(*stream);
        s << data.player_name
          << data.level
          << data.score
          << data.play_time
          << data.unlocked_items;
        
        // Write stats
        s << static_cast<Uint32>(data.stats.size());
        for (const auto& [key, value] : data.stats) {
            s << key << value;
        }
        
        return true;
    }
    
    static std::optional<save_data> load(const std::filesystem::path& path) {
        auto stream = sdlpp::iostream::open(path, sdlpp::file_mode::read);
        if (!stream) return std::nullopt;
        
        // Verify header
        char magic[4];
        if (stream->read(magic, 4) != 4 || 
            memcmp(magic, MAGIC, 4) != 0) {
            return std::nullopt;
        }
        
        // Check version
        auto version = stream->read_u32_le();
        if (!version || version.value() != SAVE_VERSION) {
            return std::nullopt;
        }
        
        // Skip timestamp
        stream->seek(8, sdlpp::seek_from::current);
        
        // Deserialize data
        save_data data;
        deserializer d(*stream);
        d >> data.player_name
          >> data.level
          >> data.score
          >> data.play_time
          >> data.unlocked_items;
        
        // Read stats
        Uint32 stat_count;
        d >> stat_count;
        
        for (Uint32 i = 0; i < stat_count; ++i) {
            std::string key;
            int value;
            d >> key >> value;
            data.stats[key] = value;
        }
        
        return data;
    }
};
```