#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <sdlpp/system/process.hh>
#include <sdlpp/core/core.hh>

void basic_example() {
    std::cout << "\n=== Basic Process Example ===\n";
    
    // Simple command execution
    #ifdef _WIN32
        std::vector<std::string> list_cmd = {"cmd", "/c", "dir", "/b"};
    #else
        std::vector<std::string> list_cmd = {"ls", "-1"};
    #endif
    
    std::cout << "Running: ";
    for (const auto& arg : list_cmd) {
        std::cout << arg << " ";
    }
    std::cout << "\n\n";
    
    auto result = sdlpp::process::create(list_cmd);
    if (!result) {
        std::cout << "Failed to create process: " << result.error() << "\n";
        return;
    }
    
    auto& proc = result.value();
    std::cout << "Process created successfully\n";
    
    // Wait for completion
    auto status = proc.wait();
    if (status) {
        if (status.value().success()) {
            std::cout << "Process completed successfully\n";
        } else if (status.value().exited) {
            std::cout << "Process exited with code: " << status.value().exit_code << "\n";
        } else if (status.value().signaled) {
            std::cout << "Process was terminated by signal\n";
        }
    }
}

void io_redirection_example() {
    std::cout << "\n=== I/O Redirection Example ===\n";
    
    // Create a process that reads from stdin and writes to stdout
    #ifdef _WIN32
        std::vector<std::string> sort_cmd = {"sort"};
    #else
        std::vector<std::string> sort_cmd = {"sort", "-n"};
    #endif
    
    auto result = sdlpp::process_builder()
        .set_command(sort_cmd)
        .stdin_from_pipe()
        .stdout_to_pipe()
        .spawn();
    
    if (!result) {
        std::cout << "Failed to create process: " << result.error() << "\n";
        return;
    }
    
    auto& proc = result.value();
    
    // Write numbers to stdin
    std::cout << "Writing unsorted numbers to process stdin...\n";
    std::string input = "42\n17\n99\n3\n65\n";
    [[maybe_unused]] auto write_result = proc.write_stdin(input);
    if (!write_result) {
        std::cout << "Failed to write: " << write_result.error() << "\n";
    }
    
    // Close stdin to signal EOF
    proc.close_stdin();
    
    // Read sorted output
    std::cout << "\nSorted output:\n";
    auto output = proc.read_stdout_all();
    if (output) {
        std::cout << output.value();
    } else {
        std::cout << "Failed to read output: " << output.error() << "\n";
    }
    
    // Wait for completion
    auto status = proc.wait();
    if (status && status.value().success()) {
        std::cout << "\nSort process completed successfully\n";
    }
}

void pipeline_example() {
    std::cout << "\n=== Pipeline Example ===\n";
    std::cout << "Simulating: echo 'Hello World' | grep 'World'\n\n";
    
    // First process: echo
    #ifdef _WIN32
        std::vector<std::string> echo_cmd = {"cmd", "/c", "echo", "Hello World"};
        std::vector<std::string> grep_cmd = {"findstr", "World"};
    #else
        std::vector<std::string> echo_cmd = {"echo", "Hello World"};
        std::vector<std::string> grep_cmd = {"grep", "World"};
    #endif
    
    // Create echo process with stdout redirected
    auto echo_result = sdlpp::process_builder()
        .set_command(echo_cmd)
        .stdout_to_pipe()
        .spawn();
    
    if (!echo_result) {
        std::cout << "Failed to create echo process: " << echo_result.error() << "\n";
        return;
    }
    
    // Create grep process with stdin and stdout redirected
    auto grep_result = sdlpp::process_builder()
        .set_command(grep_cmd)
        .stdin_from_pipe()
        .stdout_to_pipe()
        .spawn();
    
    if (!grep_result) {
        std::cout << "Failed to create grep process: " << grep_result.error() << "\n";
        return;
    }
    
    auto& echo_proc = echo_result.value();
    auto& grep_proc = grep_result.value();
    
    // Connect the pipeline: read from echo and write to grep
    auto echo_output = echo_proc.read_stdout_all();
    if (echo_output) {
        // Write to grep
        [[maybe_unused]] auto write_res = grep_proc.write_stdin(echo_output.value());
    }
    // Close grep's stdin
    grep_proc.close_stdin();
    
    // Read final output
    std::string final_output;
    auto grep_output = grep_proc.read_stdout_all();
    if (grep_output) {
        final_output = grep_output.value();
    }
    
    // Wait for both processes
    [[maybe_unused]] auto echo_status = echo_proc.wait();
    auto grep_status = grep_proc.wait();
    
    std::cout << "Pipeline output: " << final_output;
    if (grep_status && grep_status.value().success()) {
        std::cout << "Pipeline completed successfully\n";
    }
}

