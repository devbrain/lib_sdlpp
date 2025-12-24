#pragma once

/**
 * @file gpu_resources.hh
 * @brief GPU resource wrappers (buffers, textures, etc.)
 */

#include <sdlpp/video/gpu.hh>

namespace sdlpp::gpu {
    /**
     * @brief GPU buffer resource
     */
    class buffer {
        public:
            buffer() = default;
            buffer(const buffer&) = delete;
            buffer& operator=(const buffer&) = delete;

            buffer(buffer&& other) noexcept
                : device_(std::exchange(other.device_, nullptr)),
                  buffer_(std::exchange(other.buffer_, nullptr)) {
            }

            buffer& operator=(buffer&& other) noexcept {
                if (this != &other) {
                    reset();
                    device_ = std::exchange(other.device_, nullptr);
                    buffer_ = std::exchange(other.buffer_, nullptr);
                }
                return *this;
            }

            ~buffer() {
                reset();
            }

            /**
             * @brief Create a buffer
             * @param device GPU device
             * @param info Creation parameters
             * @return Buffer or error
             */
            [[nodiscard]] static tl::expected <buffer, std::string> create(
                const device& device,
                const buffer_create_info& info) {
                SDL_GPUBufferCreateInfo sdl_info{
                    .usage = static_cast <SDL_GPUBufferUsageFlags>(info.usage),
                    .size = info.size,
                    .props = info.props
                };

                SDL_GPUBuffer* buf = SDL_CreateGPUBuffer(device.get(), &sdl_info);
                if (!buf) {
                    return make_unexpected(get_error());
                }

                return buffer(device.get(), buf);
            }

            /**
             * @brief Set buffer name for debugging
             * @param name Debug name
             */
            void set_name(const char* name) {
                if (buffer_ && device_) {
                    SDL_SetGPUBufferName(device_, buffer_, name);
                }
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return buffer_ != nullptr;
            }

            [[nodiscard]] SDL_GPUBuffer* get() const noexcept {
                return buffer_;
            }

            void reset() {
                if (buffer_ && device_) {
                    SDL_ReleaseGPUBuffer(device_, buffer_);
                }
                buffer_ = nullptr;
                device_ = nullptr;
            }

        private:
            buffer(SDL_GPUDevice* dev, SDL_GPUBuffer* buf) noexcept
                : device_(dev), buffer_(buf) {
            }

            SDL_GPUDevice* device_{nullptr};
            SDL_GPUBuffer* buffer_{nullptr};
    };

    /**
     * @brief Transfer buffer for CPU-GPU data transfers
     */
    class transfer_buffer {
        public:
            transfer_buffer() = default;
            transfer_buffer(const transfer_buffer&) = delete;
            transfer_buffer& operator=(const transfer_buffer&) = delete;

            transfer_buffer(transfer_buffer&& other) noexcept
                : device_(std::exchange(other.device_, nullptr)),
                  buffer_(std::exchange(other.buffer_, nullptr)) {
            }

            transfer_buffer& operator=(transfer_buffer&& other) noexcept {
                if (this != &other) {
                    reset();
                    device_ = std::exchange(other.device_, nullptr);
                    buffer_ = std::exchange(other.buffer_, nullptr);
                }
                return *this;
            }

            ~transfer_buffer() {
                reset();
            }

            /**
             * @brief Create a transfer buffer
             * @param device GPU device
             * @param info Creation parameters
             * @return Transfer buffer or error
             */
            [[nodiscard]] static tl::expected <transfer_buffer, std::string> create(
                const device& device,
                const transfer_buffer_create_info& info) {
                SDL_GPUTransferBufferCreateInfo sdl_info{
                    .usage = static_cast <SDL_GPUTransferBufferUsage>(info.usage),
                    .size = info.size,
                    .props = info.props
                };

                SDL_GPUTransferBuffer* buf = SDL_CreateGPUTransferBuffer(device.get(), &sdl_info);
                if (!buf) {
                    return make_unexpected(get_error());
                }

                return transfer_buffer(device.get(), buf);
            }

            /**
             * @brief Map buffer for CPU access
             * @param cycle Cycle to next buffer if busy
             * @return Mapped memory pointer or nullptr
             */
            [[nodiscard]] void* map(bool cycle = true) {
                if (!buffer_ || !device_) return nullptr;
                return SDL_MapGPUTransferBuffer(device_, buffer_, cycle);
            }

