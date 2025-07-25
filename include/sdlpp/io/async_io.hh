/**
 * @file async_io.hh
 * @brief Asynchronous I/O operations
 */
#pragma once

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/io/io_common.hh>

#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <string>
#include <vector>
#include <chrono>
#include <filesystem>

namespace sdlpp {
    // Forward declarations
    class async_io;
    class async_io_queue;

    /**
     * @brief Result types for async I/O operations
     */
    enum class async_io_result {
        complete = SDL_ASYNCIO_COMPLETE,
        error = SDL_ASYNCIO_FAILURE,
        canceled = SDL_ASYNCIO_CANCELED,
    };

    /**
     * @brief Task types for async I/O operations
     */
    enum class async_io_task_type {
        read = SDL_ASYNCIO_TASK_READ,
        write = SDL_ASYNCIO_TASK_WRITE,
        close = SDL_ASYNCIO_TASK_CLOSE,
    };

    /**
     * @brief Outcome of an async I/O task
     */
    struct async_io_outcome {
        async_io_result result{async_io_result::error};
        std::int64_t bytes_transferred{-1};
        std::string error_message;

        [[nodiscard]] bool is_complete() const noexcept {
            return result == async_io_result::complete;
        }

        [[nodiscard]] bool is_error() const noexcept {
            return result == async_io_result::error;
        }

        [[nodiscard]] bool is_canceled() const noexcept {
            return result == async_io_result::canceled;
        }

        [[nodiscard]] static async_io_outcome from_sdl(const SDL_AsyncIOOutcome& outcome) noexcept {
            async_io_outcome out;
            out.result = static_cast <async_io_result>(outcome.result);
            out.bytes_transferred = static_cast <std::int64_t>(outcome.bytes_transferred);
            if (outcome.result == SDL_ASYNCIO_FAILURE) {
                out.error_message = get_error();
            }
            return out;
        }
    };

    /**
     * @brief RAII wrapper for SDL_AsyncIOQueue
     */
    class async_io_queue {
        public:
            using queue_ptr = pointer <SDL_AsyncIOQueue, SDL_DestroyAsyncIOQueue>;

            // Constructors
            async_io_queue() noexcept = default;

            explicit async_io_queue(queue_ptr ptr) noexcept
                : ptr_(std::move(ptr)) {
            }

            /**
             * @brief Create a new async I/O queue
             * @return Queue instance or error
             */
            [[nodiscard]] static expected <async_io_queue, std::string> create() noexcept {
                auto* raw = SDL_CreateAsyncIOQueue();
                if (!raw) {
                    return make_unexpected(get_error());
                }
                return async_io_queue{queue_ptr{raw}};
            }

            /**
             * @brief Wait for any async I/O task to complete
             * @param task Reference to store the completed task
             * @param outcome Reference to store the outcome
             * @param timeout_ms Timeout in milliseconds (-1 for infinite)
             * @return true if a task completed, false on timeout
             */
            [[nodiscard]] bool wait_result(
                std::shared_ptr <void>& task,
                async_io_outcome& outcome,
                std::int32_t timeout_ms = -1) noexcept {
                SDL_AsyncIOOutcome sdl_outcome;

                bool got_result = SDL_WaitAsyncIOResult(ptr_.get(), &sdl_outcome, timeout_ms);
                if (got_result) {
                    // Get the task from the outcome
                    if (sdl_outcome.userdata) {
                        auto* task_ptr = static_cast <std::shared_ptr <void>*>(sdl_outcome.userdata);
                        task = *task_ptr;
                        delete task_ptr;
                    }

                    outcome = async_io_outcome::from_sdl(sdl_outcome);
                }
                return got_result;
            }

            /**
             * @brief Wait for any async I/O task to complete (with SDL outcome)
             * @param sdl_outcome Reference to store the SDL outcome
             * @param timeout_ms Timeout in milliseconds (-1 for infinite)
             * @return true if a task completed, false on timeout
             */
            [[nodiscard]] bool wait_result_raw(
                SDL_AsyncIOOutcome& sdl_outcome,
                std::int32_t timeout_ms = -1) noexcept {
                return SDL_WaitAsyncIOResult(ptr_.get(), &sdl_outcome, timeout_ms);
            }

            /**
             * @brief Try to get any completed async I/O task (non-blocking)
             * @param task Reference to store the completed task
             * @param outcome Reference to store the outcome
             * @return true if a task was available, false otherwise
             */
            [[nodiscard]] bool try_get_result(
                std::shared_ptr <void>& task,
                async_io_outcome& outcome) noexcept {
                return wait_result(task, outcome, 0);
            }

