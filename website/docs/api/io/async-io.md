---
title: Asynchronous I/O
sidebar_label: Async I/O
---

# Asynchronous I/O Operations

SDL++ provides asynchronous file I/O operations that allow non-blocking file access, improving application responsiveness.

## Creating Async Operations

### Opening Files Asynchronously

```cpp
#include <sdlpp/io/async_io.hh>

// Create async I/O queue
auto queue = sdlpp::async_io_queue::create(32); // 32 concurrent operations
if (!queue) {
    std::cerr << "Failed to create async queue: " << queue.error() << std::endl;
    return;
}

// Open file for async reading
auto async_file = sdlpp::async_io::open("large_file.dat", 
                                        sdlpp::file_mode::read);
if (!async_file) {
    std::cerr << "Failed to open file: " << async_file.error() << std::endl;
    return;
}
```

### Async Read Operations

```cpp
// Read data asynchronously
std::vector<Uint8> buffer(4096);
auto read_op = async_file->read_async(buffer.data(), buffer.size(), 0);

// Submit to queue
queue->submit(read_op);

// Do other work while reading...
update_game_logic();
render_frame();

// Check if complete
if (read_op.is_complete()) {
    if (read_op.succeeded()) {
        size_t bytes_read = read_op.bytes_transferred();
        process_data(buffer.data(), bytes_read);
    } else {
        std::cerr << "Read failed: " << read_op.error() << std::endl;
    }
}
```

### Async Write Operations

```cpp
// Open file for writing
auto output = sdlpp::async_io::open("output.dat",
    sdlpp::file_mode::write | sdlpp::file_mode::create);

// Prepare data
std::vector<Uint8> data = generate_data();

// Write asynchronously
auto write_op = output->write_async(data.data(), data.size(), 0);
queue->submit(write_op);

// Wait for completion with timeout
if (write_op.wait_for(std::chrono::milliseconds(100))) {
    std::cout << "Wrote " << write_op.bytes_transferred() << " bytes" << std::endl;
}
```

## Queue Management

### Processing Operations

```cpp
class async_io_manager {
    sdlpp::async_io_queue queue;
    std::vector<sdlpp::async_operation> pending_ops;
    
public:
    async_io_manager() : queue(sdlpp::async_io_queue::create(64).value()) {}
    
    void submit_read(sdlpp::async_io& file, void* buffer, size_t size, Uint64 offset) {
        auto op = file.read_async(buffer, size, offset);
        queue.submit(op);
        pending_ops.push_back(op);
    }
    
    void update() {
        // Process completed operations
        auto it = pending_ops.begin();
        while (it != pending_ops.end()) {
            if (it->is_complete()) {
                handle_completion(*it);
                it = pending_ops.erase(it);
            } else {
                ++it;
            }
        }
        
        // Poll queue for any completed operations
        queue.poll();
    }
    
    void wait_all() {
        for (auto& op : pending_ops) {
            op.wait();
        }
        pending_ops.clear();
    }
    
private:
    void handle_completion(const sdlpp::async_operation& op) {
        if (op.succeeded()) {
            std::cout << "Operation completed: " 
                     << op.bytes_transferred() << " bytes" << std::endl;
        } else {
            std::cerr << "Operation failed: " << op.error() << std::endl;
        }
    }
};
```

### Queue Statistics

```cpp
// Get queue information
size_t pending = queue.pending_operations();
size_t completed = queue.completed_operations();
size_t failed = queue.failed_operations();

std::cout << "Queue stats - Pending: " << pending 
          << ", Completed: " << completed
          << ", Failed: " << failed << std::endl;

// Set queue limits
queue.set_max_operations(128);
queue.set_timeout(std::chrono::seconds(30));
```

## Callback-Based Operations

```cpp
// Read with callback
async_file.read_async(buffer.data(), buffer.size(), 0,
    [](sdlpp::async_operation& op) {
        if (op.succeeded()) {
            std::cout << "Read complete: " 
                     << op.bytes_transferred() << " bytes" << std::endl;
        }
    });

// Write with completion handler
struct write_handler {
    std::string filename;
    
    void operator()(sdlpp::async_operation& op) {
        if (op.succeeded()) {
            std::cout << filename << " written successfully" << std::endl;
        } else {
            std::cerr << filename << " write failed: " 
                     << op.error() << std::endl;
        }
    }
};

async_file.write_async(data.data(), data.size(), 0,
    write_handler{"output.dat"});
```

## Scatter-Gather I/O