            /**
             * @brief Unmap buffer after CPU access
             */
            void unmap() {
                if (buffer_ && device_) {
                    SDL_UnmapGPUTransferBuffer(device_, buffer_);
                }
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return buffer_ != nullptr;
            }

            [[nodiscard]] SDL_GPUTransferBuffer* get() const noexcept {
                return buffer_;
            }

            void reset() {
                if (buffer_ && device_) {
                    SDL_ReleaseGPUTransferBuffer(device_, buffer_);
                }
                buffer_ = nullptr;
                device_ = nullptr;
            }

        private:
            transfer_buffer(SDL_GPUDevice* dev, SDL_GPUTransferBuffer* buf) noexcept
                : device_(dev), buffer_(buf) {
            }

            SDL_GPUDevice* device_{nullptr};
            SDL_GPUTransferBuffer* buffer_{nullptr};
    };

    /**
     * @brief GPU texture resource
     */
    class texture {
        public:
            texture() = default;
            texture(const texture&) = delete;
            texture& operator=(const texture&) = delete;

            texture(texture&& other) noexcept
                : device_(std::exchange(other.device_, nullptr)),
                  texture_(std::exchange(other.texture_, nullptr)) {
            }

            texture& operator=(texture&& other) noexcept {
                if (this != &other) {
                    reset();
                    device_ = std::exchange(other.device_, nullptr);
                    texture_ = std::exchange(other.texture_, nullptr);
                }
                return *this;
            }

            ~texture() {
                reset();
            }

            /**
             * @brief Create a texture
             * @param device GPU device
             * @param info Creation parameters
             * @return Texture or error
             */
            [[nodiscard]] static tl::expected <texture, std::string> create(
                const device& device,
                const texture_create_info& info) {
                SDL_GPUTextureCreateInfo sdl_info{
                    .type = static_cast <SDL_GPUTextureType>(info.type),
                    .format = static_cast <SDL_GPUTextureFormat>(info.format),
                    .usage = static_cast <SDL_GPUTextureUsageFlags>(info.usage),
                    .width = info.width,
                    .height = info.height,
                    .layer_count_or_depth = info.layer_count_or_depth,
                    .num_levels = info.num_levels,
                    .sample_count = static_cast <SDL_GPUSampleCount>(info.sample_count),
                    .props = info.props
                };

                SDL_GPUTexture* tex = SDL_CreateGPUTexture(device.get(), &sdl_info);
                if (!tex) {
                    return make_unexpected(get_error());
                }

                return texture(device.get(), tex);
            }

            /**
             * @brief Set texture name for debugging
             * @param name Debug name
             */
            void set_name(const char* name) {
                if (texture_ && device_) {
                    SDL_SetGPUTextureName(device_, texture_, name);
                }
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return texture_ != nullptr;
            }

            [[nodiscard]] SDL_GPUTexture* get() const noexcept {
                return texture_;
            }

            void reset() {
                if (texture_ && device_) {
                    SDL_ReleaseGPUTexture(device_, texture_);
                }
                texture_ = nullptr;
                device_ = nullptr;
            }

        private:
            texture(SDL_GPUDevice* dev, SDL_GPUTexture* tex) noexcept
                : device_(dev), texture_(tex) {
            }

            SDL_GPUDevice* device_{nullptr};
            SDL_GPUTexture* texture_{nullptr};
    };

    /**
     * @brief Texture sampler
     */
    class sampler {
        public:
            sampler() = default;
            sampler(const sampler&) = delete;
            sampler& operator=(const sampler&) = delete;

            sampler(sampler&& other) noexcept
                : device_(std::exchange(other.device_, nullptr)),
                  sampler_(std::exchange(other.sampler_, nullptr)) {
            }

            sampler& operator=(sampler&& other) noexcept {
                if (this != &other) {
                    reset();
                    device_ = std::exchange(other.device_, nullptr);
                    sampler_ = std::exchange(other.sampler_, nullptr);
                }
                return *this;
            }

            ~sampler() {
                reset();
            }

