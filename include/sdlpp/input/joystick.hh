//
// Created by igor on 6/13/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_INPUT_JOYSTICK_HH_
#define SDLPP_INCLUDE_SDLPP_INPUT_JOYSTICK_HH_

#include <sdlpp/system/joystick.hh>

namespace neutrino::sdl {
	class joystick_input {
	 public:
		explicit joystick_input (joystick& j)
		: m_joystick(j) {}

		joystick& get_joystick() {
			return m_joystick;
		}

		[[nodiscard]] int16_t get_axis(std::size_t axis_id) const {
			return m_joystick.get_axis (axis_id);
		}

		[[nodiscard]] std::optional<int16_t> get_axis_initial_state(std::size_t axis_id) const {
			return m_joystick.get_axis_initial_state (axis_id);
		}

		[[nodiscard]] std::tuple<int, int> get_ball(std::size_t ball_id) const {
			return m_joystick.get_ball (ball_id);
		}

		[[nodiscard]] bool get_button(std::size_t button_id) const {
			return m_joystick.get_button (button_id);
		}

		[[nodiscard]] events::joystick_hat_state get_hat(std::size_t hat_idx) const {
			return m_joystick.get_hat (hat_idx);
		}
	 private:
		joystick& m_joystick;
	};
}

#endif //SDLPP_INCLUDE_SDLPP_INPUT_JOYSTICK_HH_
