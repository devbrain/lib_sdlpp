#pragma once

/**
 * @file gpu_defs.hh
 * @brief GPU API definitions and enumerations
 */

#include <sdlpp/core/sdl.hh>
#include <sdlpp/detail/export.hh>
#include <cstdint>

namespace sdlpp::gpu {
    /**
     * @brief Primitive topology types
     */
    enum class primitive_type {
        triangle_list = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST, ///< Separate triangles
        triangle_strip = SDL_GPU_PRIMITIVETYPE_TRIANGLESTRIP, ///< Connected triangles
        line_list = SDL_GPU_PRIMITIVETYPE_LINELIST, ///< Separate lines
        line_strip = SDL_GPU_PRIMITIVETYPE_LINESTRIP, ///< Connected lines
        point_list = SDL_GPU_PRIMITIVETYPE_POINTLIST ///< Separate points
    };

    /**
     * @brief Load operation for render pass attachments
     */
    enum class load_op {
        load = SDL_GPU_LOADOP_LOAD, ///< Preserve previous contents
        clear = SDL_GPU_LOADOP_CLEAR, ///< Clear to specified color
        dont_care = SDL_GPU_LOADOP_DONT_CARE ///< Contents undefined
    };

    /**
     * @brief Store operation for render pass attachments
     */
    enum class store_op {
        store = SDL_GPU_STOREOP_STORE, ///< Write to memory
        dont_care = SDL_GPU_STOREOP_DONT_CARE, ///< Discard contents
        resolve = SDL_GPU_STOREOP_RESOLVE, ///< Resolve multisample
        resolve_and_store = SDL_GPU_STOREOP_RESOLVE_AND_STORE ///< Resolve and store
    };

    /**
     * @brief Index buffer element size
     */
    enum class index_element_size {
        uint16 = SDL_GPU_INDEXELEMENTSIZE_16BIT, ///< 16-bit indices
        uint32 = SDL_GPU_INDEXELEMENTSIZE_32BIT ///< 32-bit indices
    };

    /**
     * @brief Texture format
     */
    enum class texture_format {
        invalid = SDL_GPU_TEXTUREFORMAT_INVALID,

        // Unsigned normalized formats
        a8_unorm = SDL_GPU_TEXTUREFORMAT_A8_UNORM,
        r8_unorm = SDL_GPU_TEXTUREFORMAT_R8_UNORM,
        r8g8_unorm = SDL_GPU_TEXTUREFORMAT_R8G8_UNORM,
        r8g8b8a8_unorm = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        r16_unorm = SDL_GPU_TEXTUREFORMAT_R16_UNORM,
        r16g16_unorm = SDL_GPU_TEXTUREFORMAT_R16G16_UNORM,
        r16g16b16a16_unorm = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_UNORM,
        r10g10b10a2_unorm = SDL_GPU_TEXTUREFORMAT_R10G10B10A2_UNORM,
        b5g6r5_unorm = SDL_GPU_TEXTUREFORMAT_B5G6R5_UNORM,
        b5g5r5a1_unorm = SDL_GPU_TEXTUREFORMAT_B5G5R5A1_UNORM,
        b4g4r4a4_unorm = SDL_GPU_TEXTUREFORMAT_B4G4R4A4_UNORM,
        b8g8r8a8_unorm = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM,

        // Compressed formats
        bc1_rgba_unorm = SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM,
        bc2_rgba_unorm = SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM,
        bc3_rgba_unorm = SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM,
        bc4_r_unorm = SDL_GPU_TEXTUREFORMAT_BC4_R_UNORM,
        bc5_rg_unorm = SDL_GPU_TEXTUREFORMAT_BC5_RG_UNORM,
        bc7_rgba_unorm = SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM,

        // Signed normalized formats
        r8_snorm = SDL_GPU_TEXTUREFORMAT_R8_SNORM,
        r8g8_snorm = SDL_GPU_TEXTUREFORMAT_R8G8_SNORM,
        r8g8b8a8_snorm = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_SNORM,
        r16_snorm = SDL_GPU_TEXTUREFORMAT_R16_SNORM,
        r16g16_snorm = SDL_GPU_TEXTUREFORMAT_R16G16_SNORM,
        r16g16b16a16_snorm = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_SNORM,