            /**
             * @brief Create a sampler
             * @param device GPU device
             * @param info Creation parameters
             * @return Sampler or error
             */
            [[nodiscard]] static tl::expected <sampler, std::string> create(
                const device& device,
                const sampler_create_info& info) {
                SDL_GPUSamplerCreateInfo sdl_info{
                    .min_filter = static_cast <SDL_GPUFilter>(info.min_filter),
                    .mag_filter = static_cast <SDL_GPUFilter>(info.mag_filter),
                    .mipmap_mode = static_cast <SDL_GPUSamplerMipmapMode>(info.mipmap_mode),
                    .address_mode_u = static_cast <SDL_GPUSamplerAddressMode>(info.address_mode_u),
                    .address_mode_v = static_cast <SDL_GPUSamplerAddressMode>(info.address_mode_v),
                    .address_mode_w = static_cast <SDL_GPUSamplerAddressMode>(info.address_mode_w),
                    .mip_lod_bias = info.mip_lod_bias,
                    .max_anisotropy = info.max_anisotropy,
                    .compare_op = static_cast <SDL_GPUCompareOp>(info.compare_op),
                    .min_lod = info.min_lod,
                    .max_lod = info.max_lod,
                    .enable_anisotropy = info.enable_anisotropy,
                    .enable_compare = info.enable_compare,
                    .padding1 = 0,
                    .padding2 = 0,
                    .props = info.props
                };

                SDL_GPUSampler* samp = SDL_CreateGPUSampler(device.get(), &sdl_info);
                if (!samp) {
                    return make_unexpected(get_error());
                }

                return sampler(device.get(), samp);
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return sampler_ != nullptr;
            }

            [[nodiscard]] SDL_GPUSampler* get() const noexcept {
                return sampler_;
            }

            void reset() {
                if (sampler_ && device_) {
                    SDL_ReleaseGPUSampler(device_, sampler_);
                }
                sampler_ = nullptr;
                device_ = nullptr;
            }

        private:
            sampler(SDL_GPUDevice* dev, SDL_GPUSampler* samp) noexcept
                : device_(dev), sampler_(samp) {
            }

            SDL_GPUDevice* device_{nullptr};
            SDL_GPUSampler* sampler_{nullptr};
    };

    /**
     * @brief Shader module
     */
    class shader {
        public:
            shader() = default;
            shader(const shader&) = delete;
            shader& operator=(const shader&) = delete;

            shader(shader&& other) noexcept
                : device_(std::exchange(other.device_, nullptr)),
                  shader_(std::exchange(other.shader_, nullptr)) {
            }

            shader& operator=(shader&& other) noexcept {
                if (this != &other) {
                    reset();
                    device_ = std::exchange(other.device_, nullptr);
                    shader_ = std::exchange(other.shader_, nullptr);
                }
                return *this;
            }

            ~shader() {
                reset();
            }

            /**
             * @brief Create a shader
             * @param device GPU device
             * @param info Creation parameters
             * @return Shader or error
             */
            [[nodiscard]] static tl::expected <shader, std::string> create(
                const device& device,
                const shader_create_info& info) {
                SDL_GPUShaderCreateInfo sdl_info{
                    .code_size = info.code.size(),
                    .code = info.code.data(),
                    .entrypoint = info.entrypoint,
                    .format = static_cast <SDL_GPUShaderFormat>(info.format),
                    .stage = static_cast <SDL_GPUShaderStage>(info.stage),
                    .num_samplers = info.num_samplers,
                    .num_storage_textures = info.num_storage_textures,
                    .num_storage_buffers = info.num_storage_buffers,
                    .num_uniform_buffers = info.num_uniform_buffers,
                    .props = info.props
                };

                SDL_GPUShader* shad = SDL_CreateGPUShader(device.get(), &sdl_info);
                if (!shad) {
                    return make_unexpected(get_error());
                }

                return shader(device.get(), shad);
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return shader_ != nullptr;
            }

            [[nodiscard]] SDL_GPUShader* get() const noexcept {
                return shader_;
            }

            void reset() {
                if (shader_ && device_) {
                    SDL_ReleaseGPUShader(device_, shader_);
                }
                shader_ = nullptr;
                device_ = nullptr;
            }

        private:
            shader(SDL_GPUDevice* dev, SDL_GPUShader* shad) noexcept
                : device_(dev), shader_(shad) {
            }

            SDL_GPUDevice* device_{nullptr};
            SDL_GPUShader* shader_{nullptr};
    };

    /**
     * @brief Graphics pipeline
     */
    class graphics_pipeline {
        public:
            graphics_pipeline() = default;
            graphics_pipeline(const graphics_pipeline&) = delete;
            graphics_pipeline& operator=(const graphics_pipeline&) = delete;

            graphics_pipeline(graphics_pipeline&& other) noexcept
                : device_(std::exchange(other.device_, nullptr)),
                  pipeline_(std::exchange(other.pipeline_, nullptr)) {
            }

