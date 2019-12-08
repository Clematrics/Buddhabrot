#pragma once

#include "abstract_image.h"

#include <algorithm>
#include <cstdint>
#include <vector>

template<typename Int>
class image : public abstractImage<Int> {
public:
	image(uint16_t, uint16_t);
	Int read(uint16_t, uint16_t);
	void set(uint16_t, uint16_t, Int);
	void incr(uint16_t, uint16_t);
	std::vector<pixel> getImage();
private:
	Int& at(uint16_t, uint16_t);

	bool changed;
	Int max;
	std::vector<Int> data;
};


template<typename Int>
image<Int>::image(uint16_t w, uint16_t h) {
	this->m_width = w;
	this->m_height = h;
	max = 0;
	data.resize(w * h, 0);
}

template<typename Int>
std::vector<pixel> image<Int>::getImage() {
	std::vector<pixel> image(this->m_width * this->m_height);
	std::transform(data.begin(), data.end(), image.begin(), [&](Int e){
		// uint8_t c = max ? e * 255 / max : 0;
		uint8_t c = 0;
		if (max) {
			float frac = static_cast<double>(e) / max;
			c = - frac + 2 * sqrt(frac) * 255;
		}
		return (pixel){ c , c , c };
	});
	return image;
}

template<typename Int>
Int& image<Int>::at(uint16_t x, uint16_t y) {
	return data.at(x + this->m_width * y);
}


template<typename Int>
Int image<Int>::read(uint16_t x, uint16_t y) {
	return at(x, y);
}

template<typename Int>
void image<Int>::set(uint16_t x, uint16_t y, Int value) {
	at(x, y) = value;
	if (value > max)
		max = value;
}

template<typename Int>
void image<Int>::incr(uint16_t x, uint16_t y) {
	if (++at(x, y) > max)
		max++;
}