        // Unsigned integer formats
        r8_uint = SDL_GPU_TEXTUREFORMAT_R8_UINT,
        r8g8_uint = SDL_GPU_TEXTUREFORMAT_R8G8_UINT,
        r8g8b8a8_uint = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UINT,
        r16_uint = SDL_GPU_TEXTUREFORMAT_R16_UINT,
        r16g16_uint = SDL_GPU_TEXTUREFORMAT_R16G16_UINT,
        r16g16b16a16_uint = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_UINT,
        r32_uint = SDL_GPU_TEXTUREFORMAT_R32_UINT,
        r32g32_uint = SDL_GPU_TEXTUREFORMAT_R32G32_UINT,
        r32g32b32a32_uint = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_UINT,

        // Signed integer formats
        r8_int = SDL_GPU_TEXTUREFORMAT_R8_INT,
        r8g8_int = SDL_GPU_TEXTUREFORMAT_R8G8_INT,
        r8g8b8a8_int = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_INT,
        r16_int = SDL_GPU_TEXTUREFORMAT_R16_INT,
        r16g16_int = SDL_GPU_TEXTUREFORMAT_R16G16_INT,
        r16g16b16a16_int = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_INT,
        r32_int = SDL_GPU_TEXTUREFORMAT_R32_INT,
        r32g32_int = SDL_GPU_TEXTUREFORMAT_R32G32_INT,
        r32g32b32a32_int = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_INT,

        // Float formats
        r16_float = SDL_GPU_TEXTUREFORMAT_R16_FLOAT,
        r16g16_float = SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT,
        r16g16b16a16_float = SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
        r32_float = SDL_GPU_TEXTUREFORMAT_R32_FLOAT,
        r32g32_float = SDL_GPU_TEXTUREFORMAT_R32G32_FLOAT,
        r32g32b32a32_float = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,
        r11g11b10_ufloat = SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT,

        // sRGB formats
        r8g8b8a8_unorm_srgb = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB,
        b8g8r8a8_unorm_srgb = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM_SRGB,

        // Compressed sRGB formats
        bc1_rgba_unorm_srgb = SDL_GPU_TEXTUREFORMAT_BC1_RGBA_UNORM_SRGB,
        bc2_rgba_unorm_srgb = SDL_GPU_TEXTUREFORMAT_BC2_RGBA_UNORM_SRGB,
        bc3_rgba_unorm_srgb = SDL_GPU_TEXTUREFORMAT_BC3_RGBA_UNORM_SRGB,
        bc7_rgba_unorm_srgb = SDL_GPU_TEXTUREFORMAT_BC7_RGBA_UNORM_SRGB,

        // Depth/stencil formats
        d16_unorm = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
        d24_unorm = SDL_GPU_TEXTUREFORMAT_D24_UNORM,
        d32_float = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        d24_unorm_s8_uint = SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT,
        d32_float_s8_uint = SDL_GPU_TEXTUREFORMAT_D32_FLOAT_S8_UINT,

        // Compressed formats (additional)
        astc_4x4_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM,
        astc_5x4_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_5x4_UNORM,
        astc_5x5_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_5x5_UNORM,
        astc_6x5_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_6x5_UNORM,
        astc_6x6_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_6x6_UNORM,
        astc_8x5_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_8x5_UNORM,
        astc_8x6_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_8x6_UNORM,
        astc_8x8_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_8x8_UNORM,
        astc_10x5_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_10x5_UNORM,
        astc_10x6_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_10x6_UNORM,
        astc_10x8_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_10x8_UNORM,
        astc_10x10_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_10x10_UNORM,
        astc_12x10_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_12x10_UNORM,
        astc_12x12_unorm = SDL_GPU_TEXTUREFORMAT_ASTC_12x12_UNORM,

