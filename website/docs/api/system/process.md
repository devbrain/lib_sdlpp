---
title: Process Management
sidebar_label: Process
---

# Process Management

SDL++ provides cross-platform process creation and management capabilities, including inter-process communication through pipes.

## Creating Processes

### Basic Process Execution

```cpp
#include <sdlpp/system/process.hh>

// Execute a simple command
auto result = sdlpp::process::create("/usr/bin/ls", {"-la", "/tmp"});
if (!result) {
    std::cerr << "Failed to start process: " << result.error() << std::endl;
    return;
}

auto& proc = result.value();

// Wait for process to complete
int exit_code = proc.wait();
std::cout << "Process exited with code: " << exit_code << std::endl;
```

### Process with Arguments

```cpp
// Execute with multiple arguments
std::vector<std::string> args = {
    "--verbose",
    "--output", "result.txt",
    "input.txt"
};

auto proc = sdlpp::process::create("/usr/local/bin/mytool", args);
```

### Process with Environment Variables

```cpp
// Set environment variables for child process
std::unordered_map<std::string, std::string> env = {
    {"PATH", "/usr/local/bin:/usr/bin:/bin"},
    {"LANG", "en_US.UTF-8"},
    {"MY_CONFIG", "/etc/myapp.conf"}
};

auto proc = sdlpp::process::create_with_env(
    "/usr/bin/myapp",
    {"--start"},
    env
);
```

## Process I/O

### Reading Process Output

```cpp
// Capture stdout
auto proc = sdlpp::process::create("/usr/bin/echo", {"Hello, World!"});
if (proc) {
    std::string output = proc->read_stdout_string();
    std::cout << "Output: " << output << std::endl;
}

// Read output line by line
auto proc2 = sdlpp::process::create("/usr/bin/find", {".", "-name", "*.txt"});
if (proc2) {
    std::string line;
    while (proc2->read_stdout_line(line)) {
        std::cout << "Found: " << line << std::endl;
    }
}
```

### Writing to Process Input

```cpp
// Send input to process
auto proc = sdlpp::process::create("/usr/bin/grep", {"error"});
if (proc) {
    // Write to stdin
    proc->write_stdin("This is a test\n");
    proc->write_stdin("Error: something went wrong\n");
    proc->write_stdin("All done\n");
    
    // Close stdin to signal EOF
    proc->close_stdin();
    
    // Read filtered output
    std::string output = proc->read_stdout_string();
    std::cout << "Grep found: " << output << std::endl;
}
```

### Bidirectional Communication

```cpp
// Interactive process communication
auto calc = sdlpp::process::create("/usr/bin/bc", {"-l"});
if (calc) {
    // Send calculation
    calc->write_stdin("2 + 2\n");
    std::string result;
    if (calc->read_stdout_line(result)) {
        std::cout << "2 + 2 = " << result << std::endl;
    }
    
    // More calculations
    calc->write_stdin("sqrt(2)\n");
    if (calc->read_stdout_line(result)) {
        std::cout << "sqrt(2) = " << result << std::endl;
    }
    
    // Terminate
    calc->write_stdin("quit\n");
    calc->wait();
}
```

## Process Control

### Checking Process Status

```cpp
auto proc = sdlpp::process::create("/usr/bin/sleep", {"10"});
if (proc) {
    // Check if still running
    while (proc->is_running()) {
        std::cout << "Process still running..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    // Get exit code
    int exit_code = proc->get_exit_code();
    std::cout << "Process exited with code: " << exit_code << std::endl;
}
```

### Terminating Processes

```cpp
auto proc = sdlpp::process::create("/usr/bin/long_running_task");
if (proc) {
    // Give it some time
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    // Terminate gracefully
    proc->terminate();
    
    // Wait with timeout
    if (!proc->wait_for(std::chrono::seconds(2))) {
        // Force kill if not terminated
        proc->kill();
    }
}
```

### Background Processes

```cpp
// Launch in background (detached)
auto result = sdlpp::process::create_detached(
    "/usr/bin/my_daemon",
    {"--config", "/etc/daemon.conf"}
);

if (result) {
    std::cout << "Daemon started with PID: " << result.value() << std::endl;
}
```

## Error Handling

### Handling Process Errors

```cpp
// Capture stderr
auto proc = sdlpp::process::create("/usr/bin/compile", {"program.cpp"});
if (proc) {
    int exit_code = proc->wait();
    
    if (exit_code != 0) {
        std::string errors = proc->read_stderr_string();
        std::cerr << "Compilation failed:\n" << errors << std::endl;
    } else {
        std::cout << "Compilation successful!" << std::endl;
    }
}
```

### Pipe Errors