            graphics_pipeline& operator=(graphics_pipeline&& other) noexcept {
                if (this != &other) {
                    reset();
                    device_ = std::exchange(other.device_, nullptr);
                    pipeline_ = std::exchange(other.pipeline_, nullptr);
                }
                return *this;
            }

            ~graphics_pipeline() {
                reset();
            }

            /**
             * @brief Create a graphics pipeline
             * @param device GPU device
             * @param info Creation parameters
             * @return Pipeline or error
             */
            [[nodiscard]] static tl::expected <graphics_pipeline, std::string> create(
                const device& device,
                const graphics_pipeline_create_info& info) {
                // Convert color target descriptions
                std::vector <SDL_GPUColorTargetDescription> sdl_color_targets;
                sdl_color_targets.reserve(info.target_formats.size());
                for (size_t i = 0; i < info.target_formats.size(); ++i) {
                    SDL_GPUColorTargetDescription desc{
                        .format = static_cast <SDL_GPUTextureFormat>(info.target_formats[i]),
                        .blend_state = i < info.blend_states.size()
                                           ? info.blend_states[i].to_sdl()
                                           : SDL_GPUColorTargetBlendState{}
                    };
                    sdl_color_targets.push_back(desc);
                }

                // Convert vertex input state
                auto vertex_input = info.vertex_input_state.to_sdl();

                // Convert other states
                auto rasterizer = info.rasterizer_state.to_sdl();
                auto multisample = info.multisample_state.to_sdl();
                auto depth_stencil = info.depth_stencil_state.to_sdl();

                // Convert formats
                std::vector <SDL_GPUTextureFormat> sdl_formats;
                sdl_formats.reserve(info.target_formats.size());
                for (auto fmt : info.target_formats) {
                    sdl_formats.push_back(static_cast <SDL_GPUTextureFormat>(fmt));
                }

                SDL_GPUGraphicsPipelineTargetInfo target_info{
                    .color_target_descriptions = sdl_color_targets.data(),
                    .num_color_targets = static_cast <Uint32>(sdl_color_targets.size()),
                    .depth_stencil_format = static_cast <SDL_GPUTextureFormat>(info.depth_stencil_format),
                    .has_depth_stencil_target = info.has_depth_stencil_target,
                    .padding1 = 0,
                    .padding2 = 0,
                    .padding3 = 0
                };

                SDL_GPUGraphicsPipelineCreateInfo sdl_info{
                    .vertex_shader = info.vertex_shader ? info.vertex_shader->get() : nullptr,
                    .fragment_shader = info.fragment_shader ? info.fragment_shader->get() : nullptr,
                    .vertex_input_state = vertex_input,
                    .primitive_type = static_cast <SDL_GPUPrimitiveType>(info.primitive_type),
                    .rasterizer_state = rasterizer,
                    .multisample_state = multisample,
                    .depth_stencil_state = depth_stencil,
                    .target_info = target_info,
                    .props = info.props
                };

                SDL_GPUGraphicsPipeline* pipe = SDL_CreateGPUGraphicsPipeline(device.get(), &sdl_info);
                if (!pipe) {
                    return make_unexpected(get_error());
                }

                return graphics_pipeline(device.get(), pipe);
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return pipeline_ != nullptr;
            }

            [[nodiscard]] SDL_GPUGraphicsPipeline* get() const noexcept {
                return pipeline_;
            }

            void reset() {
                if (pipeline_ && device_) {
                    SDL_ReleaseGPUGraphicsPipeline(device_, pipeline_);
                }
                pipeline_ = nullptr;
                device_ = nullptr;
            }

        private:
            graphics_pipeline(SDL_GPUDevice* dev, SDL_GPUGraphicsPipeline* pipe) noexcept
                : device_(dev), pipeline_(pipe) {
            }

            SDL_GPUDevice* device_{nullptr};
            SDL_GPUGraphicsPipeline* pipeline_{nullptr};
    };

    /**
     * @brief Compute pipeline
     */
    class compute_pipeline {
        public:
            compute_pipeline() = default;
            compute_pipeline(const compute_pipeline&) = delete;
            compute_pipeline& operator=(const compute_pipeline&) = delete;

            compute_pipeline(compute_pipeline&& other) noexcept
                : device_(std::exchange(other.device_, nullptr)),
                  pipeline_(std::exchange(other.pipeline_, nullptr)) {
            }

            compute_pipeline& operator=(compute_pipeline&& other) noexcept {
                if (this != &other) {
                    reset();
                    device_ = std::exchange(other.device_, nullptr);
                    pipeline_ = std::exchange(other.pipeline_, nullptr);
                }
                return *this;
            }