            /**
             * @brief Signal all tasks in the queue with an error
             * @param error_msg Optional error message
             */
            void signal_error(const std::string& error_msg = "") noexcept {
                if (!error_msg.empty()) {
                    [[maybe_unused]] auto res = set_error(error_msg);
                }
                if (ptr_) {
                    SDL_SignalAsyncIOQueue(ptr_.get());
                }
            }

            /**
             * @brief Get the native SDL_AsyncIOQueue handle
             * @return Raw pointer to SDL_AsyncIOQueue
             */
            [[nodiscard]] SDL_AsyncIOQueue* native_handle() const noexcept {
                return ptr_.get();
            }

            // Accessors
            [[nodiscard]] SDL_AsyncIOQueue* get() const noexcept { return ptr_.get(); }
            [[nodiscard]] explicit operator bool() const noexcept { return static_cast <bool>(ptr_); }

        private:
            queue_ptr ptr_;
    };

    /**
     * @brief RAII wrapper for SDL_AsyncIO
     *
     * @note SDL3's async I/O implementation may have issues with certain operations.
     * The SDL_ReadAsyncIO/SDL_WriteAsyncIO functions may trigger assertions in SDL's
     * generic async I/O implementation. Consider using SDL_LoadFileAsync for simple
     * file loading operations instead.
     */
    class async_io {
        public:
            // Custom deleter for SDL_AsyncIO that doesn't call SDL_CloseAsyncIO
            // (since we handle closing through async operations)
            struct async_io_deleter {
                void operator()(SDL_AsyncIO* ptr) const noexcept {
                    // Don't delete here - handled by close_async
                    (void)ptr;
                }
            };

            using async_io_ptr = std::unique_ptr <SDL_AsyncIO, async_io_deleter>;

            // Task handle for tracking async operations
            struct task_handle {
                async_io_task_type type;
                std::size_t size;
                std::chrono::steady_clock::time_point start_time;
            };

            // Constructors
            async_io() noexcept = default;

            async_io(async_io_ptr ptr, async_io_queue& queue) noexcept
                : ptr_(std::move(ptr)), queue_(&queue) {
            }

            /**
             * @brief Open a file for async I/O
             * @param path Path to the file
             * @param mode File open mode
             * @param queue The async I/O queue to use
             * @return Async I/O instance or error
             */
            [[nodiscard]] static expected <async_io, std::string> open_file(
                const std::filesystem::path& path,
                file_mode mode,
                async_io_queue& queue) noexcept {
                auto* raw = SDL_AsyncIOFromFile(path.string().c_str(), to_string(mode));
                if (!raw) {
                    return make_unexpected(get_error());
                }
                return async_io{async_io_ptr{raw}, queue};
            }

            /**
             * @brief Open a file for async I/O (string overload for convenience)
             * @param filename Path to the file
             * @param mode File open mode
             * @param queue The async I/O queue to use
             * @return Async I/O instance or error
             */
            [[nodiscard]] static expected <async_io, std::string> open_file(
                const std::string& filename,
                file_mode mode,
                async_io_queue& queue) noexcept {
                return open_file(std::filesystem::path(filename), mode, queue);
            }

            // Note: SDL3 doesn't have SDL_AsyncIOFromFP, only SDL_AsyncIOFromFile

            /**
             * @brief Get the size of the async I/O source
             * @return Size in bytes or error
             */
            [[nodiscard]] expected <std::int64_t, std::string> size() const noexcept {
                if (!ptr_) {
                    return make_unexpected("Invalid async I/O handle");
                }
                std::int64_t size = SDL_GetAsyncIOSize(ptr_.get());
                if (size < 0) {
                    return make_unexpected(get_error());
                }
                return size;
            }

            /**
             * @brief Read data asynchronously
             * @param offset Offset to read from
             * @param buffer Buffer to read into
             * @param num_bytes Number of bytes to read
             * @return Task handle for tracking the operation
             */
            [[nodiscard]] std::shared_ptr <task_handle> read_async(
                std::uint64_t offset,
                void* buffer,
                std::uint64_t size) noexcept {
                if (!ptr_ || !queue_) {
                    return nullptr;
                }

                auto handle = std::make_shared <task_handle>(task_handle{
                    async_io_task_type::read,
                    static_cast <std::size_t>(size),
                    std::chrono::steady_clock::now()
                });

                // Create a copy of the shared_ptr to pass as user data
                auto* user_data = new std::shared_ptr <void>(handle);

                if (!SDL_ReadAsyncIO(ptr_.get(), buffer, offset, size,
                                     queue_->get(), user_data)) {
                    delete user_data;
                    return nullptr;
                }

                return handle;
            }

