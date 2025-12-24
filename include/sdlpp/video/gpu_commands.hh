#pragma once

/**
 * @file gpu_commands.hh
 * @brief GPU command buffer and pass wrappers (updated for current SDL3 API)
 */

#include <sdlpp/video/gpu.hh>
#include <sdlpp/video/gpu_resources.hh>

namespace sdlpp::gpu {
    // Forward declarations
    class render_pass;
    class compute_pass;
    class copy_pass;

    /**
     * @brief Viewport region
     */
    struct viewport {
        float x{0.0f}; ///< X position
        float y{0.0f}; ///< Y position
        float w{0.0f}; ///< Width
        float h{0.0f}; ///< Height
        float min_depth{0.0f}; ///< Minimum depth
        float max_depth{1.0f}; ///< Maximum depth

        [[nodiscard]] SDL_GPUViewport to_sdl() const noexcept {
            return SDL_GPUViewport{
                .x = x,
                .y = y,
                .w = w,
                .h = h,
                .min_depth = min_depth,
                .max_depth = max_depth
            };
        }
    };

    /**
     * @brief Texture region for operations
     */
    struct texture_region {
        class texture* texture{nullptr}; ///< Texture resource
        Uint32 mip_level{0}; ///< Mipmap level
        Uint32 layer{0}; ///< Array layer
        Uint32 x{0}; ///< X offset
        Uint32 y{0}; ///< Y offset
        Uint32 z{0}; ///< Z offset (for 3D textures)
        Uint32 w{0}; ///< Width
        Uint32 h{0}; ///< Height
        Uint32 d{1}; ///< Depth

        [[nodiscard]] SDL_GPUTextureRegion to_sdl() const noexcept {
            return SDL_GPUTextureRegion{
                .texture = texture ? texture->get() : nullptr,
                .mip_level = mip_level,
                .layer = layer,
                .x = x,
                .y = y,
                .z = z,
                .w = w,
                .h = h,
                .d = d
            };
        }
    };

    /**
     * @brief Texture transfer info for upload/download
     */
    struct texture_transfer_info {
        transfer_buffer* buffer{nullptr}; ///< Transfer buffer (renamed to avoid conflict)
        Uint32 offset{0}; ///< Byte offset in buffer
        Uint32 pixels_per_row{0}; ///< Pixels per row
        Uint32 rows_per_layer{0}; ///< Rows per layer

        [[nodiscard]] SDL_GPUTextureTransferInfo to_sdl() const noexcept {
            return SDL_GPUTextureTransferInfo{
                .transfer_buffer = buffer ? buffer->get() : nullptr,
                .offset = offset,
                .pixels_per_row = pixels_per_row,
                .rows_per_layer = rows_per_layer
            };
        }
    };

    /**
     * @brief Color attachment info for render pass
     */
    struct color_attachment_info {
        class texture* tex{nullptr}; ///< Render target texture (renamed to avoid conflict)
        Uint32 mip_level{0}; ///< Mipmap level
        Uint32 layer_or_depth_plane{0}; ///< Layer or depth slice
        fcolor clear_color{0.0f, 0.0f, 0.0f, 1.0f}; ///< Clear color
        enum load_op load{load_op::clear}; ///< Load operation
        enum store_op store{store_op::store}; ///< Store operation
        class texture* resolve_texture{nullptr}; ///< MSAA resolve target
        Uint32 resolve_mip_level{0}; ///< Resolve mip level
        Uint32 resolve_layer{0}; ///< Resolve layer
        bool cycle{false}; ///< Cycle texture if busy
        bool cycle_resolve_texture{false}; ///< Cycle resolve texture