```cpp
auto proc = sdlpp::process::create("/usr/bin/cat", {"/nonexistent/file"});
if (proc) {
    // Check for read errors
    std::string output;
    auto read_result = proc->try_read_stdout_string();
    if (!read_result) {
        std::cerr << "Read error: " << read_result.error() << std::endl;
    }
    
    // Check stderr for error message
    std::string error = proc->read_stderr_string();
    if (!error.empty()) {
        std::cerr << "Process error: " << error << std::endl;
    }
}
```

## Common Patterns

### Process Pipeline

```cpp
class process_pipeline {
    std::vector<sdlpp::process> processes;
    
public:
    void add(const std::string& cmd, const std::vector<std::string>& args) {
        auto proc = sdlpp::process::create(cmd, args);
        if (proc) {
            processes.push_back(std::move(proc.value()));
        }
    }
    
    std::string execute(const std::string& input) {
        if (processes.empty()) return "";
        
        std::string data = input;
        
        for (auto& proc : processes) {
            proc.write_stdin(data);
            proc.close_stdin();
            
            data = proc.read_stdout_string();
            proc.wait();
        }
        
        return data;
    }
};

// Usage: equivalent to "echo 'hello world' | tr a-z A-Z | sed 's/O/0/g'"
process_pipeline pipeline;
pipeline.add("/usr/bin/echo", {"hello world"});
pipeline.add("/usr/bin/tr", {"a-z", "A-Z"});
pipeline.add("/usr/bin/sed", {"s/O/0/g"});

std::string result = pipeline.execute("");
// Result: "HELL0 W0RLD"
```

### Process Pool

```cpp
class process_pool {
    struct worker {
        sdlpp::process proc;
        bool busy = false;
        std::string current_task;
    };
    
    std::vector<worker> workers;
    std::queue<std::string> task_queue;
    std::mutex queue_mutex;
    
public:
    process_pool(const std::string& worker_cmd, size_t pool_size) {
        for (size_t i = 0; i < pool_size; ++i) {
            auto proc = sdlpp::process::create(worker_cmd, {"--worker"});
            if (proc) {
                workers.push_back({std::move(proc.value()), false, ""});
            }
        }
    }
    
    void submit_task(const std::string& task) {
        std::lock_guard<std::mutex> lock(queue_mutex);
        task_queue.push(task);
        assign_tasks();
    }
    
private:
    void assign_tasks() {
        for (auto& worker : workers) {
            if (!worker.busy && !task_queue.empty()) {
                worker.current_task = task_queue.front();
                task_queue.pop();
                worker.busy = true;
                
                // Send task to worker
                worker.proc.write_stdin(worker.current_task + "\n");
            }
        }
    }
    
    void check_workers() {
        for (auto& worker : workers) {
            if (worker.busy) {
                std::string result;
                if (worker.proc.read_stdout_line(result)) {
                    // Task completed
                    handle_result(worker.current_task, result);
                    worker.busy = false;
                    worker.current_task.clear();
                }
            }
        }
        assign_tasks();
    }
    
    void handle_result(const std::string& task, const std::string& result) {
        std::cout << "Task '" << task << "' completed: " << result << std::endl;
    }
};
```

### Command Executor

```cpp
class command_executor {
public:
    struct command_result {
        int exit_code;
        std::string stdout;
        std::string stderr;
        std::chrono::milliseconds duration;
    };
    
    static command_result execute(
        const std::string& cmd,
        const std::vector<std::string>& args = {},
        std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        
        auto start = std::chrono::steady_clock::now();
        
        auto proc = sdlpp::process::create(cmd, args);
        if (!proc) {
            return {-1, "", proc.error(), std::chrono::milliseconds(0)};
        }
        
        command_result result;
        
        if (timeout > std::chrono::milliseconds(0)) {
            if (!proc->wait_for(timeout)) {
                proc->terminate();
                result.exit_code = -1;
                result.stderr = "Process timed out";
            } else {
                result.exit_code = proc->get_exit_code();
            }
        } else {
            result.exit_code = proc->wait();
        }
        
        result.stdout = proc->read_stdout_string();
        result.stderr = proc->read_stderr_string();
        
        auto end = std::chrono::steady_clock::now();
        result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start
        );
        
        return result;
    }
    
    static command_result execute_shell(const std::string& command) {
        #ifdef _WIN32
        return execute("cmd.exe", {"/C", command});
        #else
        return execute("/bin/sh", {"-c", command});
        #endif
    }
};

// Usage
auto result = command_executor::execute("ping", {"-c", "4", "google.com"});
std::cout << "Exit code: " << result.exit_code << std::endl;
std::cout << "Duration: " << result.duration.count() << "ms" << std::endl;
std::cout << "Output:\n" << result.stdout << std::endl;
```

