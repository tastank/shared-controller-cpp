#include <asio.hpp>
#include <SDL.h>
#include <iostream>
#include <algorithm>
#include <vector>

#include "CalibrationResult.h"
#include "ControlState.h"

//#define DEBUG

CalibrationResult calibrate_steering(std::vector<SDL_Joystick*> joysticks);
CalibrationResult calibrate_pedal(std::vector<SDL_Joystick*> joysticks, const char* pedal);

int main(int argc, char* args[]) {

	std::string ip_addr;
	std::cout << "Enter the server's IP Address: ";
	std::cin >> ip_addr;

	asio::io_service io_service;
	asio::ip::udp::socket socket(io_service);
	asio::ip::udp::endpoint remote_endpoint;
	socket.open(asio::ip::udp::v4());
	remote_endpoint = asio::ip::udp::endpoint(asio::ip::address::from_string(ip_addr), 42069);

	if (SDL_Init(SDL_INIT_JOYSTICK) < 0) {
		std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << '\n';
	}
	else {
		if (SDL_NumJoysticks() < 1) {
			std::cout << "Warning: No joysticks found!\n";
		}
		int num_joysticks = SDL_NumJoysticks();
#ifdef DEBUG
		std::cout << "Found " << num_joysticks << " joysticks\n";
#endif
		// To do it as an array. Vectors are nicer though.
		//SDL_Joystick **joysticks = new SDL_Joystick*[num_joysticks];
		std::vector<SDL_Joystick*> joysticks;
		for (int c = 0; c < SDL_NumJoysticks(); c++) {
			joysticks.push_back(SDL_JoystickOpen(c));
		}

#ifdef DEBUG

		for (std::pair<int, std::vector<SDL_Joystick*>::iterator> joy_it(0, begin(joysticks)); joy_it.second != end(joysticks); ++joy_it.first, ++joy_it.second) {
			std::cout << "Device " << joy_it.first << ": " << SDL_JoystickName(*joy_it.second) << '\n';
			std::cout << " Axis count: " << SDL_JoystickNumAxes(*joy_it.second) << '\n';
			std::cout << " Button count: " << SDL_JoystickNumButtons(*joy_it.second) << '\n';
			std::cout << " Ball count: " << SDL_JoystickNumBalls(*joy_it.second) << '\n';
		}

#endif

		CalibrationResult steering_calibration = calibrate_steering(joysticks);
		CalibrationResult throttle_calibration = calibrate_pedal(joysticks, "throttle");
		CalibrationResult brake_calibration = calibrate_pedal(joysticks, "brake");

#ifdef DEBUG
		std::cout << steering_calibration.to_string();
		std::cout << throttle_calibration.to_string();
		std::cout << brake_calibration.to_string();
#endif

		SDL_Event e;
		bool quit = false;
		ControlState control_state;
		while (!quit) {
			while (SDL_PollEvent(&e) != 0) {
				if (e.type == SDL_JOYAXISMOTION) {
					int device_num = e.jaxis.which;
					int axis_num = e.jaxis.axis;
					if (SDL_JoystickInstanceID(steering_calibration.get_device()) == device_num && steering_calibration.get_axis() == axis_num) {
						control_state.set_steering(steering_calibration.get_calibrated_value(e.jaxis.value));
					} else if (SDL_JoystickInstanceID(throttle_calibration.get_device()) == device_num && throttle_calibration.get_axis() == axis_num) {
						control_state.set_throttle(throttle_calibration.get_calibrated_value(e.jaxis.value));
					} else if (SDL_JoystickInstanceID(brake_calibration.get_device()) == device_num && brake_calibration.get_axis() == axis_num) {
						control_state.set_brake(throttle_calibration.get_calibrated_value(e.jaxis.value));
					}
				}
				if (e.type == SDL_QUIT) {
					quit = true;
				}
#ifdef DEBUG
				std::cout << control_state.to_string() << '\n';
#endif
			}
			socket.send_to(asio::buffer(control_state.to_string(), 15), remote_endpoint, 0);
			SDL_Delay(5);
		}
		for (std::vector<SDL_Joystick*>::iterator joy_it = begin(joysticks); joy_it != end(joysticks); ++joy_it) {
			if (SDL_JoystickGetAttached(*joy_it)) {
				SDL_JoystickClose(*joy_it);
			}
		}
		std::cout << "Exiting.\n";
	}

	SDL_Quit();

	return 0;
}

void wait_button_push() {
	SDL_Event e;
	bool button_push = false;
	while (!button_push) {
		if (SDL_PollEvent(&e) == 0) {
			SDL_Delay(10);
		} else if (e.type == SDL_JOYBUTTONDOWN) {
			button_push = true;
		}
	}
}