        [[nodiscard]] SDL_GPUColorTargetInfo to_sdl() const noexcept {
            return SDL_GPUColorTargetInfo{
                .texture = tex ? tex->get() : nullptr,
                .mip_level = mip_level,
                .layer_or_depth_plane = layer_or_depth_plane,
                .clear_color = clear_color.to_sdl(),
                .load_op = static_cast <SDL_GPULoadOp>(load),
                .store_op = static_cast <SDL_GPUStoreOp>(store),
                .resolve_texture = resolve_texture ? resolve_texture->get() : nullptr,
                .resolve_mip_level = resolve_mip_level,
                .resolve_layer = resolve_layer,
                .cycle = cycle,
                .cycle_resolve_texture = cycle_resolve_texture,
                .padding1 = 0,
                .padding2 = 0
            };
        }
    };

    /**
     * @brief Depth/stencil attachment info
     */
    struct depth_stencil_attachment_info {
        class texture* tex{nullptr}; ///< Depth/stencil texture (renamed to avoid conflict)
        float clear_depth{1.0f}; ///< Clear depth value
        Uint8 clear_stencil{0}; ///< Clear stencil value
        Uint8 mip_level{0}; ///< Mip level to use as depth stencil target
        Uint8 layer{0}; ///< Layer index to use as depth stencil target
        enum load_op load{load_op::clear}; ///< Depth load operation
        enum store_op store{store_op::store}; ///< Depth store operation
        enum load_op stencil_load{load_op::clear}; ///< Stencil load operation
        enum store_op stencil_store{store_op::store}; ///< Stencil store operation
        bool cycle{false}; ///< Cycle texture if busy

        [[nodiscard]] SDL_GPUDepthStencilTargetInfo to_sdl() const noexcept {
            return SDL_GPUDepthStencilTargetInfo{
                .texture = tex ? tex->get() : nullptr,
                .clear_depth = clear_depth,
                .load_op = static_cast <SDL_GPULoadOp>(load),
                .store_op = static_cast <SDL_GPUStoreOp>(store),
                .stencil_load_op = static_cast <SDL_GPULoadOp>(stencil_load),
                .stencil_store_op = static_cast <SDL_GPUStoreOp>(stencil_store),
                .cycle = cycle,
                .clear_stencil = clear_stencil,
                .mip_level = mip_level,
                .layer = layer
            };
        }
    };

    /**
     * @brief Buffer binding for vertex/index buffers
     */
    struct buffer_binding {
        class buffer* buf{nullptr}; ///< Buffer resource (renamed to avoid conflict)
        Uint32 offset{0}; ///< Byte offset

        [[nodiscard]] SDL_GPUBufferBinding to_sdl() const noexcept {
            return SDL_GPUBufferBinding{
                .buffer = buf ? buf->get() : nullptr,
                .offset = offset
            };
        }
    };

    /**
     * @brief Transfer buffer location
     */
    struct transfer_buffer_location {
        class transfer_buffer* buffer{nullptr}; ///< Transfer buffer (renamed to avoid conflict)
        Uint32 offset{0}; ///< Byte offset

        [[nodiscard]] SDL_GPUTransferBufferLocation to_sdl() const noexcept {
            return SDL_GPUTransferBufferLocation{
                .transfer_buffer = buffer ? buffer->get() : nullptr,
                .offset = offset
            };
        }
    };

    /**
     * @brief Storage texture read/write binding
     */
    struct storage_texture_read_write_binding {
        class texture* tex{nullptr}; ///< Texture resource (renamed to avoid conflict)
        Uint32 mip_level{0}; ///< Mipmap level
        Uint32 layer{0}; ///< Array layer
        bool cycle{false}; ///< Cycle if busy

        [[nodiscard]] SDL_GPUStorageTextureReadWriteBinding to_sdl() const noexcept {
            return SDL_GPUStorageTextureReadWriteBinding{
                .texture = tex ? tex->get() : nullptr,
                .mip_level = mip_level,
                .layer = layer,
                .cycle = cycle,
                .padding1 = 0,
                .padding2 = 0,
                .padding3 = 0
            };
        }
    };

    /**
     * @brief Storage buffer read/write binding
     */
    struct storage_buffer_read_write_binding {
        buffer* buff{nullptr}; ///< Buffer resource (renamed to avoid name conflict)
        bool cycle{false}; ///< Cycle if busy

