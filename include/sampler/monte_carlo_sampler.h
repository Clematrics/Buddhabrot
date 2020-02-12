#pragma once
#include "sampler/sampler.h"

#include <random>
#include <queue>

#include "sampler/monte_carlo_tree.h"
#include "types.h"

class monte_carlo_sampler : public sampler {
public:
	monte_carlo_sampler(std::complex<Real> corner_a, std::complex<Real> corner_b, Int layers, Int layer_resolution);
	sample_result sample();

	monte_carlo_tree tree;
private:
	Int layers, layer_resolution;
	std::complex<Real> corner_a, corner_b;

	std::random_device rd;
	std::ranlux48 engine;
	std::uniform_real_distribution<Real> real_distrib;
	std::uniform_real_distribution<Real> imag_distrib;
};