        // Compressed sRGB formats (additional)
        astc_4x4_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_4x4_UNORM_SRGB,
        astc_5x4_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_5x4_UNORM_SRGB,
        astc_5x5_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_5x5_UNORM_SRGB,
        astc_6x5_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_6x5_UNORM_SRGB,
        astc_6x6_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_6x6_UNORM_SRGB,
        astc_8x5_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_8x5_UNORM_SRGB,
        astc_8x6_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_8x6_UNORM_SRGB,
        astc_8x8_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_8x8_UNORM_SRGB,
        astc_10x5_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_10x5_UNORM_SRGB,
        astc_10x6_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_10x6_UNORM_SRGB,
        astc_10x8_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_10x8_UNORM_SRGB,
        astc_10x10_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_10x10_UNORM_SRGB,
        astc_12x10_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_12x10_UNORM_SRGB,
        astc_12x12_unorm_srgb = SDL_GPU_TEXTUREFORMAT_ASTC_12x12_UNORM_SRGB
    };

    /**
     * @brief Texture type
     */
    enum class texture_type {
        texture_2d = SDL_GPU_TEXTURETYPE_2D, ///< 2D texture
        texture_2d_array = SDL_GPU_TEXTURETYPE_2D_ARRAY, ///< 2D texture array
        texture_3d = SDL_GPU_TEXTURETYPE_3D, ///< 3D texture
        cube = SDL_GPU_TEXTURETYPE_CUBE, ///< Cube map
        cube_array = SDL_GPU_TEXTURETYPE_CUBE_ARRAY ///< Cube map array
    };

    /**
     * @brief Sample count for multisampling
     */
    enum class sample_count {
        count_1 = SDL_GPU_SAMPLECOUNT_1, ///< No multisampling
        count_2 = SDL_GPU_SAMPLECOUNT_2, ///< 2x MSAA
        count_4 = SDL_GPU_SAMPLECOUNT_4, ///< 4x MSAA
        count_8 = SDL_GPU_SAMPLECOUNT_8 ///< 8x MSAA
    };

    /**
     * @brief Cube map face
     */
    enum class cube_map_face {
        positive_x = SDL_GPU_CUBEMAPFACE_POSITIVEX, ///< +X face
        negative_x = SDL_GPU_CUBEMAPFACE_NEGATIVEX, ///< -X face
        positive_y = SDL_GPU_CUBEMAPFACE_POSITIVEY, ///< +Y face
        negative_y = SDL_GPU_CUBEMAPFACE_NEGATIVEY, ///< -Y face
        positive_z = SDL_GPU_CUBEMAPFACE_POSITIVEZ, ///< +Z face
        negative_z = SDL_GPU_CUBEMAPFACE_NEGATIVEZ ///< -Z face
    };

    /**
     * @brief Transfer buffer usage
     */
    enum class transfer_buffer_usage {
        upload = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, ///< CPU to GPU
        download = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD ///< GPU to CPU
    };

    /**
     * @brief Shader stage
     */
    enum class shader_stage {
        vertex = SDL_GPU_SHADERSTAGE_VERTEX, ///< Vertex shader
        fragment = SDL_GPU_SHADERSTAGE_FRAGMENT ///< Fragment/pixel shader
    };

    /**
     * @brief Shader format flags
     */
    enum class shader_format : Uint32 {
        invalid = SDL_GPU_SHADERFORMAT_INVALID,
        spirv = SDL_GPU_SHADERFORMAT_SPIRV, ///< SPIR-V format
        dxbc = SDL_GPU_SHADERFORMAT_DXBC, ///< DirectX bytecode
        dxil = SDL_GPU_SHADERFORMAT_DXIL, ///< DirectX IL
        msl = SDL_GPU_SHADERFORMAT_MSL, ///< Metal shading language
        metallib = SDL_GPU_SHADERFORMAT_METALLIB ///< Metal library
    };

    [[nodiscard]] constexpr shader_format operator|(shader_format lhs, shader_format rhs) noexcept {
        return static_cast <shader_format>(
            static_cast <Uint32>(lhs) | static_cast <Uint32>(rhs)
        );
    }

