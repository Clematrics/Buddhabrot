#include "sampler/uniform_sampler.h"

#include "helper.h"

uniform_sampler::uniform_sampler(std::complex<Real> corner_a, std::complex<Real> corner_b) {
	engine = std::ranlux48(rd());

	auto [real_m, real_M] = minmax(corner_a.real(), corner_b.real());
	auto [imag_m, imag_M] = minmax(corner_a.imag(), corner_b.imag());
	real_distrib = std::uniform_real_distribution(real_m, real_M);
	imag_distrib = std::uniform_real_distribution(imag_m, imag_M);
}

sample_result uniform_sampler::sample() {
	double real = real_distrib(engine);
	double imag = imag_distrib(engine);
	return sample_result{ std::complex(real, imag), nullptr, [](std::shared_ptr<void>, Int, Int){ return; } };
}