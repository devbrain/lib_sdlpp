#pragma once

/**
 * @file camera.hh
 * @brief Camera capture functionality
 * 
 * This header provides C++ wrappers around SDL3's camera API,
 * offering cross-platform access to camera devices for video capture.
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <sdlpp/core/error.hh>
#include <sdlpp/detail/expected.hh>
#include <sdlpp/detail/pointer.hh>
#include <sdlpp/video/surface.hh>
#include <sdlpp/video/pixels.hh>

#include <string>
#include <vector>
#include <cstdint>
#include <chrono>
#include <span>

namespace sdlpp {
    /**
     * @brief Camera device ID type
     */
    using camera_id = SDL_CameraID;

    /**
     * @brief Camera position enum
     */
    enum class camera_position : int {
        unknown = SDL_CAMERA_POSITION_UNKNOWN,
        front_facing = SDL_CAMERA_POSITION_FRONT_FACING,
        back_facing = SDL_CAMERA_POSITION_BACK_FACING
    };

    /**
     * @brief Camera specification
     *
     * Describes the format and framerate for camera capture.
     */
    struct camera_spec {
        pixel_format_enum format = pixel_format_enum::unknown; ///< Pixel format
        size_t width = 0; ///< Frame width
        size_t height = 0; ///< Frame height
        int framerate_numerator = 30; ///< Framerate numerator
        int framerate_denominator = 1; ///< Framerate denominator

        /**
         * @brief Get framerate as floating point
         * @return Framerate in frames per second
         */
        [[nodiscard]] float get_framerate() const noexcept {
            return framerate_denominator > 0
                       ? static_cast <float>(framerate_numerator) / static_cast<float>(framerate_denominator)
                       : 0.0f;
        }

        /**
         * @brief Convert to SDL camera spec
         */
        [[nodiscard]] SDL_CameraSpec to_sdl() const noexcept {
            SDL_CameraSpec spec;
            spec.format = static_cast <SDL_PixelFormat>(format);
            spec.width = static_cast<int>(width);
            spec.height = static_cast<int>(height);
            spec.framerate_numerator = framerate_numerator;
            spec.framerate_denominator = framerate_denominator;
            return spec;
        }

        /**
         * @brief Create from SDL camera spec
         */
        static camera_spec from_sdl(const SDL_CameraSpec& spec) noexcept {
            return {
                .format = static_cast <pixel_format_enum>(spec.format),
                .width = spec.width > 0 ? static_cast<size_t>(spec.width) : 0,
                .height = spec.height > 0 ? static_cast<size_t>(spec.height) : 0,
                .framerate_numerator = spec.framerate_numerator,
                .framerate_denominator = spec.framerate_denominator
            };
        }
    };

    /**
     * @brief Smart pointer type for SDL_Camera with automatic cleanup
     */
    using camera_ptr = pointer <SDL_Camera, SDL_CloseCamera>;

    /**
     * @brief Get list of available camera devices
     *
     * @return Vector of camera device IDs
     *
     * Example:
     * @code
     * auto cameras = sdlpp::get_cameras();
     * std::cout << "Found " << cameras.size() << " camera(s)\n";
     * @endcode
     */
    [[nodiscard]] inline std::vector <camera_id> get_cameras() {
        int count = 0;
        SDL_CameraID* cameras = SDL_GetCameras(&count);
        if (!cameras || count <= 0) {
            return {};
        }

        std::vector <camera_id> camera_list(cameras, cameras + count);
        SDL_free(cameras);
        return camera_list;
    }

    /**
     * @brief Get the human-readable name of a camera
     *
     * @param instance_id The camera instance ID
     * @return Camera name, or empty string on error
     */
    [[nodiscard]] inline std::string get_camera_name(camera_id instance_id) {
        const char* name = SDL_GetCameraName(instance_id);
        return name ? name : "";
    }

    /**
     * @brief Get the position of a camera (front/back)
     *
     * @param instance_id The camera instance ID
     * @return Camera position
     */
    [[nodiscard]] inline camera_position get_camera_position(camera_id instance_id) {
        return static_cast <camera_position>(SDL_GetCameraPosition(instance_id));
    }

    /**
     * @brief Camera permission state (SDL 3.4.0+)
     *
     * Represents the state of camera access permissions on platforms
     * that require explicit user permission (iOS, Android, etc.).
     */
    enum class camera_permission_state : int {
        unknown = -1,   ///< Permission state not yet determined
        denied = 0,     ///< Permission denied by user or system
        granted = 1     ///< Permission granted
    };

    /**
     * @brief Get supported formats for a camera
     *
     * @param instance_id The camera instance ID
     * @return Vector of supported camera specifications
     */
    [[nodiscard]] inline std::vector <camera_spec> get_camera_supported_formats(camera_id instance_id) {
        int count = 0;
        SDL_CameraSpec** specs = SDL_GetCameraSupportedFormats(instance_id, &count);
        if (!specs || count <= 0) {
            return {};
        }

        std::vector <camera_spec> formats;
        formats.reserve(static_cast<size_t>(count));

        for (int i = 0; i < count; ++i) {
            if (specs[i]) {
                formats.push_back(camera_spec::from_sdl(*specs[i]));
            }
        }

        SDL_free(specs);
        return formats;
    }

    /**
     * @brief RAII wrapper for SDL_Camera
     *
     * This class provides a safe, RAII-managed interface to SDL's camera functionality.
     * The camera is automatically closed when the object goes out of scope.
     */
    class camera {
        private:
            camera_ptr ptr;

        public:
            /**
             * @brief Default constructor - creates an empty camera
             */
            camera() = default;

            /**
             * @brief Construct from existing SDL_Camera pointer
             * @param cam Existing SDL_Camera pointer (takes ownership)
             */
            explicit camera(SDL_Camera* cam)
                : ptr(cam) {
            }

            /**
             * @brief Move constructor
             */
            camera(camera&&) = default;

            /**
             * @brief Move assignment operator
             */
            camera& operator=(camera&&) = default;

            // Delete copy operations - cameras are move-only
            camera(const camera&) = delete;
            camera& operator=(const camera&) = delete;

            /**
             * @brief Check if the camera is valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return ptr != nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the underlying SDL_Camera pointer
             */
            [[nodiscard]] SDL_Camera* get() const noexcept { return ptr.get(); }

            /**
             * @brief Get the camera permission state (SDL 3.4.0+)
             *
             * Returns the current state of camera permissions. This is useful
             * on platforms like iOS and Android where camera access requires
             * explicit user permission.
             *
             * @return The current permission state
             */
            [[nodiscard]] camera_permission_state get_permission_state() const {
                if (!ptr) {
                    return camera_permission_state::unknown;
                }
                return static_cast<camera_permission_state>(SDL_GetCameraPermissionState(ptr.get()));
            }

            /**
             * @brief Open a camera device
             *
             * @param instance_id The camera instance ID
             * @param spec Optional desired specification (nullptr for default)
             * @return Expected containing camera, or error message
             *
             * Example:
             * @code
             * auto cameras = sdlpp::get_cameras();
             * if (!cameras.empty()) {
             *     auto cam = sdlpp::camera::open(cameras[0]);
             *     if (cam) {
             *         // Use camera
             *     }
             * }
             * @endcode
             */
            static expected <camera, std::string> open(camera_id instance_id,
                                                       const camera_spec* spec = nullptr) {
                SDL_CameraSpec sdl_spec_val;
                SDL_CameraSpec* sdl_spec_ptr = nullptr;

                if (spec) {
                    sdl_spec_val = spec->to_sdl();
                    sdl_spec_ptr = &sdl_spec_val;
                }

                SDL_Camera* cam = SDL_OpenCamera(instance_id, sdl_spec_ptr);
                if (!cam) {
                    return make_unexpectedf(get_error());
                }
                return camera(cam);
            }

            /**
             * @brief Get the instance ID of this camera
             *
             * @return Camera instance ID, or 0 if invalid
             */
            [[nodiscard]] camera_id get_id() const noexcept {
                return ptr ? SDL_GetCameraID(ptr.get()) : 0;
            }

            /**
             * @brief Get the name of this camera
             *
             * @return Camera name, or empty string if invalid
             */
            [[nodiscard]] std::string get_name() const {
                if (!ptr) return "";
                const char* name = SDL_GetCameraName(get_id());
                return name ? name : "";
            }

            /**
             * @brief Get the position of this camera
             *
             * @return Camera position
             */
            [[nodiscard]] camera_position get_position() const noexcept {
                return ptr
                           ? static_cast <camera_position>(SDL_GetCameraPosition(get_id()))
                           : camera_position::unknown;
            }

            /**
             * @brief Get the current format of the camera
             *
             * @return Expected containing camera spec, or error message
             */
            [[nodiscard]] expected <camera_spec, std::string> get_format() const {
                if (!ptr) {
                    return make_unexpectedf("Invalid camera");
                }

                SDL_CameraSpec spec;
                if (!SDL_GetCameraFormat(ptr.get(), &spec)) {
                    return make_unexpectedf(get_error());
                }

                return camera_spec::from_sdl(spec);
            }

            /**
             * @brief Get supported formats for this camera
             *
             * @return Vector of supported specifications
             */
            [[nodiscard]] std::vector <camera_spec> get_supported_formats() const {
                return ptr ? get_camera_supported_formats(get_id()) : std::vector <camera_spec>{};
            }

            /**
             * @brief Check if a format is supported
             *
             * @param spec The format to check
             * @return true if supported
             */
            [[nodiscard]] bool is_format_supported(const camera_spec& spec) const {
                auto formats = get_supported_formats();
                for (const auto& fmt : formats) {
                    if (fmt.format == spec.format &&
                        fmt.width == spec.width &&
                        fmt.height == spec.height) {
                        return true;
                    }
                }
                return false;
            }

            /**
             * @brief Acquire a frame from the camera
             *
             * The surface remains valid until the next call to acquire_frame()
             * or until the camera is closed.
             *
             * @param timestampNS Output parameter for frame timestamp in nanoseconds
             * @return Non-owning pointer to surface, or nullptr if no frame available
             *
             * Example:
             * @code
             * Uint64 timestamp;
             * SDL_Surface* frame = camera.acquire_frame(&timestamp);
             * if (frame) {
             *     // Process frame
             *     camera.release_frame(frame);
             * }
             * @endcode
             */
            [[nodiscard]] SDL_Surface* acquire_frame(Uint64* timestampNS = nullptr) const {
                return ptr ? SDL_AcquireCameraFrame(ptr.get(), timestampNS) : nullptr;
            }

            /**
             * @brief Release a frame acquired from the camera
             *
             * @param frame The frame to release
             * @return true on success
             */
            bool release_frame(SDL_Surface* frame) const {
                if (ptr && frame) {
                    SDL_ReleaseCameraFrame(ptr.get(), frame);
                    return true;
                }
                return false;
            }

            // Note: SDL_GetCameraPermission was removed in SDL3
            // Camera permission is now handled at the system level
    };

    /**
     * @brief RAII helper for camera frames
     *
     * Automatically releases a camera frame when going out of scope.
     */
    class camera_frame {
        private:
            const camera* cam = nullptr;
            SDL_Surface* frame = nullptr;
            Uint64 timestamp_ns = 0;

        public:
            camera_frame() = default;

            /**
             * @brief Acquire a frame from camera
             *
             * @param camera The camera to acquire from
             */
            explicit camera_frame(const camera& camera)
                : cam(&camera) {
                frame = cam->acquire_frame(&timestamp_ns);
            }

            ~camera_frame() {
                if (cam && frame) {
                    cam->release_frame(frame);
                }
            }

            // Move only
            camera_frame(camera_frame&& other) noexcept
                : cam(other.cam), frame(other.frame), timestamp_ns(other.timestamp_ns) {
                other.cam = nullptr;
                other.frame = nullptr;
            }

            camera_frame& operator=(camera_frame&& other) noexcept {
                if (this != &other) {
                    if (cam && frame) {
                        cam->release_frame(frame);
                    }
                    cam = other.cam;
                    frame = other.frame;
                    timestamp_ns = other.timestamp_ns;
                    other.cam = nullptr;
                    other.frame = nullptr;
                }
                return *this;
            }

            // Delete copy
            camera_frame(const camera_frame&) = delete;
            camera_frame& operator=(const camera_frame&) = delete;

            /**
             * @brief Check if frame is valid
             */
            [[nodiscard]] bool is_valid() const noexcept { return frame != nullptr; }
            [[nodiscard]] explicit operator bool() const noexcept { return is_valid(); }

            /**
             * @brief Get the frame surface
             */
            [[nodiscard]] SDL_Surface* get() const noexcept { return frame; }
            [[nodiscard]] SDL_Surface* operator->() const noexcept { return frame; }
            [[nodiscard]] SDL_Surface& operator*() const noexcept { return *frame; }

            /**
             * @brief Get frame timestamp in nanoseconds
             */
            [[nodiscard]] Uint64 get_timestamp_ns() const noexcept { return timestamp_ns; }

            /**
             * @brief Get frame timestamp as std::chrono time_point
             */
            [[nodiscard]] std::chrono::nanoseconds get_timestamp() const noexcept {
                return std::chrono::nanoseconds(timestamp_ns);
            }

            /**
             * @brief Manually release the frame
             */
            void release() {
                if (cam && frame) {
                    cam->release_frame(frame);
                    frame = nullptr;
                }
            }
    };
} // namespace sdlpp


// Stream operators for enums
#include <iosfwd>

namespace sdlpp {
/**
 * @brief Stream output operator for camera_position
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, camera_position value);

/**
 * @brief Stream input operator for camera_position
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, camera_position& value);

}