            /**
             * @brief Read data asynchronously into a vector
             * @param offset Offset to read from
             * @param num_bytes Number of bytes to read
             * @param buffer Vector to store the data (will be resized)
             * @return Task handle for tracking the operation
             */
            [[nodiscard]] std::shared_ptr <task_handle> read_async(
                std::uint64_t offset,
                std::uint64_t size,
                std::vector <std::uint8_t>& buffer) noexcept {
                buffer.resize(static_cast <std::size_t>(size));
                return read_async(offset, buffer.data(), size);
            }

            /**
             * @brief Write data asynchronously
             * @param offset Offset to write to
             * @param buffer Buffer containing data to write
             * @param num_bytes Number of bytes to write
             * @return Task handle for tracking the operation
             */
            [[nodiscard]] std::shared_ptr <task_handle> write_async(
                std::uint64_t offset,
                const void* buffer,
                std::uint64_t size) noexcept {
                if (!ptr_ || !queue_) {
                    return nullptr;
                }

                auto handle = std::make_shared <task_handle>(task_handle{
                    async_io_task_type::write,
                    static_cast <std::size_t>(size),
                    std::chrono::steady_clock::now()
                });

                // Create a copy of the shared_ptr to pass as user data
                auto* user_data = new std::shared_ptr <void>(handle);

                if (!SDL_WriteAsyncIO(ptr_.get(), const_cast <void*>(buffer),
                                      offset, size, queue_->get(), user_data)) {
                    delete user_data;
                    return nullptr;
                }

                return handle;
            }

            /**
             * @brief Write data asynchronously from a span
             * @param offset Offset to write to
             * @param data Span containing data to write
             * @return Task handle for tracking the operation
             */
            template<typename T>
            [[nodiscard]] std::shared_ptr <task_handle> write_async(
                std::uint64_t offset,
                std::span <const T> data) noexcept {
                return write_async(offset, data.data(), data.size_bytes());
            }

            /**
             * @brief Close the async I/O handle asynchronously
             * @param wait_pending Whether to wait for pending operations
             * @return Task handle for tracking the operation
             */
            [[nodiscard]] std::shared_ptr <task_handle> close_async(
                bool wait_pending = true) noexcept {
                if (!ptr_ || !queue_) {
                    return nullptr;
                }

                auto handle = std::make_shared <task_handle>(task_handle{
                    async_io_task_type::close,
                    0,
                    std::chrono::steady_clock::now()
                });

                // Create a copy of the shared_ptr to pass as user data
                auto* user_data = new std::shared_ptr <void>(handle);

                if (!SDL_CloseAsyncIO(ptr_.release(), wait_pending,
                                      queue_->get(), user_data)) {
                    delete user_data;
                    return nullptr;
                }

                return handle;
            }

            /**
             * @brief Cancel all pending operations for this async I/O
             * @return true on success, false on error
             */
            [[nodiscard]] bool cancel_all() noexcept {
                if (!ptr_) {
                    return false;
                }
                // SDL3 doesn't have SDL_CancelAsyncIO - operations need to be tracked individually
                return false;
            }

            /**
             * @brief Get the native SDL_AsyncIO handle
             * @return Raw pointer to SDL_AsyncIO
             */
            [[nodiscard]] SDL_AsyncIO* native_handle() const noexcept {
                return ptr_.get();
            }

            // Accessors
            [[nodiscard]] SDL_AsyncIO* get() const noexcept { return ptr_.get(); }
            [[nodiscard]] explicit operator bool() const noexcept { return static_cast <bool>(ptr_); }

        private:
            async_io_ptr ptr_;
            async_io_queue* queue_{nullptr};
    };

    /**
     * @brief Load a file asynchronously (simplified API)
     *
     * This is a wrapper around SDL_LoadFileAsync which is more reliable than
     * the generic async I/O operations in current SDL3 implementations.
     *
     * @param path Path to the file to load
     * @param queue The async I/O queue to use
     * @param userdata Optional user data passed to the completion handler
     * @return true on success, false on failure
     */
    [[nodiscard]] inline bool load_file_async(
        const std::filesystem::path& path,
        async_io_queue& queue,
        void* userdata = nullptr) noexcept {
        return SDL_LoadFileAsync(path.string().c_str(), queue.get(), userdata);
    }

