#pragma once

/**
 * @file process.hh
 * @brief Process creation and management
 * 
 * This header provides RAII wrappers for creating and managing
 * child processes, including pipes for inter-process communication.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/io/iostream.hh>
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <filesystem>

namespace sdlpp {
    /**
     * @brief Process I/O redirection options
     */
    enum class process_io {
        inherited, ///< Inherit from parent process
        null, ///< Redirect to null device
        app, ///< Create pipe for application use
        redirect ///< Redirect to another stream
    };

    /**
     * @brief Process exit status information
     */
    struct process_exit_status {
        int exit_code{0}; ///< Exit code (valid if exited normally)
        bool exited{false}; ///< True if process exited normally
        bool signaled{false}; ///< True if process was terminated by signal

        /**
         * @brief Check if process exited successfully (exit code 0)
         */
        [[nodiscard]] bool success() const noexcept {
            return exited && exit_code == 0;
        }
    };

    /**
     * @brief RAII wrapper for child processes
     *
     * This class provides a safe interface for creating and managing
     * child processes. The process is automatically terminated and
     * cleaned up when the object is destroyed.
     *
     * Example usage:
     * @code
     * // Simple command execution
     * auto proc = sdlpp::process::create({"ls", "-la"});
     * if (proc) {
     *     auto status = proc.value().wait();
     *     if (status.success()) {
     *         std::cout << "Command succeeded\n";
     *     }
     * }
     *
     * // With I/O redirection
     * sdlpp::process_builder builder;
     * builder.set_command({"grep", "pattern"})
     *        .stdin_from_pipe()
     *        .stdout_to_pipe();
     *
     * auto proc = builder.spawn();
     * if (proc) {
     *     // Write to stdin
     *     if (auto stdin = proc.value().get_stdin()) {
     *         stdin.value().write_string("text with pattern\n");
     *     }
     *
     *     // Read from stdout
     *     if (auto stdout = proc.value().get_stdout()) {
     *         auto output = stdout.value().read_all();
     *     }
     * }
     * @endcode
     */
    class process {
        private:
            // No-op deleter for process I/O streams (managed by SDL internally)
            static void no_close(SDL_IOStream*) noexcept {
            }

        public:
            /**
             * @brief Default constructor creates an invalid process
             */
            process() noexcept = default;

            /**
             * @brief Move constructor
             */
            process(process&& other) noexcept
                : handle_(other.handle_)
                  , stdin_(std::move(other.stdin_))
                  , stdout_(std::move(other.stdout_))
                  , stderr_(std::move(other.stderr_)) {
                other.handle_ = nullptr;
            }

            /**
             * @brief Move assignment operator
             */
            process& operator=(process&& other) noexcept {
                if (this != &other) {
                    reset();
                    handle_ = other.handle_;
                    stdin_ = std::move(other.stdin_);
                    stdout_ = std::move(other.stdout_);
                    stderr_ = std::move(other.stderr_);
                    other.handle_ = nullptr;
                }
                return *this;
            }

            /**
             * @brief Destructor terminates and cleans up the process
             */
            ~process() {
                reset();
            }

            // Delete copy operations
            process(const process&) = delete;
            process& operator=(const process&) = delete;

            /**
             * @brief Create a process with the given command and arguments
             * @param args Command and arguments (first element is the command)
             * @param pipe_stdio Whether to create pipes for stdio
             * @return Expected containing the process or error message
             */
            [[nodiscard]] static expected <process, std::string> create(
                const std::vector <std::string>& args,
                bool pipe_stdio = false) noexcept;

            /**
             * @brief Wait for the process to exit
             * @param block If true, block until process exits; if false, return immediately
             * @return Exit status if process has exited, nullopt if still running
             */
            [[nodiscard]] std::optional <process_exit_status> wait(bool block = true) noexcept {
                if (!handle_) {
                    return std::nullopt;
                }

                int exit_code = 0;
                bool wait_result = SDL_WaitProcess(handle_, block, &exit_code);

                if (!wait_result && !block) {
                    // Process still running
                    return std::nullopt;
                }

                process_exit_status status;
                if (wait_result) {
                    // Process exited normally
                    status.exited = true;
                    status.exit_code = exit_code;
                } else {
                    // Process was likely signaled (on Unix)
                    status.signaled = true;
                }

                return status;
            }

            /**
             * @brief Kill the process
             * @param force If true, forcefully terminate (SIGKILL on Unix)
             * @return true on success
             */
            bool kill(bool force = false) noexcept {
                if (!handle_) {
                    return false;
                }

                return SDL_KillProcess(handle_, force);
            }

            /**
             * @brief Get the stdin stream (if redirected to pipe)
             * @return Optional iostream for writing to process stdin
             */
            [[nodiscard]] std::optional <iostream> get_stdin() noexcept {
                if (stdin_) {
                    return iostream(stdin_.get());
                }
                return std::nullopt;
            }

            /**
             * @brief Get the stdout stream (if redirected to pipe)
             * @return Optional iostream for reading from process stdout
             */
            [[nodiscard]] std::optional <iostream> get_stdout() noexcept {
                if (stdout_) {
                    return iostream(stdout_.get());
                }
                return std::nullopt;
            }

            /**
             * @brief Get the stderr stream (if redirected to pipe)
             * @return Optional iostream for reading from process stderr
             */
            [[nodiscard]] std::optional <iostream> get_stderr() noexcept {
                if (stderr_) {
                    return iostream(stderr_.get());
                }
                return std::nullopt;
            }

            /**
             * @brief Close the stdin pipe
             *
             * This signals EOF to the child process
             */
            void close_stdin() noexcept {
                stdin_.reset();
            }

            /**
             * @brief Write a string to stdin (convenience method)
             * @param str String to write
             * @return Expected containing bytes written or error
             */
            [[nodiscard]] expected <size_t, std::string> write_stdin(const std::string& str) noexcept {
                if (!stdin_) {
                    return make_unexpected("stdin not redirected to pipe");
                }

                size_t bytes_written = SDL_WriteIO(stdin_.get(), str.data(), str.size());
                if (bytes_written < str.size()) {
                    auto status = SDL_GetIOStatus(stdin_.get());
                    if (status == SDL_IO_STATUS_ERROR) {
                        return make_unexpected(get_error());
                    }
                }
                return bytes_written;
            }

            /**
             * @brief Read all available data from stdout (convenience method)
             * @return Expected containing the data as string or error
             */
            [[nodiscard]] expected <std::string, std::string> read_stdout_all() noexcept {
                if (!stdout_) {
                    return make_unexpected("stdout not redirected to pipe");
                }

                std::string output_data;
                std::vector <uint8_t> buffer(4096);
                SDL_IOStream* stream = stdout_.get();

                while (true) {
                    size_t bytes_read = SDL_ReadIO(stream, buffer.data(), buffer.size());

                    if (bytes_read == 0) {
                        // Check if it's EOF or error
                        auto status = SDL_GetIOStatus(stream);
                        if (status == SDL_IO_STATUS_ERROR) {
                            return make_unexpected(get_error());
                        }
                        // Otherwise it's EOF
                        break;
                    }

                    output_data.append(reinterpret_cast <const char*>(buffer.data()), bytes_read);
                }

                return output_data;
            }

            /**
             * @brief Read all available data from stderr (convenience method)
             * @return Expected containing the data as string or error
             */
            [[nodiscard]] expected <std::string, std::string> read_stderr_all() noexcept {
                if (!stderr_) {
                    return make_unexpected("stderr not redirected to pipe");
                }

                std::string output_data;
                std::vector <uint8_t> buffer(4096);
                SDL_IOStream* stream = stderr_.get();

                while (true) {
                    size_t bytes_read = SDL_ReadIO(stream, buffer.data(), buffer.size());

                    if (bytes_read == 0) {
                        // Check if it's EOF or error
                        auto status = SDL_GetIOStatus(stream);
                        if (status == SDL_IO_STATUS_ERROR) {
                            return make_unexpected(get_error());
                        }
                        // Otherwise it's EOF
                        break;
                    }

                    output_data.append(reinterpret_cast <const char*>(buffer.data()), bytes_read);
                }

                return output_data;
            }

            /**
             * @brief Check if the process is valid
             * @return true if a process is running
             */
            [[nodiscard]] bool is_valid() const noexcept {
                return handle_ != nullptr;
            }

            /**
             * @brief Check if the process is valid (explicit bool conversion)
             */
            [[nodiscard]] explicit operator bool() const noexcept {
                return is_valid();
            }

            /**
             * @brief Terminate and clean up the process
             */
            void reset() noexcept {
                if (handle_) {
                    // Close pipes first
                    stdin_.reset();
                    stdout_.reset();
                    stderr_.reset();

                    // Then destroy the process
                    SDL_DestroyProcess(handle_);
                    handle_ = nullptr;
                }
            }

            /**
             * @brief Release ownership of the handle without terminating
             * @return The raw handle (caller takes ownership)
             */
            [[nodiscard]] SDL_Process* release() noexcept {
                SDL_Process* temp = handle_;
                handle_ = nullptr;
                stdin_.reset();
                stdout_.reset();
                stderr_.reset();
                return temp;
            }

            /**
             * @brief Get the raw handle (does not transfer ownership)
             * @return The raw handle
             */
            [[nodiscard]] SDL_Process* get() const noexcept {
                return handle_;
            }

        private:
            friend class process_builder;

            SDL_Process* handle_ = nullptr;
            std::unique_ptr <SDL_IOStream, decltype(&no_close)> stdin_{nullptr, no_close};
            std::unique_ptr <SDL_IOStream, decltype(&no_close)> stdout_{nullptr, no_close};
            std::unique_ptr <SDL_IOStream, decltype(&no_close)> stderr_{nullptr, no_close};
    };

    /**
     * @brief Builder for creating processes with custom I/O configuration
     *
     * This class provides a fluent interface for configuring process
     * creation with redirected I/O streams.
     *
     * Example:
     * @code
     * auto proc = sdlpp::process_builder()
     *     .set_command({"python", "-c", "print('Hello')"})
     *     .stdout_to_pipe()
     *     .stderr_to_null()
     *     .set_env("PYTHONPATH", "/custom/path")
     *     .spawn();
     * @endcode
     */
    class process_builder {
        public:
            /**
             * @brief Set the command and arguments
             * @param args Command and arguments (first element is the command)
             * @return Reference to this builder
             */
            process_builder& set_command(const std::vector <std::string>& args) {
                args_ = args;
                return *this;
            }

            /**
             * @brief Set the command
             * @param cmd Command to execute
             * @return Reference to this builder
             */
            process_builder& set_command(const std::string& cmd) {
                args_ = {cmd};
                return *this;
            }

            /**
             * @brief Add an argument
             * @param arg Argument to add
             * @return Reference to this builder
             */
            process_builder& add_arg(const std::string& arg) {
                args_.push_back(arg);
                return *this;
            }

            /**
             * @brief Redirect stdin to a pipe
             * @return Reference to this builder
             */
            process_builder& stdin_from_pipe() {
                stdin_mode_ = process_io::app;
                return *this;
            }

            /**
             * @brief Redirect stdin to null
             * @return Reference to this builder
             */
            process_builder& stdin_from_null() {
                stdin_mode_ = process_io::null;
                return *this;
            }

            /**
             * @brief Redirect stdout to a pipe
             * @return Reference to this builder
             */
            process_builder& stdout_to_pipe() {
                stdout_mode_ = process_io::app;
                return *this;
            }

            /**
             * @brief Redirect stdout to null
             * @return Reference to this builder
             */
            process_builder& stdout_to_null() {
                stdout_mode_ = process_io::null;
                return *this;
            }

            /**
             * @brief Redirect stderr to a pipe
             * @return Reference to this builder
             */
            process_builder& stderr_to_pipe() {
                stderr_mode_ = process_io::app;
                return *this;
            }

            /**
             * @brief Redirect stderr to null
             * @return Reference to this builder
             */
            process_builder& stderr_to_null() {
                stderr_mode_ = process_io::null;
                return *this;
            }

            /**
             * @brief Redirect stderr to stdout
             * @return Reference to this builder
             */
            process_builder& stderr_to_stdout() {
                stderr_mode_ = process_io::redirect;
                return *this;
            }

            /**
             * @brief Set an environment variable
             * @param key Variable name
             * @param value Variable value
             * @return Reference to this builder
             */
            process_builder& set_env(const std::string& key, const std::string& value) {
                env_[key] = value;
                return *this;
            }

            /**
             * @brief Clear all environment variables
             * @return Reference to this builder
             */
            process_builder& clear_env() {
                env_.clear();
                env_cleared_ = true;
                return *this;
            }

            /**
             * @brief Spawn the process
             * @return Expected containing the process or error message
             */
            [[nodiscard]] expected <process, std::string> spawn() noexcept;

        private:
            std::vector <std::string> args_;
            process_io stdin_mode_ = process_io::inherited;
            process_io stdout_mode_ = process_io::inherited;
            process_io stderr_mode_ = process_io::inherited;
            std::unordered_map <std::string, std::string> env_;
            bool env_cleared_ = false;
    };

    // Implementation

    inline expected <process, std::string> process::create(
        const std::vector <std::string>& args,
        bool pipe_stdio) noexcept {
        if (args.empty()) {
            return make_unexpected("No command specified");
        }

        // Convert arguments to C-style array
        std::vector <const char*> argv;
        argv.reserve(args.size() + 1);
        for (const auto& arg : args) {
            argv.push_back(arg.c_str());
        }
        argv.push_back(nullptr);

        // Create process with optional I/O redirection
        SDL_Process* handle = SDL_CreateProcess(argv.data(), pipe_stdio);

        if (!handle) {
            return make_unexpected(get_error());
        }

        process proc;
        proc.handle_ = handle;

        // Get I/O streams if pipes were created
        if (pipe_stdio) {
            SDL_IOStream* stdin_stream = SDL_GetProcessInput(handle);
            if (stdin_stream) {
                proc.stdin_.reset(stdin_stream);
            }

            SDL_IOStream* stdout_stream = SDL_GetProcessOutput(handle);
            if (stdout_stream) {
                proc.stdout_.reset(stdout_stream);
            }

            // Note: SDL3 doesn't have separate stderr when using pipe_stdio
            // stderr is combined with stdout
        }

        return proc;
    }

    inline expected <process, std::string> process_builder::spawn() noexcept {
        if (args_.empty()) {
            return make_unexpected("No command specified");
        }

        // Convert arguments to C-style array
        std::vector <const char*> argv;
        argv.reserve(args_.size() + 1);
        for (const auto& arg : args_) {
            argv.push_back(arg.c_str());
        }
        argv.push_back(nullptr);

        // Set up process properties
        SDL_PropertiesID props = SDL_CreateProperties();
        if (props == 0) {
            return make_unexpected(get_error());
        }

        // Auto-cleanup properties
        struct PropertiesGuard {
            SDL_PropertiesID id;
            ~PropertiesGuard() { SDL_DestroyProperties(id); }
        } guard{props};

        // Set command arguments
        SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ARGS_POINTER,
                               argv.data());

        // Configure I/O redirection
        SDL_IOStream* stdin_stream = nullptr;
        SDL_IOStream* stdout_stream = nullptr;

        // Configure stdin
        switch (stdin_mode_) {
            case process_io::null:
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDIN_NUMBER,
                                      SDL_PROCESS_STDIO_NULL);
                break;
            case process_io::app:
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDIN_NUMBER,
                                      SDL_PROCESS_STDIO_APP);
                break;
            default:
                // Inherited (default)
                break;
        }

        // Configure stdout
        switch (stdout_mode_) {
            case process_io::null:
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER,
                                      SDL_PROCESS_STDIO_NULL);
                break;
            case process_io::app:
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDOUT_NUMBER,
                                      SDL_PROCESS_STDIO_APP);
                break;
            default:
                // Inherited (default)
                break;
        }

        // Configure stderr
        switch (stderr_mode_) {
            case process_io::null:
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDERR_NUMBER,
                                      SDL_PROCESS_STDIO_NULL);
                break;
            case process_io::app:
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDERR_NUMBER,
                                      SDL_PROCESS_STDIO_APP);
                break;
            case process_io::redirect:
                SDL_SetNumberProperty(props, SDL_PROP_PROCESS_CREATE_STDERR_NUMBER,
                                      SDL_PROCESS_STDIO_REDIRECT);
                break;
            default:
                // Inherited (default)
                break;
        }

        // Set environment variables
        std::vector <std::string> env_strings; // Move outside to keep strings alive
        if (!env_.empty() || env_cleared_) {
            std::vector <const char*> env_array;

            if (!env_cleared_) {
                // Start with current environment
                // Note: SDL will copy the current environment by default
            }

            // Add custom environment variables
            env_strings.reserve(env_.size());

            for (const auto& [key, value] : env_) {
                env_strings.push_back(key + "=" + value);
            }

            for (const auto& env_str : env_strings) {
                env_array.push_back(env_str.c_str());
            }
            env_array.push_back(nullptr);

            if (env_array.size() > 1) {
                // More than just the null terminator
                SDL_SetPointerProperty(props, SDL_PROP_PROCESS_CREATE_ENVIRONMENT_POINTER,
                                       env_array.data());
            }
        }

        // Create the process
        SDL_Process* handle = SDL_CreateProcessWithProperties(props);
        if (!handle) {
            return make_unexpected(get_error());
        }

        process proc;
        proc.handle_ = handle;

        // Get I/O streams if requested
        if (stdin_mode_ == process_io::app) {
            stdin_stream = SDL_GetProcessInput(handle);
            if (stdin_stream) {
                proc.stdin_.reset(stdin_stream);
            }
        }

        if (stdout_mode_ == process_io::app) {
            stdout_stream = SDL_GetProcessOutput(handle);
            if (stdout_stream) {
                proc.stdout_.reset(stdout_stream);
            }
        }

        // Note: SDL3 doesn't appear to have a separate stderr stream function
        // stderr might be combined with stdout or need different handling
        if (stderr_mode_ == process_io::app) {
            // For now, leave stderr as null since SDL3 doesn't provide
            // a separate function for getting stderr
            // TODO: Check SDL3 documentation for proper stderr handling
        }

        return proc;
    }

    /**
     * @brief Execute a command and wait for completion
     * @param args Command and arguments
     * @return Exit status or error
     */
    [[nodiscard]] inline expected <process_exit_status, std::string> execute(
        const std::vector <std::string>& args) noexcept {
        auto proc_result = process::create(args);
        if (!proc_result) {
            return make_unexpected(proc_result.error());
        }

        auto status = proc_result.value().wait();
        if (!status) {
            return make_unexpected("Failed to wait for process");
        }

        return status.value();
    }

    /**
     * @brief Execute a command and capture its output
     * @param args Command and arguments
     * @return Pair of exit status and output, or error
     */
    [[nodiscard]] inline expected <std::pair <process_exit_status, std::string>, std::string>
    execute_with_output(const std::vector <std::string>& args) noexcept {
        auto proc_result = process_builder()
                           .set_command(args)
                           .stdout_to_pipe()
                           .stderr_to_stdout()
                           .spawn();

        if (!proc_result) {
            return make_unexpected(proc_result.error());
        }

        auto& proc = proc_result.value();

        // Read output
        std::string output;
        auto output_result = proc.read_stdout_all();
        if (output_result) {
            output = std::move(output_result.value());
        }

        // Wait for completion
        auto status = proc.wait();
        if (!status) {
            return make_unexpected("Failed to wait for process");
        }

        return std::make_pair(status.value(), std::move(output));
    }
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for process_io
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, process_io value);

/**
 * @brief Stream input operator for process_io
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, process_io& value);

}