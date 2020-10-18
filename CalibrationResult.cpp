#include "CalibrationResult.h"

#include <SDL.h>
#include <cstdint>
#include <sstream>
#include <string>

// TODO Should add a bidirectional boolean or a bidirectional subclass and remove the above or below center determination from calibration unless it's necesary.
CalibrationResult::CalibrationResult(
	const char *name,
	SDL_Joystick *device,
	int16_t axis,
	int16_t min,
	int16_t max,
	int16_t center,
	bool reverse
) {
	this->name = name;
	this->device = device;
	this->axis = axis;
	this->min = min;
	this->max = max;
	this->center = center;
	this->reverse = reverse;
}

CalibrationResult::CalibrationResult(
	const char *name,
	SDL_Joystick *device,
	int16_t axis,
	int16_t min,
	int16_t max,
	bool reverse
) {
	this->name = name;
	this->device = device;
	this->axis = axis;
	this->min = min;
	this->max = max;
	this->center = (min + max) / 2;
	this->reverse = reverse;
}

std::string CalibrationResult::to_string() {
	std::ostringstream str;
	str << "Axis: " << name << '\n';
	str << " Device name: " << SDL_JoystickName(device) << '\n';
	str << " Axis number: " << (int)axis << '\n';
	str << " Min: " << (int)min << '\n';
	str << " Max: " << (int)max << '\n';
	str << " Center: " << (int)center << '\n';
	str << " Reverse: " << reverse << '\n';
	return str.str();
}

SDL_Joystick* CalibrationResult::get_device() {
	return device;
}

int16_t CalibrationResult::get_axis() {
	return axis;
}

int16_t CalibrationResult::get_calibrated_value(int raw_value) {
	float prop_val;
	int calibrated_val;
	if (raw_value < center) {
		prop_val = (float)(raw_value - min) / (float)(center - min);
		calibrated_val = (int)(prop_val * (float)0x8000);
	}
	else {
		prop_val = (float)(raw_value - center) / (float)(max - center);
		calibrated_val = (int)(prop_val * (float)0x7FFF) + 0x8000;
	}
	if (calibrated_val < 0x0000) {
		calibrated_val = 0x0000;
	} else if (calibrated_val > 0xFFFF) {
		calibrated_val = 0xFFFF;
	}
	return (int16_t)calibrated_val;
}
