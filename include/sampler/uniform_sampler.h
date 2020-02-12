#pragma once

#include "sampler/sampler.h"

#include <random>

class uniform_sampler : public sampler {
public:
	uniform_sampler(std::complex<Real> corner_a, std::complex<Real> corner_b);
	sample_result sample();
private:
	std::random_device rd;
	std::ranlux48 engine;
	std::uniform_real_distribution<Real> real_distrib;
	std::uniform_real_distribution<Real> imag_distrib;
};