    [[nodiscard]] constexpr shader_format operator&(shader_format lhs, shader_format rhs) noexcept {
        return static_cast <shader_format>(
            static_cast <Uint32>(lhs) & static_cast <Uint32>(rhs)
        );
    }

    /**
     * @brief Vertex element format
     */
    enum class vertex_element_format {
        invalid = SDL_GPU_VERTEXELEMENTFORMAT_INVALID,

        // Integer formats
        int1 = SDL_GPU_VERTEXELEMENTFORMAT_INT,
        int2 = SDL_GPU_VERTEXELEMENTFORMAT_INT2,
        int3 = SDL_GPU_VERTEXELEMENTFORMAT_INT3,
        int4 = SDL_GPU_VERTEXELEMENTFORMAT_INT4,

        uint1 = SDL_GPU_VERTEXELEMENTFORMAT_UINT,
        uint2 = SDL_GPU_VERTEXELEMENTFORMAT_UINT2,
        uint3 = SDL_GPU_VERTEXELEMENTFORMAT_UINT3,
        uint4 = SDL_GPU_VERTEXELEMENTFORMAT_UINT4,

        // Float formats
        float1 = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT,
        float2 = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
        float3 = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
        float4 = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,

        // Byte formats
        byte2 = SDL_GPU_VERTEXELEMENTFORMAT_BYTE2,
        byte4 = SDL_GPU_VERTEXELEMENTFORMAT_BYTE4,

        ubyte2 = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2,
        ubyte4 = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4,

        byte2_norm = SDL_GPU_VERTEXELEMENTFORMAT_BYTE2_NORM,
        byte4_norm = SDL_GPU_VERTEXELEMENTFORMAT_BYTE4_NORM,

        ubyte2_norm = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE2_NORM,
        ubyte4_norm = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,

        // Short formats
        short2 = SDL_GPU_VERTEXELEMENTFORMAT_SHORT2,
        short4 = SDL_GPU_VERTEXELEMENTFORMAT_SHORT4,

        ushort2 = SDL_GPU_VERTEXELEMENTFORMAT_USHORT2,
        ushort4 = SDL_GPU_VERTEXELEMENTFORMAT_USHORT4,

        short2_norm = SDL_GPU_VERTEXELEMENTFORMAT_SHORT2_NORM,
        short4_norm = SDL_GPU_VERTEXELEMENTFORMAT_SHORT4_NORM,

        ushort2_norm = SDL_GPU_VERTEXELEMENTFORMAT_USHORT2_NORM,
        ushort4_norm = SDL_GPU_VERTEXELEMENTFORMAT_USHORT4_NORM,

        // Half float formats
        half2 = SDL_GPU_VERTEXELEMENTFORMAT_HALF2,
        half4 = SDL_GPU_VERTEXELEMENTFORMAT_HALF4
    };

    /**
     * @brief Vertex input rate
     */
    enum class vertex_input_rate {
        vertex = SDL_GPU_VERTEXINPUTRATE_VERTEX, ///< Per-vertex data
        instance = SDL_GPU_VERTEXINPUTRATE_INSTANCE ///< Per-instance data
    };

    /**
     * @brief Fill mode for polygon rendering
     */
    enum class fill_mode {
        fill = SDL_GPU_FILLMODE_FILL, ///< Solid fill
        line = SDL_GPU_FILLMODE_LINE ///< Wireframe
    };

    /**
     * @brief Face culling mode
     */
    enum class cull_mode {
        none = SDL_GPU_CULLMODE_NONE, ///< No culling
        front = SDL_GPU_CULLMODE_FRONT, ///< Cull front faces
        back = SDL_GPU_CULLMODE_BACK ///< Cull back faces
    };

    /**
     * @brief Front face winding order
     */
    enum class front_face {
        counter_clockwise = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE, ///< CCW is front
        clockwise = SDL_GPU_FRONTFACE_CLOCKWISE ///< CW is front
    };

