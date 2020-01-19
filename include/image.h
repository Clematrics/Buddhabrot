#pragma once

#include "abstract_image.h"

#include <cstdint>
#include <vector>

#include "types.h"

class image : public abstractImage {
public:
	image(uint16_t, uint16_t);
	Int read(uint16_t, uint16_t);
	void set(uint16_t, uint16_t, Int);
	void incr(uint16_t, uint16_t);
	std::vector<pixel> get_image();
private:
	Int& at(uint16_t, uint16_t);

	bool changed;
	Int max;
	std::vector<Int> data;
};