        [[nodiscard]] SDL_GPUStorageBufferReadWriteBinding to_sdl() const noexcept {
            return SDL_GPUStorageBufferReadWriteBinding{
                .buffer = buff ? buff->get() : nullptr,
                .cycle = cycle,
                .padding1 = 0,
                .padding2 = 0,
                .padding3 = 0
            };
        }
    };

    /**
     * @brief Command buffer for GPU operations
     */
    class command_buffer {
        public:
            command_buffer() = default;
            command_buffer(const command_buffer&) = delete;
            command_buffer& operator=(const command_buffer&) = delete;

            command_buffer(command_buffer&& other) noexcept
                : device_(std::exchange(other.device_, nullptr)),
                  cmd_buffer_(std::exchange(other.cmd_buffer_, nullptr)) {
            }

            command_buffer& operator=(command_buffer&& other) noexcept {
                if (this != &other) {
                    device_ = std::exchange(other.device_, nullptr);
                    cmd_buffer_ = std::exchange(other.cmd_buffer_, nullptr);
                }
                return *this;
            }

            /**
             * @brief Acquire a command buffer from device
             * @param device GPU device
             * @return Command buffer or error
             */
            [[nodiscard]] static tl::expected <command_buffer, std::string> acquire(
                const device& device) {
                SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device.get());
                if (!cmd) {
                    return tl::unexpected(get_error());
                }

                return command_buffer(device.get(), cmd);
            }

            /**
             * @brief Begin a render pass
             * @param color_attachments Color render targets
             * @param depth_stencil Optional depth/stencil target
             * @return Render pass handle
             */
            [[nodiscard]] render_pass begin_render_pass(
                std::span <const color_attachment_info> color_attachments,
                const depth_stencil_attachment_info* depth_stencil = nullptr);

            /**
             * @brief Begin a compute pass
             * @param read_write_textures Storage textures for read/write
             * @param read_write_buffers Storage buffers for read/write
             * @return Compute pass handle
             */
            [[nodiscard]] compute_pass begin_compute_pass(
                std::span <const storage_texture_read_write_binding> read_write_textures = {},
                std::span <const storage_buffer_read_write_binding> read_write_buffers = {});

            /**
             * @brief Begin a copy pass
             * @return Copy pass handle
             */
            [[nodiscard]] copy_pass begin_copy_pass();

            /**
             * @brief Submit command buffer for execution
             * @return Success or error
             */
            [[nodiscard]] tl::expected <void, std::string> submit() {
                if (!cmd_buffer_) {
                    return tl::unexpected("Invalid command buffer");
                }

                if (!SDL_SubmitGPUCommandBuffer(cmd_buffer_)) {
                    return tl::unexpected(get_error());
                }

                cmd_buffer_ = nullptr; // Command buffer consumed
                return {};
            }

            /**
             * @brief Submit command buffer and acquire fence
             * @return Fence or error
             */
            [[nodiscard]] tl::expected <fence, std::string> submit_and_acquire_fence() {
                if (!cmd_buffer_) {
                    return tl::unexpected("Invalid command buffer");
                }

                SDL_GPUFence* f = SDL_SubmitGPUCommandBufferAndAcquireFence(cmd_buffer_);
                if (!f) {
                    return tl::unexpected(get_error());
                }

                cmd_buffer_ = nullptr; // Command buffer consumed
                return fence(device_, f);
            }

            /**
             * @brief Acquire swapchain texture for rendering
             * @param window Window to render to
             * @param acquired_texture Output texture pointer
             * @param swapchain_texture_width Output width
             * @param swapchain_texture_height Output height
             * @return Success or error
             */
            [[nodiscard]] tl::expected <void, std::string> wait_and_acquire_swapchain_texture(
                const sdlpp::window& window,
                SDL_GPUTexture** acquired_texture,
                Uint32* swapchain_texture_width = nullptr,
                Uint32* swapchain_texture_height = nullptr) {
                if (!cmd_buffer_) {
                    return tl::unexpected("Invalid command buffer");
                }

                if (!SDL_WaitAndAcquireGPUSwapchainTexture(
                    cmd_buffer_,
                    window.get(),
                    acquired_texture,
                    swapchain_texture_width,
                    swapchain_texture_height)) {
                    return tl::unexpected(get_error());
                }

                return {};
            }

