#include "monte_carlo_sampler.h"

#include "helper.h"

monte_carlo_sampler::monte_carlo_sampler(std::complex<Real> corner_a, std::complex<Real> corner_b, Int layers, Int layer_resolution) :
	tree(layers, layer_resolution)
{
	// TODO : could be supposed as correct and make this transormation once during the generator creation
	auto [real_m, real_M] = minmax(corner_a.real(), corner_b.real());
	auto [imag_m, imag_M] = minmax(corner_a.imag(), corner_b.imag());
	this->corner_a = std::complex(real_m, imag_m);
	this->corner_b = std::complex(real_M, imag_M);

	engine = std::ranlux48(rd());
	this->layers = layers;
	this->layer_resolution = layer_resolution;
}

sample_result monte_carlo_sampler::sample() {
	monte_carlo_tree::path path = tree.sample_path();
	monte_carlo_tree::path new_path;

	std::complex<Real> shrinking_corner_a = corner_a;
	std::complex<Real> shrinking_corner_b = corner_b;

	while (!path.empty()) {
		monte_carlo_tree::coordinate coord = path.front();
		path.pop();

		Int x = coord.first;
		Int y = coord.second;

		Real dx = (shrinking_corner_b.real() - shrinking_corner_a.real()) / layer_resolution;
		Real dy = (shrinking_corner_b.imag() - shrinking_corner_a.imag()) / layer_resolution;

		shrinking_corner_b.real(shrinking_corner_a.real() + dx * (x + 1));
		shrinking_corner_b.imag(shrinking_corner_a.imag() + dy * (y + 1));

		shrinking_corner_a.real(shrinking_corner_a.real() + dx * x);
		shrinking_corner_a.imag(shrinking_corner_a.imag() + dy * y);

		new_path.push(coord);
	}

	real_distrib = std::uniform_real_distribution(shrinking_corner_a.real(), shrinking_corner_b.real());
	imag_distrib = std::uniform_real_distribution(shrinking_corner_a.imag(), shrinking_corner_b.imag());

	std::complex<Real> res(real_distrib(engine), imag_distrib(engine));

	// now, path is handled by data_ptr to be used by the callback
	std::shared_ptr<void> data_ptr = std::make_shared<monte_carlo_tree::path>(new_path);

	auto feedback = [this](std::shared_ptr<void> data_ptr, Int success, Int total){
		monte_carlo_tree::path* p = reinterpret_cast<monte_carlo_tree::path*>(data_ptr.get());
		this->tree.feedback(*p, success, total);
	};

	return sample_result{ res, data_ptr, feedback };
}