### Process Monitor

```cpp
class process_monitor {
    struct monitored_process {
        std::string name;
        std::string command;
        std::vector<std::string> args;
        std::unique_ptr<sdlpp::process> proc;
        int restart_count = 0;
        std::chrono::steady_clock::time_point last_start;
    };
    
    std::vector<monitored_process> processes;
    std::thread monitor_thread;
    std::atomic<bool> running{true};
    
public:
    void add_process(const std::string& name,
                    const std::string& cmd,
                    const std::vector<std::string>& args) {
        processes.push_back({name, cmd, args, nullptr, 0, {}});
    }
    
    void start() {
        // Start all processes
        for (auto& mp : processes) {
            start_process(mp);
        }
        
        // Start monitor thread
        monitor_thread = std::thread([this] { monitor_loop(); });
    }
    
    void stop() {
        running = false;
        
        // Stop all processes
        for (auto& mp : processes) {
            if (mp.proc && mp.proc->is_running()) {
                mp.proc->terminate();
            }
        }
        
        if (monitor_thread.joinable()) {
            monitor_thread.join();
        }
    }
    
private:
    void start_process(monitored_process& mp) {
        auto proc = sdlpp::process::create(mp.command, mp.args);
        if (proc) {
            mp.proc = std::make_unique<sdlpp::process>(std::move(proc.value()));
            mp.last_start = std::chrono::steady_clock::now();
            std::cout << "Started " << mp.name << std::endl;
        }
    }
    
    void monitor_loop() {
        while (running) {
            for (auto& mp : processes) {
                if (!mp.proc || !mp.proc->is_running()) {
                    // Process died
                    if (mp.proc) {
                        int exit_code = mp.proc->get_exit_code();
                        std::cout << mp.name << " exited with code " 
                                 << exit_code << std::endl;
                    }
                    
                    // Check if we should restart
                    auto now = std::chrono::steady_clock::now();
                    auto elapsed = now - mp.last_start;
                    
                    if (elapsed > std::chrono::seconds(10)) {
                        // Reset restart count if running for >10s
                        mp.restart_count = 0;
                    }
                    
                    if (mp.restart_count < 5) {
                        mp.restart_count++;
                        std::cout << "Restarting " << mp.name 
                                 << " (attempt " << mp.restart_count << ")"
                                 << std::endl;
                        start_process(mp);
                    } else {
                        std::cerr << mp.name << " failed too many times" 
                                 << std::endl;
                    }
                }
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
};
```

## Best Practices

1. **Always Check Return Values**: Process creation can fail
2. **Handle Zombie Processes**: Call wait() or use detached processes
3. **Close Unused Pipes**: Close stdin when done writing
4. **Use Timeouts**: Prevent hanging on process I/O
5. **Validate Input**: Sanitize command arguments to prevent injection

## Example: Build System Integration

```cpp
class build_system {
    struct build_config {
        std::string compiler = "g++";
        std::vector<std::string> flags = {"-std=c++20", "-O2"};
        std::vector<std::string> includes;
        std::vector<std::string> libs;
    };
    
    build_config config;
    
public:
    bool compile(const std::string& source, const std::string& output) {
        std::vector<std::string> args = config.flags;
        
        // Add includes
        for (const auto& inc : config.includes) {
            args.push_back("-I" + inc);
        }
        
        // Add source and output
        args.push_back("-c");
        args.push_back(source);
        args.push_back("-o");
        args.push_back(output);
        
        // Run compiler
        auto proc = sdlpp::process::create(config.compiler, args);
        if (!proc) {
            std::cerr << "Failed to start compiler" << std::endl;
            return false;
        }
        
        int exit_code = proc->wait();
        
        if (exit_code != 0) {
            std::string errors = proc->read_stderr_string();
            std::cerr << "Compilation errors:\n" << errors << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool link(const std::vector<std::string>& objects, 
              const std::string& output) {
        std::vector<std::string> args = objects;
        
        // Add output
        args.push_back("-o");
        args.push_back(output);
        
        // Add libraries
        for (const auto& lib : config.libs) {
            args.push_back("-l" + lib);
        }
        
        // Run linker
        auto proc = sdlpp::process::create(config.compiler, args);
        if (!proc) {
            std::cerr << "Failed to start linker" << std::endl;
            return false;
        }
        
        int exit_code = proc->wait();
        
        if (exit_code != 0) {
            std::string errors = proc->read_stderr_string();
            std::cerr << "Linking errors:\n" << errors << std::endl;
            return false;
        }
        
        return true;
    }
};
```