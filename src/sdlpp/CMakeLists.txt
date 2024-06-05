add_library(sdlpp INTERFACE
        ${PROJECT_INCLUDE}/sdlpp/call.hh
        ${PROJECT_INCLUDE}/sdlpp/color.hh
        ${PROJECT_INCLUDE}/sdlpp/events/events_dispatcher.hh
        ${PROJECT_INCLUDE}/sdlpp/events/event_types.hh
        ${PROJECT_INCLUDE}/sdlpp/events/system_events.hh
        ${PROJECT_INCLUDE}/sdlpp/events/events.hh
        ${PROJECT_INCLUDE}/sdlpp/geometry.hh
        ${PROJECT_INCLUDE}/sdlpp/io.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/iterator.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/object.hh
        ${PROJECT_INCLUDE}/sdlpp/palette.hh
        ${PROJECT_INCLUDE}/sdlpp/pixel_format.hh
        ${PROJECT_INCLUDE}/sdlpp/render.hh
        ${PROJECT_INCLUDE}/sdlpp/rtc.hh
        ${PROJECT_INCLUDE}/sdlpp/sdl2.hh
        ${PROJECT_INCLUDE}/sdlpp/surface.hh
        ${PROJECT_INCLUDE}/sdlpp/system.hh
        ${PROJECT_INCLUDE}/sdlpp/texture.hh
        ${PROJECT_INCLUDE}/sdlpp/timers.hh
        ${PROJECT_INCLUDE}/sdlpp/ttf.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/ttf_helper.hh
        ${PROJECT_INCLUDE}/sdlpp/window.hh


)

add_library(neutrino::sdlpp ALIAS sdlpp)
target_compile_features(sdlpp INTERFACE cxx_std_17)

target_include_directories(sdlpp INTERFACE ${PROJECT_INCLUDE})
target_link_libraries(sdlpp INTERFACE thirdparty::sdl2 thirdparty::sdl2_ttf thirdparty::sdl2_mixer thirdparty::sdl2_image thirdparty::bitflags bsw)