    /**
     * @brief Load a file asynchronously (string overload for convenience)
     * @param filename Path to the file to load
     * @param queue The async I/O queue to use
     * @param userdata Optional user data passed to the completion handler
     * @return true on success, false on failure
     */
    [[nodiscard]] inline bool load_file_async(
        const std::string& filename,
        async_io_queue& queue,
        void* userdata = nullptr) noexcept {
        return load_file_async(std::filesystem::path(filename), queue, userdata);
    }

    /**
     * @brief Result of a load_file_async operation
     */
    struct load_file_result {
        std::unique_ptr <void, decltype(&SDL_free)> data{nullptr, SDL_free};
        std::size_t size{0};

        [[nodiscard]] bool is_valid() const noexcept {
            return data != nullptr;
        }

        [[nodiscard]] std::string_view as_string_view() const noexcept {
            if (!data) return {};
            return std::string_view(static_cast <const char*>(data.get()), size);
        }

        [[nodiscard]] std::span <const std::uint8_t> as_bytes() const noexcept {
            if (!data) return {};
            return std::span <const std::uint8_t>(
                static_cast <const std::uint8_t*>(data.get()), size);
        }
    };

    /**
     * @brief Extract load_file_async result from an SDL_AsyncIOOutcome
     *
     * @param outcome The SDL async I/O outcome from a completed operation
     * @return Load file result with the data and size
     */
    [[nodiscard]] inline load_file_result get_load_file_result(
        const SDL_AsyncIOOutcome& outcome) noexcept {
        load_file_result file_result;
        if (outcome.result == SDL_ASYNCIO_COMPLETE && outcome.bytes_transferred > 0) {
            // SDL_LoadFileAsync stores the buffer pointer in the outcome
            // The buffer needs to be freed with SDL_free
            file_result.data.reset(outcome.buffer);
            file_result.size = static_cast <std::size_t>(outcome.bytes_transferred);
        }
        return file_result;
    }

    /**
     * @brief Helper class for managing multiple async I/O operations
     */
    class async_io_manager {
        public:
            using completion_callback = std::function <void(
                const std::shared_ptr <async_io::task_handle>&,
                const async_io_outcome&)>;

            explicit async_io_manager(async_io_queue& queue)
                : queue_(&queue) {
            }

            /**
             * @brief Process completed operations (non-blocking)
             * @param callback Callback to invoke for each completed operation
             * @return Number of operations processed
             */
            std::size_t process_completed(completion_callback callback) {
                std::size_t count = 0;
                std::shared_ptr <void> task;
                async_io_outcome outcome;

                while (queue_->try_get_result(task, outcome)) {
                    if (task && task.use_count() > 0) {
                        // Try to cast to our task_handle type
                        if (auto handle = std::static_pointer_cast <async_io::task_handle>(task)) {
                            if (callback) {
                                callback(handle, outcome);
                            }
                            ++count;
                        }
                    }
                }

                return count;
            }

            /**
             * @brief Wait for at least one operation to complete
             * @param callback Callback to invoke for completed operations
             * @param timeout Timeout duration (-1 for infinite)
             * @return Number of operations processed
             */
            template<typename Rep, typename Period>
            std::size_t wait_and_process(
                completion_callback callback,
                std::chrono::duration <Rep, Period> timeout) {
                auto ms = std::chrono::duration_cast <std::chrono::milliseconds>(timeout).count();
                return wait_and_process_ms(callback, static_cast <std::int32_t>(ms));
            }

            /**
             * @brief Wait for at least one operation to complete (milliseconds)
             * @param callback Callback to invoke for completed operations
             * @param timeout_ms Timeout in milliseconds (-1 for infinite)
             * @return Number of operations processed
             */
            std::size_t wait_and_process_ms(
                completion_callback callback,
                std::int32_t timeout_ms = -1) {
                std::shared_ptr <void> task;
                async_io_outcome outcome;

                if (queue_->wait_result(task, outcome, timeout_ms)) {
                    if (task && task.use_count() > 0) {
                        // Try to cast to our task_handle type
                        if (auto handle = std::static_pointer_cast <async_io::task_handle>(task)) {
                            if (callback) {
                                callback(handle, outcome);
                            }

                            // Process any additional completed operations
                            return 1 + process_completed(callback);
                        }
                    }
                }

                return 0;
            }

        private:
            async_io_queue* queue_;
    };
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for async_io_result
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, async_io_result value);

/**
 * @brief Stream input operator for async_io_result
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, async_io_result& value);

/**
 * @brief Stream output operator for async_io_task_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, async_io_task_type value);

/**
 * @brief Stream input operator for async_io_task_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, async_io_task_type& value);

}