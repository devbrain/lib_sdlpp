add_library(sdlpp INTERFACE
        ${PROJECT_INCLUDE}/sdlpp/detail/call.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/iterator.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/object.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/rtc.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/sdl2.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/ttf_helper.hh

        ${PROJECT_INCLUDE}/sdlpp/events/event_types.hh
        ${PROJECT_INCLUDE}/sdlpp/events/events.hh
        ${PROJECT_INCLUDE}/sdlpp/events/events_dispatcher.hh
        ${PROJECT_INCLUDE}/sdlpp/events/system_events.hh

        ${PROJECT_INCLUDE}/sdlpp/system/cpu.hh
        ${PROJECT_INCLUDE}/sdlpp/system/display.hh
        ${PROJECT_INCLUDE}/sdlpp/system/system.hh
        ${PROJECT_INCLUDE}/sdlpp/system/timers.hh

        ${PROJECT_INCLUDE}/sdlpp/io/io.hh
        ${PROJECT_INCLUDE}/sdlpp/io/rwops.hh
        ${PROJECT_INCLUDE}/sdlpp/io/rwops_stream.hh
        ${PROJECT_INCLUDE}/sdlpp/io/whence.hh

        ${PROJECT_INCLUDE}/sdlpp/io.hh
        ${PROJECT_INCLUDE}/sdlpp/video.hh
        ${PROJECT_INCLUDE}/sdlpp/events.hh
        ${PROJECT_INCLUDE}/sdlpp/sdlpp.hh
        ${PROJECT_INCLUDE}/sdlpp/system.hh

        ${PROJECT_INCLUDE}/sdlpp/video/color.hh
        ${PROJECT_INCLUDE}/sdlpp/video/geometry.hh
        ${PROJECT_INCLUDE}/sdlpp/video/palette.hh
        ${PROJECT_INCLUDE}/sdlpp/video/pixel_format.hh
        ${PROJECT_INCLUDE}/sdlpp/video/render.hh
        ${PROJECT_INCLUDE}/sdlpp/video/surface.hh
        ${PROJECT_INCLUDE}/sdlpp/video/texture.hh
        ${PROJECT_INCLUDE}/sdlpp/video/ttf.hh
        ${PROJECT_INCLUDE}/sdlpp/video/window.hh

)

add_library(neutrino::sdlpp ALIAS sdlpp)
target_compile_features(sdlpp INTERFACE cxx_std_17)

target_include_directories(sdlpp INTERFACE ${PROJECT_INCLUDE})
target_link_libraries(sdlpp INTERFACE thirdparty::sdl2 thirdparty::sdl2_ttf thirdparty::sdl2_mixer thirdparty::sdl2_image thirdparty::bitflags bsw)



