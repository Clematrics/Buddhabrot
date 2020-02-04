#include "image.h"

#include <algorithm>
#include <cmath>

image::image(uint16_t w, uint16_t h) {
	this->m_width = w;
	this->m_height = h;
	max = 0;
	data.resize(w * h, 0);
}

std::vector<pixel> image::get_image() {
	std::vector<pixel> image(this->m_width * this->m_height);
	std::transform(data.begin(), data.end(), image.begin(), [&](Int e){
		// uint8_t c = max ? e * 255 / max : 0;
		uint8_t c = 0;
		if (max) {
			float frac = static_cast<double>(e) / max;
			c = (- frac + 2 * std::sqrt(frac)) * 255;
		}
		// uint8_t c = 200;
		pixel p = { c, c, c };
		return p;
	});
	return image;
}

Int& image::at(uint16_t x, uint16_t y) {
	return data.at(x + this->m_width * y);
}


Int image::read(uint16_t x, uint16_t y) {
	return at(x, y);
}

void image::set(uint16_t x, uint16_t y, Int value) {
	at(x, y) = value;
	if (value > max)
		max = value;
}

void image::incr(uint16_t x, uint16_t y) {
	if (++at(x, y) > max)
		max++;
}
