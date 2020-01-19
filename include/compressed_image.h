#pragma once

#include <cstdint>
#include <vector>

#include "abstract_image.h"
#include "types.h"

class compressedImage : public abstractImage {
public:
	compressedImage(uint16_t, uint16_t);
	Int read(uint16_t, uint16_t);
	void set(uint16_t, uint16_t, Int);
	void incr(uint16_t, uint16_t);
private:
	Int max;
	std::vector<std::vector<Int>> data;
};