```cpp
// Multiple buffers in single operation
struct io_buffer {
    void* data;
    size_t size;
    Uint64 offset;
};

std::vector<io_buffer> buffers = {
    {header_data, sizeof(header), 0},
    {metadata, metadata_size, sizeof(header)},
    {content, content_size, sizeof(header) + metadata_size}
};

// Scatter read (read into multiple buffers)
auto scatter_op = async_file.readv_async(buffers);
queue.submit(scatter_op);

// Gather write (write from multiple buffers)
auto gather_op = output_file.writev_async(buffers);
queue.submit(gather_op);
```

## Common Patterns

### Streaming File Reader

```cpp
class async_stream_reader {
    sdlpp::async_io file;
    sdlpp::async_io_queue& queue;
    
    static constexpr size_t CHUNK_SIZE = 64 * 1024; // 64KB chunks
    
    struct read_context {
        std::vector<Uint8> buffer;
        Uint64 offset;
        std::function<void(const Uint8*, size_t)> callback;
    };
    
    std::queue<std::unique_ptr<read_context>> contexts;
    
public:
    async_stream_reader(sdlpp::async_io&& f, sdlpp::async_io_queue& q)
        : file(std::move(f)), queue(q) {}
    
    void start_streaming(std::function<void(const Uint8*, size_t)> on_data) {
        Uint64 file_size = file.size();
        Uint64 offset = 0;
        
        // Queue up initial reads
        while (offset < file_size && contexts.size() < 4) {
            auto ctx = std::make_unique<read_context>();
            ctx->buffer.resize(std::min(CHUNK_SIZE, file_size - offset));
            ctx->offset = offset;
            ctx->callback = on_data;
            
            auto op = file.read_async(ctx->buffer.data(), 
                                     ctx->buffer.size(), 
                                     ctx->offset,
                [this, ctx = ctx.get()](sdlpp::async_operation& op) {
                    handle_read_complete(ctx, op);
                });
                
            queue.submit(op);
            contexts.push(std::move(ctx));
            offset += CHUNK_SIZE;
        }
    }
    
private:
    void handle_read_complete(read_context* ctx, sdlpp::async_operation& op) {
        if (op.succeeded()) {
            ctx->callback(ctx->buffer.data(), op.bytes_transferred());
            
            // Queue next read if more data
            Uint64 next_offset = ctx->offset + contexts.size() * CHUNK_SIZE;
            if (next_offset < file.size()) {
                ctx->offset = next_offset;
                size_t read_size = std::min(CHUNK_SIZE, 
                                           file.size() - next_offset);
                ctx->buffer.resize(read_size);
                
                auto next_op = file.read_async(ctx->buffer.data(),
                                              ctx->buffer.size(),
                                              ctx->offset,
                    [this, ctx](sdlpp::async_operation& op) {
                        handle_read_complete(ctx, op);
                    });
                    
                queue.submit(next_op);
            }
        }
    }
};
```

### Parallel File Processor

```cpp
class parallel_file_processor {
    struct task {
        std::string input_file;
        std::string output_file;
        std::function<std::vector<Uint8>(const std::vector<Uint8>&)> process;
    };
    
    sdlpp::async_io_queue queue;
    std::vector<task> tasks;
    std::atomic<size_t> completed{0};
    
public:
    parallel_file_processor() 
        : queue(sdlpp::async_io_queue::create(32).value()) {}
    
    void add_task(const std::string& input, 
                  const std::string& output,
                  std::function<std::vector<Uint8>(const std::vector<Uint8>&)> proc) {
        tasks.push_back({input, output, proc});
    }
    
    void process_all() {
        for (auto& task : tasks) {
            process_file(task);
        }
        
        // Wait for all to complete
        while (completed < tasks.size()) {
            queue.poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
private:
    void process_file(task& t) {
        // Open input file
        auto input = sdlpp::async_io::open(t.input_file, 
                                          sdlpp::file_mode::read);
        if (!input) return;
        
        // Allocate buffer for entire file
        size_t file_size = input->size();
        auto buffer = std::make_unique<std::vector<Uint8>>(file_size);
        
        // Read asynchronously
        auto read_op = input->read_async(buffer->data(), file_size, 0,
            [this, &t, buffer = std::move(buffer)](sdlpp::async_operation& op) {
                if (op.succeeded()) {
                    // Process data
                    auto result = t.process(*buffer);
                    
                    // Write result
                    write_result(t.output_file, std::move(result));
                }
            });
            
        queue.submit(read_op);
    }
    
    void write_result(const std::string& output_file, 
                     std::vector<Uint8> data) {
        auto output = sdlpp::async_io::open(output_file,
            sdlpp::file_mode::write | sdlpp::file_mode::create);
            
        if (!output) {
            completed++;
            return;
        }
        
        auto buffer = std::make_shared<std::vector<Uint8>>(std::move(data));
        
        auto write_op = output->write_async(buffer->data(), buffer->size(), 0,
            [this, buffer](sdlpp::async_operation& op) {
                completed++;
                if (!op.succeeded()) {
                    std::cerr << "Write failed: " << op.error() << std::endl;
                }
            });
            
        queue.submit(write_op);
    }
};
```