            /**
             * @brief Push debug group
             * @param name Group name
             */
            void push_debug_group(const char* name) {
                if (cmd_buffer_) {
                    SDL_PushGPUDebugGroup(cmd_buffer_, name);
                }
            }

            /**
             * @brief Pop debug group
             */
            void pop_debug_group() {
                if (cmd_buffer_) {
                    SDL_PopGPUDebugGroup(cmd_buffer_);
                }
            }

            /**
             * @brief Insert debug label
             * @param text Label text
             */
            void insert_debug_label(const char* text) {
                if (cmd_buffer_) {
                    SDL_InsertGPUDebugLabel(cmd_buffer_, text);
                }
            }

            /**
             * @brief Generate mipmaps for texture
             * @param texture Texture to generate mipmaps for
             */
            void generate_mipmaps(texture& texture) {
                if (cmd_buffer_) {
                    SDL_GenerateMipmapsForGPUTexture(cmd_buffer_, texture.get());
                }
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return cmd_buffer_ != nullptr;
            }

            [[nodiscard]] SDL_GPUCommandBuffer* get() const noexcept {
                return cmd_buffer_;
            }

        private:
            command_buffer(SDL_GPUDevice* dev, SDL_GPUCommandBuffer* cmd) noexcept
                : device_(dev), cmd_buffer_(cmd) {
            }

