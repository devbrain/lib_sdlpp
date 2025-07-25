#pragma once

/**
 * @file event_impl.hh
 * @brief Event implementation details
 */

namespace sdlpp {
    inline void event::ensure_variant() const {
        if (variant_.has_value()) return;

        switch (type()) {
            case event_type::quit:
            case event_type::terminating:
            case event_type::low_memory:
            case event_type::will_enter_background:
            case event_type::did_enter_background:
            case event_type::will_enter_foreground:
            case event_type::did_enter_foreground:
            case event_type::locale_changed:
            case event_type::system_theme_changed:
                variant_ = quit_event{
                    .type = static_cast <event_type>(raw_.quit.type),
                    .timestamp = raw_.quit.timestamp
                };
                break;

            case event_type::window_shown:
            case event_type::window_hidden:
            case event_type::window_exposed:
            case event_type::window_moved:
            case event_type::window_resized:
            case event_type::window_pixel_size_changed:
            case event_type::window_metal_view_resized:
            case event_type::window_minimized:
            case event_type::window_maximized:
            case event_type::window_restored:
            case event_type::window_mouse_enter:
            case event_type::window_mouse_leave:
            case event_type::window_focus_gained:
            case event_type::window_focus_lost:
            case event_type::window_close_requested:
            case event_type::window_hit_test:
            case event_type::window_iccprof_changed:
            case event_type::window_display_changed:
            case event_type::window_display_scale_changed:
            case event_type::window_safe_area_changed:
            case event_type::window_occluded:
            case event_type::window_enter_fullscreen:
            case event_type::window_leave_fullscreen:
            case event_type::window_destroyed:
            case event_type::window_hdr_state_changed:
                variant_ = window_event{
                    .type = static_cast <event_type>(raw_.window.type),
                    .timestamp = raw_.window.timestamp,
                    .windowID = raw_.window.windowID,
                    .data1 = raw_.window.data1,
                    .data2 = raw_.window.data2
                };
                break;

            case event_type::keyboard_added:
            case event_type::keyboard_removed:
                variant_ = keyboard_device_event{
                    .type = static_cast <event_type>(raw_.kdevice.type),
                    .timestamp = raw_.kdevice.timestamp,
                    .which = raw_.kdevice.which
                };
                break;

            case event_type::key_down:
            case event_type::key_up:
                variant_ = keyboard_event{
                    .type = static_cast <event_type>(raw_.key.type),
                    .timestamp = raw_.key.timestamp,
                    .windowID = raw_.key.windowID,
                    .which = raw_.key.which,
                    .key = static_cast <keycode>(raw_.key.key),
                    .scan = static_cast <scancode>(raw_.key.scancode),
                    .mod = static_cast <keymod>(raw_.key.mod),
                    .raw = raw_.key.raw,
                    .down = raw_.key.down,
                    .repeat = raw_.key.repeat
                };
                break;

            case event_type::text_editing: {
                text_editing_event evt{
                    .type = static_cast <event_type>(raw_.edit.type),
                    .timestamp = raw_.edit.timestamp,
                    .windowID = raw_.edit.windowID,
                    .text = {},  // Initialize text
                    .start = raw_.edit.start,
                    .length = raw_.edit.length
                };
                evt.set_text_from_sdl(raw_.edit.text);
                variant_ = std::move(evt);
                break;
            }

            case event_type::text_editing_candidates: {
                text_editing_candidates_event evt{
                    .type = static_cast <event_type>(raw_.edit_candidates.type),
                    .timestamp = raw_.edit_candidates.timestamp,
                    .windowID = raw_.edit_candidates.windowID,
                    .candidates = {},  // Initialize candidates
                    .selected_candidate = raw_.edit_candidates.selected_candidate,
                    .horizontal = raw_.edit_candidates.horizontal != 0
                };
                evt.set_candidates_from_sdl(raw_.edit_candidates.candidates, raw_.edit_candidates.num_candidates);
                variant_ = std::move(evt);
                break;
            }

            case event_type::text_input: {
                text_input_event evt{
                    .type = static_cast <event_type>(raw_.text.type),
                    .timestamp = raw_.text.timestamp,
                    .windowID = raw_.text.windowID,
                    .text = {}  // Initialize text
                };
                evt.set_text_from_sdl(raw_.text.text);
                variant_ = std::move(evt);
                break;
            }

            case event_type::keymap_changed:
                variant_ = common_event{
                    .type = static_cast <event_type>(raw_.common.type),
                    .timestamp = raw_.common.timestamp
                };
                break;

            case event_type::mouse_added:
            case event_type::mouse_removed:
                variant_ = mouse_device_event{
                    .type = static_cast <event_type>(raw_.mdevice.type),
                    .timestamp = raw_.mdevice.timestamp,
                    .which = raw_.mdevice.which
                };
                break;

            case event_type::mouse_motion:
                variant_ = mouse_motion_event{
                    .type = static_cast <event_type>(raw_.motion.type),
                    .timestamp = raw_.motion.timestamp,
                    .windowID = raw_.motion.windowID,
                    .which = raw_.motion.which,
                    .state = static_cast <mouse_button_mask>(raw_.motion.state),
                    .x = raw_.motion.x,
                    .y = raw_.motion.y,
                    .xrel = raw_.motion.xrel,
                    .yrel = raw_.motion.yrel
                };
                break;

            case event_type::mouse_button_down:
            case event_type::mouse_button_up:
                variant_ = mouse_button_event{
                    .type = static_cast <event_type>(raw_.button.type),
                    .timestamp = raw_.button.timestamp,
                    .windowID = raw_.button.windowID,
                    .which = raw_.button.which,
                    .button = raw_.button.button,
                    .down = raw_.button.down,
                    .clicks = raw_.button.clicks,
                    .x = raw_.button.x,
                    .y = raw_.button.y
                };
                break;

            case event_type::mouse_wheel:
                variant_ = mouse_wheel_event{
                    .type = static_cast <event_type>(raw_.wheel.type),
                    .timestamp = raw_.wheel.timestamp,
                    .windowID = raw_.wheel.windowID,
                    .which = raw_.wheel.which,
                    .x = raw_.wheel.x,
                    .y = raw_.wheel.y,
                    .direction = static_cast <mouse_wheel_direction>(raw_.wheel.direction),
                    .mouse_x = raw_.wheel.mouse_x,
                    .mouse_y = raw_.wheel.mouse_y
                };
                break;

            case event_type::joystick_added:
            case event_type::joystick_removed:
            case event_type::joystick_update_complete:
                variant_ = joystick_device_event{
                    .type = static_cast <event_type>(raw_.jdevice.type),
                    .timestamp = raw_.jdevice.timestamp,
                    .which = raw_.jdevice.which
                };
                break;

            case event_type::joystick_axis_motion:
                variant_ = joystick_axis_event{
                    .type = static_cast <event_type>(raw_.jaxis.type),
                    .timestamp = raw_.jaxis.timestamp,
                    .which = raw_.jaxis.which,
                    .axis = raw_.jaxis.axis,
                    .value = raw_.jaxis.value
                };
                break;

            case event_type::joystick_ball_motion:
                variant_ = joystick_ball_event{
                    .type = static_cast <event_type>(raw_.jball.type),
                    .timestamp = raw_.jball.timestamp,
                    .which = raw_.jball.which,
                    .ball = raw_.jball.ball,
                    .xrel = raw_.jball.xrel,
                    .yrel = raw_.jball.yrel
                };
                break;

            case event_type::joystick_hat_motion:
                variant_ = joystick_hat_event{
                    .type = static_cast <event_type>(raw_.jhat.type),
                    .timestamp = raw_.jhat.timestamp,
                    .which = raw_.jhat.which,
                    .hat = raw_.jhat.hat,
                    .value = raw_.jhat.value
                };
                break;

            case event_type::joystick_button_down:
            case event_type::joystick_button_up:
                variant_ = joystick_button_event{
                    .type = static_cast <event_type>(raw_.jbutton.type),
                    .timestamp = raw_.jbutton.timestamp,
                    .which = raw_.jbutton.which,
                    .button = raw_.jbutton.button,
                    .down = raw_.jbutton.down
                };
                break;

            case event_type::joystick_battery_updated:
                variant_ = joystick_battery_event{
                    .type = static_cast <event_type>(raw_.jbattery.type),
                    .timestamp = raw_.jbattery.timestamp,
                    .which = raw_.jbattery.which,
                    .state = static_cast<power_state>(raw_.jbattery.state),
                    .percent = raw_.jbattery.percent
                };
                break;

            case event_type::gamepad_added:
            case event_type::gamepad_removed:
            case event_type::gamepad_remapped:
            case event_type::gamepad_update_complete:
            case event_type::gamepad_steam_handle_updated:
                variant_ = gamepad_device_event{
                    .type = static_cast <event_type>(raw_.gdevice.type),
                    .timestamp = raw_.gdevice.timestamp,
                    .which = raw_.gdevice.which
                };
                break;

            case event_type::gamepad_axis_motion:
                variant_ = gamepad_axis_event{
                    .type = static_cast <event_type>(raw_.gaxis.type),
                    .timestamp = raw_.gaxis.timestamp,
                    .which = raw_.gaxis.which,
                    .axis = static_cast <Uint8>(raw_.gaxis.axis),
                    .value = raw_.gaxis.value
                };
                break;

            case event_type::gamepad_button_down:
            case event_type::gamepad_button_up:
                variant_ = gamepad_button_event{
                    .type = static_cast <event_type>(raw_.gbutton.type),
                    .timestamp = raw_.gbutton.timestamp,
                    .which = raw_.gbutton.which,
                    .button = static_cast <Uint8>(raw_.gbutton.button),
                    .down = raw_.gbutton.down
                };
                break;

            case event_type::gamepad_touchpad_down:
            case event_type::gamepad_touchpad_motion:
            case event_type::gamepad_touchpad_up:
                variant_ = gamepad_touchpad_event{
                    .type = static_cast <event_type>(raw_.gtouchpad.type),
                    .timestamp = raw_.gtouchpad.timestamp,
                    .which = raw_.gtouchpad.which,
                    .touchpad = raw_.gtouchpad.touchpad,
                    .finger = raw_.gtouchpad.finger,
                    .x = raw_.gtouchpad.x,
                    .y = raw_.gtouchpad.y,
                    .pressure = raw_.gtouchpad.pressure
                };
                break;

            case event_type::gamepad_sensor_update:
                variant_ = gamepad_sensor_event{
                    .type = static_cast <event_type>(raw_.gsensor.type),
                    .timestamp = raw_.gsensor.timestamp,
                    .which = raw_.gsensor.which,
                    .sensor = static_cast<sensor_type>(raw_.gsensor.sensor),
                    .data = {raw_.gsensor.data[0], raw_.gsensor.data[1], raw_.gsensor.data[2]},
                    .sensor_timestamp = raw_.gsensor.sensor_timestamp
                };
                break;

            case event_type::finger_down:
            case event_type::finger_up:
            case event_type::finger_motion:
                variant_ = touch_finger_event{
                    .type = static_cast <event_type>(raw_.tfinger.type),
                    .timestamp = raw_.tfinger.timestamp,
                    .touchID = raw_.tfinger.touchID,
                    .fingerID = raw_.tfinger.fingerID,
                    .x = raw_.tfinger.x,
                    .y = raw_.tfinger.y,
                    .dx = raw_.tfinger.dx,
                    .dy = raw_.tfinger.dy,
                    .pressure = raw_.tfinger.pressure
                };
                break;

            case event_type::pen_proximity_in:
            case event_type::pen_proximity_out:
                variant_ = pen_proximity_event{
                    .type = static_cast <event_type>(raw_.pproximity.type),
                    .timestamp = raw_.pproximity.timestamp,
                    .windowID = raw_.pproximity.windowID,
                    .which = raw_.pproximity.which
                };
                break;

            case event_type::pen_down:
            case event_type::pen_up:
                variant_ = pen_touch_event{
                    .type = static_cast <event_type>(raw_.ptouch.type),
                    .timestamp = raw_.ptouch.timestamp,
                    .windowID = raw_.ptouch.windowID,
                    .which = raw_.ptouch.which,
                    .pen_state = static_cast<pen_input_flags>(raw_.ptouch.pen_state),
                    .x = raw_.ptouch.x,
                    .y = raw_.ptouch.y,
                    .eraser = raw_.ptouch.eraser,
                    .down = raw_.ptouch.down
                };
                break;

            case event_type::pen_motion:
                variant_ = pen_motion_event{
                    .type = static_cast <event_type>(raw_.pmotion.type),
                    .timestamp = raw_.pmotion.timestamp,
                    .windowID = raw_.pmotion.windowID,
                    .which = raw_.pmotion.which,
                    .pen_state = static_cast<pen_input_flags>(raw_.pmotion.pen_state),
                    .x = raw_.pmotion.x,
                    .y = raw_.pmotion.y
                };
                break;

            case event_type::pen_button_down:
            case event_type::pen_button_up:
                variant_ = pen_button_event{
                    .type = static_cast <event_type>(raw_.pbutton.type),
                    .timestamp = raw_.pbutton.timestamp,
                    .windowID = raw_.pbutton.windowID,
                    .which = raw_.pbutton.which,
                    .pen_state = static_cast<pen_input_flags>(raw_.pbutton.pen_state),
                    .x = raw_.pbutton.x,
                    .y = raw_.pbutton.y,
                    .button = raw_.pbutton.button,
                    .down = raw_.pbutton.down
                };
                break;

            case event_type::pen_axis:
                variant_ = pen_axis_event{
                    .type = static_cast <event_type>(raw_.paxis.type),
                    .timestamp = raw_.paxis.timestamp,
                    .windowID = raw_.paxis.windowID,
                    .which = raw_.paxis.which,
                    .pen_state = static_cast<pen_input_flags>(raw_.paxis.pen_state),
                    .x = raw_.paxis.x,
                    .y = raw_.paxis.y,
                    .axis = static_cast<pen_axis>(raw_.paxis.axis),
                    .value = raw_.paxis.value
                };
                break;

            case event_type::clipboard_update:
                variant_ = clipboard_event{
                    .type = static_cast <event_type>(raw_.clipboard.type),
                    .timestamp = raw_.clipboard.timestamp,
                    .owner = raw_.clipboard.owner
                };
                break;

            case event_type::drop_file:
            case event_type::drop_text:
            case event_type::drop_begin:
            case event_type::drop_complete:
            case event_type::drop_position: {
                drop_event evt{
                    .type = static_cast <event_type>(raw_.drop.type),
                    .timestamp = raw_.drop.timestamp,
                    .windowID = raw_.drop.windowID,
                    .x = raw_.drop.x,
                    .y = raw_.drop.y,
                    .source = {},  // Initialize source
                    .data = {}  // Initialize data
                };
                evt.set_source_from_sdl(raw_.drop.source);
                evt.set_data_from_sdl(raw_.drop.data);
                variant_ = std::move(evt);
                break;
            }

            case event_type::audio_device_added:
            case event_type::audio_device_removed:
            case event_type::audio_device_format_changed:
                variant_ = audio_device_event{
                    .type = static_cast <event_type>(raw_.adevice.type),
                    .timestamp = raw_.adevice.timestamp,
                    .which = raw_.adevice.which,
                    .recording = raw_.adevice.recording
                };
                break;

            case event_type::camera_device_added:
            case event_type::camera_device_removed:
            case event_type::camera_device_approved:
            case event_type::camera_device_denied:
                variant_ = camera_device_event{
                    .type = static_cast <event_type>(raw_.cdevice.type),
                    .timestamp = raw_.cdevice.timestamp,
                    .which = raw_.cdevice.which
                };
                break;

            case event_type::sensor_update:
                variant_ = sensor_event{
                    .type = static_cast <event_type>(raw_.sensor.type),
                    .timestamp = raw_.sensor.timestamp,
                    .which = raw_.sensor.which,
                    .data = {
                        raw_.sensor.data[0], raw_.sensor.data[1], raw_.sensor.data[2], raw_.sensor.data[3],
                        raw_.sensor.data[4], raw_.sensor.data[5]
                    },
                    .sensor_timestamp = raw_.sensor.sensor_timestamp
                };
                break;

            case event_type::render_targets_reset:
            case event_type::render_device_reset:
            case event_type::render_device_lost:
                variant_ = render_event{
                    .type = static_cast <event_type>(raw_.render.type),
                    .timestamp = raw_.render.timestamp,
                    .windowID = raw_.render.windowID
                };
                break;

            case event_type::display_orientation:
            case event_type::display_added:
            case event_type::display_removed:
            case event_type::display_moved:
            case event_type::display_desktop_mode_changed:
            case event_type::display_current_mode_changed:
            case event_type::display_content_scale_changed:
                variant_ = display_event{
                    .type = static_cast <event_type>(raw_.display.type),
                    .timestamp = raw_.display.timestamp,
                    .displayID = raw_.display.displayID,
                    .data1 = raw_.display.data1,
                    .data2 = raw_.display.data2
                };
                break;

            default:
                // Handle user events and unknown events
                if (static_cast <Uint32>(type()) >= static_cast <Uint32>(event_type::user)) {
                    variant_ = user_event{
                        .type = static_cast <event_type>(raw_.user.type),
                        .timestamp = raw_.user.timestamp,
                        .windowID = raw_.user.windowID,
                        .code = raw_.user.code,
                        .data1 = raw_.user.data1,
                        .data2 = raw_.user.data2
                    };
                } else {
                    variant_ = common_event{
                        .type = static_cast <event_type>(raw_.common.type),
                        .timestamp = raw_.common.timestamp
                    };
                }
                break;
        }
    }
} // namespace sdlpp