### Async Copy with Progress

```cpp
class async_file_copier {
public:
    using progress_callback = std::function<void(Uint64 copied, Uint64 total)>;
    
private:
    static constexpr size_t BUFFER_SIZE = 1024 * 1024; // 1MB chunks
    
    struct copy_state {
        sdlpp::async_io source;
        sdlpp::async_io dest;
        Uint64 total_size;
        std::atomic<Uint64> bytes_copied{0};
        progress_callback on_progress;
        std::vector<std::vector<Uint8>> buffers;
        size_t active_buffer = 0;
    };
    
    sdlpp::async_io_queue queue;
    
public:
    async_file_copier() 
        : queue(sdlpp::async_io_queue::create(16).value()) {}
    
    bool copy_file(const std::filesystem::path& src,
                   const std::filesystem::path& dst,
                   progress_callback progress = {}) {
        
        auto source = sdlpp::async_io::open(src, sdlpp::file_mode::read);
        if (!source) return false;
        
        auto dest = sdlpp::async_io::open(dst, 
            sdlpp::file_mode::write | sdlpp::file_mode::create);
        if (!dest) return false;
        
        auto state = std::make_shared<copy_state>();
        state->source = std::move(source.value());
        state->dest = std::move(dest.value());
        state->total_size = state->source.size();
        state->on_progress = progress;
        
        // Allocate double buffers for pipelining
        state->buffers.resize(2);
        for (auto& buffer : state->buffers) {
            buffer.resize(BUFFER_SIZE);
        }
        
        // Start first read
        start_read(state, 0);
        
        // Process until complete
        while (state->bytes_copied < state->total_size) {
            queue.poll();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        return true;
    }
    
private:
    void start_read(std::shared_ptr<copy_state> state, Uint64 offset) {
        if (offset >= state->total_size) return;
        
        size_t read_size = std::min(BUFFER_SIZE, state->total_size - offset);
        auto& buffer = state->buffers[state->active_buffer];
        
        auto read_op = state->source.read_async(buffer.data(), read_size, offset,
            [this, state, offset, read_size](sdlpp::async_operation& op) {
                if (op.succeeded()) {
                    start_write(state, offset, op.bytes_transferred());
                }
            });
            
        queue.submit(read_op);
    }
    
    void start_write(std::shared_ptr<copy_state> state, 
                    Uint64 offset, size_t size) {
        auto& buffer = state->buffers[state->active_buffer];
        
        auto write_op = state->dest.write_async(buffer.data(), size, offset,
            [this, state, offset, size](sdlpp::async_operation& op) {
                if (op.succeeded()) {
                    state->bytes_copied += size;
                    
                    if (state->on_progress) {
                        state->on_progress(state->bytes_copied, 
                                         state->total_size);
                    }
                    
                    // Switch buffers and continue
                    state->active_buffer = 1 - state->active_buffer;
                    start_read(state, offset + size);
                }
            });
            
        queue.submit(write_op);
    }
};

// Usage
async_file_copier copier;
copier.copy_file("source.bin", "dest.bin",
    [](Uint64 copied, Uint64 total) {
        float percent = (copied * 100.0f) / total;
        std::cout << "\rCopying: " << std::fixed << std::setprecision(1) 
                  << percent << "%";
    });
```

## Error Handling

