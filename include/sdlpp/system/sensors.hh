//
// Created by igor on 6/13/24.
//

#ifndef SDLPP_INCLUDE_SDLPP_SYSTEM_SENSORS_HH_
#define SDLPP_INCLUDE_SDLPP_SYSTEM_SENSORS_HH_

#include <cstddef>
#include <algorithm>
#include <array>
#include <type_traits>
#include <vector>
#include <cstdint>
#include <string>
#include <tuple>
#include <chrono>

#include <strong_type/strong_type.hpp>

#include <sdlpp/detail/sdl2.hh>
#include <sdlpp/detail/object.hh>
#include <sdlpp/detail/call.hh>
#include <sdlpp/detail/ostreamops.hh>

namespace neutrino::sdl {
	using sensor_device_t = strong::type<std::size_t, struct _sensor_device_, strong::bicrementable, strong::ordered, strong::ostreamable>;
	using sensor_device_id_t = strong::type<SDL_SensorID, struct _sensor_device_id_, strong::bicrementable, strong::ordered, strong::ostreamable>;
	class sensor : public object<SDL_Sensor> {
	 public:
		enum class type {
			UNKNOWN = SDL_SENSOR_UNKNOWN,         /**< Unknown sensor type */
			ACCEL = SDL_SENSOR_ACCEL,           /**< Accelerometer */
			GYRO = SDL_SENSOR_GYRO,            /**< Gyroscope */
			ACCEL_L = SDL_SENSOR_ACCEL_L,         /**< Accelerometer for left Joy-Con controller and Wii nunchuk */
			GYRO_L = SDL_SENSOR_GYRO_L,          /**< Gyroscope for left Joy-Con controller */
			ACCEL_R = SDL_SENSOR_ACCEL_R,         /**< Accelerometer for right Joy-Con controller */
			GYRO_R = SDL_SENSOR_GYRO_R           /**< Gyroscope for right Joy-Con controller */
		};

		static sensor_device_t count () noexcept {
			return sensor_device_t{static_cast<std::size_t> (SDL_NumSensors())};
		}

		static void lock() noexcept {
			SDL_LockSensors();
		}

		static void unlock() noexcept {
			SDL_UnlockSensors();
		}

		static sensor open (sensor_device_t idx) {
			return sensor(static_cast<int>(idx.value_of()));
		}

		static std::string get_name (sensor_device_t idx) {
			return SAFE_SDL_CALL(SDL_SensorGetDeviceName, static_cast<int>(idx.value_of()));
		}

		static int get_platform_dependent_type (sensor_device_t idx) {
			return SAFE_SDL_CALL(SDL_SensorGetDeviceNonPortableType, static_cast<int>(idx.value_of()));
		}

		static type get_type (sensor_device_t idx) {
			auto rc = SDL_SensorGetDeviceType (static_cast<int>(idx.value_of()));
			if (rc == SDL_SENSOR_INVALID) {
				RAISE_SDL_EX("Bad sensor index", idx);
			}
			return static_cast<type>(rc);
		}

		static sensor_device_id_t get_id (sensor_device_t idx) {
			return sensor_device_id_t{SAFE_SDL_CALL(SDL_SensorGetDeviceInstanceID, static_cast<int>(idx.value_of()))};
		}

		[[nodiscard]] sensor_device_id_t get_id() const;
		[[nodiscard]] std::string get_name() const;
		[[nodiscard]] int get_platform_dependent_type() const;
		[[nodiscard]] type get_type () const;

		using timed_data_t = std::tuple<std::chrono::microseconds, float>;

		void get_data(uint64_t* timestamp, float* data, std::size_t num_values) const;
		void get_data(std::chrono::microseconds* timestamp, float* data, std::size_t num_values) const;
		void get_data(float* data, std::size_t num_values) const;

		template <class T>
		std::vector<T> get_data(std::size_t num_values, typename std::enable_if<std::is_same_v<T, float>>::type* = nullptr) const;

