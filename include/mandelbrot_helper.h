#pragma once

#include <complex>

template<typename real>
bool insideCardioids(std::complex<real> z) {
	real squared_img = z.imag() * z.imag();
	real q = (z.real() - (real)0.25) - (z.real() - (real)0.25) + squared_img;

	if (q * (q + z.real() - (real)0.25) < (real)0.25 * squared_img)
		// inside first cardioid
		return true;

	// return true if inside second cardioid
	return (z.real() + (real)1.) * (z.real() + (real)1.) + squared_img < (real)0.0625;
}