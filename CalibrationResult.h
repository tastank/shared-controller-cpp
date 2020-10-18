#pragma once

#include <SDL.h>
#include <cstdint>
#include <string>

class CalibrationResult {
	const char *name;
	SDL_Joystick *device;
	int16_t axis;
	int16_t min;
	int16_t max;
	int16_t center;
	bool reverse;

  public:
	  CalibrationResult(const char *name, SDL_Joystick *device, int16_t axis, int16_t min, int16_t max, bool reverse);
	  CalibrationResult(const char *name, SDL_Joystick *device, int16_t axis, int16_t min, int16_t max, int16_t center, bool reverse);
	  std::string to_string();
	  SDL_Joystick *get_device();
	  int16_t get_axis();
	  int16_t get_calibrated_value(int raw_value);
};