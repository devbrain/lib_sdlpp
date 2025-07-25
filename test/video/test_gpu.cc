#include <doctest/doctest.h>
#include <sdlpp/video/gpu.hh>
#include <sdlpp/video/gpu_resources.hh>
#include <sdlpp/video/gpu_commands.hh>
#include <sdlpp/core/core.hh>
#include <sdlpp/video/window.hh>


TEST_SUITE("gpu") {
    TEST_CASE("enums") {
        // Test enum conversions
        CHECK(static_cast<int>(sdlpp::gpu::primitive_type::triangle_list) == SDL_GPU_PRIMITIVETYPE_TRIANGLELIST);
        CHECK(static_cast<int>(sdlpp::gpu::texture_format::r8g8b8a8_unorm) == SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM);
        CHECK(static_cast<int>(sdlpp::gpu::shader_stage::vertex) == SDL_GPU_SHADERSTAGE_VERTEX);
        
        // Test flag operations
        auto usage = sdlpp::gpu::texture_usage::sampler | sdlpp::gpu::texture_usage::color_target;
        CHECK(static_cast<Uint32>(usage) == (SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET));
        
        auto shader_formats = sdlpp::gpu::shader_format::spirv | sdlpp::gpu::shader_format::dxbc;
        CHECK(static_cast<Uint32>(shader_formats) == (SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC));
    }
    
    TEST_CASE("vertex_input_state") {
        sdlpp::gpu::vertex_input_state state;
        
        // Add vertex buffer description
        state.vertex_buffer_descriptions.push_back({
            .slot = 0,
            .pitch = 32,
            .input_rate = sdlpp::gpu::vertex_input_rate::vertex,
            .instance_step_rate = 1
        });
        
        // Add vertex attributes
        state.vertex_attributes.push_back({
            .location = 0,
            .buffer_slot = 0,
            .format = sdlpp::gpu::vertex_element_format::float3,
            .offset = 0
        });
        
        state.vertex_attributes.push_back({
            .location = 1,
            .buffer_slot = 0,
            .format = sdlpp::gpu::vertex_element_format::float2,
            .offset = 12
        });
        
        // Convert to SDL
        auto sdl_state = state.to_sdl();
        CHECK(sdl_state.num_vertex_buffers == 1);
        CHECK(sdl_state.num_vertex_attributes == 2);
    }
    
    TEST_CASE("color_blend_state") {
        sdlpp::gpu::color_target_blend_state blend;
        blend.enable_blend = true;
        blend.src_color_blendfactor = sdlpp::gpu::blend_factor::src_alpha;
        blend.dst_color_blendfactor = sdlpp::gpu::blend_factor::one_minus_src_alpha;
        blend.color_blend_op = sdlpp::gpu::blend_op::add;
        
        auto sdl_blend = blend.to_sdl();
        CHECK(sdl_blend.enable_blend == true);
        CHECK(sdl_blend.src_color_blendfactor == SDL_GPU_BLENDFACTOR_SRC_ALPHA);
        CHECK(sdl_blend.dst_color_blendfactor == SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA);
        CHECK(sdl_blend.color_blend_op == SDL_GPU_BLENDOP_ADD);
    }
    
    TEST_CASE("rasterizer_state") {
        sdlpp::gpu::rasterizer_state raster;
        raster.cull_mode = sdlpp::gpu::cull_mode::back;
        raster.front_face = sdlpp::gpu::front_face::counter_clockwise;
        raster.fill_mode = sdlpp::gpu::fill_mode::fill;
        
        auto sdl_raster = raster.to_sdl();
        CHECK(sdl_raster.cull_mode == SDL_GPU_CULLMODE_BACK);
        CHECK(sdl_raster.front_face == SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE);
        CHECK(sdl_raster.fill_mode == SDL_GPU_FILLMODE_FILL);
    }
    
    TEST_CASE("depth_stencil_state") {
        sdlpp::gpu::depth_stencil_state depth;
        depth.enable_depth_test = true;
        depth.enable_depth_write = true;
        depth.compare_op = sdlpp::gpu::compare_op::less;
        
        auto sdl_depth = depth.to_sdl();
        CHECK(sdl_depth.enable_depth_test == true);
        CHECK(sdl_depth.enable_depth_write == true);
        CHECK(sdl_depth.compare_op == SDL_GPU_COMPAREOP_LESS);
    }
    
    TEST_CASE("texture_region") {
        sdlpp::gpu::texture_region region;
        region.x = 10;
        region.y = 20;
        region.w = 100;
        region.h = 200;
        region.mip_level = 0;
        region.layer = 0;
        
        auto sdl_region = region.to_sdl();
        CHECK(sdl_region.x == 10);
        CHECK(sdl_region.y == 20);
        CHECK(sdl_region.w == 100);
        CHECK(sdl_region.h == 200);
    }
    
    TEST_CASE("viewport") {
        sdlpp::gpu::viewport vp;
        vp.x = 0.0f;
        vp.y = 0.0f;
        vp.w = 800.0f;
        vp.h = 600.0f;
        vp.min_depth = 0.0f;
        vp.max_depth = 1.0f;
        
        auto sdl_vp = vp.to_sdl();
        CHECK(sdl_vp.x == 0.0f);
        CHECK(sdl_vp.y == 0.0f);
        CHECK(sdl_vp.w == 800.0f);
        CHECK(sdl_vp.h == 600.0f);
        CHECK(sdl_vp.min_depth == 0.0f);
        CHECK(sdl_vp.max_depth == 1.0f);
    }
    
    // Note: Actual GPU device creation tests would require proper initialization
    // and may not work in all test environments, so we keep tests focused on
    // data structure conversions and basic functionality.
}