		template <class T>
		std::vector<T> get_data(std::size_t num_values, typename std::enable_if<std::is_same_v<T, timed_data_t>>::type* = nullptr) const;
	 private:
		explicit sensor(int idx);
	};

	namespace detail {
		static inline constexpr std::array<sensor::type, 7> s_vals_of_sensor_type = {
			sensor::type::UNKNOWN,
			sensor::type::ACCEL,
			sensor::type::GYRO,
			sensor::type::ACCEL_L,
			sensor::type::GYRO_L,
			sensor::type::ACCEL_R,
			sensor::type::GYRO_R,
		};
	}
	template <typename T>
	static inline constexpr const decltype(detail::s_vals_of_sensor_type)&
	values(typename std::enable_if<std::is_same_v<sensor::type, T>>::type* = nullptr) {
		return detail::s_vals_of_sensor_type;
	}
	template <typename T>
	static inline constexpr auto
	begin(typename std::enable_if<std::is_same_v<sensor::type, T>>::type* = nullptr) {
		return detail::s_vals_of_sensor_type.begin();
	}
	template <typename T>
	static inline constexpr auto
	end(typename std::enable_if<std::is_same_v<sensor::type, T>>::type* = nullptr) {
		return detail::s_vals_of_sensor_type.end();
	}

	d_SDLPP_OSTREAM(sensor::type);

	inline
	sensor::sensor (int idx)
	: object<SDL_Sensor>(SAFE_SDL_CALL(SDL_SensorOpen ,idx), true) {

	}

	inline
	sensor_device_id_t sensor::get_id () const {
		return sensor_device_id_t{SAFE_SDL_CALL(SDL_SensorGetInstanceID, const_handle())};
	}

	inline
	std::string sensor::get_name () const {
		return SAFE_SDL_CALL(SDL_SensorGetName, const_handle());
	}

	inline
	int sensor::get_platform_dependent_type () const {
		return SAFE_SDL_CALL(SDL_SensorGetNonPortableType, const_handle());
	}

	inline
	sensor::type sensor::get_type () const {
		return static_cast<type>(SDL_SensorGetType (const_handle()));
	}

	inline
	void sensor::get_data (uint64_t* timestamp, float* data, std::size_t num_values) const {
		SAFE_SDL_CALL(SDL_SensorGetDataWithTimestamp, const_handle(), timestamp, data, static_cast<int>(num_values));
	}

	inline
	void sensor::get_data (std::chrono::microseconds* timestamp, float* data, std::size_t num_values) const {
		std::vector<uint64_t> t(num_values);
		get_data (t.data(), data, num_values);
		std::transform(t.begin(), t.end(), timestamp, [](auto v) {return std::chrono::microseconds (v);});
	}

	inline
	void sensor::get_data (float* data, std::size_t num_values) const {
		SAFE_SDL_CALL(SDL_SensorGetData, const_handle(), data, static_cast<int>(num_values));
	}

	template <class T>
	inline
	std::vector<T> sensor::get_data (std::size_t num_values, typename std::enable_if<std::is_same_v<T, float>>::type*) const {
		std::vector<T> t(num_values);
		get_data ((float*)t.data(), num_values);
		return t;
	}

	template <class T>
	std::vector<T> sensor::get_data(std::size_t num_values, typename std::enable_if<std::is_same_v<T, timed_data_t>>::type*) const {
		std::vector<uint64_t> times(num_values);
		std::vector<float> values(num_values);
		std::vector<T> out;
		std::transform(times.begin(), times.end(), values.begin(),
					   std::back_inserter (out),
					   [] (auto t, auto v) {
			return timed_data_t (std::chrono::microseconds(t), v);
		});
		return out;
	}
}

#endif //SDLPP_INCLUDE_SDLPP_SYSTEM_SENSORS_HH_
