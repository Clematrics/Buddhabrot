#pragma once

#include <complex>
#include <functional>
#include <memory>

#include "types.h"

struct sample_result {
	using feedback_fn = std::function<void(std::shared_ptr<void> data_ptr, Int success, Int total)>;
	std::complex<Real> sample;
	std::shared_ptr<void> data_ptr;
	feedback_fn feedback;

	void feedback_result(Int success, Int total) {
		feedback(data_ptr, success, total);
	}
};

class sampler {
public:
	virtual sample_result sample() = 0;
};