            SDL_GPUDevice* device_{nullptr};
            SDL_GPUCommandBuffer* cmd_buffer_{nullptr};
    };

    /**
     * @brief Render pass handle
     */
    class render_pass {
        public:
            render_pass() = default;
            render_pass(const render_pass&) = delete;
            render_pass& operator=(const render_pass&) = delete;

            render_pass(render_pass&& other) noexcept
                : pass_(std::exchange(other.pass_, nullptr)) {
            }

            render_pass& operator=(render_pass&& other) noexcept {
                if (this != &other) {
                    pass_ = std::exchange(other.pass_, nullptr);
                }
                return *this;
            }

            /**
             * @brief Bind graphics pipeline
             * @param pipeline Pipeline to bind
             */
            void bind_graphics_pipeline(const graphics_pipeline& pipeline) {
                if (pass_) {
                    SDL_BindGPUGraphicsPipeline(pass_, pipeline.get());
                }
            }

            /**
             * @brief Set viewport
             * @param viewport Viewport region
             */
            void set_viewport(const viewport& viewport) {
                if (pass_) {
                    auto vp = viewport.to_sdl();
                    SDL_SetGPUViewport(pass_, &vp);
                }
            }

            /**
             * @brief Set scissor rectangle
             * @param scissor Scissor region
             */
            template<rect_like R>
            void set_scissor(const R& scissor) {
                if (pass_) {
                    SDL_Rect rect{
                        .x = static_cast <int>(scissor.x),
                        .y = static_cast <int>(scissor.y),
                        .w = static_cast <int>(scissor.width),
                        .h = static_cast <int>(scissor.height)
                    };
                    SDL_SetGPUScissor(pass_, &rect);
                }
            }

            /**
             * @brief Bind vertex buffers
             * @param first_slot First binding slot
             * @param bindings Buffer bindings
             */
            void bind_vertex_buffers(Uint32 first_slot,
                                     std::span <const buffer_binding> bindings) {
                if (!pass_ || bindings.empty()) return;

                std::vector <SDL_GPUBufferBinding> sdl_bindings;
                sdl_bindings.reserve(bindings.size());
                for (const auto& binding : bindings) {
                    sdl_bindings.push_back(binding.to_sdl());
                }

                SDL_BindGPUVertexBuffers(pass_, first_slot, sdl_bindings.data(),
                                         static_cast <Uint32>(sdl_bindings.size()));
            }

            /**
             * @brief Bind index buffer
             * @param binding Buffer binding
             * @param index_element_size Element size
             */
            void bind_index_buffer(const buffer_binding& binding,
                                   index_element_size element_size) {
                if (pass_) {
                    auto bind = binding.to_sdl();
                    SDL_BindGPUIndexBuffer(pass_, &bind,
                                           static_cast <SDL_GPUIndexElementSize>(element_size));
                }
            }

            /**
             * @brief Bind vertex samplers
             * @param first_slot First binding slot
             * @param texture_sampler_bindings Texture/sampler pairs
             */
            void bind_vertex_samplers(Uint32 first_slot,
                                      std::span <const std::pair <texture*, sampler*>> bindings) {
                if (!pass_ || bindings.empty()) return;

                std::vector <SDL_GPUTextureSamplerBinding> sdl_bindings;
                sdl_bindings.reserve(bindings.size());
                for (const auto& [tex, samp] : bindings) {
                    sdl_bindings.push_back({
                        .texture = tex ? tex->get() : nullptr,
                        .sampler = samp ? samp->get() : nullptr
                    });
                }

                SDL_BindGPUVertexSamplers(pass_, first_slot, sdl_bindings.data(),
                                          static_cast <Uint32>(sdl_bindings.size()));
            }

            /**
             * @brief Bind fragment samplers
             * @param first_slot First binding slot
             * @param texture_sampler_bindings Texture/sampler pairs
             */
            void bind_fragment_samplers(Uint32 first_slot,
                                        std::span <const std::pair <texture*, sampler*>> bindings) {
                if (!pass_ || bindings.empty()) return;

                std::vector <SDL_GPUTextureSamplerBinding> sdl_bindings;
                sdl_bindings.reserve(bindings.size());
                for (const auto& [tex, samp] : bindings) {
                    sdl_bindings.push_back({
                        .texture = tex ? tex->get() : nullptr,
                        .sampler = samp ? samp->get() : nullptr
                    });
                }

                SDL_BindGPUFragmentSamplers(pass_, first_slot, sdl_bindings.data(),
                                            static_cast <Uint32>(sdl_bindings.size()));
            }

            /**
             * @brief Draw primitives
             * @param num_vertices Number of vertices
             * @param num_instances Number of instances
             * @param first_vertex First vertex index
             * @param first_instance First instance index
             */
            void draw_primitives(Uint32 num_vertices,
                                 Uint32 num_instances = 1,
                                 Uint32 first_vertex = 0,
                                 Uint32 first_instance = 0) {
                if (pass_) {
                    SDL_DrawGPUPrimitives(pass_, num_vertices, num_instances,
                                          first_vertex, first_instance);
                }
            }

            /**
             * @brief Draw indexed primitives
             * @param num_indices Number of indices
             * @param num_instances Number of instances
             * @param first_index First index
             * @param vertex_offset Vertex offset
             * @param first_instance First instance
             */
            void draw_indexed_primitives(Uint32 num_indices,
                                         Uint32 num_instances = 1,
                                         Uint32 first_index = 0,
                                         Sint32 vertex_offset = 0,
                                         Uint32 first_instance = 0) {
                if (pass_) {
                    SDL_DrawGPUIndexedPrimitives(pass_, num_indices, num_instances,
                                                 first_index, vertex_offset, first_instance);
                }
            }

            /**
             * @brief End render pass
             */
            void end() {
                if (pass_) {
                    SDL_EndGPURenderPass(pass_);
                    pass_ = nullptr;
                }
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return pass_ != nullptr;
            }

            [[nodiscard]] SDL_GPURenderPass* get() const noexcept {
                return pass_;
            }

        private:
            friend class command_buffer;

            explicit render_pass(SDL_GPURenderPass* p) noexcept
                : pass_(p) {
            }

            SDL_GPURenderPass* pass_{nullptr};
    };

    /**
     * @brief Compute pass handle
     */
    class compute_pass {
        public:
            compute_pass() = default;
            compute_pass(const compute_pass&) = delete;
            compute_pass& operator=(const compute_pass&) = delete;

            compute_pass(compute_pass&& other) noexcept
                : pass_(std::exchange(other.pass_, nullptr)) {
            }

            compute_pass& operator=(compute_pass&& other) noexcept {
                if (this != &other) {
                    pass_ = std::exchange(other.pass_, nullptr);
                }
                return *this;
            }

            /**
             * @brief Bind compute pipeline
             * @param pipeline Pipeline to bind
             */
            void bind_compute_pipeline(const compute_pipeline& pipeline) {
                if (pass_) {
                    SDL_BindGPUComputePipeline(pass_, pipeline.get());
                }
            }

            /**
             * @brief Bind compute storage textures
             * @param first_slot First binding slot
             * @param storage_textures Storage texture bindings
             */
            void bind_storage_textures(Uint32 first_slot,
                                       std::span <const texture*> storage_textures) {
                if (!pass_ || storage_textures.empty()) return;

                std::vector <SDL_GPUTexture*> textures;
                textures.reserve(storage_textures.size());
                for (auto tex : storage_textures) {
                    textures.push_back(tex ? tex->get() : nullptr);
                }

                SDL_BindGPUComputeStorageTextures(pass_, first_slot, textures.data(),
                                                  static_cast <Uint32>(textures.size()));
            }

            /**
             * @brief Bind compute storage buffers
             * @param first_slot First binding slot
             * @param storage_buffers Storage buffer bindings
             */
            void bind_storage_buffers(Uint32 first_slot,
                                      std::span <const buffer*> storage_buffers) {
                if (!pass_ || storage_buffers.empty()) return;

                std::vector <SDL_GPUBuffer*> buffers;
                buffers.reserve(storage_buffers.size());
                for (auto buf : storage_buffers) {
                    buffers.push_back(buf ? buf->get() : nullptr);
                }

                SDL_BindGPUComputeStorageBuffers(pass_, first_slot, buffers.data(),
                                                 static_cast <Uint32>(buffers.size()));
            }

            /**
             * @brief Dispatch compute work
             * @param groupcount_x X dimension groups
             * @param groupcount_y Y dimension groups
             * @param groupcount_z Z dimension groups
             */
            void dispatch(Uint32 groupcount_x,
                          Uint32 groupcount_y = 1,
                          Uint32 groupcount_z = 1) {
                if (pass_) {
                    SDL_DispatchGPUCompute(pass_, groupcount_x, groupcount_y, groupcount_z);
                }
            }

            /**
             * @brief End compute pass
             */
            void end() {
                if (pass_) {
                    SDL_EndGPUComputePass(pass_);
                    pass_ = nullptr;
                }
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return pass_ != nullptr;
            }

            [[nodiscard]] SDL_GPUComputePass* get() const noexcept {
                return pass_;
            }

        private:
            friend class command_buffer;

            explicit compute_pass(SDL_GPUComputePass* p) noexcept
                : pass_(p) {
            }

            SDL_GPUComputePass* pass_{nullptr};
    };

    /**
     * @brief Copy pass handle
     */
    class copy_pass {
        public:
            copy_pass() = default;
            copy_pass(const copy_pass&) = delete;
            copy_pass& operator=(const copy_pass&) = delete;

            copy_pass(copy_pass&& other) noexcept
                : pass_(std::exchange(other.pass_, nullptr)) {
            }

            copy_pass& operator=(copy_pass&& other) noexcept {
                if (this != &other) {
                    pass_ = std::exchange(other.pass_, nullptr);
                }
                return *this;
            }

            /**
             * @brief Upload to texture from transfer buffer
             * @param source Source transfer info
             * @param destination Destination texture region
             * @param cycle Cycle texture if busy
             */
            void upload_to_texture(const texture_transfer_info& source,
                                   const texture_region& destination,
                                   bool cycle = false) {
                if (pass_) {
                    auto src = source.to_sdl();
                    auto dst = destination.to_sdl();
                    SDL_UploadToGPUTexture(pass_, &src, &dst, cycle);
                }
            }

            /**
             * @brief Download from texture to transfer buffer
             * @param source Source texture region
             * @param destination Destination transfer info
             */
            void download_from_texture(const texture_region& source,
                                       const texture_transfer_info& destination) {
                if (pass_) {
                    auto src = source.to_sdl();
                    auto dst = destination.to_sdl();
                    SDL_DownloadFromGPUTexture(pass_, &src, &dst);
                }
            }

            /**
             * @brief End copy pass
             */
            void end() {
                if (pass_) {
                    SDL_EndGPUCopyPass(pass_);
                    pass_ = nullptr;
                }
            }

            [[nodiscard]] bool is_valid() const noexcept {
                return pass_ != nullptr;
            }

            [[nodiscard]] SDL_GPUCopyPass* get() const noexcept {
                return pass_;
            }

        private:
            friend class command_buffer;

            explicit copy_pass(SDL_GPUCopyPass* p) noexcept
                : pass_(p) {
            }

            SDL_GPUCopyPass* pass_{nullptr};
    };

    // Implementation of command_buffer methods that return passes

    inline render_pass command_buffer::begin_render_pass(
        std::span <const color_attachment_info> color_attachments,
        const depth_stencil_attachment_info* depth_stencil) {
        if (!cmd_buffer_) return render_pass(nullptr);

        std::vector <SDL_GPUColorTargetInfo> sdl_colors;
        sdl_colors.reserve(color_attachments.size());
        for (const auto& color : color_attachments) {
            sdl_colors.push_back(color.to_sdl());
        }

        SDL_GPUDepthStencilTargetInfo sdl_depth_stencil;
        SDL_GPUDepthStencilTargetInfo* depth_ptr = nullptr;
        if (depth_stencil) {
            sdl_depth_stencil = depth_stencil->to_sdl();
            depth_ptr = &sdl_depth_stencil;
        }

        SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(
            cmd_buffer_,
            sdl_colors.data(),
            static_cast <Uint32>(sdl_colors.size()),
            depth_ptr
        );

        return render_pass(pass);
    }

    inline compute_pass command_buffer::begin_compute_pass(
        std::span <const storage_texture_read_write_binding> read_write_textures,
        std::span <const storage_buffer_read_write_binding> read_write_buffers) {
        if (!cmd_buffer_) return compute_pass(nullptr);

        std::vector <SDL_GPUStorageTextureReadWriteBinding> sdl_textures;
        std::vector <SDL_GPUStorageBufferReadWriteBinding> sdl_buffers;

        if (!read_write_textures.empty()) {
            sdl_textures.reserve(read_write_textures.size());
            for (const auto& tex : read_write_textures) {
                sdl_textures.push_back(tex.to_sdl());
            }
        }

        if (!read_write_buffers.empty()) {
            sdl_buffers.reserve(read_write_buffers.size());
            for (const auto& buf : read_write_buffers) {
                sdl_buffers.push_back(buf.to_sdl());
            }
        }

        SDL_GPUComputePass* pass = SDL_BeginGPUComputePass(
            cmd_buffer_,
            sdl_textures.empty() ? nullptr : sdl_textures.data(),
            static_cast <Uint32>(sdl_textures.size()),
            sdl_buffers.empty() ? nullptr : sdl_buffers.data(),
            static_cast <Uint32>(sdl_buffers.size())
        );

        return compute_pass(pass);
    }

    inline copy_pass command_buffer::begin_copy_pass() {
        if (!cmd_buffer_) return copy_pass(nullptr);

        SDL_GPUCopyPass* pass = SDL_BeginGPUCopyPass(cmd_buffer_);
        return copy_pass(pass);
    }
} // namespace sdlpp::gpu
