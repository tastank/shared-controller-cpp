#include "ControlState.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <iomanip>

ControlState::ControlState() {
	steering = 0x8000;
	throttle = 0x0000;
	brake = 0x0000;
}

int16_t ControlState::get_steering() {
	return steering;
}

void ControlState::set_steering(int16_t steering) {
	this->steering = steering;
}

int16_t ControlState::get_throttle() {
	return throttle;
}
void ControlState::set_throttle(int16_t throttle) {
	this->throttle = throttle;
}

int16_t ControlState::get_brake() {
	return brake;
}

void ControlState::set_brake(int16_t brake) {
	this->brake = brake;
}

std::string ControlState::to_string() {
	std::ostringstream string;
	string << 'S' << std::setfill('0') << std::setw(4) << std::hex << steering;
	string << 'T' << std::setfill('0') << std::setw(4) << std::hex << throttle;
	string << 'B' << std::setfill('0') << std::setw(4) << std::hex << brake;
	return string.str();
}