    /**
     * @brief Comparison function
     */
    enum class compare_op {
        invalid = SDL_GPU_COMPAREOP_INVALID,
        never = SDL_GPU_COMPAREOP_NEVER, ///< Always false
        less = SDL_GPU_COMPAREOP_LESS, ///< a < b
        equal = SDL_GPU_COMPAREOP_EQUAL, ///< a == b
        less_or_equal = SDL_GPU_COMPAREOP_LESS_OR_EQUAL, ///< a <= b
        greater = SDL_GPU_COMPAREOP_GREATER, ///< a > b
        not_equal = SDL_GPU_COMPAREOP_NOT_EQUAL, ///< a != b
        greater_or_equal = SDL_GPU_COMPAREOP_GREATER_OR_EQUAL, ///< a >= b
        always = SDL_GPU_COMPAREOP_ALWAYS ///< Always true
    };

    /**
     * @brief Stencil operation
     */
    enum class stencil_op {
        invalid = SDL_GPU_STENCILOP_INVALID,
        keep = SDL_GPU_STENCILOP_KEEP, ///< Keep current value
        zero = SDL_GPU_STENCILOP_ZERO, ///< Set to 0
        replace = SDL_GPU_STENCILOP_REPLACE, ///< Replace with reference
        increment_and_clamp = SDL_GPU_STENCILOP_INCREMENT_AND_CLAMP, ///< Increment, clamp to max
        decrement_and_clamp = SDL_GPU_STENCILOP_DECREMENT_AND_CLAMP, ///< Decrement, clamp to 0
        invert = SDL_GPU_STENCILOP_INVERT, ///< Bitwise invert
        increment_and_wrap = SDL_GPU_STENCILOP_INCREMENT_AND_WRAP, ///< Increment, wrap to 0
        decrement_and_wrap = SDL_GPU_STENCILOP_DECREMENT_AND_WRAP ///< Decrement, wrap to max
    };

    /**
     * @brief Blend operation
     */
    enum class blend_op {
        invalid = SDL_GPU_BLENDOP_INVALID,
        add = SDL_GPU_BLENDOP_ADD, ///< Src + Dst
        subtract = SDL_GPU_BLENDOP_SUBTRACT, ///< Src - Dst
        reverse_subtract = SDL_GPU_BLENDOP_REVERSE_SUBTRACT, ///< Dst - Src
        min = SDL_GPU_BLENDOP_MIN, ///< min(Src, Dst)
        max = SDL_GPU_BLENDOP_MAX ///< max(Src, Dst)
    };

    /**
     * @brief Blend factor
     */
    enum class blend_factor {
        invalid = SDL_GPU_BLENDFACTOR_INVALID,
        zero = SDL_GPU_BLENDFACTOR_ZERO, ///< 0
        one = SDL_GPU_BLENDFACTOR_ONE, ///< 1
        src_color = SDL_GPU_BLENDFACTOR_SRC_COLOR, ///< Source color
        one_minus_src_color = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_COLOR, ///< 1 - source color
        dst_color = SDL_GPU_BLENDFACTOR_DST_COLOR, ///< Destination color
        one_minus_dst_color = SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_COLOR, ///< 1 - destination color
        src_alpha = SDL_GPU_BLENDFACTOR_SRC_ALPHA, ///< Source alpha
        one_minus_src_alpha = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA, ///< 1 - source alpha
        dst_alpha = SDL_GPU_BLENDFACTOR_DST_ALPHA, ///< Destination alpha
        one_minus_dst_alpha = SDL_GPU_BLENDFACTOR_ONE_MINUS_DST_ALPHA, ///< 1 - destination alpha
        constant_color = SDL_GPU_BLENDFACTOR_CONSTANT_COLOR, ///< Constant color
        one_minus_constant_color = SDL_GPU_BLENDFACTOR_ONE_MINUS_CONSTANT_COLOR, ///< 1 - constant color
        src_alpha_saturate = SDL_GPU_BLENDFACTOR_SRC_ALPHA_SATURATE ///< min(src alpha, 1 - dst alpha)
    };

    /**
     * @brief Texture filter mode
     */
    enum class filter {
        nearest = SDL_GPU_FILTER_NEAREST, ///< Nearest neighbor
        linear = SDL_GPU_FILTER_LINEAR ///< Linear interpolation
    };

