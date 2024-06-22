//
// Created by igor on 6/13/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_INPUT_SENSOR_HH_
#define SDLPP_INCLUDE_SDLPP_INPUT_SENSOR_HH_

#include <sdlpp/system/sensors.hh>

namespace neutrino::sdl {
	class sensor_input {
		public:
			explicit sensor_input(sensor& si)
				: m_sensor(si) {
			}

			void get_data(uint64_t* timestamp, float* data, std::size_t num_values) const {
				m_sensor.get_data(timestamp, data, num_values);
			}

			void get_data(std::chrono::microseconds* timestamp, float* data, std::size_t num_values) const {
				m_sensor.get_data(timestamp, data, num_values);
			}

			void get_data(float* data, std::size_t num_values) const {
				m_sensor.get_data(data, num_values);
			}

			template<class T>
			std::vector <T> get_data(std::size_t num_values) const {
				return m_sensor.get_data <T>(num_values);
			}

		private:
			sensor& m_sensor;
	};
}

#endif //SDLPP_INCLUDE_SDLPP_INPUT_SENSOR_HH_