            ~compute_pipeline() {
                reset();
            }

            /**
             * @brief Create a compute pipeline
             * @param device GPU device
             * @param shader_info Shader creation info for compute shader
             * @param thread_count_x X dimension thread count
             * @param thread_count_y Y dimension thread count
             * @param thread_count_z Z dimension thread count
             * @param props Additional properties
             * @return Pipeline or error
             */
            [[nodiscard]] static tl::expected <compute_pipeline, std::string> create(
                const device& device,
                const shader_create_info& shader_info,
                Uint32 thread_count_x,
                Uint32 thread_count_y = 1,
                Uint32 thread_count_z = 1,
                SDL_PropertiesID props = 0) {
                SDL_GPUComputePipelineCreateInfo sdl_info{
                    .code_size = shader_info.code.size(),
                    .code = shader_info.code.data(),
                    .entrypoint = shader_info.entrypoint,
                    .format = static_cast <SDL_GPUShaderFormat>(shader_info.format),
                    .num_samplers = shader_info.num_samplers,
                    .num_readonly_storage_textures = 0, // TODO: Need to separate read-only vs read-write
                    .num_readonly_storage_buffers = 0,
                    .num_readwrite_storage_textures = shader_info.num_storage_textures,
                    .num_readwrite_storage_buffers = shader_info.num_storage_buffers,
                    .num_uniform_buffers = shader_info.num_uniform_buffers,
                    .threadcount_x = thread_count_x,
                    .threadcount_y = thread_count_y,
                    .threadcount_z = thread_count_z,
                    .props = props
                };

                SDL_GPUComputePipeline* pipe = SDL_CreateGPUComputePipeline(device.get(), &sdl_info);
                if (!pipe) {
                    return make_unexpected(get_error());
                }

                return compute_pipeline(device.get(), pipe);
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return pipeline_ != nullptr;
            }

            [[nodiscard]] SDL_GPUComputePipeline* get() const noexcept {
                return pipeline_;
            }

            void reset() {
                if (pipeline_ && device_) {
                    SDL_ReleaseGPUComputePipeline(device_, pipeline_);
                }
                pipeline_ = nullptr;
                device_ = nullptr;
            }

        private:
            compute_pipeline(SDL_GPUDevice* dev, SDL_GPUComputePipeline* pipe) noexcept
                : device_(dev), pipeline_(pipe) {
            }

            SDL_GPUDevice* device_{nullptr};
            SDL_GPUComputePipeline* pipeline_{nullptr};
    };

    /**
     * @brief GPU fence for synchronization
     */
    class fence {
        public:
            fence() = default;
            fence(const fence&) = delete;
            fence& operator=(const fence&) = delete;

            fence(fence&& other) noexcept
                : device_(std::exchange(other.device_, nullptr)),
                  fence_(std::exchange(other.fence_, nullptr)) {
            }

            fence& operator=(fence&& other) noexcept {
                if (this != &other) {
                    reset();
                    device_ = std::exchange(other.device_, nullptr);
                    fence_ = std::exchange(other.fence_, nullptr);
                }
                return *this;
            }

            ~fence() {
                reset();
            }

            /**
             * @brief Query fence status
             * @return true if signaled, false otherwise
             */
            [[nodiscard]] bool is_signaled() const {
                if (!fence_ || !device_) return false;
                return SDL_QueryGPUFence(device_, fence_);
            }

            /**
             * @brief Wait for fence to be signaled
             * @return true if signaled
             */
            [[nodiscard]] bool wait() const {
                if (!fence_ || !device_) return false;
                SDL_GPUFence* fences[] = {fence_};
                return SDL_WaitForGPUFences(device_, true, fences, 1);
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return fence_ != nullptr;
            }

            [[nodiscard]] SDL_GPUFence* get() const noexcept {
                return fence_;
            }

            void reset() {
                if (fence_ && device_) {
                    SDL_ReleaseGPUFence(device_, fence_);
                }
                fence_ = nullptr;
                device_ = nullptr;
            }

            // Private constructor accessible to command_buffer
            friend class command_buffer;

        private:
            fence(SDL_GPUDevice* dev, SDL_GPUFence* f) noexcept
                : device_(dev), fence_(f) {
            }

            SDL_GPUDevice* device_{nullptr};
            SDL_GPUFence* fence_{nullptr};
    };
} // namespace sdlpp::gpu
