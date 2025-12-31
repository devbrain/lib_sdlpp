//
// Created by igor on 7/13/25.
//

#pragma once

/**
 * @file iostream.hh
 * @brief C++ wrapper for SDL3 IOStream functionality
 * 
 * This header provides modern C++ wrappers around SDL3's IOStream system,
 * which provides an abstract interface for reading and writing data streams.
 * Supports file I/O, memory I/O, and custom stream implementations.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/io/io_common.hh>
#include <string>
#include <vector>
#include <span>
#include <memory>
#include <istream>
#include <ostream>
#include <filesystem>

namespace sdlpp {
    /**
     * @brief Seek origin positions for IOStream operations
     */
    enum class io_seek_pos {
        set = SDL_IO_SEEK_SET, ///< Seek from the beginning of the stream
        current = SDL_IO_SEEK_CUR, ///< Seek relative to current position
        end = SDL_IO_SEEK_END ///< Seek relative to the end of the stream
    };

    /**
     * @brief I/O operation status codes
     */
    enum class io_status {
        ready = SDL_IO_STATUS_READY, ///< Stream is ready for operations
        error = SDL_IO_STATUS_ERROR, ///< An error occurred
        eof = SDL_IO_STATUS_EOF, ///< End of file/stream reached
        not_ready = SDL_IO_STATUS_NOT_READY, ///< Stream not ready (e.g., async operation pending)
        read_only = SDL_IO_STATUS_READONLY, ///< Stream is read-only
        write_only = SDL_IO_STATUS_WRITEONLY ///< Stream is write-only
    };

    /**
     * @brief Smart pointer type for SDL_IOStream with automatic cleanup
     */
    using iostream_ptr = pointer <SDL_IOStream, SDL_CloseIO>;

    /**
     * @brief RAII wrapper for SDL IOStream operations
     *
     * This class provides a safe, RAII-managed interface to SDL's IOStream
     * functionality. It supports reading, writing, seeking, and provides
     * convenient methods for common I/O operations.
     */
    class iostream {
        private:
            iostream_ptr stream;

        public:
            /**
             * @brief Default constructor - creates an empty iostream
             */
            iostream() = default;

            /**
             * @brief Construct from existing SDL_IOStream pointer
             * @param io Existing SDL_IOStream pointer (takes ownership)
             */
            explicit iostream(SDL_IOStream* io)
                : stream(io) {
            }

            /**
             * @brief Move constructor
             */
            iostream(iostream&&) = default;

            /**
             * @brief Move assignment operator
             */
            iostream& operator=(iostream&&) = default;

            // Delete copy operations
            iostream(const iostream&) = delete;
            iostream& operator=(const iostream&) = delete;

            /**
             * @brief Check if the stream is valid
             * @return true if the stream is valid, false otherwise
             */
            [[nodiscard]] bool is_valid() const { return stream != nullptr; }

            /**
             * @brief Check if the stream is valid (conversion operator)
             */
            [[nodiscard]] explicit operator bool() const { return is_valid(); }

            /**
             * @brief Get the underlying SDL_IOStream pointer
             * @return Raw SDL_IOStream pointer (does not transfer ownership)
             */
            [[nodiscard]] SDL_IOStream* get() const { return stream.get(); }

            /**
             * @brief Get the size of the stream
             * @return Expected containing the size in bytes, or error message
             */
            [[nodiscard]] expected <Sint64, std::string> size() const {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Sint64 io_size = SDL_GetIOSize(stream.get());
                if (io_size < 0) {
                    return make_unexpectedf(get_error());
                }
                return io_size;
            }

            /**
             * @brief Get the current position in the stream
             * @return Expected containing the position, or error message
             */
            [[nodiscard]] expected <Sint64, std::string> tell() const {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Sint64 position = SDL_TellIO(stream.get());
                if (position < 0) {
                    return make_unexpectedf(get_error());
                }
                return position;
            }

            /**
             * @brief Seek to a position in the stream
             * @param offset Offset in bytes
             * @param whence Origin position for the seek
             * @return Expected containing the new position, or error message
             */
            expected <Sint64, std::string> seek(Sint64 offset, io_seek_pos whence) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Sint64 new_position = SDL_SeekIO(stream.get(), offset, static_cast <SDL_IOWhence>(whence));
                if (new_position < 0) {
                    return make_unexpectedf(get_error());
                }
                return new_position;
            }

            /**
             * @brief Read data from the stream
             * @param buffer Buffer to read into
             * @param num_bytes Number of bytes to read
             * @return Expected containing number of bytes read, or error message
             */
            expected <size_t, std::string> read(void* buffer, size_t num_bytes) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                size_t bytes_read = SDL_ReadIO(stream.get(), buffer, num_bytes);
                if (bytes_read == 0 && num_bytes > 0) {
                    // Check if it's an error or EOF
                    auto status = SDL_GetIOStatus(stream.get());
                    if (status == SDL_IO_STATUS_ERROR) {
                        return make_unexpectedf(get_error());
                    }
                }
                return bytes_read;
            }

            /**
             * @brief Read data into a vector
             * @param num_bytes Number of bytes to read
             * @return Expected containing vector with data, or error message
             */
            expected <std::vector <uint8_t>, std::string> read(size_t num_bytes) {
                std::vector <uint8_t> buffer(num_bytes);
                auto read_result = read(buffer.data(), num_bytes);
                if (!read_result) {
                    return make_unexpectedf(read_result.error());
                }
                buffer.resize(*read_result);
                return buffer;
            }

            /**
             * @brief Write data to the stream
             * @param buffer Buffer containing data to write
             * @param num_bytes Number of bytes to write
             * @return Expected containing number of bytes written, or error message
             */
            expected <size_t, std::string> write(const void* buffer, size_t num_bytes) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                size_t bytes_written = SDL_WriteIO(stream.get(), buffer, num_bytes);
                if (bytes_written < num_bytes) {
                    auto status = SDL_GetIOStatus(stream.get());
                    if (status == SDL_IO_STATUS_ERROR) {
                        return make_unexpectedf(get_error());
                    }
                }
                return bytes_written;
            }

            /**
             * @brief Write data from a span
             * @param data Span containing data to write
             * @return Expected containing number of bytes written, or error message
             */
            template<typename T>
            expected <size_t, std::string> write(std::span <const T> data) {
                return write(data.data(), data.size_bytes());
            }

            /**
             * @brief Flush any buffered data to the underlying stream
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> flush() {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                if (!SDL_FlushIO(stream.get())) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            /**
             * @brief Get the current I/O status
             * @return Current status of the stream
             */
            [[nodiscard]] io_status get_status() const {
                if (!stream) {
                    return io_status::error;
                }
                return static_cast <io_status>(SDL_GetIOStatus(stream.get()));
            }

            /**
             * @brief Read a single byte
             * @return Expected containing the byte value, or error message
             */
            expected <uint8_t, std::string> read_u8() {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Uint8 value;
                if (!SDL_ReadU8(stream.get(), &value)) {
                    return make_unexpectedf(get_error());
                }
                return value;
            }

            /**
             * @brief Write a single byte
             * @param value Value to write
             * @return Expected<void> - empty on success, error message on failure
             */
            expected <void, std::string> write_u8(uint8_t value) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                if (!SDL_WriteU8(stream.get(), value)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            // Additional read methods for different types
            expected <uint16_t, std::string> read_u16_le() {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Uint16 value;
                if (!SDL_ReadU16LE(stream.get(), &value)) {
                    return make_unexpectedf(get_error());
                }
                return value;
            }

            expected <uint16_t, std::string> read_u16_be() {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Uint16 value;
                if (!SDL_ReadU16BE(stream.get(), &value)) {
                    return make_unexpectedf(get_error());
                }
                return value;
            }

            expected <uint32_t, std::string> read_u32_le() {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Uint32 value;
                if (!SDL_ReadU32LE(stream.get(), &value)) {
                    return make_unexpectedf(get_error());
                }
                return value;
            }

            expected <uint32_t, std::string> read_u32_be() {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Uint32 value;
                if (!SDL_ReadU32BE(stream.get(), &value)) {
                    return make_unexpectedf(get_error());
                }
                return value;
            }

            expected <uint64_t, std::string> read_u64_le() {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Uint64 value;
                if (!SDL_ReadU64LE(stream.get(), &value)) {
                    return make_unexpectedf(get_error());
                }
                return value;
            }

            expected <uint64_t, std::string> read_u64_be() {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                Uint64 value;
                if (!SDL_ReadU64BE(stream.get(), &value)) {
                    return make_unexpectedf(get_error());
                }
                return value;
            }

            // Write methods for different types
            expected <void, std::string> write_u16_le(uint16_t value) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                if (!SDL_WriteU16LE(stream.get(), value)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            expected <void, std::string> write_u16_be(uint16_t value) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                if (!SDL_WriteU16BE(stream.get(), value)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            expected <void, std::string> write_u32_le(uint32_t value) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                if (!SDL_WriteU32LE(stream.get(), value)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            expected <void, std::string> write_u32_be(uint32_t value) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                if (!SDL_WriteU32BE(stream.get(), value)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            expected <void, std::string> write_u64_le(uint64_t value) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                if (!SDL_WriteU64LE(stream.get(), value)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }

            expected <void, std::string> write_u64_be(uint64_t value) {
                if (!stream) {
                    return make_unexpectedf("Invalid stream");
                }
                if (!SDL_WriteU64BE(stream.get(), value)) {
                    return make_unexpectedf(get_error());
                }
                return {};
            }
    };

    /**
     * @brief Implementation details for C++ stream integration
     */
    namespace detail {
        /**
         * @brief SDL_IOStreamInterface implementation for std::istream
         */
        struct istream_interface {
            static Sint64 size(void* userdata) {
                auto* stream = static_cast <std::istream*>(userdata);
                auto current = stream->tellg();
                stream->seekg(0, std::ios::end);
                auto size = stream->tellg();
                stream->seekg(current);
                return size;
            }

            static Sint64 seek(void* userdata, Sint64 offset, SDL_IOWhence whence) {
                auto* stream = static_cast <std::istream*>(userdata);
                std::ios::seekdir dir;
                switch (whence) {
                    case SDL_IO_SEEK_SET: dir = std::ios::beg;
                        break;
                    case SDL_IO_SEEK_CUR: dir = std::ios::cur;
                        break;
                    case SDL_IO_SEEK_END: dir = std::ios::end;
                        break;
                    default: return -1;
                }
                stream->seekg(offset, dir);
                return stream->fail() ? -1 : static_cast <Sint64>(stream->tellg());
            }

            static size_t read(void* userdata, void* ptr, size_t size, SDL_IOStatus* status) {
                auto* stream = static_cast <std::istream*>(userdata);
                stream->read(static_cast <char*>(ptr), static_cast <std::streamsize>(size));
                auto bytes_read = stream->gcount();

                if (status) {
                    if (stream->eof()) {
                        *status = SDL_IO_STATUS_EOF;
                    } else if (stream->fail()) {
                        *status = SDL_IO_STATUS_ERROR;
                    } else {
                        *status = SDL_IO_STATUS_READY;
                    }
                }

                return static_cast <size_t>(bytes_read);
            }

            static size_t write([[maybe_unused]] void* userdata, [[maybe_unused]] const void* ptr,
                                [[maybe_unused]] size_t size, SDL_IOStatus* status) {
                // std::istream is read-only
                if (status) {
                    *status = SDL_IO_STATUS_READONLY;
                }
                return 0;
            }

            static bool flush([[maybe_unused]] void* userdata, SDL_IOStatus* status) {
                // No-op for input streams
                if (status) {
                    *status = SDL_IO_STATUS_READY;
                }
                return true;
            }

            static bool close([[maybe_unused]] void* userdata) {
                // We don't own the stream, so don't close it
                return true;
            }
        };

        /**
         * @brief SDL_IOStreamInterface implementation for std::ostream
         */
        struct ostream_interface {
            static Sint64 size(void* userdata) {
                auto* stream = static_cast <std::ostream*>(userdata);
                auto current = stream->tellp();
                stream->seekp(0, std::ios::end);
                auto size = stream->tellp();
                stream->seekp(current);
                return size;
            }

            static Sint64 seek(void* userdata, Sint64 offset, SDL_IOWhence whence) {
                auto* stream = static_cast <std::ostream*>(userdata);
                std::ios::seekdir dir;
                switch (whence) {
                    case SDL_IO_SEEK_SET: dir = std::ios::beg;
                        break;
                    case SDL_IO_SEEK_CUR: dir = std::ios::cur;
                        break;
                    case SDL_IO_SEEK_END: dir = std::ios::end;
                        break;
                    default: return -1;
                }
                stream->seekp(offset, dir);
                return stream->fail() ? -1 : static_cast <Sint64>(stream->tellp());
            }

            static size_t read([[maybe_unused]] void* userdata, [[maybe_unused]] void* ptr,
                               [[maybe_unused]] size_t size, SDL_IOStatus* status) {
                // std::ostream is write-only
                if (status) {
                    *status = SDL_IO_STATUS_WRITEONLY;
                }
                return 0;
            }

            static size_t write(void* userdata, const void* ptr, size_t size, SDL_IOStatus* status) {
                auto* stream = static_cast <std::ostream*>(userdata);
                stream->write(static_cast <const char*>(ptr), static_cast <std::streamsize>(size));

                if (status) {
                    if (stream->fail()) {
                        *status = SDL_IO_STATUS_ERROR;
                    } else {
                        *status = SDL_IO_STATUS_READY;
                    }
                }

                return stream->good() ? size : 0;
            }

            static bool flush(void* userdata, SDL_IOStatus* status) {
                auto* stream = static_cast <std::ostream*>(userdata);
                stream->flush();

                if (status) {
                    *status = stream->fail() ? SDL_IO_STATUS_ERROR : SDL_IO_STATUS_READY;
                }

                return !stream->fail();
            }

            static bool close([[maybe_unused]] void* userdata) {
                // We don't own the stream, so don't close it
                return true;
            }
        };

        /**
         * @brief Helper to create SDL_IOStreamInterface
         */
        template<typename Interface>
        SDL_IOStreamInterface make_interface() {
            SDL_IOStreamInterface iface;
            SDL_INIT_INTERFACE(&iface);
            iface.size = Interface::size;
            iface.seek = Interface::seek;
            iface.read = Interface::read;
            iface.write = Interface::write;
            iface.flush = Interface::flush;
            iface.close = Interface::close;
            return iface;
        }
    } // namespace detail

    /**
     * @brief Create an IOStream from memory (read-only)
     * @param mem Pointer to memory buffer
     * @param mem_size Size of the buffer in bytes
     * @return Expected containing the iostream, or error message
     */
    inline expected <iostream, std::string> from_memory(const void* mem, size_t mem_size) {
        SDL_IOStream* io = SDL_IOFromConstMem(mem, mem_size);
        if (!io) {
            return make_unexpectedf(get_error());
        }
        return iostream(io);
    }

    /**
     * @brief Create an IOStream from memory (read-write)
     * @param mem Pointer to memory buffer
     * @param mem_size Size of the buffer in bytes
     * @return Expected containing the iostream, or error message
     */
    inline expected <iostream, std::string> from_memory(void* mem, size_t mem_size) {
        SDL_IOStream* io = SDL_IOFromMem(mem, mem_size);
        if (!io) {
            return make_unexpectedf(get_error());
        }
        return iostream(io);
    }

    /**
     * @brief Create an IOStream with dynamic memory allocation
     * @return Expected containing the iostream, or error message
     */
    inline expected <iostream, std::string> from_dynamic_memory() {
        SDL_IOStream* io = SDL_IOFromDynamicMem();
        if (!io) {
            return make_unexpectedf(get_error());
        }
        return iostream(io);
    }

    /**
     * @brief Create an SDL IOStream from a std::istream
     * @param stream Reference to the input stream (must remain valid for the lifetime of the SDL_IOStream)
     * @return Expected containing the iostream, or error message
     * @note The stream is not owned by the SDL_IOStream and won't be closed when the SDL_IOStream is destroyed
     */
    inline expected <iostream, std::string> from_istream(std::istream& stream) {
        static SDL_IOStreamInterface iface = detail::make_interface <detail::istream_interface>();

        SDL_IOStream* io = SDL_OpenIO(&iface, &stream);
        if (!io) {
            return make_unexpectedf(get_error());
        }

        return iostream(io);
    }

    /**
     * @brief Create an SDL IOStream from a std::ostream
     * @param stream Reference to the output stream (must remain valid for the lifetime of the SDL_IOStream)
     * @return Expected containing the iostream, or error message
     * @note The stream is not owned by the SDL_IOStream and won't be closed when the SDL_IOStream is destroyed
     */
    inline expected <iostream, std::string> from_ostream(std::ostream& stream) {
        static SDL_IOStreamInterface iface = detail::make_interface <detail::ostream_interface>();

        SDL_IOStream* io = SDL_OpenIO(&iface, &stream);
        if (!io) {
            return make_unexpectedf(get_error());
        }

        return iostream(io);
    }

    /**
     * @brief Create an SDL IOStream from a std::iostream (bidirectional stream)
     * @param stream Reference to the bidirectional stream (must remain valid for the lifetime of the SDL_IOStream)
     * @return Expected containing the iostream, or error message
     * @note The stream is not owned by the SDL_IOStream and won't be closed when the SDL_IOStream is destroyed
     */
    inline expected <iostream, std::string> from_iostream(std::iostream& stream) {
        // For bidirectional streams, we need a combined interface
        struct iostream_interface {
            static Sint64 size(void* userdata) {
                return detail::istream_interface::size(userdata);
            }

            static Sint64 seek(void* userdata, Sint64 offset, SDL_IOWhence whence) {
                auto* stream = static_cast <std::iostream*>(userdata);
                std::ios::seekdir dir;
                switch (whence) {
                    case SDL_IO_SEEK_SET: dir = std::ios::beg;
                        break;
                    case SDL_IO_SEEK_CUR: dir = std::ios::cur;
                        break;
                    case SDL_IO_SEEK_END: dir = std::ios::end;
                        break;
                    default: return -1;
                }
                stream->seekg(offset, dir);
                stream->seekp(offset, dir);
                return stream->fail() ? -1 : static_cast <Sint64>(stream->tellg());
            }

            static size_t read(void* userdata, void* ptr, size_t num_bytes, SDL_IOStatus* status) {
                return detail::istream_interface::read(userdata, ptr, num_bytes, status);
            }

            static size_t write(void* userdata, const void* ptr, size_t num_bytes, SDL_IOStatus* status) {
                return detail::ostream_interface::write(userdata, ptr, num_bytes, status);
            }

            static bool flush(void* userdata, SDL_IOStatus* status) {
                return detail::ostream_interface::flush(userdata, status);
            }

            static bool close([[maybe_unused]] void* userdata) {
                return true;
            }
        };

        static SDL_IOStreamInterface iface = detail::make_interface <iostream_interface>();

        SDL_IOStream* io = SDL_OpenIO(&iface, &stream);
        if (!io) {
            return make_unexpectedf(get_error());
        }

        return iostream(io);
    }

    /**
     * @brief Create an IOStream from a file
     * @param path Path to the file
     * @param mode File open mode
     * @return Expected containing the iostream, or error message
     */
    inline expected <iostream, std::string> open_file(const std::filesystem::path& path, file_mode mode) {
        SDL_IOStream* io = SDL_IOFromFile(path.string().c_str(), to_string(mode));
        if (!io) {
            return make_unexpectedf(get_error());
        }
        return iostream(io);
    }

    /**
     * @brief Create an IOStream from a file (string overload for convenience)
     * @param file Path to the file
     * @param mode File open mode
     * @return Expected containing the iostream, or error message
     */
    inline expected <iostream, std::string> open_file(const std::string& file, file_mode mode) {
        return open_file(std::filesystem::path(file), mode);
    }

    /**
     * @brief Load an entire file into memory
     * @param file Path to the file
     * @return Expected containing vector with file contents, or error message
     */
    inline expected <std::vector <uint8_t>, std::string> load_file(const std::string& file) {
        size_t datasize;
        void* data = SDL_LoadFile(file.c_str(), &datasize);
        if (!data) {
            return make_unexpectedf(get_error());
        }

        // Create vector and copy data
        std::vector <uint8_t> file_data(static_cast <uint8_t*>(data),
                                        static_cast <uint8_t*>(data) + datasize);
        SDL_free(data);
        return file_data;
    }

    /**
     * @brief Save data to a file
     * @param file Path to the file
     * @param data Data to write
     * @param data_size Size of data in bytes
     * @return Expected<void> - empty on success, error message on failure
     */
    inline expected <void, std::string> save_file(const std::string& file,
                                                  const void* data, size_t data_size) {
        if (!SDL_SaveFile(file.c_str(), data, data_size)) {
            return make_unexpectedf(get_error());
        }
        return {};
    }

    /**
     * @brief Save data to a file from a span
     * @param file Path to the file
     * @param data Span containing data to write
     * @return Expected<void> - empty on success, error message on failure
     */
    template<typename T>
    inline expected <void, std::string> save_file(const std::string& file,
                                                  std::span <const T> data) {
        return save_file(file, data.data(), data.size_bytes());
    }

    /**
     * @brief Stream output operator for io_seek_pos
     */
    SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, io_seek_pos value);

    /**
     * @brief Stream input operator for io_seek_pos
     */
    SDLPP_EXPORT std::istream& operator>>(std::istream& is, io_seek_pos& value);

    /**
     * @brief Stream output operator for io_status
     */
    SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, io_status value);

    /**
     * @brief Stream input operator for io_status
     */
    SDLPP_EXPORT std::istream& operator>>(std::istream& is, io_status& value);

} // namespace sdlpp