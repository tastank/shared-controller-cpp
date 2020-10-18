#pragma once

#include <cstdint>
#include <string>

class ControlState
{
	// why does setting these to int16_t cause compilation errors?
	int16_t steering;
	int16_t throttle;
	int16_t brake;
public:
	ControlState();
	int16_t get_steering();
	void set_steering(int16_t);
	int16_t get_throttle();
	void set_throttle(int16_t);
	int16_t get_brake();
	void set_brake(int16_t);
	std::string to_string();
};

