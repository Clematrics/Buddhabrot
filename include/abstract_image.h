#pragma once

#include <cstdint>
#include <vector>

struct pixel {
	uint8_t r, g, b;
};

template<typename Int>
class abstractImage {
public:
	virtual Int read(uint16_t, uint16_t) = 0;
	virtual void set(uint16_t, uint16_t, Int) = 0;
	virtual void incr(uint16_t, uint16_t) = 0;
	virtual std::vector<pixel> getImage() = 0;

	uint16_t width() { return m_width; }
	uint16_t height() { return m_height; }
protected:
	uint16_t m_width, m_height;
};