```cpp
// Handle specific async errors
void handle_async_error(const sdlpp::async_operation& op) {
    auto error_code = op.error_code();
    
    switch (error_code) {
        case sdlpp::async_error::io_error:
            std::cerr << "I/O error: " << op.error() << std::endl;
            break;
            
        case sdlpp::async_error::cancelled:
            std::cout << "Operation cancelled" << std::endl;
            break;
            
        case sdlpp::async_error::invalid_parameter:
            std::cerr << "Invalid parameter" << std::endl;
            break;
            
        case sdlpp::async_error::out_of_memory:
            std::cerr << "Out of memory" << std::endl;
            break;
            
        default:
            std::cerr << "Unknown error: " << op.error() << std::endl;
    }
}

// Retry failed operations
class async_retry_handler {
    struct retry_info {
        sdlpp::async_operation op;
        int attempts = 0;
        std::chrono::steady_clock::time_point next_retry;
    };
    
    std::vector<retry_info> failed_ops;
    static constexpr int MAX_RETRIES = 3;
    
public:
    void handle_failed(sdlpp::async_operation op) {
        if (should_retry(op)) {
            retry_info info;
            info.op = op;
            info.attempts = 1;
            info.next_retry = std::chrono::steady_clock::now() + 
                             std::chrono::seconds(1);
            failed_ops.push_back(info);
        }
    }
    
    void update(sdlpp::async_io_queue& queue) {
        auto now = std::chrono::steady_clock::now();
        
        auto it = failed_ops.begin();
        while (it != failed_ops.end()) {
            if (now >= it->next_retry) {
                // Retry operation
                queue.submit(it->op);
                
                if (++it->attempts >= MAX_RETRIES) {
                    std::cerr << "Operation failed after " 
                             << MAX_RETRIES << " attempts" << std::endl;
                    it = failed_ops.erase(it);
                } else {
                    // Exponential backoff
                    it->next_retry = now + 
                        std::chrono::seconds(1 << it->attempts);
                    ++it;
                }
            } else {
                ++it;
            }
        }
    }
    
private:
    bool should_retry(const sdlpp::async_operation& op) {
        auto error = op.error_code();
        return error == sdlpp::async_error::io_error ||
               error == sdlpp::async_error::timeout;
    }
};
```

## Best Practices

1. **Use Appropriate Queue Size**: Balance memory usage and concurrency
2. **Double Buffer**: For streaming, use multiple buffers
3. **Handle Errors**: Always check operation results
4. **Avoid Blocking**: Use callbacks or polling, not wait()
5. **Clean Shutdown**: Cancel pending operations before destroying queue

## Example: Asset Loading System

```cpp
class async_asset_loader {
    struct asset_request {
        std::string path;
        std::string type;
        std::promise<std::shared_ptr<void>> promise;
    };
    
    sdlpp::async_io_queue queue;
    std::queue<asset_request> requests;
    std::unordered_map<std::string, std::weak_ptr<void>> cache;
    std::mutex mutex;
    
public:
    async_asset_loader() 
        : queue(sdlpp::async_io_queue::create(16).value()) {}
    
    template<typename T>
    std::future<std::shared_ptr<T>> load_asset(const std::string& path) {
        std::lock_guard<std::mutex> lock(mutex);
        
        // Check cache
        if (auto it = cache.find(path); it != cache.end()) {
            if (auto cached = it->second.lock()) {
                std::promise<std::shared_ptr<T>> promise;
                promise.set_value(std::static_pointer_cast<T>(cached));
                return promise.get_future();
            }
        }
        
        // Queue new request
        asset_request req;
        req.path = path;
        req.type = typeid(T).name();
        
        auto future = req.promise.get_future();
        requests.push(std::move(req));
        
        return std::async(std::launch::async,
            [future = std::move(future)]() mutable {
                return std::static_pointer_cast<T>(future.get());
            });
    }
    
    void update() {
        process_requests();
        queue.poll();
    }
    
private:
    void process_requests() {
        std::lock_guard<std::mutex> lock(mutex);
        
        while (!requests.empty()) {
            auto req = std::move(requests.front());
            requests.pop();
            
            load_file_async(std::move(req));
        }
    }
    
    void load_file_async(asset_request req) {
        auto file = sdlpp::async_io::open(req.path, sdlpp::file_mode::read);
        if (!file) {
            req.promise.set_exception(
                std::make_exception_ptr(
                    std::runtime_error("Failed to open: " + req.path)
                )
            );
            return;
        }
        
        size_t file_size = file->size();
        auto buffer = std::make_shared<std::vector<Uint8>>(file_size);
        auto promise = std::make_shared<std::promise<std::shared_ptr<void>>>(
            std::move(req.promise)
        );
        
        auto op = file->read_async(buffer->data(), file_size, 0,
            [this, buffer, promise, path = req.path, type = req.type]
            (sdlpp::async_operation& op) {
                if (op.succeeded()) {
                    auto asset = parse_asset(type, *buffer);
                    
                    // Cache the asset
                    {
                        std::lock_guard<std::mutex> lock(mutex);
                        cache[path] = asset;
                    }
                    
                    promise->set_value(asset);
                } else {
                    promise->set_exception(
                        std::make_exception_ptr(
                            std::runtime_error("Read failed: " + op.error())
                        )
                    );
                }
            });
            
        queue.submit(op);
    }
    
    std::shared_ptr<void> parse_asset(const std::string& type,
                                     const std::vector<Uint8>& data) {
        // Parse based on type
        // This is simplified - real implementation would parse different formats
        return std::make_shared<std::vector<Uint8>>(data);
    }
};
```