int** get_all_axis_positions(std::vector<SDL_Joystick*> joysticks) {
	int** positions = new int* [joysticks.size()];
	for (int device_it = 0; device_it < joysticks.size(); device_it++) {
		SDL_Joystick* device = joysticks[device_it];
		positions[device_it] = new int[SDL_JoystickNumAxes(device)];
		for (int axis_it = 0; axis_it < SDL_JoystickNumAxes(device); axis_it++) {
			positions[device_it][axis_it] = SDL_JoystickGetAxis(device, axis_it);
#ifdef DEBUG
			std::cout
				<< "Device " << device_it
				<< " Axis " << axis_it
				<< " position: " << positions[device_it][axis_it]
				<< '\n';
#endif
		}
	}
	return positions;
}

CalibrationResult calibrate_steering(std::vector<SDL_Joystick*> joysticks) {
	std::cout << "Center the steering wheel, then press any button.\n";
	wait_button_push();
	int** starting_positions = get_all_axis_positions(joysticks);
	std::cout << "Turn and hold the wheel fully left and press any button.\n";
	wait_button_push();
	int **left_positions = get_all_axis_positions(joysticks);
	std::cout << "Turn and hold the wheel fully right and press any button.\n";
	wait_button_push();
	int** right_positions = get_all_axis_positions(joysticks);
	int greatest_diff_device;
	int greatest_diff_axis;
	int greatest_diff = 0;
	for (int device_it = 0; device_it < joysticks.size(); device_it++) {
		SDL_Joystick *device = joysticks[device_it];
		for (int axis_it = 0; axis_it < SDL_JoystickNumAxes(device); axis_it++) {
			if (std::abs(right_positions[device_it][axis_it] - left_positions[device_it][axis_it]) > greatest_diff) {
				greatest_diff = std::abs(right_positions[device_it][axis_it] - left_positions[device_it][axis_it]);
				greatest_diff_device = device_it;
				greatest_diff_axis = axis_it;
			}
		}
	}
	int min_pos = std::min(left_positions[greatest_diff_device][greatest_diff_axis], right_positions[greatest_diff_device][greatest_diff_axis]);
	int max_pos = std::max(left_positions[greatest_diff_device][greatest_diff_axis], right_positions[greatest_diff_device][greatest_diff_axis]);
	int center_pos = starting_positions[greatest_diff_device][greatest_diff_axis];
	bool reverse = left_positions[greatest_diff_device][greatest_diff_axis] > right_positions[greatest_diff_device][greatest_diff_axis];
	return CalibrationResult("steering", joysticks[greatest_diff_device], greatest_diff_axis, min_pos, max_pos, center_pos, reverse);
}

CalibrationResult calibrate_pedal(std::vector<SDL_Joystick*> joysticks, const char *pedal) {
	std::cout << "Center the steering wheel, release all pedals, then press any button.\n";
	wait_button_push();
	int** starting_positions = get_all_axis_positions(joysticks);
	std::cout << "Fully press and hold the " << pedal << " pedal and press any button.\n";
	wait_button_push();
	int** full_pedal_positions = get_all_axis_positions(joysticks);
	int greatest_diff_device;
	int greatest_diff_axis;
	int greatest_diff = 0;
	for (int device_it = 0; device_it < joysticks.size(); device_it++) {
		SDL_Joystick* device = joysticks[device_it];
		for (int axis_it = 0; axis_it < SDL_JoystickNumAxes(device); axis_it++) {
			if (std::abs(full_pedal_positions[device_it][axis_it] - starting_positions[device_it][axis_it]) > greatest_diff) {
				greatest_diff = std::abs(full_pedal_positions[device_it][axis_it] - starting_positions[device_it][axis_it]);
				greatest_diff_device = device_it;
				greatest_diff_axis = axis_it;
			}
		}
	}
	int min_pos = std::min(starting_positions[greatest_diff_device][greatest_diff_axis], full_pedal_positions[greatest_diff_device][greatest_diff_axis]);
	int max_pos = std::max(starting_positions[greatest_diff_device][greatest_diff_axis], full_pedal_positions[greatest_diff_device][greatest_diff_axis]);
	bool reverse = starting_positions[greatest_diff_device][greatest_diff_axis] > full_pedal_positions[greatest_diff_device][greatest_diff_axis];
	return CalibrationResult(pedal, joysticks[greatest_diff_device], greatest_diff_axis, min_pos, max_pos, reverse);
}