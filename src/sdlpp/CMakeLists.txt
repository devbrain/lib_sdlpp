include(GenerateExportHeader)

add_library(sdlpp
        ${PROJECT_INCLUDE}/sdlpp/detail/call.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/iterator.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/object.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/ostreamops.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/rtc.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/sdl2.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/sdl_string.hh
        ${PROJECT_INCLUDE}/sdlpp/detail/ttf_helper.hh

        ${PROJECT_INCLUDE}/sdlpp/events/event_types.hh
        ${PROJECT_INCLUDE}/sdlpp/events/events.hh
        ${PROJECT_INCLUDE}/sdlpp/events/events_dispatcher.hh
        ${PROJECT_INCLUDE}/sdlpp/events/system_events.hh

        ${PROJECT_INCLUDE}/sdlpp/system/clipboard.hh
        ${PROJECT_INCLUDE}/sdlpp/system/cpu.hh
        ${PROJECT_INCLUDE}/sdlpp/system/display.hh
        ${PROJECT_INCLUDE}/sdlpp/system/joystick.hh
        ${PROJECT_INCLUDE}/sdlpp/system/locale.hh
        ${PROJECT_INCLUDE}/sdlpp/system/power.hh
        ${PROJECT_INCLUDE}/sdlpp/system/sensors.hh
        ${PROJECT_INCLUDE}/sdlpp/system/shared_object.hh
        ${PROJECT_INCLUDE}/sdlpp/system/system.hh
        ${PROJECT_INCLUDE}/sdlpp/system/timers.hh
        ${PROJECT_INCLUDE}/sdlpp/system/touch.hh

        ${PROJECT_INCLUDE}/sdlpp/input.hh
        ${PROJECT_INCLUDE}/sdlpp/input/joystick.hh
        ${PROJECT_INCLUDE}/sdlpp/input/mouse.hh
        ${PROJECT_INCLUDE}/sdlpp/input/keyboard.hh
        ${PROJECT_INCLUDE}/sdlpp/input/sensor.hh

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
        ${PROJECT_INCLUDE}/sdlpp/video/color_names.hh
        ${PROJECT_INCLUDE}/sdlpp/video/cursor.hh
        ${PROJECT_INCLUDE}/sdlpp/video/geometry.hh
        ${PROJECT_INCLUDE}/sdlpp/video/palette.hh
        ${PROJECT_INCLUDE}/sdlpp/video/pixel_format.hh
        ${PROJECT_INCLUDE}/sdlpp/video/render.hh
        ${PROJECT_INCLUDE}/sdlpp/video/surface.hh
        ${PROJECT_INCLUDE}/sdlpp/video/texture.hh
        ${PROJECT_INCLUDE}/sdlpp/video/ttf.hh
        ${PROJECT_INCLUDE}/sdlpp/video/window.hh

        thirdparty/gfx/SDL2_gfxPrimitives.c
        thirdparty/gfx/SDL2_gfxPrimitives.h
        thirdparty/gfx/SDL2_gfxPrimitives_font.h
        thirdparty/gfx/SDL2_rotozoom.c
        thirdparty/gfx/SDL2_rotozoom.h

        events/event_types.cc
        events/system_events.cc

        io/whence.cc
        system/cpu.cc
        system/joystic.cc
        system/power.cc
        system/sensor.cc
        video/color.cc
        video/cursor.cc
        video/display.cc
        video/geometry.cc
        video/pixel_format.cc
        video/render.cc
        video/surface.cc
        video/texture.cc
        video/ttf.cc
        video/window.cc

        utils/switch_ostream.hh
)

generate_export_header(sdlpp EXPORT_FILE_NAME ${PROJECT_CONFIG_INCLUDE}/sdlpp/export_defines.h)

add_library(neutrino::sdlpp ALIAS sdlpp)

target_include_directories(sdlpp
        PUBLIC ${PROJECT_INCLUDE} ${PROJECT_CONFIG_INCLUDE}
        PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(sdlpp PUBLIC thirdparty::sdl2 thirdparty::sdl2_ttf thirdparty::sdl2_mixer thirdparty::sdl2_image thirdparty::bitflags thirdparty::strong_type bsw)



