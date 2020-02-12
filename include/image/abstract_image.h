#pragma once

#include <cstdint>
#include <vector>

#include "types.h"

struct pixel {
	uint8_t r, g, b;
};

class abstractImage {
public:
	virtual Int read(uint16_t x, uint16_t y) = 0;
	virtual void set(uint16_t x, uint16_t y, Int value) = 0;
	virtual void incr(uint16_t x, uint16_t y) = 0;
	virtual std::vector<pixel> get_image() = 0;

	uint16_t width() { return m_width; }
	uint16_t height() { return m_height; }
protected:
	uint16_t m_width, m_height;
};