    /**
     * @brief Sampler mipmap mode
     */
    enum class sampler_mipmap_mode {
        nearest = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST, ///< Nearest mipmap
        linear = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR ///< Linear mipmap interpolation
    };

    /**
     * @brief Sampler address mode
     */
    enum class sampler_address_mode {
        repeat = SDL_GPU_SAMPLERADDRESSMODE_REPEAT, ///< Repeat texture
        mirrored_repeat = SDL_GPU_SAMPLERADDRESSMODE_MIRRORED_REPEAT, ///< Mirror and repeat
        clamp_to_edge = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE ///< Clamp to edge
    };

    /**
     * @brief Texture usage flags
     */
    enum class texture_usage : Uint32 {
        sampler = SDL_GPU_TEXTUREUSAGE_SAMPLER, ///< Can be sampled
        color_target = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET, ///< Can be rendered to
        depth_stencil_target = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET, ///< Can be depth/stencil target
        storage_read = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_READ, ///< Can be read in compute
        storage_write = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE, ///< Can be written in compute
        compute_storage_simultaneous_read_write = SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_SIMULTANEOUS_READ_WRITE
        ///< Compute simultaneous R/W
    };

    [[nodiscard]] constexpr texture_usage operator|(texture_usage lhs, texture_usage rhs) noexcept {
        return static_cast <texture_usage>(
            static_cast <Uint32>(lhs) | static_cast <Uint32>(rhs)
        );
    }

    [[nodiscard]] constexpr texture_usage operator&(texture_usage lhs, texture_usage rhs) noexcept {
        return static_cast <texture_usage>(
            static_cast <Uint32>(lhs) & static_cast <Uint32>(rhs)
        );
    }

    /**
     * @brief Buffer usage flags
     */
    enum class buffer_usage : Uint32 {
        vertex = SDL_GPU_BUFFERUSAGE_VERTEX, ///< Vertex buffer
        index = SDL_GPU_BUFFERUSAGE_INDEX, ///< Index buffer
        indirect = SDL_GPU_BUFFERUSAGE_INDIRECT, ///< Indirect draw/dispatch
        graphics_storage_read = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ, ///< Graphics storage read
        compute_storage_read = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ, ///< Compute storage read
        compute_storage_write = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE ///< Compute storage write
    };

    [[nodiscard]] constexpr buffer_usage operator|(buffer_usage lhs, buffer_usage rhs) noexcept {
        return static_cast <buffer_usage>(
            static_cast <Uint32>(lhs) | static_cast <Uint32>(rhs)
        );
    }

    [[nodiscard]] constexpr buffer_usage operator&(buffer_usage lhs, buffer_usage rhs) noexcept {
        return static_cast <buffer_usage>(
            static_cast <Uint32>(lhs) & static_cast <Uint32>(rhs)
        );
    }

    /**
     * @brief Color component flags
     */
    enum class color_component : Uint8 {
        r = SDL_GPU_COLORCOMPONENT_R, ///< Red component
        g = SDL_GPU_COLORCOMPONENT_G, ///< Green component
        b = SDL_GPU_COLORCOMPONENT_B, ///< Blue component
        a = SDL_GPU_COLORCOMPONENT_A ///< Alpha component
    };

    [[nodiscard]] constexpr color_component operator|(color_component lhs, color_component rhs) noexcept {
        return static_cast <color_component>(
            static_cast <Uint8>(lhs) | static_cast <Uint8>(rhs)
        );
    }

    [[nodiscard]] constexpr color_component operator&(color_component lhs, color_component rhs) noexcept {
        return static_cast <color_component>(
            static_cast <Uint8>(lhs) & static_cast <Uint8>(rhs)
        );
    }
} // namespace sdlpp::gpu


// Stream operators for enums
#include <iosfwd>

