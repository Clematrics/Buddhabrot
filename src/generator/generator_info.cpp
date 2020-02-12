#include "generator/generator_info.h"

std::string_view status_to_string(status s) {
	switch (s) {
	case status::Running:
		return "Running";
	case status::Stopping:
		return "Stopping";
	case status::Paused:
		return "Paused";
	case status::Stopped:
		return "Stopped";
	default:
		return "No string for this status";
	}
}