void environment_example() {
    std::cout << "\n=== Environment Variables Example ===\n";
    
    // Create a process with custom environment variables
    #ifdef _WIN32
        std::vector<std::string> env_cmd = {"cmd", "/c", "echo", "Path: %MY_CUSTOM_PATH%"};
    #else
        std::vector<std::string> env_cmd = {"sh", "-c", "echo \"Path: $MY_CUSTOM_PATH\""};
    #endif
    
    auto result = sdlpp::process_builder()
        .set_command(env_cmd)
        .set_env("MY_CUSTOM_PATH", "/custom/path/to/something")
        .set_env("MY_CUSTOM_VAR", "custom_value")
        .stdout_to_pipe()
        .spawn();
    
    if (!result) {
        std::cout << "Failed to create process: " << result.error() << "\n";
        return;
    }
    
    auto& proc = result.value();
    
    // Read output
    auto output = proc.read_stdout_all();
    if (output) {
        std::cout << "Process output: " << output.value();
    }
    
    [[maybe_unused]] auto wait_result = proc.wait();
}

void timeout_example() {
    std::cout << "\n=== Process Timeout Example ===\n";
    
    // Create a long-running process
    #ifdef _WIN32
        std::vector<std::string> sleep_cmd = {"timeout", "/t", "5"};
    #else
        std::vector<std::string> sleep_cmd = {"sleep", "5"};
    #endif
    
    std::cout << "Starting process that sleeps for 5 seconds...\n";
    auto result = sdlpp::process::create(sleep_cmd);
    if (!result) {
        std::cout << "Failed to create process: " << result.error() << "\n";
        return;
    }
    
    auto& proc = result.value();
    std::cout << "Process started\n";
    
    // Wait for 2 seconds
    std::cout << "Waiting 2 seconds...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Check if still running (non-blocking wait)
    auto status = proc.wait(false);
    if (!status) {
        std::cout << "Process is still running\n";
        
        // Kill the process
        std::cout << "Killing the process...\n";
        if (proc.kill()) {
            std::cout << "Process killed successfully\n";
            
            // Now wait for it to actually terminate
            status = proc.wait(true);
            if (status) {
                if (status.value().signaled) {
                    std::cout << "Process was terminated by signal\n";
                } else {
                    std::cout << "Process exited with code: " << status.value().exit_code << "\n";
                }
            }
        }
    } else {
        std::cout << "Process already completed\n";
    }
}

void convenience_function_example() {
    std::cout << "\n=== Convenience Functions Example ===\n";
    
    // Simple execution
    #ifdef _WIN32
        std::vector<std::string> date_cmd = {"cmd", "/c", "date", "/t"};
    #else
        std::vector<std::string> date_cmd = {"date"};
    #endif
    
    std::cout << "Getting current date...\n";
    auto status_result = sdlpp::execute(date_cmd);
    if (status_result) {
        if (status_result.value().success()) {
            std::cout << "Date command succeeded\n";
        }
    }
    
    // Execute with output capture
    std::cout << "\nCapturing command output...\n";
    auto output_result = sdlpp::execute_with_output(date_cmd);
    if (output_result) {
        auto& [status, output] = output_result.value();
        if (status.success()) {
            std::cout << "Current date: " << output;
        }
    }
}

int main() {
    try {
        // SDL is required for process functionality
        sdlpp::init sdl_init(sdlpp::init_flags::none);
        
        std::cout << "SDL Process Example\n";
        std::cout << "===================\n";
        
        basic_example();
        io_redirection_example();
        pipeline_example();
        environment_example();
        timeout_example();
        convenience_function_example();
        
        std::cout << "\nAll examples completed.\n";
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}