namespace sdlpp::gpu {
/**
 * @brief Stream output operator for primitive_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, primitive_type value);

/**
 * @brief Stream input operator for primitive_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, primitive_type& value);

/**
 * @brief Stream output operator for load_op
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, load_op value);

/**
 * @brief Stream input operator for load_op
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, load_op& value);

/**
 * @brief Stream output operator for store_op
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, store_op value);

/**
 * @brief Stream input operator for store_op
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, store_op& value);

/**
 * @brief Stream output operator for index_element_size
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, index_element_size value);

/**
 * @brief Stream input operator for index_element_size
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, index_element_size& value);

/**
 * @brief Stream output operator for texture_format
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, texture_format value);

/**
 * @brief Stream input operator for texture_format
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, texture_format& value);

/**
 * @brief Stream output operator for texture_type
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, texture_type value);

/**
 * @brief Stream input operator for texture_type
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, texture_type& value);

/**
 * @brief Stream output operator for sample_count
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, sample_count value);

/**
 * @brief Stream input operator for sample_count
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, sample_count& value);

/**
 * @brief Stream output operator for cube_map_face
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, cube_map_face value);

/**
 * @brief Stream input operator for cube_map_face
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, cube_map_face& value);

/**
 * @brief Stream output operator for transfer_buffer_usage
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, transfer_buffer_usage value);

/**
 * @brief Stream input operator for transfer_buffer_usage
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, transfer_buffer_usage& value);

/**
 * @brief Stream output operator for shader_stage
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, shader_stage value);

/**
 * @brief Stream input operator for shader_stage
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, shader_stage& value);

/**
 * @brief Stream output operator for shader_format
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, shader_format value);

/**
 * @brief Stream input operator for shader_format
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, shader_format& value);

/**
 * @brief Stream output operator for vertex_element_format
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, vertex_element_format value);

/**
 * @brief Stream input operator for vertex_element_format
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, vertex_element_format& value);

/**
 * @brief Stream output operator for vertex_input_rate
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, vertex_input_rate value);

/**
 * @brief Stream input operator for vertex_input_rate
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, vertex_input_rate& value);

/**
 * @brief Stream output operator for fill_mode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, fill_mode value);

/**
 * @brief Stream input operator for fill_mode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, fill_mode& value);

/**
 * @brief Stream output operator for cull_mode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, cull_mode value);

/**
 * @brief Stream input operator for cull_mode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, cull_mode& value);

/**
 * @brief Stream output operator for front_face
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, front_face value);

/**
 * @brief Stream input operator for front_face
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, front_face& value);

/**
 * @brief Stream output operator for compare_op
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, compare_op value);

/**
 * @brief Stream input operator for compare_op
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, compare_op& value);

/**
 * @brief Stream output operator for stencil_op
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, stencil_op value);

/**
 * @brief Stream input operator for stencil_op
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, stencil_op& value);

/**
 * @brief Stream output operator for blend_op
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, blend_op value);

/**
 * @brief Stream input operator for blend_op
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, blend_op& value);

/**
 * @brief Stream output operator for blend_factor
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, blend_factor value);

/**
 * @brief Stream input operator for blend_factor
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, blend_factor& value);

/**
 * @brief Stream output operator for filter
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, filter value);

/**
 * @brief Stream input operator for filter
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, filter& value);

/**
 * @brief Stream output operator for sampler_mipmap_mode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, sampler_mipmap_mode value);

/**
 * @brief Stream input operator for sampler_mipmap_mode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, sampler_mipmap_mode& value);

/**
 * @brief Stream output operator for sampler_address_mode
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, sampler_address_mode value);

/**
 * @brief Stream input operator for sampler_address_mode
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, sampler_address_mode& value);

/**
 * @brief Stream output operator for texture_usage
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, texture_usage value);

/**
 * @brief Stream input operator for texture_usage
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, texture_usage& value);

/**
 * @brief Stream output operator for buffer_usage
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, buffer_usage value);

/**
 * @brief Stream input operator for buffer_usage
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, buffer_usage& value);

/**
 * @brief Stream output operator for color_component
 */
SDLPP_EXPORT std::ostream& operator<<(std::ostream& os, color_component value);

/**
 * @brief Stream input operator for color_component
 */
SDLPP_EXPORT std::istream& operator>>(std::istream& is, color_component& value);

} // namespace sdlpp::gpu
